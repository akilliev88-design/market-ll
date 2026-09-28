#include "MarketGame.h"
#include "ProductCatalog.h"

#include "Components/TextRenderComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

FVector AMarketGameMode::DeliverySpot(int32 ProductIndex) const
{
    const int32 Slot = FMath::Max(0, ProductIndex);
    return FVector(-525.f + (Slot % 3) * 70.f, StoreBack() - 105.f - (Slot / 3) * 52.f, 0.f);
}

int32 AMarketGameMode::NearbyDelivery() const
{
    const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Pawn) return INDEX_NONE;
    int32 Best = INDEX_NONE;
    float BestDistance = 150.f;
    for (int32 I = 0; I < DeliveryCrates.Num(); ++I)
    {
        if (!DeliveryCrates[I] || !State.Stock.IsValidIndex(I) || State.Stock[I].Dock <= 0) continue;
        const float Distance = FVector::Dist2D(Pawn->GetActorLocation(), DeliveryCrates[I]->GetActorLocation());
        if (Distance < BestDistance) { BestDistance = Distance; Best = I; }
    }
    return Best;
}

void AMarketGameMode::RefreshDeliveryCrates()
{
    for (AActor* Actor : DeliveryCrates) if (Actor) Actor->Destroy();
    for (UTextRenderComponent* Text : DeliveryCrateLabels) if (Text && Text->GetOwner()) Text->GetOwner()->Destroy();
    DeliveryCrates.Init(nullptr, Products.Num());
    DeliveryCrateLabels.Init(nullptr, Products.Num());
    for (int32 I = 0; I < Products.Num() && State.Stock.IsValidIndex(I); ++I)
    {
        if (State.Stock[I].Dock <= 0) continue;
        const FVector Spot = DeliverySpot(I);
        DeliveryCrates[I] = SurfaceBox(Spot + FVector(0, 0, 17.f), FVector(44.f, 34.f, 34.f), EMarketSurface::Cardboard, true);
        DeliveryCrateLabels[I] = Label(Spot + FVector(0, -18.f, 38.f), FRotator(0, -90, 0),
            FString::Printf(TEXT("%s  x%d"), *ProductName(I), State.Stock[I].Dock), 5.5f, FColor(45, 30, 20), true);
        DeliveryCrateLabels[I]->SetCullDistance(1000.f);
    }
}

bool AMarketGameMode::StartPlayerDelivery(int32 ProductIndex)
{
    if (CarriedDeliveryProduct != INDEX_NONE || !State.Stock.IsValidIndex(ProductIndex) || State.Stock[ProductIndex].Dock <= 0) return false;
    CarriedDeliveryProduct = ProductIndex;
    CarriedDeliveryCrate = SurfaceBox(FVector::ZeroVector, FVector(44.f, 34.f, 34.f), EMarketSurface::Cardboard, false);
    TickPlayerDelivery();
    return true;
}

bool AMarketGameMode::FinishPlayerDelivery()
{
    if (CarriedDeliveryProduct == INDEX_NONE) return false;
    APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Pawn || FVector::Dist2D(Pawn->GetActorLocation(), DepotSpot()) > 155.f) return false;
    const int32 Product = CarriedDeliveryProduct;
    const int32 Limit = Products.IsValidIndex(Product) ? FMath::Clamp(Products[Product].CaseUnits, 1, 48) : MAX_int32;
    const int32 Units = State.ReceiveDelivery(Product, Limit);
    if (CarriedDeliveryCrate) CarriedDeliveryCrate->Destroy();
    CarriedDeliveryCrate = nullptr;
    CarriedDeliveryProduct = INDEX_NONE;
    RefreshDeliveryCrates();
    Notify(Units > 0 ? FString::Printf(TEXT("%d adet %s mal kabulden depoya tasindi."), Units, *ProductName(Product))
                     : FString(TEXT("Bu koli bir gorevli tarafindan zaten tasinmis.")));
    return true;
}

void AMarketGameMode::TickPlayerDelivery()
{
    if (!CarriedDeliveryCrate) return;
    const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Pawn) return;
    const FVector Forward = Pawn->GetActorForwardVector();
    CarriedDeliveryCrate->SetActorLocationAndRotation(Pawn->GetActorLocation() + Forward * 55.f + FVector(0, 0, 55.f),
        FRotator(0.f, Forward.Rotation().Yaw, 0.f));
}

int32 AMarketGameMode::OrderDraftCaseCount() const
{
    int32 Total = 0;
    for (const int32 Cases : OrderDraftCases) Total += Cases;
    return Total;
}

int64 AMarketGameMode::OrderDraftBill() const
{
    int64 Bill = 0;
    for (int32 I = 0; I < Products.Num() && OrderDraftCases.IsValidIndex(I); ++I)
        Bill += static_cast<int64>(OrderDraftCases[I]) * FMath::Clamp(Products[I].CaseUnits, 1, 48) * Products[I].Cost;
    return Bill;
}

FString AMarketGameMode::OrderDraftSummary() const
{
    const int32 Cases = OrderDraftCaseCount();
    if (Cases <= 0) return TEXT("Siparis listesi bos");
    FString Lines;
    int32 Shown = 0;
    for (int32 I = 0; I < Products.Num() && OrderDraftCases.IsValidIndex(I); ++I)
    {
        if (OrderDraftCases[I] <= 0) continue;
        if (Shown++ < 3) Lines += (Lines.IsEmpty() ? FString() : TEXT("  /  ")) + ProductName(I) + FString::Printf(TEXT(" x%d koli"), OrderDraftCases[I]);
    }
    if (Shown > 3) Lines += FString::Printf(TEXT("  /  +%d urun"), Shown - 3);
    const FString Bill = MarketCatalog::Money(OrderDraftBill()) + TEXT(" TL");
    return FString::Printf(TEXT("%d koli  |  %s\n%s"), Cases, *Bill, *Lines);
}
