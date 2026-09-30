#include "MarketGame.h"
#include "ProductCatalog.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/TextRenderComponent.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"

class SCategoryPopup : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCategoryPopup) {} SLATE_ARGUMENT(AMarketGameMode*, Game) SLATE_END_ARGS()
    AMarketGameMode* Game = nullptr;
    void Construct(const FArguments& Args)
    {
        Game = Args._Game;
        auto Rows = SNew(SScrollBox).ConsumeMouseWheel(EConsumeMouseWheel::Never);
        for (int32 I = 0; I < Game->CategoryChoices.Num(); ++I)
        {
            Rows->AddSlot()[SNew(SButton).OnClicked_Lambda([this, I]() { Game->CategorySelection = I; Game->CloseCategoryPicker(true); return FReply::Handled(); })
                [SNew(STextBlock).Text_Lambda([this, I]() { return FText::FromString((Game->CategorySelection == I ? TEXT("\u25b6 ") : TEXT("   ")) + (Game->CategoryChoices[I].IsEmpty() ? FString(TEXT("Kategorisiz")) : Game->CategoryChoices[I])); })]];
        }
        ChildSlot.HAlign(HAlign_Center).VAlign(VAlign_Center)
        [SNew(SBox).WidthOverride(350).HeightOverride(440)
            [SNew(SBorder).Padding(18)[SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)[SNew(STextBlock).Text(FText::FromString(TEXT("Raf kategorisi\nOklar / tekerlek: se\u00e7 \u00b7 E: onay \u00b7 Esc: kapat")))]
                + SVerticalBox::Slot().FillHeight(1)[Rows]]]];
    }
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry&, const FKeyEvent& Event) override
    {
        const FKey Key = Event.GetKey();
        if (Key == EKeys::Escape) Game->CloseCategoryPicker(false);
        else if (Key == EKeys::E || Key == EKeys::Enter) Game->CloseCategoryPicker(true);
        else if (Key == EKeys::Up || Key == EKeys::Down) Game->CategorySelection = (Game->CategorySelection + Game->CategoryChoices.Num() + (Key == EKeys::Up ? -1 : 1)) % Game->CategoryChoices.Num();
        return FReply::Handled();
    }
    virtual FReply OnMouseWheel(const FGeometry&, const FPointerEvent& Event) override
    {
        Game->CategorySelection = (Game->CategorySelection + Game->CategoryChoices.Num() + (Event.GetWheelDelta() > 0 ? -1 : 1)) % Game->CategoryChoices.Num();
        return FReply::Handled();
    }
};

int32 AMarketGameMode::CategoryTarget(FString& OutFace) const
{
    if (bMenuOpen || CategoryPicker.IsValid()) return INDEX_NONE;
    if (bArrange && bArrangeAim) { OutFace = AimFace; return AimFixture; }
    const auto* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (!Camera) return INDEX_NONE;
    float Best = 400.f; int32 Found = INDEX_NONE;
    for (int32 I = 0; I < Planogram.Fixtures.Num(); ++I)
    {
        const auto& F = Planogram.Fixtures[I]; const auto E = MarketPlanogram::Equipment(F.EquipmentId);
        if (E.Levels <= 0) continue;
        const FTransform Xf(FRotator(0, F.Yaw, 0), F.Location);
        const FVector Eye = Xf.InverseTransformPosition(Camera->GetCameraLocation());
        const FVector Ray = Xf.InverseTransformVectorNoScale(Camera->GetActorForwardVector());
        if (FMath::Abs(Ray.Y) < .001f) continue;
        const FString Face = Eye.Y < 0 ? TEXT("front") : TEXT("back");
        if (Face == TEXT("back") && !E.bDoubleSided) continue;
        const float T = ((E.bSignOnTop ? 0.f : E.SignY) - Eye.Y) / Ray.Y;
        const FVector Hit = Eye + Ray * T;
        if (T > 0 && T < Best && FMath::Abs(Hit.X) <= E.SignWidthCm / 2 && FMath::Abs(Hit.Z - E.SignZ) < 15)
        { Best = T; Found = I; OutFace = Face; }
    }
    return Found;
}

void AMarketGameMode::RefreshCategorySigns()
{
    for (int32 I = 0; I < CategorySigns.Num(); ++I)
    {
        FString Id, Face; CategorySignKeys[I].Split(TEXT("/"), &Id, &Face);
        const auto* F = Planogram.FindFixture(Id);
        if (!F || !CategorySigns[I]) continue;
        const FString C = F->CategoryForFace(Face);
        TSet<FString> Wrong;
        for (const auto& Block : Planogram.Placements)
            if (Block.FixtureId == Id && Block.Face == Face)
                if (const auto* P = MarketCatalog::FindProduct(Products, Block.ProductId))
                    if (!StaffPlanner::SameCategory(C, P->Category)) Wrong.Add(P->Id);
        FString Text = MarketCatalog::UpperTurkish(C.IsEmpty() ? TEXT("Kategorisiz") : C);
        if (Wrong.Num()) Text += FString::Printf(TEXT("\n%d \u00fcr\u00fcn bu reyona ait de\u011fil"), Wrong.Num());
        CategorySigns[I]->SetText(FText::FromString(Text));
    }
}

