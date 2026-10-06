#include "MarketTelevisionDisplay.h"
#include "MarketWorldText.h"
#include "MarketVisuals.h"
#include "ProductCatalog.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/PointLight.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool MarketTelevisionDisplay::IsDisplay(const FString& Id)
{
    return Id == TEXT("tv_wall_4800") || Id == TEXT("tv_plinth_2400") || Id == TEXT("tv_island_3000");
}

bool MarketTelevisionDisplay::Parse(const FString& Json, TMap<FString, FProfile>& Out, FString& Error)
{
    Out.Reset(); Error.Empty();
    TSharedPtr<FJsonObject> Root;
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid()
        || !Root->TryGetArrayField(TEXT("products"), Rows)) { Error = TEXT("TV display: invalid products array"); return false; }
    TMap<FString, FProfile> Parsed;
    for (const auto& Value : *Rows)
    {
        const auto Row = Value->AsObject(); FString Id; FProfile Profile;
        if (!Row.IsValid() || !Row->TryGetStringField(TEXT("id"), Id) || !MarketCatalog::IsValidId(Id) || Parsed.Contains(Id))
        { Error = TEXT("TV display: invalid or duplicate product id"); return false; }
        Row->TryGetStringField(TEXT("technology"), Profile.Technology);
        Row->TryGetStringField(TEXT("resolution"), Profile.Resolution);
        Row->TryGetNumberField(TEXT("inches"), Profile.Inches);
        Row->TryGetNumberField(TEXT("refreshHz"), Profile.RefreshHz);
        Row->TryGetBoolField(TEXT("rearLight"), Profile.bRearLight);
        FString Color; if (Row->TryGetStringField(TEXT("rearColor"), Color)) Profile.RearColor = FColor::FromHex(Color);
        if (Profile.Inches < 0 || Profile.Inches > 120 || Profile.RefreshHz < 0 || Profile.RefreshHz > 1000
            || Profile.Technology.Len() > 24 || Profile.Resolution.Len() > 24
            || Profile.Technology.Contains(TEXT("\n")) || Profile.Resolution.Contains(TEXT("\n")))
        { Error = TEXT("TV display: invalid specifications"); return false; }
        Parsed.Add(Id, Profile);
    }
    Out = MoveTemp(Parsed); return true;
}

TMap<FString, MarketTelevisionDisplay::FProfile> MarketTelevisionDisplay::Load()
{
    TMap<FString, FProfile> Result; FString Json, Error;
    const FString Path = FPaths::ProjectDir() / TEXT("AssetInbox/Products/Televisions/display.json");
    if (FFileHelper::LoadFileToString(Json, *Path) && !Parse(Json, Result, Error)) UE_LOG(LogTemp, Warning, TEXT("%s"), *Error);
    return Result;
}

FString MarketTelevisionDisplay::Features(const FProfile& Profile)
{
    TArray<FString> Parts;
    if (Profile.Inches > 0) Parts.Add(FString::Printf(TEXT("%d\""), Profile.Inches));
    if (!Profile.Technology.IsEmpty()) Parts.Add(Profile.Technology);
    if (!Profile.Resolution.IsEmpty()) Parts.Add(Profile.Resolution);
    if (Profile.RefreshHz > 0) Parts.Add(FString::Printf(TEXT("%d Hz"), Profile.RefreshHz));
    return FString::Join(Parts, TEXT(" / "));
}

void MarketTelevisionDisplay::Decorate(UWorld* World, const FMarketPlanogram& Plan, const TArray<FMarketProduct>& Products,
    const FPlanogramPlacement& Placement, const FString& Name, const FProfile* Profile, TArray<AActor*>& Actors)
{
    const auto* Fixture = Plan.FindFixture(Placement.FixtureId);
    const auto* Product = MarketCatalog::FindProduct(Products, Placement.ProductId);
    if (!World || !Fixture || !Product || !IsDisplay(Fixture->EquipmentId)) return;
    const auto Equipment = MarketPlanogram::Equipment(Fixture->EquipmentId);
    if (Placement.Level < 0 || Placement.Level >= Equipment.Levels) return;
    const float Sign = Placement.Face == TEXT("back") ? 1.f : -1.f;
    const FTransform Transform(FRotator(0, Fixture->Yaw, 0), Fixture->Location);
    const float X = MarketPlanogram::PlacementCenterX(Plan, Products, Placement);
    const float Width = FMath::Max(1.f, MarketPlanogram::BlockWidthCm(*Product, Placement) - 5.f);
    const float ShelfZ = Equipment.LevelTopZ[Placement.Level];
    auto* Plate = World->SpawnActor<AActor>(); Actors.Add(Plate);
    auto* Panel = NewObject<UStaticMeshComponent>(Plate); Plate->SetRootComponent(Panel);
    Panel->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")));
    Panel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Panel->SetMaterial(0, MarketVisuals::CreateSurface(Plate, EMarketSurface::DarkMetal)); Panel->RegisterComponent();
    Panel->SetWorldTransform(FTransform(FRotator(0, Fixture->Yaw, 0),
        Transform.TransformPosition(FVector(X, Sign * (Equipment.RailFrontY[Placement.Level] + .4f), ShelfZ - 20)), FVector(Width / 100, .008f, .14f)));
    auto Text = [&](const FString& Value, float Z, FColor Color)
    {
        if (Value.IsEmpty()) return;
        auto* Actor = World->SpawnActor<AActor>(); Actors.Add(Actor);
        auto* Component = NewObject<UTextRenderComponent>(Actor);
        Actor->SetRootComponent(Component); Component->RegisterComponent(); MarketWorldText::Apply(Component);
        Component->SetWorldLocationAndRotation(Transform.TransformPosition(FVector(X, Sign * (Equipment.RailFrontY[Placement.Level] + 1), Z)), FRotator(0, Fixture->Yaw + Sign * 90, 0));
        Component->SetText(FText::FromString(Value)); Component->SetTextRenderColor(Color);
        Component->SetHorizontalAlignment(EHTA_Center); Component->SetVerticalAlignment(EVRTA_TextCenter);
        Component->SetWorldSize(4); Component->SetCullDistance(2200);
        const float ActualWidth = Component->GetTextLocalSize().Y;
        if (ActualWidth > Width) Component->SetWorldSize(4 * Width / ActualWidth);
    };
    const float Z = ShelfZ;
    Text(Name, Z - 17, FColor::White);
    if (Profile) Text(Features(*Profile), Z - 24, FColor(165, 215, 255));
    if (Profile && Profile->bRearLight)
    {
        // A fixed product-specified rear glow, not a claim of video-synchronised lighting.
        auto* Light = World->SpawnActor<APointLight>(Transform.TransformPosition(FVector(X, Sign * 3, Z + MarketPlanogram::NominalHeightCm(*Product) * .55f)), FRotator::ZeroRotator);
        Actors.Add(Light);
        auto* Component = Cast<UPointLightComponent>(Light->GetLightComponent());
        Component->SetMobility(EComponentMobility::Movable); Component->SetLightColor(Profile->RearColor);
        Component->SetIntensity(120); Component->SetAttenuationRadius(95); Component->SetCastShadows(false);
    }
}
