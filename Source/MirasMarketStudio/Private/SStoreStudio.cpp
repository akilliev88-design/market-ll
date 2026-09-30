#include "SStoreStudio.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "MarketStoreEditing.h"
#include "StoreEquipment.h"
#include "ProductCatalog.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "EngineUtils.h"
#include "Components/PrimitiveComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSpacer.h"

namespace StoreUI
{
    FText Text(const FString& S){return FText::FromString(S);}
    FString EquipmentLabel(const FString& Family)
    {
        static const TMap<FString,FString> Labels={{TEXT("checkout"),TEXT("Kasa hatt\u0131")},{TEXT("cooler"),TEXT("So\u011futucu")},{TEXT("freezer"),TEXT("Dondurucu")},{TEXT("produce"),TEXT("Manav tezg\u00e2h\u0131")},{TEXT("deli"),TEXT("\u015eark\u00fcteri")},{TEXT("fish"),TEXT("Bal\u0131k tezg\u00e2h\u0131")},{TEXT("butcher"),TEXT("Kasap tezg\u00e2h\u0131")},{TEXT("bakery"),TEXT("F\u0131r\u0131n raf\u0131")},{TEXT("tobacco"),TEXT("T\u00fct\u00fcn dolab\u0131")},{TEXT("electronics"),TEXT("Teknoloji te\u015fhiri")},{TEXT("home"),TEXT("Ev e\u015fyas\u0131 te\u015fhiri")},{TEXT("textile"),TEXT("Tekstil te\u015fhiri")},{TEXT("pallet"),TEXT("Palet te\u015fhiri")},{TEXT("basket"),TEXT("Sepet alan\u0131")},{TEXT("cart"),TEXT("Al\u0131\u015fveri\u015f arabalar\u0131")},{TEXT("service"),TEXT("Hizmet tezg\u00e2h\u0131")}};
        if(const auto* Name=Labels.Find(Family))return *Name;return TEXT("Raf");
    }
    FLinearColor Family(const FString& F){if(F==TEXT("cooler"))return FLinearColor(.18f,.52f,.64f);if(F==TEXT("freezer"))return FLinearColor(.35f,.53f,.81f);if(F==TEXT("produce"))return FLinearColor(.35f,.61f,.23f);if(F==TEXT("butcher"))return FLinearColor(.68f,.30f,.27f);if(F==TEXT("electronics"))return FLinearColor(.53f,.38f,.75f);if(F==TEXT("checkout"))return FLinearColor(.74f,.54f,.25f);return FLinearColor(.53f,.52f,.47f);}
}
class SStoreMap : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SStoreMap){} SLATE_ARGUMENT(SStoreStudio*,Owner) SLATE_END_ARGS()
    void Construct(const FArguments& A){Owner=A._Owner;SetClipping(EWidgetClipping::ClipToBoundsAlways);}
    virtual FVector2D ComputeDesiredSize(float) const override{return FVector2D(700,650);}
    virtual bool SupportsKeyboardFocus() const override{return true;}
    SStoreStudio* Owner=nullptr;bool Drag=false,Changed=false,PanDrag=false,BoxDrag=false;double Zoom=1;FVector2D Pan=FVector2D::ZeroVector,LastPointer,BoxStart,BoxEnd;FVector Hover,DragStart;bool bHover=false;FStoreTemplate DragStore;
    void Fit(){Zoom=1;Pan=FVector2D::ZeroVector;}
    void FocusSelected(){if(!Owner->Store.Fixtures.IsValidIndex(Owner->Selected)){Fit();return;}Zoom=4;const auto P=Owner->Store.Fixtures[Owner->Selected].Location;Pan=-FVector2D(P.X,-P.Y)*Scale(GetCachedGeometry());}
    double Scale(const FGeometry& G)const{return Zoom*FMath::Max(.001f,FMath::Min((G.GetLocalSize().X-70)/Owner->Store.FootprintCm.X,(G.GetLocalSize().Y-70)/Owner->Store.FootprintCm.Y));}
    FVector2D ToMap(const FGeometry& G,FVector2D P)const{return G.GetLocalSize()/2+Pan+FVector2D(P.X,-P.Y)*Scale(G);}
    FVector ToWorld(const FGeometry& G,FVector2D P)const{auto V=(G.AbsoluteToLocal(P)-G.GetLocalSize()/2-Pan)/Scale(G);return FVector(V.X,-V.Y,0);}
    virtual int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle&,bool)const override
    {
        const auto* Brush=FAppStyle::GetBrush(TEXT("WhiteBrush"));const auto& S=Owner->Store;
        FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(),Brush,ESlateDrawEffect::None,FLinearColor(.055f,.071f,.084f));
        auto Box=[&](FVector At,FVector2D H,FLinearColor Color){const auto P=ToMap(G,FVector2D(At.X-H.X,At.Y+H.Y));FSlateDrawElement::MakeBox(Out,Layer+1,G.ToPaintGeometry(H*2*Scale(G),FSlateLayoutTransform(P)),Brush,ESlateDrawEffect::None,Color);};
        auto Line=[&](TArray<FVector2D> P,FLinearColor C,float Width=1){FSlateDrawElement::MakeLines(Out,Layer+2,G.ToPaintGeometry(),P,ESlateDrawEffect::None,C,true,Width);};
        if(Owner->bGrid){for(double X=-S.FootprintCm.X/2;X<=S.FootprintCm.X/2;X+=100)Line({ToMap(G,FVector2D(X,-S.FootprintCm.Y/2)),ToMap(G,FVector2D(X,S.FootprintCm.Y/2))},FLinearColor(.13f,.16f,.18f));for(double Y=-S.FootprintCm.Y/2;Y<=S.FootprintCm.Y/2;Y+=100)Line({ToMap(G,FVector2D(-S.FootprintCm.X/2,Y)),ToMap(G,FVector2D(S.FootprintCm.X/2,Y))},FLinearColor(.13f,.16f,.18f));}
        TArray<FVector2D> Poly;for(auto P:S.Outline)Poly.Add(ToMap(G,P));if(!Poly.IsEmpty()){const auto First=Poly[0];Poly.Add(First);Line(Poly,FLinearColor(.84f,.85f,.80f),3);}
        Box(S.Backroom.GetCenter(),FVector2D(S.Backroom.GetSize().X/2,S.Backroom.GetSize().Y/2),FLinearColor(.20f,.27f,.30f));
        for(auto O:S.Obstacles)Box(O.At,FVector2D(O.Size.X/2,O.Size.Y/2),FLinearColor(.78f,.78f,.71f));
        for(int32 I=0;I<S.Fixtures.Num();++I)
        {
            const auto& F=S.Fixtures[I];const auto D=MarketPlanogram::Equipment(F.EquipmentId);const auto H=MarketStoreEditing::HalfSize(F);const bool Fits=MarketStoreEditing::CanPlace(S,F,I);
            if(Owner->Selection.Contains(I))Box(F.Location,H+FVector2D(5,5),FLinearColor(1,.77f,.19f));
            Box(F.Location,H,Fits?StoreUI::Family(D.Family):FLinearColor(.91f,.16f,.17f));
            const auto P=ToMap(G,FVector2D(F.Location.X,F.Location.Y));const auto Facing=FRotator(0,F.Yaw-90,0).Vector();Line({P,ToMap(G,FVector2D(F.Location.X+Facing.X*H.GetMin(),F.Location.Y+Facing.Y*H.GetMin()))},FLinearColor::White,2);
        }
        if(bHover&&!Owner->ArmedEquipment.IsEmpty()){const auto F=Owner->Placement(Hover);Box(F.Location,MarketStoreEditing::HalfSize(F),MarketStoreEditing::CanPlace(S,F)?FLinearColor(.2f,.85f,.4f,.7f):FLinearColor(1,.15f,.1f,.7f));}
        Box(S.Entrance.At,FVector2D(65,18),FLinearColor(.25f,.90f,.55f));Box(S.Receiving.At,FVector2D(65,18),FLinearColor(.26f,.62f,.98f));Box(S.PlayerStart.At,FVector2D(18,18),FLinearColor(1,.80f,.20f));
        FSlateDrawElement::MakeText(Out,Layer+3,G.ToPaintGeometry(FVector2D(500,30),FSlateLayoutTransform(FVector2D(15,12))),StoreUI::Text(TEXT("\u00dcSTTEN PLAN  |  ye\u015fil: giri\u015f  mavi: mal kabul  sar\u0131: oyuncu")),FAppStyle::GetFontStyle(TEXT("SmallFont")),ESlateDrawEffect::None,FLinearColor(.78f,.84f,.86f));
        if(BoxDrag){const auto Min=FVector2D(FMath::Min(BoxStart.X,BoxEnd.X),FMath::Min(BoxStart.Y,BoxEnd.Y)),Max=FVector2D(FMath::Max(BoxStart.X,BoxEnd.X),FMath::Max(BoxStart.Y,BoxEnd.Y));FSlateDrawElement::MakeBox(Out,Layer+3,G.ToPaintGeometry(Max-Min,FSlateLayoutTransform(Min)),Brush,ESlateDrawEffect::None,FLinearColor(.15f,.55f,1,.18f));Line({Min,FVector2D(Max.X,Min.Y),Max,FVector2D(Min.X,Max.Y),Min},FLinearColor(.3f,.7f,1),2);}
        return Layer+4;
    }
    void Begin(const FGeometry& G,FVector2D Screen,FKey Button,bool Shift=false,bool Ctrl=false)
    {
        LastPointer=G.AbsoluteToLocal(Screen);const FVector At=ToWorld(G,Screen);
        if(Button==EKeys::MiddleMouseButton||Button==EKeys::RightMouseButton){PanDrag=true;return;}
        if(Button!=EKeys::LeftMouseButton)return;
        if(Owner->bMarquee||Shift){BoxDrag=true;BoxStart=BoxEnd=LastPointer;if(!Ctrl)Owner->Select(INDEX_NONE);return;}
        if(!Owner->ArmedEquipment.IsEmpty()){Owner->AddAt(At);return;}
        int32 Hit=INDEX_NONE;for(int32 I=Owner->Store.Fixtures.Num()-1;I>=0;--I){const auto& F=Owner->Store.Fixtures[I];const auto H=MarketStoreEditing::HalfSize(F);if(FMath::Abs(At.X-F.Location.X)<=H.X&&FMath::Abs(At.Y-F.Location.Y)<=H.Y){Hit=I;break;}}
        if(Hit==INDEX_NONE){PanDrag=true;return;}
        if(Ctrl){Owner->Select(Hit,true);return;}
        if(!Owner->Selection.Contains(Hit))Owner->Select(Hit);else Owner->Selected=Hit;
        Drag=true;Changed=false;DragStart=At;DragStore=Owner->Store;
    }
    void Move(const FGeometry& G,FVector2D Screen)
    {
        const auto Local=G.AbsoluteToLocal(Screen);Hover=ToWorld(G,Screen);bHover=true;
        if(PanDrag){Pan+=Local-LastPointer;LastPointer=Local;}
        else if(BoxDrag)BoxEnd=Local;
        else if(Drag){if(!Changed){Owner->Remember();Changed=true;}auto Candidate=DragStore;if(MarketStoreEditing::MoveGroup(Candidate,Owner->Selection,Hover-DragStart,Owner->bWallSnap,Owner->bNeighbourSnap,Owner->bGrid?10:0)){Owner->Store=MoveTemp(Candidate);Owner->Changed();Owner->Message=TEXT("Secili ekipmanlar tasindi.");}else Owner->Message=TEXT("Burada duvar veya baska ekipman var.");}
        Invalidate(EInvalidateWidgetReason::Paint);
    }
    void End(const FGeometry& G)
    {
        if(BoxDrag){const auto Min=FVector2D(FMath::Min(BoxStart.X,BoxEnd.X),FMath::Min(BoxStart.Y,BoxEnd.Y)),Max=FVector2D(FMath::Max(BoxStart.X,BoxEnd.X),FMath::Max(BoxStart.Y,BoxEnd.Y));for(int32 I=0;I<Owner->Store.Fixtures.Num();++I){const auto& F=Owner->Store.Fixtures[I];const auto H=MarketStoreEditing::HalfSize(F);const auto A=ToMap(G,FVector2D(F.Location.X-H.X,F.Location.Y+H.Y)),B=ToMap(G,FVector2D(F.Location.X+H.X,F.Location.Y-H.Y));if(A.X<=Max.X&&B.X>=Min.X&&A.Y<=Max.Y&&B.Y>=Min.Y)Owner->Selection.Add(I);}Owner->Selected=Owner->Selection.IsEmpty()?INDEX_NONE:Owner->Selection.Array()[0];Owner->bMarquee=false;Owner->Message=FString::Printf(TEXT("%d parca secildi. Birini tutup hepsini tasi."),Owner->Selection.Num());}
        PanDrag=BoxDrag=Drag=false;Invalidate(EInvalidateWidgetReason::Paint);
    }
    virtual FReply OnMouseButtonDown(const FGeometry& G,const FPointerEvent& E)override{Begin(G,E.GetScreenSpacePosition(),E.GetEffectingButton(),E.IsShiftDown(),E.IsControlDown());Invalidate(EInvalidateWidgetReason::Paint);return FReply::Handled().SetUserFocus(SharedThis(this)).CaptureMouse(SharedThis(this));}
    virtual FReply OnMouseMove(const FGeometry& G,const FPointerEvent& E)override{Move(G,E.GetScreenSpacePosition());return FReply::Handled();}
    virtual void OnMouseLeave(const FPointerEvent& E)override{bHover=false;Invalidate(EInvalidateWidgetReason::Paint);SLeafWidget::OnMouseLeave(E);}
    virtual FReply OnMouseWheel(const FGeometry& G,const FPointerEvent& E)override{const FVector Before=ToWorld(G,E.GetScreenSpacePosition());Zoom=FMath::Clamp(Zoom*FMath::Pow(1.2,E.GetWheelDelta()),.25,30.);Pan+=G.AbsoluteToLocal(E.GetScreenSpacePosition())-ToMap(G,FVector2D(Before.X,Before.Y));Invalidate(EInvalidateWidgetReason::Paint);return FReply::Handled();}
    virtual FReply OnMouseButtonUp(const FGeometry& G,const FPointerEvent&)override{End(G);return FReply::Handled().ReleaseMouseCapture();}
    virtual FReply OnKeyDown(const FGeometry& G,const FKeyEvent& E)override{return Owner->OnKeyDown(G,E);}
};
TSharedRef<SWidget> SStoreStudio::Button(FString Label,FString Command){return SNew(SButton).Text(StoreUI::Text(Label)).ButtonColorAndOpacity_Lambda([this,Command]{const bool Active=(Command==TEXT("marquee")&&bMarquee)||(Command.StartsWith(TEXT("arm:"))&&Command.Mid(4)==ArmedEquipment);return Active?FSlateColor(FLinearColor(.16f,.43f,.62f)):FSlateColor(FLinearColor::White);}).OnClicked_Lambda([this,Command]{return Action(Command);});}
double SStoreStudio::Dimension(int32 F)const{if(F==0)return Store.FootprintCm.X/100;if(F==1)return Store.FootprintCm.Y/100;if(F==2)return Store.Backroom.GetSize().X/100;if(F==3)return Store.Backroom.GetSize().Y/100;if(F==4)return Store.CeilingCm/100;if(F==5)return Store.Entrance.At.X/100;return Store.Receiving.At.X/100;}
void SStoreStudio::SetDimension(double Value,int32 F)
{
    Remember();FString Error;
    if(F<5){double V[5];for(int32 I=0;I<5;++I)V[I]=Dimension(I)*100;V[F]=Value*100;if(!MarketStoreEditing::Resize(Store,V[0],V[1],V[2],V[3],V[4],Error)){UndoStack.Pop();Message=Error;return;}}
    else{auto& P=F==5?Store.Entrance:Store.Receiving;P.At.X=FMath::Clamp(Value*100,-Store.FootprintCm.X/2+90,Store.FootprintCm.X/2-90);Store.bEditableShell=true;}
    Changed();Invalidate(EInvalidateWidgetReason::Paint);
}
TSharedRef<SWidget> SStoreStudio::SizeControl(FString Label,int32 Field)
{
    return SNew(SHorizontalBox)+SHorizontalBox::Slot().FillWidth(1)[SNew(STextBlock).Text(StoreUI::Text(Label))]+SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(90)[SNew(SSpinBox<double>).MinValue(Field<5?.1f:-100.).MaxValue(120.).Delta(.1f).Value_Lambda([this,Field]{return Dimension(Field);}).OnValueCommitted_Lambda([this,Field](double V,ETextCommit::Type){SetDimension(V,Field);})]];
}
TSharedRef<SWidget> SStoreStudio::NumberControl(FString Label,int32 Field)
{
    return SNew(SHorizontalBox)+SHorizontalBox::Slot().FillWidth(1)[SNew(STextBlock).Text(StoreUI::Text(Label))]+SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(90)[SNew(SSpinBox<double>).Delta(Field==2?15:.1f).Value_Lambda([this,Field]{if(!Store.Fixtures.IsValidIndex(Selected))return 0.;auto F=Store.Fixtures[Selected];return Field==0?F.Location.X/100:Field==1?F.Location.Y/100:double(F.Yaw);}).OnValueCommitted_Lambda([this,Field](double V,ETextCommit::Type){if(!Store.Fixtures.IsValidIndex(Selected))return;auto Old=Store.Fixtures[Selected];Remember();auto& F=Store.Fixtures[Selected];if(Field==0)F.Location.X=V*100;else if(Field==1)F.Location.Y=V*100;else F.Yaw=V;if(!MarketStoreEditing::CanPlace(Store,F,Selected)){F=Old;UndoStack.Pop();Message=TEXT("Buraya yerle\u015fmez: \u00e7ak\u0131\u015fma veya d\u0131\u015far\u0131 ta\u015fma.");}else{Changed();Invalidate(EInvalidateWidgetReason::Paint);}})]];
}
void SStoreStudio::Construct(const FArguments&)
{
    TArray<FString> Errors;MarketStoreKit::Load(Errors);
    for(const TCHAR* F:{TEXT("mahalle"),TEXT("kucuk"),TEXT("buyuk"),TEXT("hiper")})for(auto Id:MarketStoreKit::TemplatesFor(F))Stores.Add(MakeShared<FString>(Id));
    if(Stores.IsEmpty()){ChildSlot[SNew(STextBlock).Text(StoreUI::Text(FString::Join(Errors,TEXT("\n"))))];return;}
    Store=*MarketStoreKit::Find(*Stores[0]);
    TArray<FString> DraftFiles;IFileManager::Get().FindFiles(DraftFiles,*(FPaths::ProjectSavedDir()/TEXT("StoreDrafts/*.json")),true,false);
    for(const auto& File:DraftFiles){const FString Id=FPaths::GetBaseFilename(File);if(!Stores.ContainsByPredicate([&](auto P){return *P==Id;}))Stores.Add(MakeShared<FString>(Id));}
    TArray<FMarketProduct> Products;MarketCatalog::LoadFile(MarketCatalog::DefaultPath(),Products,Errors);TSet<FString> Cats;for(auto P:Products)Cats.Add(P.Category);TArray<FString> Names=Cats.Array();Names.Sort();Categories.Add(MakeShared<FString>(TEXT("Kategorisiz")));for(auto C:Names)Categories.Add(MakeShared<FString>(C));
    TArray<FString> Files;IFileManager::Get().FindFilesRecursive(Files,*(FPaths::ProjectDir()/TEXT("AssetInbox/Environment/Stores")),TEXT("equipment.json"),true,false);
    Palette.Add({TEXT("gondola_double_1200"),TEXT("\u00c7ift y\u00fczl\u00fc gondol"),TEXT("shelf")});Palette.Add({TEXT("wall_shelf_2400"),TEXT("Duvar raf\u0131"),TEXT("shelf")});
    for(auto Path:Files){FString Text;TSharedPtr<FJsonObject> O;if(FFileHelper::LoadFileToString(Text,*Path)&&FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text),O)&&O->HasField(TEXT("planogram"))){FStorePaletteItem I;I.Id=O->GetStringField(TEXT("id"));I.Family=O->GetStringField(TEXT("family"));if(!O->TryGetStringField(TEXT("displayName"),I.Name))I.Name=FString::Printf(TEXT("%s (%.1f m)"),*StoreUI::EquipmentLabel(I.Family),MarketPlanogram::Equipment(I.Id).DimensionsCm.X/100);Palette.Add(I);}}
    Palette.Sort([](auto A,auto B){return A.Name<B.Name;});
    auto Heading=[](FString Label){return SNew(STextBlock).Text(StoreUI::Text(Label)).Font(FAppStyle::GetFontStyle(TEXT("HeadingExtraSmall")));};
    auto Options=SNew(SVerticalBox);
    Options->AddSlot().AutoHeight().Padding(0,8)[Heading(TEXT("B\u0130NA VE DEPO (metre)"))];
    const TCHAR* Labels[]={TEXT("Ma\u011faza eni"),TEXT("Ma\u011faza boyu"),TEXT("Depo eni"),TEXT("Depo boyu"),TEXT("Tavan y\u00fcksekli\u011fi"),TEXT("Giri\u015f sa\u011f / sol"),TEXT("Mal kabul sa\u011f / sol")};for(int32 I=0;I<7;++I)Options->AddSlot().AutoHeight().Padding(0,3)[SizeControl(Labels[I],I)];
    Options->AddSlot().AutoHeight().Padding(0,8)[Heading(TEXT("YERLE\u015eT\u0130RME"))];
    auto Check=[&](FString Label,bool* Value){Options->AddSlot().AutoHeight().Padding(0,3)[SNew(SCheckBox).IsChecked_Lambda([Value]{return *Value?ECheckBoxState::Checked:ECheckBoxState::Unchecked;}).OnCheckStateChanged_Lambda([Value](ECheckBoxState S){*Value=S==ECheckBoxState::Checked;})[SNew(STextBlock).Text(StoreUI::Text(Label))]];};
    Check(TEXT("10 cm \u0131zgara"),&bGrid);Check(TEXT("Duvara yasla"),&bWallSnap);Check(TEXT("Kom\u015fuya yasla / hizala"),&bNeighbourSnap);
    Options->AddSlot().AutoHeight().Padding(0,5)[SNew(SHorizontalBox)+SHorizontalBox::Slot()[Button(TEXT("Sola yasla"),TEXT("left"))]+SHorizontalBox::Slot()[Button(TEXT("Sa\u011fa yasla"),TEXT("right"))]];
    Options->AddSlot().AutoHeight()[SNew(SHorizontalBox)+SHorizontalBox::Slot()[Button(TEXT("\u00d6ne yasla"),TEXT("front"))]+SHorizontalBox::Slot()[Button(TEXT("Depoya yasla"),TEXT("rear"))]];
    Options->AddSlot().AutoHeight().Padding(0,4)[SNew(SHorizontalBox)+SHorizontalBox::Slot()[Button(TEXT("D\u00f6nd\u00fcr 90\u00b0 [R]"),TEXT("rotate"))]+SHorizontalBox::Slot()[Button(TEXT("Yan yana kopyala"),TEXT("duplicate"))]];
    Options->AddSlot().AutoHeight()[SNew(SHorizontalBox)+SHorizontalBox::Slot()[Button(TEXT("Sil [Del]"),TEXT("delete"))]+SHorizontalBox::Slot()[Button(TEXT("Se\u00e7im modu [Esc]"),TEXT("select"))]];
    Options->AddSlot().AutoHeight().Padding(0,6)[SNew(STextBlock).Text_Lambda([this]{return StoreUI::Text(Store.Fixtures.IsValidIndex(Selected)?Store.Fixtures[Selected].Id:TEXT("Par\u00e7ay\u0131 planda se\u00e7 ve s\u00fcr\u00fckle"));})];
    for(int32 I=0;I<3;++I)Options->AddSlot().AutoHeight()[NumberControl(I==0?TEXT("X (m)"):I==1?TEXT("Y (m)"):TEXT("A\u00e7\u0131"),I)];
    Options->AddSlot().AutoHeight().Padding(0,5)[SNew(SComboBox<TSharedPtr<FString>>).OptionsSource(&Categories).OnGenerateWidget_Lambda([](auto V){return SNew(STextBlock).Text(StoreUI::Text(*V));}).OnSelectionChanged_Lambda([this](auto V,ESelectInfo::Type){if(!V)return;Category=*V==TEXT("Kategorisiz")?FString():*V;if(Store.Fixtures.IsValidIndex(Selected)){Remember();Store.Fixtures[Selected].Category=Category;Store.Fixtures[Selected].FaceCategories.Reset();Changed();Invalidate(EInvalidateWidgetReason::Paint);}})[SNew(STextBlock).Text_Lambda([this]{return StoreUI::Text(Category.IsEmpty()?TEXT("Kategori: Kategorisiz"):Category);})]];
    Options->AddSlot().AutoHeight().Padding(0,8)[Heading(TEXT("HAZIR B\u00d6L\u00dcM"))];
    for(const TCHAR* K:{TEXT("Kasap"),TEXT("\u015eark\u00fcteri"),TEXT("Manav"),TEXT("Teknoloji")})Options->AddSlot().AutoHeight().Padding(0,2)[Button(K,FString(TEXT("department:"))+K)];
    Options->AddSlot().AutoHeight().Padding(0,8)[Heading(TEXT("ZEM\u0130N"))];
    for(const TCHAR* K:{TEXT("Krem"),TEXT("A\u00e7\u0131k gri"),TEXT("Koyu gri"),TEXT("Toprak"),TEXT("Ye\u015fil")})Options->AddSlot().AutoHeight().Padding(0,2)[Button(K,FString(TEXT("color:"))+K)];
    Options->AddSlot().AutoHeight()[SNew(SHorizontalBox)+SHorizontalBox::Slot()[Button(TEXT("Seramik"),TEXT("finish:tile"))]+SHorizontalBox::Slot()[Button(TEXT("Beton"),TEXT("finish:concrete"))]+SHorizontalBox::Slot()[Button(TEXT("Parlak"),TEXT("finish:polished"))]];
    for(int32 Channel=0;Channel<3;++Channel)Options->AddSlot().AutoHeight().Padding(0,3)[SNew(SHorizontalBox)+SHorizontalBox::Slot()[SNew(STextBlock).Text(StoreUI::Text(Channel==0?TEXT("K\u0131rm\u0131z\u0131"):Channel==1?TEXT("Ye\u015fil"):TEXT("Mavi")))]+SHorizontalBox::Slot()[SNew(SSpinBox<double>).MinValue(0).MaxValue(255).Delta(1).Value_Lambda([this,Channel]{return double((&Store.FloorColor.R)[Channel])*255;}).OnValueCommitted_Lambda([this,Channel](double V,ETextCommit::Type){Remember();(&Store.FloorColor.R)[Channel]=float(V/255);Store.bEditableShell=true;Changed();Invalidate(EInvalidateWidgetReason::Paint);})]];
    ChildSlot[SNew(SVerticalBox)
        +SVerticalBox::Slot().AutoHeight().Padding(8)[SNew(SHorizontalBox)
            +SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(200)[SNew(SComboBox<TSharedPtr<FString>>).OptionsSource(&Stores).OnGenerateWidget_Lambda([](auto V){return SNew(STextBlock).Text(StoreUI::Text(*V));}).OnSelectionChanged_Lambda([this](auto V,ESelectInfo::Type){if(V)LoadStore(*V);})[SNew(STextBlock).Text_Lambda([this]{return StoreUI::Text(Store.Id+(bDirty?TEXT(" *"):TEXT("")));})]]]
            +SHorizontalBox::Slot().AutoWidth().Padding(5,0)[Button(TEXT("Geri al [Ctrl Z]"),TEXT("undo"))]+SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Yinele"),TEXT("redo"))]
            +SHorizontalBox::Slot().AutoWidth().Padding(8,0)[Button(TEXT("Taslak kaydet"),TEXT("draft"))]+SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Tasla\u011f\u0131 a\u00e7"),TEXT("loadDraft"))]
            +SHorizontalBox::Slot().AutoWidth().Padding(8,0)[Button(TEXT("Oyuna kaydet"),TEXT("publish"))]
        ]
        +SVerticalBox::Slot().AutoHeight().Padding(8,0,8,8)[SNew(SHorizontalBox)
            +SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Yeni ma\u011faza"),TEXT("new"))]
            +SHorizontalBox::Slot().AutoWidth().Padding(8,0)[Button(TEXT("Toplu sec [Shift]"),TEXT("marquee"))]
            +SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Hepsini sec [Ctrl A]"),TEXT("all"))]
            +SHorizontalBox::Slot().AutoWidth()[Button(TEXT("T\u00fcm\u00fcn\u00fc g\u00f6ster [Home]"),TEXT("focus"))]
            +SHorizontalBox::Slot().AutoWidth().Padding(8,0)[Button(TEXT("Se\u00e7ime git [F]"),TEXT("focusSelection"))]
            +SHorizontalBox::Slot().FillWidth(1)[SNew(SSpacer)]
            +SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Ekipmanlar"),TEXT("library"))]
            +SHorizontalBox::Slot().AutoWidth().Padding(8,0)[Button(TEXT("\u00d6zellikler"),TEXT("properties"))]
            +SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Geni\u015f alan [F11]"),TEXT("workspace"))]]
        +SVerticalBox::Slot().AutoHeight().Padding(8,0,8,8)[SNew(SHorizontalBox)
            +SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Sec / tasi [Esc]"),TEXT("select"))]
            +SHorizontalBox::Slot().AutoWidth().Padding(8,0)[Button(TEXT("D\u00f6nd\u00fcr [R]"),TEXT("rotate"))]
            +SHorizontalBox::Slot().AutoWidth()[Button(TEXT("Kopyala [Ctrl D]"),TEXT("duplicate"))]
            +SHorizontalBox::Slot().AutoWidth().Padding(8,0)[Button(TEXT("Kald\u0131r [Del]"),TEXT("delete"))]
            +SHorizontalBox::Slot().FillWidth(1)[SNew(STextBlock).AutoWrapText(true).Text_Lambda([this]{return StoreUI::Text(TEXT("Bos alani tut: plani kaydir. Parcayi tut: tasi. Toplu sec: cerceve ciz. Tekerlek: yakinlas."));})]]
        +SVerticalBox::Slot().FillHeight(1)[SNew(SHorizontalBox)
            +SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(250).Visibility_Lambda([this]{return bLibrary?EVisibility::Visible:EVisibility::Collapsed;})[SNew(SVerticalBox)+SVerticalBox::Slot().AutoHeight().Padding(8)[Heading(TEXT("EK\u0130PMAN K\u00dcT\u00dcPHANES\u0130"))]+SVerticalBox::Slot().AutoHeight().Padding(8)[SNew(SSearchBox).HintText(StoreUI::Text(TEXT("Dolap, kasap, teknoloji..."))).OnTextChanged_Lambda([this](FText T){Search=T.ToString();PopulateBank();})]+SVerticalBox::Slot().FillHeight(1)[SNew(SScrollBox)+SScrollBox::Slot()[SAssignNew(Bank,SVerticalBox)]]]]
             +SHorizontalBox::Slot().FillWidth(1)[SAssignNew(Map,SStoreMap).Owner(this).Clipping(EWidgetClipping::ClipToBoundsAlways)]
            +SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(280).Visibility_Lambda([this]{return bProperties?EVisibility::Visible:EVisibility::Collapsed;})[SNew(SScrollBox)+SScrollBox::Slot().Padding(10)[Options]]]]
        +SVerticalBox::Slot().AutoHeight().Padding(10,5)[SNew(STextBlock).AutoWrapText(true).Text_Lambda([this]{return StoreUI::Text(FString::Printf(TEXT("%.1f m\u00b2 sat\u0131\u015f  |  %.1f m\u00b2 depo  |  %d ekipman  |  %s"),Store.SalesAreaM2,Store.BackroomM2,Store.Fixtures.Num(),*Message));})]];
    PopulateBank();Invalidate(EInvalidateWidgetReason::Paint);
}
void SStoreStudio::PopulateBank(){Bank->ClearChildren();for(auto I:Palette)if(Search.IsEmpty()||I.Name.Contains(Search)||I.Id.Contains(Search))Bank->AddSlot().AutoHeight().Padding(8,3)[Button(I.Name,TEXT("arm:")+I.Id)];}
void SStoreStudio::LoadStore(FString Id){FStoreTemplate Draft;FString DraftError;const auto* S=MarketStoreKit::Find(Id);if(MarketStoreEditing::LoadDraft(FPaths::ProjectSavedDir()/TEXT("StoreDrafts")/(Id+TEXT(".json")),Draft,DraftError))S=&Draft;if(S){if(bDirty){FString Error;if(!MarketStoreEditing::Save(Store,FPaths::ProjectSavedDir()/TEXT("StoreDrafts")/(Store.Id+TEXT(".json")),Error)){Message=TEXT("\u00d6nce tasla\u011f\u0131 d\u00fczelt: ")+Error;return;}}Store=*S;if(Map)Map->Fit();Selection.Reset();Selected=INDEX_NONE;UndoStack.Reset();RedoStack.Reset();bDirty=false;ArmedEquipment.Empty();Message=TEXT("Ekipman se\u00e7, planda bo\u015f yere t\u0131kla. Ta\u015f\u0131mak i\u00e7in s\u00fcr\u00fckle.");Invalidate(EInvalidateWidgetReason::Paint);}}
void SStoreStudio::Remember(){UndoStack.Add(Store);if(UndoStack.Num()>50)UndoStack.RemoveAt(0);RedoStack.Reset();}
void SStoreStudio::Changed(){bDirty=true;Store.Stats=MarketStoreKit::CalculateStats(Store);if(Map)Map->Invalidate(EInvalidateWidgetReason::Paint);Invalidate(EInvalidateWidgetReason::Paint);}
bool SStoreStudio::MoveSelected(FVector At){if(!Store.Fixtures.IsValidIndex(Selected))return false;auto F=Store.Fixtures[Selected];F.Location=At;F.Location=MarketStoreEditing::Snap(Store,F,bWallSnap,bNeighbourSnap,bGrid?10:0,0);if(!MarketStoreEditing::CanPlace(Store,F,Selected)){Message=TEXT("Buraya yerle\u015fmez. Duvar, depo veya ekipmanla \u00e7ak\u0131\u015f\u0131yor.");return false;}Store.Fixtures[Selected]=F;Changed();Message=TEXT("Yerle\u015ftirildi.");return true;}
void SStoreStudio::EndDrag(){Invalidate(EInvalidateWidgetReason::Paint);}
void SStoreStudio::AddAt(FVector At){Remember();if(MarketStoreEditing::Place(Store,ArmedEquipment,Category,At,Selected,bWallSnap,bNeighbourSnap,bGrid?10:0,PlacementYaw,0)){Selection.Reset();Selection.Add(Selected);Changed();Message=TEXT("Eklendi. Esc ile se\u00e7im moduna d\u00f6n.");Invalidate(EInvalidateWidgetReason::Paint);}else{UndoStack.Pop();Message=TEXT("Buraya yerle\u015fmez; bo\u015f bir alan se\u00e7.");}}
void SStoreStudio::AddDepartment(FString Kind)
{
    TArray<FString> Parts;if(Kind==TEXT("Kasap"))Parts={TEXT("butcher_display_2500"),TEXT("butcher_workbench_1800")};else if(Kind==TEXT("\u015eark\u00fcteri"))Parts={TEXT("deli_display_2500")};else if(Kind==TEXT("Teknoloji"))Parts={TEXT("tech_table_1800"),TEXT("tech_wall_2400")};else Parts={TEXT("produce_small")};
    const auto Before=Store;Remember();int32 First=INDEX_NONE;
    for(auto Part:Parts)
    {
        bool Placed=false;for(double Y=Store.Backroom.Min.Y-170;Y>-Store.FootprintCm.Y/2+160&&!Placed;Y-=100)for(double X=-Store.FootprintCm.X/2+160;X<Store.FootprintCm.X/2-160&&!Placed;X+=100)if(MarketStoreEditing::Place(Store,Part,FString(),FVector(X,Y,0),Selected)){Placed=true;if(First==INDEX_NONE)First=Selected;}
        if(!Placed){Store=Before;UndoStack.Pop();Message=TEXT("B\u00f6l\u00fcm i\u00e7in yeterli bo\u015f alan yok. Par\u00e7alar\u0131 ayr\u0131 yerle\u015ftirebilirsin.");return;}
    }
    FStoreSection Sign;Sign.Label=Kind;Sign.At=Store.Fixtures[First].Location+FVector(0,0,FMath::Min(Store.CeilingCm-55,260.));Sign.WidthCm=250;Store.Sections.Add(Sign);Select(First);Changed();Invalidate(EInvalidateWidgetReason::Paint);Message=Kind+TEXT(" b\u00f6l\u00fcm\u00fc eklendi; par\u00e7alar\u0131 s\u00fcr\u00fckleyebilirsin.");
}
FReply SStoreStudio::Action(FString C)
{
    if(C==TEXT("new")){NewStoreDialog();return FReply::Handled();}
    if(C==TEXT("library")){bLibrary=!bLibrary;return FReply::Handled();}
    if(C==TEXT("properties")){bProperties=!bProperties;return FReply::Handled();}
    if(C==TEXT("workspace")){const bool Show=!bLibrary&&!bProperties;bLibrary=Show;bProperties=Show;return FReply::Handled();}
    if(C==TEXT("marquee")){ArmedEquipment.Empty();bMarquee=!bMarquee;return FReply::Handled();}
    if(C==TEXT("all")){Selection.Reset();for(int32 I=0;I<Store.Fixtures.Num();++I)Selection.Add(I);Selected=Selection.IsEmpty()?INDEX_NONE:0;Invalidate(EInvalidateWidgetReason::Paint);return FReply::Handled();}
    if(C==TEXT("focus")||C==TEXT("focusSelection")){if(C==TEXT("focusSelection"))Map->FocusSelected();else Map->Fit();Map->Invalidate(EInvalidateWidgetReason::Paint);return FReply::Handled();}
    if(C==TEXT("rotate")&&!ArmedEquipment.IsEmpty()){PlacementYaw=FMath::Fmod(PlacementYaw+90,360.f);return FReply::Handled();}
    if(C.StartsWith(TEXT("arm:"))){ArmedEquipment=C.Mid(4);Selection.Reset();Selected=INDEX_NONE;bMarquee=false;PlacementYaw=0;Message=TEXT("Planda bo\u015f yere t\u0131kla. Esc: se\u00e7im modu.");return FReply::Handled();}
    if(C==TEXT("select")){ArmedEquipment.Empty();bMarquee=false;return FReply::Handled();}
    if(C==TEXT("undo")&&!UndoStack.IsEmpty()){RedoStack.Add(Store);Store=UndoStack.Pop();Selected=INDEX_NONE;Selection.Reset();Changed();Invalidate(EInvalidateWidgetReason::Paint);}
    else if(C==TEXT("redo")&&!RedoStack.IsEmpty()){UndoStack.Add(Store);Store=RedoStack.Pop();Selected=INDEX_NONE;Selection.Reset();Changed();Invalidate(EInvalidateWidgetReason::Paint);}
    else if(C==TEXT("draft")||C==TEXT("publish")){FString Error;const FString P=C==TEXT("draft")?FPaths::ProjectSavedDir()/TEXT("StoreDrafts")/(Store.Id+TEXT(".json")):FPaths::ProjectConfigDir()/TEXT("magazalar.json");if(MarketStoreEditing::Save(Store,P,Error)){bDirty=false;Message=C==TEXT("draft")?TEXT("Taslak kaydedildi. Oyuna kaydet ile katalo\u011fa aktar."):TEXT("Oyuna kaydedildi. Ma\u011faza gezi modunda g\u00f6rebilirsin.");if(C==TEXT("publish")){TArray<FString> E;MarketStoreKit::Load(E);}}else Message=Error;}
    else if(C==TEXT("loadDraft")){FString Error;FStoreTemplate S;if(MarketStoreEditing::LoadDraft(FPaths::ProjectSavedDir()/TEXT("StoreDrafts")/(Store.Id+TEXT(".json")),S,Error)){Remember();Store=S;Selected=INDEX_NONE;Selection.Reset();Changed();Invalidate(EInvalidateWidgetReason::Paint);Message=TEXT("Taslak a\u00e7\u0131ld\u0131.");}else Message=Error;}
    else if(C.StartsWith(TEXT("department:")))AddDepartment(C.Mid(11));
    else if(C.StartsWith(TEXT("color:"))){Remember();const auto V=C.Mid(6);Store.FloorColor=V==TEXT("Krem")?FLinearColor(.70f,.64f,.51f):V==TEXT("A\u00e7\u0131k gri")?FLinearColor(.65f,.67f,.68f):V==TEXT("Koyu gri")?FLinearColor(.19f,.22f,.24f):V==TEXT("Toprak")?FLinearColor(.39f,.25f,.17f):FLinearColor(.26f,.39f,.29f);Store.bEditableShell=true;Changed();Invalidate(EInvalidateWidgetReason::Paint);}
    else if(C.StartsWith(TEXT("finish:"))){Remember();Store.FloorFinish=C.Mid(7);Store.bEditableShell=true;Changed();Invalidate(EInvalidateWidgetReason::Paint);}
    else if(C==TEXT("delete")&&!Selection.IsEmpty()){Remember();auto Indices=Selection.Array();Indices.Sort(TGreater<int32>());for(int32 I:Indices)if(Store.Fixtures.IsValidIndex(I))Store.Fixtures.RemoveAt(I);Select(INDEX_NONE);Changed();}
    else if(Store.Fixtures.IsValidIndex(Selected))
    {
        Remember();auto Old=Store.Fixtures[Selected];bool Done=false;
        if(C==TEXT("delete")){Store.Fixtures.RemoveAt(Selected);Selected=INDEX_NONE;Done=true;}
        else if(C==TEXT("duplicate")){int32 N;Done=MarketStoreEditing::Duplicate(Store,Selected,0,N);if(Done)Select(N);}
        else
        {
            auto& F=Store.Fixtures[Selected];if(C==TEXT("rotate"))F.Yaw=FMath::Fmod(F.Yaw+90,360.f);const auto H=MarketStoreEditing::HalfSize(F);
            if(C==TEXT("left"))F.Location.X=-Store.FootprintCm.X/2+H.X;if(C==TEXT("right"))F.Location.X=Store.FootprintCm.X/2-H.X;if(C==TEXT("front"))F.Location.Y=-Store.FootprintCm.Y/2+H.Y;if(C==TEXT("rear"))F.Location.Y=Store.Backroom.Min.Y-H.Y;
            Done=MarketStoreEditing::CanPlace(Store,F,Selected);if(!Done)F=Old;
        }
        if(Done){Changed();Invalidate(EInvalidateWidgetReason::Paint);Message=TEXT("Yerle\u015fim g\u00fcncellendi.");}else{UndoStack.Pop();Message=TEXT("Bu i\u015flem i\u00e7in bo\u015f alan yok.");}
    }
    return FReply::Handled();
}
FReply SStoreStudio::OnKeyDown(const FGeometry&,const FKeyEvent& E){if(E.IsControlDown()&&E.GetKey()==EKeys::A)return Action(TEXT("all"));if(E.GetKey()==EKeys::F11)return Action(TEXT("workspace"));if(E.GetKey()==EKeys::Home)return Action(TEXT("focus"));if(E.GetKey()==EKeys::F)return Action(TEXT("focusSelection"));if(E.IsControlDown()&&E.GetKey()==EKeys::S)return Action(TEXT("draft"));if(E.IsControlDown()&&E.GetKey()==EKeys::Z)return Action(TEXT("undo"));if(E.IsControlDown()&&E.GetKey()==EKeys::Y)return Action(TEXT("redo"));if(E.IsControlDown()&&E.GetKey()==EKeys::D)return Action(TEXT("duplicate"));if(E.GetKey()==EKeys::R)return Action(TEXT("rotate"));if(E.GetKey()==EKeys::Delete)return Action(TEXT("delete"));if(E.GetKey()==EKeys::Escape)return Action(TEXT("select"));return FReply::Unhandled();}

