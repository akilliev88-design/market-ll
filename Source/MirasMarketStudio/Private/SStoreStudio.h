#pragma once
#include "Widgets/SCompoundWidget.h"
#include "MarketStoreKit.h"
class SVerticalBox;
class SStoreMap;
class FJsonObject;
struct FStorePaletteItem { FString Id,Name,Family; };
class SStoreStudio : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SStoreStudio) {} SLATE_END_ARGS()
    void Construct(const FArguments&);
    virtual bool SupportsKeyboardFocus() const override {return true;}
    virtual FReply OnKeyDown(const FGeometry&,const FKeyEvent&) override;
    FStoreTemplate Store;
    int32 Selected=INDEX_NONE;
    FString ArmedEquipment,Category,Message;
    bool bGrid=false,bWallSnap=true,bNeighbourSnap=true,bMarquee=false;
    TSet<int32> Selection;
    bool bLibrary=true,bProperties=true;
    float PlacementYaw=0;
    void Remember();
    void Changed();
    bool MoveSelected(FVector At);
    void AddAt(FVector At);
    void EndDrag();
    FReply Action(FString Command);
    FPlanogramFixture Placement(FVector At) const;
    void Select(int32 Index,bool Add=false);
    bool ReviewMap(FString& Error);
    bool CreateStore(FString Format,FString Name,bool Copy=false);
private:
    TArray<FStoreTemplate> UndoStack,RedoStack;
    TArray<TSharedPtr<FString>> Stores,Categories;
    TArray<FStorePaletteItem> Palette;
    TSharedPtr<SVerticalBox> Bank;
    TSharedPtr<SStoreMap> Map;
    FString Search;
    bool bDirty=false;
    void LoadStore(FString Id);
    void PopulateBank();
    TSharedRef<SWidget> Button(FString Label,FString Command);
    TSharedRef<SWidget> SizeControl(FString Label,int32 Field);
    TSharedRef<SWidget> NumberControl(FString Label,int32 Field);
    double Dimension(int32 Field) const;
    void SetDimension(double Value,int32 Field);
    void AddDepartment(FString Kind);
    void NewStoreDialog();
};
