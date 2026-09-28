#pragma once

#include "CoreMinimal.h"
#include "Planogram.h"
#include "Widgets/SCompoundWidget.h"

class SVerticalBox;
class FAssetThumbnail;
class FAssetThumbnailPool;

class SPlanogramStudio : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SPlanogramStudio) {}
    SLATE_END_ARGS()
    void Construct(const FArguments& InArgs);

private:
    void Reload();
    void Rebuild();
    void Save(const FString& Message);
    void SelectFixture(int32 Index);
    void MoveProductHere(int32 ProductIndex);
    void ChangeValue(int32 ProductIndex, int32 Field, int32 Delta);
    void ToggleFace(int32 ProductIndex);
    void ChangeOffset(int32 ProductIndex, float DeltaCm);
    void CycleOrientation(int32 ProductIndex);
    void ChangeStack(int32 ProductIndex, int32 Delta);
    void ResetFinePlacement(int32 ProductIndex);
    void ApplyStrategy(const FString& Strategy);
    void AddFixture();
    FString PlacementSummary(const FPlanogramPlacement& Placement) const;
    TSharedRef<SWidget> BuildShelfPreview(const FPlanogramFixture& Fixture);
    TSharedRef<SWidget> BuildProductThumbnail(const FMarketProduct& Product);

    TArray<FMarketProduct> Products;
    FMarketPlanogram Planogram;
    int32 SelectedFixture = 0;
    FString Status;
    TSharedPtr<SVerticalBox> FixtureBox;
    TSharedPtr<SVerticalBox> ProductBox;
    TSharedPtr<FAssetThumbnailPool> ThumbnailPool;
    TArray<TSharedPtr<FAssetThumbnail>> Thumbnails;
};