FPlanogramFixture SStoreStudio::Placement(FVector At)const{FPlanogramFixture F;F.EquipmentId=ArmedEquipment;F.Location=At;F.Yaw=PlacementYaw;F.Location=MarketStoreEditing::Snap(Store,F,bWallSnap,bNeighbourSnap,bGrid?10:0,0);return F;}
void SStoreStudio::Select(int32 Index,bool Add){if(!Add)Selection.Reset();if(Store.Fixtures.IsValidIndex(Index)){if(Add&&Selection.Contains(Index))Selection.Remove(Index);else Selection.Add(Index);}Selected=Selection.Contains(Index)?Index:Selection.IsEmpty()?INDEX_NONE:Selection.Array()[0];ArmedEquipment.Empty();if(Store.Fixtures.IsValidIndex(Selected))Category=Store.Fixtures[Selected].Category;Invalidate(EInvalidateWidgetReason::Paint);}
bool SStoreStudio::CreateStore(FString Format,FString Name,bool Copy)
{
    FString Error;if(bDirty&&!MarketStoreEditing::Save(Store,FPaths::ProjectSavedDir()/TEXT("StoreDrafts")/(Store.Id+TEXT(".json")),Error)){Message=Error;return false;}
    int32 N=1;FString Id;do{Id=FString::Printf(TEXT("%s_%02d"),*Format,N++);}while(MarketStoreKit::Find(Id)||IFileManager::Get().FileExists(*(FPaths::ProjectSavedDir()/TEXT("StoreDrafts")/(Id+TEXT(".json")))));
    FStoreTemplate New;if(!MarketStoreEditing::Create(Format,Id,Name,New))return false;
    if(Copy){New=Store;New.Id=Id;New.Name=Name;New.Format=Format;}
    if(!MarketStoreEditing::Save(New,FPaths::ProjectSavedDir()/TEXT("StoreDrafts")/(Id+TEXT(".json")),Error)){Message=Error;return false;}
    Stores.Add(MakeShared<FString>(Id));Store=MoveTemp(New);Selection.Reset();Selected=INDEX_NONE;ArmedEquipment.Empty();UndoStack.Reset();RedoStack.Reset();bDirty=false;Map->Fit();Invalidate(EInvalidateWidgetReason::Paint);Message=TEXT("Yeni ma\u011faza olu\u015fturuldu. Sa\u011fdan bina/depo \u00f6l\u00e7\u00fclerini ayarla; soldan ekipman se\u00e7ip yerle\u015ftir.");return true;
}
void SStoreStudio::NewStoreDialog()
{
    auto Window=SNew(SWindow).Title(StoreUI::Text(TEXT("Yeni ma\u011faza olu\u015ftur"))).ClientSize(FVector2D(440,290)).SupportsMaximize(false).SupportsMinimize(false);
    TWeakPtr<SWindow> WeakWindow=Window;
    auto Format=MakeShared<FString>(TEXT("mahalle"));auto Name=MakeShared<FString>(TEXT("Yeni ma\u011fazam"));
    auto Box=SNew(SVerticalBox);Box->AddSlot().AutoHeight().Padding(12)[SNew(STextBlock).Text(StoreUI::Text(TEXT("Bo\u015f bina ac; \u00f6l\u00e7\u00fclerini istedigin zaman de\u011fi\u015ftirebilirsin."))).AutoWrapText(true)];
    Box->AddSlot().AutoHeight().Padding(12,0)[SNew(SEditableTextBox).Text(StoreUI::Text(*Name)).OnTextChanged_Lambda([Name](FText T){*Name=T.ToString();})];
    auto Types=MakeShared<TArray<TSharedPtr<FString>>>();for(const TCHAR* F:{TEXT("mahalle"),TEXT("kucuk"),TEXT("buyuk"),TEXT("hiper")})Types->Add(MakeShared<FString>(F));
    auto TypeName=[](FString V){return V==TEXT("mahalle")?TEXT("Mahalle"):V==TEXT("kucuk")?TEXT("Ucuzcu"):V==TEXT("buyuk")?TEXT("S\u00fcpermarket"):TEXT("Hipermarket");};
    Box->AddSlot().AutoHeight().Padding(12)[SNew(SComboBox<TSharedPtr<FString>>).OptionsSource(&Types.Get()).OnGenerateWidget_Lambda([Types,TypeName](auto V){return SNew(STextBlock).Text(StoreUI::Text(TypeName(*V)));}).OnSelectionChanged_Lambda([Format,Types](auto V,ESelectInfo::Type){if(V)*Format=*V;})[SNew(STextBlock).Text_Lambda([Format,TypeName]{return StoreUI::Text(FString(TEXT("Ma\u011faza t\u00fcr\u00fc: "))+TypeName(*Format));})]];
    Box->AddSlot().AutoHeight().Padding(12)[SNew(STextBlock).Text(StoreUI::Text(TEXT("Mahalle / ucuzcu: normal tavan. B\u00fcy\u00fck / hiper: y\u00fcksek tavan. Taslak g\u00fcvenle saklan\u0131r; oyuna aktarmada t\u00fcr ko\u015fullar\u0131 kontrol edilir."))).AutoWrapText(true)];
    Box->AddSlot().AutoHeight().Padding(12)[SNew(SHorizontalBox)+SHorizontalBox::Slot()[SNew(SButton).Text(StoreUI::Text(TEXT("Bo\u015f ma\u011faza olu\u015ftur"))).OnClicked_Lambda([this,Format,Name,WeakWindow]{if(CreateStore(*Format,*Name))if(auto W=WeakWindow.Pin())W->RequestDestroyWindow();return FReply::Handled();})]+SHorizontalBox::Slot()[SNew(SButton).Text(StoreUI::Text(TEXT("Mevcut d\u00fczeni kopyala"))).OnClicked_Lambda([this,Name,WeakWindow]{if(CreateStore(Store.Format,*Name,true))if(auto W=WeakWindow.Pin())W->RequestDestroyWindow();return FReply::Handled();})]];
    Window->SetContent(Box);FSlateApplication::Get().AddModalWindow(Window,SharedThis(this));
}

