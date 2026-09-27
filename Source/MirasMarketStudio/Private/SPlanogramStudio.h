#pragma once

#include "CoreMinimal.h"
#include "Planogram.h"
#include "Widgets/SCompoundWidget.h"

class SVerticalBox;

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
    void ApplyStrategy(const FString& Strategy);
    void AddFixture();
    FString PlacementSummary(const FPlanogramPlacement& Placement) const;

    TArray<FMarketProduct> Products;
    FMarketPlanogram Planogram;
    int32 SelectedFixture = 0;
    FString Status;
    TSharedPtr<SVerticalBox> FixtureBox;
    TSharedPtr<SVerticalBox> ProductBox;
};