bool AMarketGameMode::CategoryCommand(FName Action)
{
    if (CategoryPicker.IsValid())
    {
        if (Action == "Interact" || Action == "ArrangePlace") CloseCategoryPicker(true);
        else if (Action == "Quit") CloseCategoryPicker(false);
        else if (Action == "ArrangeNext" || Action == "ArrangeUp" || Action == "ArrangePrev" || Action == "ArrangeDown")
            CategorySelection = (CategorySelection + CategoryChoices.Num() + ((Action == "ArrangeNext" || Action == "ArrangeUp") ? -1 : 1)) % CategoryChoices.Num();
        return true;
    }
    if (Action != "Category") return false;
    FString Face; const int32 Index = CategoryTarget(Face);
    if (Index == INDEX_NONE) return true;
    const auto& F = Planogram.Fixtures[Index];
    CategoryFixture = F.Id; CategoryFace = Face;
    CategoryChoices.Reset();
    TArray<FMarketProduct> Catalog; TArray<FString> Errors; MarketCatalog::LoadFile(MarketCatalog::DefaultPath(), Catalog, Errors);
    for (const auto& P : Catalog) if (!P.Category.IsEmpty()) CategoryChoices.AddUnique(P.Category);
    CategoryChoices.Sort([](const FString& A, const FString& B)
    {
        const FString Alphabet = TEXT("ABC\u00c7DEFG\u011eHI\u0130JKLMNO\u00d6PRS\u015eTU\u00dcVYZ");
        const FString X = MarketCatalog::UpperTurkish(A), Y = MarketCatalog::UpperTurkish(B);
        for (int32 I = 0; I < FMath::Min(X.Len(), Y.Len()); ++I)
        {
            if (X[I] == Y[I]) continue;
            int32 RX = INDEX_NONE, RY = INDEX_NONE; Alphabet.FindChar(X[I], RX); Alphabet.FindChar(Y[I], RY);
            return (RX == INDEX_NONE ? 100 + int32(X[I]) : RX) < (RY == INDEX_NONE ? 100 + int32(Y[I]) : RY);
        }
        return X.Len() < Y.Len();
    });
    const FString Current = F.CategoryForFace(Face);
    CategoryChoices.Remove(Current);
    if (!Current.IsEmpty()) CategoryChoices.Insert(Current, 0);
    CategoryChoices.Add(FString()); CategorySelection = Current.IsEmpty() ? CategoryChoices.Num() - 1 : 0;
    ShowCategoryPicker(); return true;
}

void AMarketGameMode::ShowCategoryPicker()
{
    CategoryPicker = SNew(SCategoryPopup).Game(this);
    GEngine->GameViewport->AddViewportWidgetContent(CategoryPicker.ToSharedRef(), 200);
    if (auto* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->SetIgnoreMoveInput(true); PC->SetIgnoreLookInput(true); PC->bShowMouseCursor = true;
        FInputModeGameAndUI Mode; Mode.SetWidgetToFocus(CategoryPicker); PC->SetInputMode(Mode);
    }
    FSlateApplication::Get().SetKeyboardFocus(CategoryPicker, EFocusCause::SetDirectly);
}

void AMarketGameMode::CloseCategoryPicker(bool bAccept)
{
    if (!CategoryPicker.IsValid()) return;
    if (bAccept && CategoryChoices.IsValidIndex(CategorySelection))
        if (auto* F = Planogram.FindFixture(CategoryFixture))
        {
            const FString C = CategoryChoices[CategorySelection];
            const FString OtherFace = CategoryFace == TEXT("front") ? TEXT("back") : TEXT("front");
            if (MarketPlanogram::Equipment(F->EquipmentId).bDoubleSided && !F->FaceCategories.Contains(OtherFace)) F->FaceCategories.Add(OtherFace, F->CategoryForFace(OtherFace));
            F->FaceCategories.Add(CategoryFace, C);
            if (CategoryFace == TEXT("front")) { F->Category = C; F->Label = C; }
            ResetWorkerJobs(); FString Error;
            if (ActiveStoreKitId.IsEmpty()) CommitPlan(Error);
            else
            {
                StoreCategoryOverrides.Add(CategoryFixture + TEXT("/") + CategoryFace, C);
                ++ArrangeVersion;
                if (OnStoreCategoriesChanged) OnStoreCategoriesChanged(StoreCategoryOverrides);
            }
            RefreshCategorySigns();
            Notify(Error.IsEmpty() ? TEXT("Raf kategorisi de\u011fi\u015ftirildi.") : Error);
        }
    GEngine->GameViewport->RemoveViewportWidgetContent(CategoryPicker.ToSharedRef()); CategoryPicker.Reset();
    if (auto* PC = UGameplayStatics::GetPlayerController(this, 0))
    { PC->ResetIgnoreMoveInput(); PC->ResetIgnoreLookInput(); PC->bShowMouseCursor = false; PC->SetInputMode(FInputModeGameOnly()); }
}