bool SStoreStudio::ReviewMap(FString& Error)
{
    const auto G=Map->GetCachedGeometry();const auto Before=Map->Pan;const auto Start=G.LocalToAbsolute(FVector2D(12,12));
    Map->Begin(G,Start,EKeys::LeftMouseButton);Map->Move(G,G.LocalToAbsolute(FVector2D(112,62)));Map->End(G);
    if(!Map->Pan.Equals(Before+FVector2D(100,50),.01)){Error=TEXT("Empty-space pan failed");return false;}Map->Fit();
    if(Store.Fixtures.Num()<2){Error=TEXT("Missing review fixtures");return false;}
    auto Screen=[&](FVector2D P){return G.LocalToAbsolute(Map->ToMap(G,P));};
    const auto A=Store.Fixtures[0],B=Store.Fixtures[1];const auto AH=MarketStoreEditing::HalfSize(A),BH=MarketStoreEditing::HalfSize(B);
    const FVector2D Min(FMath::Min(A.Location.X-AH.X,B.Location.X-BH.X)-1,FMath::Min(A.Location.Y-AH.Y,B.Location.Y-BH.Y)-1),Max(FMath::Max(A.Location.X+AH.X,B.Location.X+BH.X)+1,FMath::Max(A.Location.Y+AH.Y,B.Location.Y+BH.Y)+1);
    Map->Begin(G,Screen(FVector2D(Min.X,Max.Y)),EKeys::LeftMouseButton,true);Map->Move(G,Screen(FVector2D(Max.X,Min.Y)));Map->End(G);
    if(!Selection.Contains(0)||!Selection.Contains(1)){Error=TEXT("Marquee missed equipment");return false;}
    const auto Offset=B.Location-A.Location;Map->Begin(G,Screen(FVector2D(A.Location.X,A.Location.Y)),EKeys::LeftMouseButton);Map->Move(G,Screen(FVector2D(A.Location.X+500,A.Location.Y-500)));Map->End(G);
    if(Store.Fixtures[0].Location.Equals(A.Location,1)||!(Store.Fixtures[1].Location-Store.Fixtures[0].Location).Equals(Offset,.01)){Error=TEXT("Group move failed or spacing changed");return false;}
    Action(TEXT("undo"));Action(TEXT("redo"));Map->Zoom=8;Map->Pan=-FVector2D(Store.Fixtures[0].Location.X,-Store.Fixtures[0].Location.Y)*Map->Scale(G);Map->Invalidate(EInvalidateWidgetReason::Paint);
    return true;
}
