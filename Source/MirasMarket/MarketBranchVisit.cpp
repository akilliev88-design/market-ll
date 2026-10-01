#include "MarketBranchVisit.h"
#include "MarketGame.h"
#include "MarketStoreKit.h"
#include "MarketStoreViews.h"
#include "MarketBranches.h"
#include "MarketDirector.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/RectLight.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SOverlay.h"
#include "MarketMenuWidget.h"
#include "Styling/CoreStyle.h"

namespace MarketBranchVisit
{
    float Fill(const FMarketBranch& Branch, const FString& ProductId)
    {
        int64 Units = 0, Capacity = 0;
        for (const FMarketBranchItem& Item : Branch.Items) if (Item.ProductId == ProductId)
        { Units += FMath::Max(0, Item.Units); Capacity += FMath::Max(0, Item.Capacity); }
        return Capacity > 0 ? FMath::Clamp(static_cast<float>(Units) / Capacity, 0.f, 1.f) : 0.f;
    }
    int32 Shoppers(const FMarketBranch& Branch) { return FMath::Clamp(FMath::DivideAndRoundUp(FMath::Max(0, Branch.LastShoppers), 20), 0, 24); }
    int32 Queue(const FMarketBranch& Branch) { return FMath::Clamp(FMath::DivideAndRoundUp(FMath::Max(0, Branch.LastQueueLost), 5), 0, 8); }
    int32 Workers(const FMarketBranch& Branch) { return FMath::Clamp(Branch.Workers, 0, 24); }
    TArray<uint8> StateBytes(const FMarketState& State)
    {
        FMarketState Copy = State;
        TArray<uint8> Bytes; FMemoryWriter Writer(Bytes);
        FObjectAndNameAsStringProxyArchive Archive(Writer, false);
        FMarketState::StaticStruct()->SerializeItem(Archive, &Copy, nullptr);
        return Bytes;
    }
    struct FActorState
    {
        TWeakObjectPtr<AActor> Actor;
        bool Hidden = false, Collision = false, Tick = false, StoreTag = false;
    };
}
struct FMarketBranchVisitSession
{
    FMarketPlanogram Plan;
    FString StoreId;
    TMap<FString,FString> Overrides;
    TArray<TObjectPtr<UTextRenderComponent>> Signs;
    TArray<FString> SignKeys;
    TArray<MarketBranchVisit::FActorState> Actors;
    TArray<TWeakObjectPtr<AActor>> Dressing;
    TSharedPtr<SWidget> Strip;
    FTransform PlayerTransform;
    FRotator ControlRotation;
    FVector Velocity;
    EMovementMode Movement = MOVE_Walking;
    uint8 CustomMovement = 0;
    TWeakObjectPtr<AActor> ViewTarget;
    bool Menu = false, MenuReport = false, Cursor = false, IgnoreMove = false, IgnoreLook = false;
    int32 Page = 0;
    float Dilation = 1, ReportTime = 0, MessageTime = 0;
    FString Message;
};
bool AMarketGameMode::StartBranchVisit(int32 BranchIndex)
{
    if (IsBranchVisit() || bStoreTour || bNeedStart || bArrange || !State.Branches.IsValidIndex(BranchIndex)) return false;
    const FMarketBranch& Branch = State.Branches[BranchIndex];
    if (Branch.Stage != static_cast<uint8>(MarketBranches::EStage::Open)) return false;
    auto* Player = GetWorld()->GetFirstPlayerController(); APawn* Pawn = Player ? Player->GetPawn() : nullptr;
    if (!Pawn || !GEngine || !GEngine->GameViewport) return false;
    TArray<FString> Errors; if (!MarketStoreKit::Load(Errors)) return false;
    const auto* View = MarketStoreViews::Find(Branch.StoreView);
    const FStoreTemplate* Store = View ? MarketStoreKit::Find(View->Id) : nullptr;
    if (!Store)
    {
        const auto Ids = MarketStoreKit::TemplatesFor(Branch.Format);
        if (!Ids.IsEmpty()) Store = MarketStoreKit::Find(Ids[0]);
    }
    if (!Store) return false;
    auto Session = MakeShared<FMarketBranchVisitSession>();
    Session->Plan = Planogram; Session->StoreId = ActiveStoreKitId; Session->Overrides = StoreCategoryOverrides;
    Session->Signs = CategorySigns; Session->SignKeys = CategorySignKeys;
    Session->PlayerTransform = Pawn->GetActorTransform(); Session->ControlRotation = Player->GetControlRotation(); Session->ViewTarget = Player->GetViewTarget();
    Session->Menu = bMenuOpen; Session->MenuReport = bMenuDayReport; Session->Page = MenuPage;
    Session->Cursor = Player->bShowMouseCursor; Session->IgnoreMove = Player->IsMoveInputIgnored(); Session->IgnoreLook = Player->IsLookInputIgnored();
    Session->Dilation = UGameplayStatics::GetGlobalTimeDilation(this);
    Session->ReportTime = ReportTime; Session->MessageTime = MessageTime; Session->Message = Message;
    if (auto* Character = Cast<ACharacter>(Pawn))
    {
        auto* Movement = Character->GetCharacterMovement(); Session->Velocity = Movement->Velocity;
        Session->Movement = Movement->MovementMode; Session->CustomMovement = Movement->CustomMovementMode;
        Movement->StopMovementImmediately();
    }
    // Hold the family scene in place, including customers and the crate in the player's hands.
    // StoreKit::Clear must not destroy a pre-existing store scene while making the temporary one.
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        TArray<UPrimitiveComponent*> Primitives; Actor->GetComponents(Primitives);
        if (Actor == Pawn || (Primitives.IsEmpty() && !Actor->IsA<ARectLight>())) continue;
        MarketBranchVisit::FActorState Saved;
        Saved.Actor = Actor; Saved.Hidden = Actor->IsHidden(); Saved.Collision = Actor->GetActorEnableCollision(); Saved.Tick = Actor->IsActorTickEnabled();
        Saved.StoreTag = Actor->Tags.Remove(TEXT("MirasStoreKit")) > 0;
        Session->Actors.Add(Saved); Actor->SetActorHiddenInGame(true); Actor->SetActorEnableCollision(false); Actor->SetActorTickEnabled(false);
    }
    BranchVisit = Session; BranchVisitIndex = BranchIndex;
    TMap<FString,FString> Overrides;
    // Each fixture gets a real branch department when the branch has that category. Per-face keys use
    // the same ToPlanogram convention as StoreCategoryOverrides; never rewrite the family's overrides.
    for (const FPlanogramFixture& Fixture : Store->Fixtures)
    {
        if (Branch.Items.ContainsByPredicate([&](const FMarketBranchItem& Item)
        { const auto* Product = Products.FindByPredicate([&](const FMarketProduct& P) { return P.Id == Item.ProductId; }); return Product && Product->Category == Fixture.Category; }))
            Overrides.Add(Fixture.Id, Fixture.Category);
    }
    auto VisitPlan = MarketStoreKit::ToPlanogram(*Store, Overrides); MarketStoreKit::Fill(VisitPlan, Products);
    if (!MarketStoreKit::Build(GetWorld(), *Store, VisitPlan)) { EndBranchVisit(); return false; }
    auto Person = [&](const FVector& At, const FLinearColor& Color)
    { AActor* Actor = SimplePerson(At, Color); Actor->SetActorEnableCollision(false); Session->Dressing.Add(Actor); };
    FVector Till = Store->Entrance.At + FVector(150, 180, 0); Till.Z = 0;
    for (const auto& Fixture : Store->Fixtures) if (MarketPlanogram::Equipment(Fixture.EquipmentId).Family == TEXT("checkout")) { Till = Fixture.Location; break; }
    for (int32 Index = 0; Index < MarketBranchVisit::Queue(Branch); ++Index) Person(Till + FVector(80, 70 + Index * 65, 0), FLinearColor(.32f,.27f,.23f));
    for (int32 Index = 0; Index < MarketBranchVisit::Shoppers(Branch); ++Index)
    {
        const auto& Fixture = Store->Fixtures[Index % Store->Fixtures.Num()];
        Person(Fixture.Location + FRotator(0,Fixture.Yaw,0).RotateVector(FVector((Index / Store->Fixtures.Num()) * 65, -110, 0)), FLinearColor(.28f,.35f,.37f));
    }
    for (int32 Index = 0; Index < MarketBranchVisit::Workers(Branch); ++Index)
        Person(Till + FVector(-80 - Index * 70, -70, 0), FLinearColor(.18f,.35f,.23f));
    const FString Grade = MarketBranches::Grade(State, BranchIndex);
    if (Grade == TEXT("D") || Grade == TEXT("F")) for (int32 Index = 0; Index < 3; ++Index)
        Session->Dressing.Add(Box(Store->Receiving.At + FVector(120 + Index * 55, 100, 25), FVector(40,35,50), FLinearColor(.38f,.28f,.18f), false));
    CloseMenu(); ReportTime = MessageTime = 0;
    UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
    Pawn->SetActorLocation(Store->PlayerStart.At, false, nullptr, ETeleportType::TeleportPhysics);
    Player->SetControlRotation(FRotator(0,Store->PlayerStart.Yaw,0)); Player->SetViewTarget(Pawn);
    Player->ResetIgnoreMoveInput(); Player->ResetIgnoreLookInput(); Player->bShowMouseCursor = false; Player->SetInputMode(FInputModeGameOnly());
    Session->Strip = SNew(SOverlay) + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(12) [SNew(SBorder).Padding(10).BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"))).BorderBackgroundColor(FLinearColor(.025f,.035f,.04f,.96f))
        [SNew(STextBlock).ColorAndOpacity(FLinearColor::White).Font(FCoreStyle::GetDefaultFontStyle(TEXT("Regular"),14)).Text(FText::FromString(FString::Printf(TEXT("%s  |  Karne %s  |  %s  |  Esc: cik  |  Zaman duruyor"), *Branch.Name, *Grade, Branch.ManagerName.IsEmpty()?TEXT("Mudur yok"):*Branch.ManagerName)))]];
    GEngine->GameViewport->AddViewportWidgetContent(Session->Strip.ToSharedRef(),60);
    FString VisitMessage; if (MarketDirector::Command(State, Products, TEXT("VisitBranch"), BranchIndex, VisitMessage) && !VisitMessage.IsEmpty()) Notify(VisitMessage); // C3: what the walk shows
    return true;
}
void AMarketGameMode::EndBranchVisit()
{
    if (!IsBranchVisit()) return;
    const auto Session = BranchVisit;
    MarketStoreKit::Clear(GetWorld());
    for (const auto& Actor : Session->Dressing) if (Actor.IsValid()) Actor->Destroy();
    if (GEngine && GEngine->GameViewport && Session->Strip.IsValid()) GEngine->GameViewport->RemoveViewportWidgetContent(Session->Strip.ToSharedRef());
    Planogram = Session->Plan; ActiveStoreKitId = Session->StoreId; StoreCategoryOverrides = Session->Overrides;
    CategorySigns = Session->Signs; CategorySignKeys = Session->SignKeys;
    for (const auto& Saved : Session->Actors) if (auto* Actor = Saved.Actor.Get())
    {
        if (Saved.StoreTag) Actor->Tags.Add(TEXT("MirasStoreKit"));
        Actor->SetActorHiddenInGame(Saved.Hidden); Actor->SetActorEnableCollision(Saved.Collision); Actor->SetActorTickEnabled(Saved.Tick);
    }
    if (auto* Player = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* Pawn = Player->GetPawn())
        {
            Pawn->SetActorTransform(Session->PlayerTransform, false, nullptr, ETeleportType::TeleportPhysics);
            if (auto* Character = Cast<ACharacter>(Pawn)) { auto* Movement = Character->GetCharacterMovement(); Movement->SetMovementMode(Session->Movement,Session->CustomMovement); Movement->Velocity = Session->Velocity; }
        }
        Player->SetControlRotation(Session->ControlRotation);
        if (Session->ViewTarget.IsValid()) Player->SetViewTarget(Session->ViewTarget.Get());
        Player->ResetIgnoreMoveInput(); Player->ResetIgnoreLookInput(); Player->SetIgnoreMoveInput(Session->IgnoreMove); Player->SetIgnoreLookInput(Session->IgnoreLook);
        Player->bShowMouseCursor = Session->Cursor;
        if (Session->Menu) { if (auto* Hud = Cast<AMarketHUD>(Player->GetHUD())) if (Hud->MenuWidget().IsValid()) { FInputModeUIOnly Mode; Mode.SetWidgetToFocus(Hud->MenuWidget()); Player->SetInputMode(Mode); } }
        else Player->SetInputMode(FInputModeGameOnly());
    }
    bMenuOpen = Session->Menu; bMenuDayReport = Session->MenuReport; MenuPage = Session->Page;
    ReportTime = Session->ReportTime; MessageTime = Session->MessageTime; Message = Session->Message;
    UGameplayStatics::SetGlobalTimeDilation(this, Session->Dilation);
    BranchVisitIndex = INDEX_NONE; BranchVisit.Reset();
}