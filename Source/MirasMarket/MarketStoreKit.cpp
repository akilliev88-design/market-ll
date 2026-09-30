#include "MarketStoreKit.h"
#include "MarketLayout.h"
#include "StaffPlanner.h"
#include "ProductCatalog.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace StoreData
{
    TArray<FStoreTemplate> Templates;
    bool Vector(const TSharedPtr<FJsonObject>& Obj, const TCHAR* Key, FVector& Out)
    {
        const TArray<TSharedPtr<FJsonValue>>* A=nullptr;
        if (!Obj.IsValid() || !Obj->TryGetArrayField(Key,A) || A->Num()!=3) return false;
        double X,Y,Z;
        if (!(*A)[0]->TryGetNumber(X)||!(*A)[1]->TryGetNumber(Y)||!(*A)[2]->TryGetNumber(Z)||!FMath::IsFinite(X)||!FMath::IsFinite(Y)||!FMath::IsFinite(Z)) return false;
        Out=FVector(X,Y,Z); return true;
    }
    bool Point(const TSharedPtr<FJsonObject>& Obj,const TCHAR* Key,FStorePoint& Out)
    {
        const TSharedPtr<FJsonObject>* P=nullptr;
        return Obj->TryGetObjectField(Key,P) && Vector(*P,TEXT("at"),Out.At) && (*P)->TryGetNumberField(TEXT("yaw"),Out.Yaw);
    }
}

bool MarketStoreKit::Parse(const FString& Json,TArray<FStoreTemplate>& Out,TArray<FString>& Errors)
{
    Out.Reset(); TSharedPtr<FJsonObject> Root; const TArray<TSharedPtr<FJsonValue>>* Stores=nullptr;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),Root)||!Root.IsValid()||!Root->TryGetArrayField(TEXT("stores"),Stores))
    { Errors.Add(TEXT("magazalar.json: invalid document")); return false; }
    int32 Version=0;
    if (!Root->TryGetNumberField(TEXT("schemaVersion"),Version)||Version!=1) { Errors.Add(TEXT("Unsupported store schema")); return false; }
    TSet<FString> Ids;
    for (const auto& V:*Stores)
    {
        const auto O=V->AsObject(); FStoreTemplate S;
        const TSharedPtr<FJsonObject>* Points=nullptr; const TArray<TSharedPtr<FJsonValue>>* Fixtures=nullptr;
        const TArray<TSharedPtr<FJsonValue>>* Footprint=nullptr;
        bool Valid=O.IsValid() && O->TryGetStringField(TEXT("id"),S.Id) && O->TryGetStringField(TEXT("format"),S.Format)
            && O->TryGetStringField(TEXT("name"),S.Name) && O->TryGetStringField(TEXT("theme"),S.Theme) && O->TryGetStringField(TEXT("shell"),S.Shell)
            && O->TryGetNumberField(TEXT("salesAreaM2"),S.SalesAreaM2) && O->TryGetNumberField(TEXT("backroomM2"),S.BackroomM2)
            && O->TryGetNumberField(TEXT("ceilingCm"),S.CeilingCm) && O->TryGetArrayField(TEXT("footprintCm"),Footprint) && Footprint->Num()==2
            && O->TryGetObjectField(TEXT("points"),Points) && O->TryGetArrayField(TEXT("fixtures"),Fixtures);
        if (!Valid) { Errors.Add(TEXT("Store required fields missing: ")+S.Id); continue; }
        S.FootprintCm=FVector2D((*Footprint)[0]->AsNumber(),(*Footprint)[1]->AsNumber());
        const TSharedPtr<FJsonObject>* Back=nullptr; FVector Lo,Hi;
        Valid=StoreData::Point(*Points,TEXT("entrance"),S.Entrance) && StoreData::Point(*Points,TEXT("receiving"),S.Receiving)
            && StoreData::Point(*Points,TEXT("playerStart"),S.PlayerStart) && (*Points)->TryGetObjectField(TEXT("backroom"),Back)
            && StoreData::Vector(*Back,TEXT("min"),Lo) && StoreData::Vector(*Back,TEXT("max"),Hi);
        if (!Valid) { Errors.Add(TEXT("Invalid store points: ")+S.Id); continue; }
        S.Backroom=FBox(Lo,Hi);
        const TArray<TSharedPtr<FJsonValue>>* Spawns=nullptr;
        if ((*Points)->TryGetArrayField(TEXT("customerSpawn"),Spawns))
            for (const auto& Spawn:*Spawns)
            {
                const auto& A=Spawn->AsArray();
                if (A.Num()==3) S.CustomerSpawn.Add(FVector(A[0]->AsNumber(),A[1]->AsNumber(),A[2]->AsNumber()));
            }
        for (const auto& FV:*Fixtures)
        {
            const auto F=FV->AsObject(); FPlanogramFixture Entry;
            if (!F.IsValid() || !F->TryGetStringField(TEXT("id"),Entry.Id) || !F->TryGetStringField(TEXT("equipment"),Entry.EquipmentId)
                || !F->TryGetStringField(TEXT("category"),Entry.Category) || !StoreData::Vector(F,TEXT("at"),Entry.Location) || !F->TryGetNumberField(TEXT("yaw"),Entry.Yaw))
            { Errors.Add(TEXT("Invalid fixture in ")+S.Id); continue; }
            Entry.Label=Entry.Category;
            const TSharedPtr<FJsonObject>* Faces=nullptr;
            if (F->TryGetObjectField(TEXT("faceCategories"),Faces)) for (const auto& Pair:(*Faces)->Values) Entry.FaceCategories.Add(FString(Pair.Key),FString(Pair.Value->AsString()));
            S.Fixtures.Add(Entry);
        }
        const TArray<TSharedPtr<FJsonValue>>* Props=nullptr;
        if (O->TryGetArrayField(TEXT("props"),Props)) for (const auto& PV:*Props)
        {
            const auto P=PV->AsObject(); FStoreProp Prop;
            if (!P.IsValid()||!P->TryGetStringField(TEXT("mesh"),Prop.Mesh)||!StoreData::Vector(P,TEXT("at"),Prop.At)||!P->TryGetNumberField(TEXT("yaw"),Prop.Yaw)) Errors.Add(TEXT("Invalid prop: ")+S.Id);
            else S.Props.Add(Prop);
        }
        if (Ids.Contains(S.Id)) Errors.Add(TEXT("Duplicate store id: ")+S.Id);
        Ids.Add(S.Id); S.Stats=CalculateStats(S);
        Validate(S,Errors); Out.Add(S);
    }
    if (Stores->Num()==0) Errors.Add(TEXT("No store templates"));
    return Errors.Num()==0;
}

FStoreStats MarketStoreKit::CalculateStats(const FStoreTemplate& S)
{
    FStoreStats Stats; Stats.BackroomPallets=FMath::FloorToInt(S.BackroomM2/3.5);
    for (const auto& F:S.Fixtures)
    {
        const auto E=MarketPlanogram::Equipment(F.EquipmentId); const double Width=E.DimensionsCm.X/100;
        if (E.Family==TEXT("shelf") || E.Family==TEXT("bakery") || E.Family==TEXT("tobacco")) Stats.ShelfFrontM+=Width*(E.bDoubleSided?2:1);
        if (E.Family==TEXT("cooler")) Stats.CoolerM+=Width;
        if (E.Family==TEXT("freezer")) Stats.FreezerM+=Width;
        if (E.Family==TEXT("produce")) Stats.ProduceM2+=Width*E.DimensionsCm.Y/100;
        if (E.Family==TEXT("deli")||E.Family==TEXT("butcher")||E.Family==TEXT("fish")||E.Family==TEXT("service")) Stats.Counters.AddUnique(E.Family);
        Stats.Checkouts+=E.CheckoutCount;
    }
    Stats.Counters.Sort(); return Stats;
}

bool MarketStoreKit::Validate(const FStoreTemplate& S,TArray<FString>& Errors)
{
    const int32 Before=Errors.Num(); auto Fail=[&](const FString& Reason){ Errors.Add(S.Id+TEXT(": ")+Reason); };
    if (!MarketCatalog::IsValidId(S.Id)||!S.Id.StartsWith(S.Format+TEXT("_"))||S.Id.Len()!=S.Format.Len()+3||!FChar::IsDigit(S.Id[S.Id.Len()-1])||!FChar::IsDigit(S.Id[S.Id.Len()-2])) Fail(TEXT("Invalid id"));
    if (S.Shell.IsEmpty()||S.Theme.IsEmpty()||S.CustomerSpawn.IsEmpty()) Fail(TEXT("Shell/theme/spawn missing"));
    const double W=S.FootprintCm.X,D=S.FootprintCm.Y;
    if (W<=0||D<=0||S.CeilingCm<=230||S.BackroomM2<=0||!FMath::IsFinite(S.SalesAreaM2)||FMath::Abs(W*D/10000-S.SalesAreaM2-S.BackroomM2)>.02) Fail(TEXT("Invalid area/dimensions"));
    if (S.Backroom.Min.X>=S.Backroom.Max.X||S.Backroom.Min.Y>=S.Backroom.Max.Y||S.Backroom.Min.Z>=S.Backroom.Max.Z
        ||FMath::Abs(S.Backroom.GetSize().X*S.Backroom.GetSize().Y/10000-S.BackroomM2)>.02) Fail(TEXT("Invalid backroom"));
    for (const FStorePoint* P:{&S.Entrance,&S.Receiving,&S.PlayerStart})
        if (P->At.ContainsNaN()||!FMath::IsFinite(P->Yaw)||FMath::Abs(P->At.X)>W/2+20||FMath::Abs(P->At.Y)>D/2+20) Fail(TEXT("Point outside shell"));
    TSet<FString> Ids,Categories,Families;
    TArray<FBox2D> Footprints;
    TArray<FMarketProduct> Products; TArray<FString> CatalogErrors; MarketCatalog::LoadFile(MarketCatalog::DefaultPath(),Products,CatalogErrors);
    TSet<FString> KnownCategories; for (const auto& P:Products) KnownCategories.Add(P.Category);
    for (const auto& F:S.Fixtures)
    {
        if (!MarketCatalog::IsValidId(F.Id)||Ids.Contains(F.Id)) Fail(TEXT("Invalid/duplicate fixture: ")+F.Id);
        Ids.Add(F.Id);
        if (!MarketPlanogram::IsKnownEquipment(F.EquipmentId)) Fail(TEXT("Unknown equipment: ")+F.EquipmentId);
        const auto E=MarketPlanogram::Equipment(F.EquipmentId); Families.Add(E.Family); Categories.Add(F.Category);
        if (!F.Category.IsEmpty()&&!KnownCategories.Contains(F.Category)) Fail(TEXT("Unknown category: ")+F.Category);
        for (const auto& Pair:F.FaceCategories)
            if ((Pair.Key!=TEXT("front")&&Pair.Key!=TEXT("back"))||(!Pair.Value.IsEmpty()&&!KnownCategories.Contains(Pair.Value))) Fail(TEXT("Invalid face category"));
        const double Angle=FMath::DegreesToRadians(F.Yaw);
        const double X=(FMath::Abs(FMath::Cos(Angle))*E.DimensionsCm.X+FMath::Abs(FMath::Sin(Angle))*E.DimensionsCm.Y)/2;
        const double Y=(FMath::Abs(FMath::Sin(Angle))*E.DimensionsCm.X+FMath::Abs(FMath::Cos(Angle))*E.DimensionsCm.Y)/2;
        if (F.Location.ContainsNaN()||FMath::Abs(F.Location.X)+X>W/2+1||FMath::Abs(F.Location.Y)+Y>D/2+1||F.Location.Z!=0) Fail(TEXT("Fixture outside shell: ")+F.Id);
        const FBox2D Bounds(FVector2D(F.Location.X-X+.25,F.Location.Y-Y+.25),FVector2D(F.Location.X+X-.25,F.Location.Y+Y-.25));
        for(const auto& Other:Footprints) if(Bounds.Intersect(Other)) { Fail(TEXT("Overlapping equipment: ")+F.Id); break; }
        Footprints.Add(Bounds);
        const double PlayerDX=FMath::Max(0.0,FMath::Abs(S.PlayerStart.At.X-F.Location.X)-X), PlayerDY=FMath::Max(0.0,FMath::Abs(S.PlayerStart.At.Y-F.Location.Y)-Y);
        if(FMath::Sqrt(PlayerDX*PlayerDX+PlayerDY*PlayerDY)<44) Fail(TEXT("Player starts inside fixture: ")+F.Id);
        if(F.Location.Y+Y>S.Backroom.Min.Y+.5) Fail(TEXT("Fixture enters backroom: ")+F.Id);
    }
    const FStoreStats Stats=CalculateStats(S);
    const double* Band=nullptr;
    static const double Mahalle[]={80,200,1,2,25,60,4,10}, Kucuk[]={250,400,2,3,60,110,8,16}, Buyuk[]={600,1500,4,8,180,400,25,60}, Hiper[]={3000,6000,15,30,700,1500,100,200};
    if (S.Format==TEXT("mahalle")) Band=Mahalle; else if(S.Format==TEXT("kucuk")) Band=Kucuk; else if(S.Format==TEXT("buyuk")) Band=Buyuk; else if(S.Format==TEXT("hiper")) Band=Hiper;
    else Fail(TEXT("Unknown format"));
    if (Band)
    {
        const double Values[]={S.SalesAreaM2,double(Stats.Checkouts),Stats.ShelfFrontM,Stats.CoolerM+Stats.FreezerM};
        for(int32 I=0;I<4;++I) if(Values[I]<Band[I*2]-.001||Values[I]>Band[I*2+1]+.001) Fail(FString::Printf(TEXT("Band %d violated: %.2f"),I,Values[I]));
    }
    for (const TCHAR* C:{TEXT("makarna-bakliyat"),TEXT("i\u00e7ecek"),TEXT("s\u00fct")}) if(!Categories.Contains(C)) Fail(FString(TEXT("Required aisle missing: "))+C);
    if(!Families.Contains(TEXT("cooler"))) Fail(TEXT("Dairy cooler missing"));
    if(S.Format!=TEXT("mahalle")&&!Categories.Contains(TEXT("temizlik"))) Fail(TEXT("Cleaning aisle missing"));
    if(S.Format!=TEXT("kucuk")) for(const TCHAR* F:{TEXT("produce"),TEXT("tobacco")}) if(!Families.Contains(F)) Fail(FString(TEXT("Department missing: "))+F);
    if(S.Format==TEXT("buyuk")||S.Format==TEXT("hiper"))
    {
        if(!Categories.Contains(TEXT("ki\u015fisel bak\u0131m"))) Fail(TEXT("Personal care aisle missing"));
        for(const TCHAR* F:{TEXT("deli"),TEXT("bakery"),TEXT("freezer")}) if(!Families.Contains(F)) Fail(FString(TEXT("Department missing: "))+F);
    }
    if(S.Format==TEXT("hiper")) for(const TCHAR* F:{TEXT("butcher"),TEXT("fish"),TEXT("home"),TEXT("electronics"),TEXT("textile"),TEXT("service")}) if(!Families.Contains(F)) Fail(FString(TEXT("Department missing: "))+F);
    return Errors.Num()==Before;
}
bool MarketStoreKit::Load(TArray<FString>& Errors)
{
    FString Json; TArray<FStoreTemplate> Loaded;
    if(!FFileHelper::LoadFileToString(Json,*(FPaths::ProjectConfigDir()/TEXT("magazalar.json")))) { Errors.Add(TEXT("magazalar.json missing")); return false; }
    if(!Parse(Json,Loaded,Errors)) return false;
    StoreData::Templates=MoveTemp(Loaded); return true;
}
TArray<FString> MarketStoreKit::TemplatesFor(const FString& Format)
{
    TArray<FString> Result; for(const auto& S:StoreData::Templates) if(S.Format==Format) Result.Add(S.Id); Result.Sort(); return Result;
}
const FStoreTemplate* MarketStoreKit::Find(const FString& Id) { return StoreData::Templates.FindByPredicate([&](const FStoreTemplate& S){return S.Id==Id;}); }
FMarketPlanogram MarketStoreKit::ToPlanogram(const FStoreTemplate& S,const TMap<FString,FString>& Overrides)
{
    FMarketPlanogram Plan;
    for(auto F:S.Fixtures)
    {
        const auto E=MarketPlanogram::Equipment(F.EquipmentId);
        if(E.Levels==0) continue;
        if(const auto* C=Overrides.Find(F.Id)) { F.Category=*C; F.FaceCategories.Reset(); }
        for(const TCHAR* Face:{TEXT("front"),TEXT("back")}) if(const auto* C=Overrides.Find(F.Id+TEXT("/")+Face)) F.FaceCategories.Add(Face,*C);
        F.Label=F.Category;
        Plan.Fixtures.Add(F);
    }
    return Plan;
}
void MarketStoreKit::Fill(FMarketPlanogram& Plan,const TArray<FMarketProduct>& Products)
{
    Plan.Placements.Reset();
    TSet<FString> Categories; for(const auto& F:Plan.Fixtures) { Categories.Add(F.CategoryForFace(TEXT("front"))); if(MarketPlanogram::Equipment(F.EquipmentId).bDoubleSided) Categories.Add(F.CategoryForFace(TEXT("back"))); }
    TArray<FString> Ordered=Categories.Array(); Ordered.Sort();
    for(const FString& C:Ordered)
    {
        if(C.IsEmpty()) continue;
        FMarketPlanogram Department;
        for(const auto& F:Plan.Fixtures) if(StaffPlanner::SameCategory(F.CategoryForFace(TEXT("front")),C)||StaffPlanner::SameCategory(F.CategoryForFace(TEXT("back")),C)) Department.Fixtures.Add(F);
        TArray<FMarketProduct> Items; for(const auto& P:Products) if(P.bActive&&StaffPlanner::SameCategory(P.Category,C)) Items.Add(P);
        // The planner may place several blocks of the same product. Repeat the department's assortment
        // to dress long aisles as well as the first shelf; this creates no economic stock.
        const TArray<FMarketProduct> Assortment=Items;
        int32 Rows=0; for(const auto& F:Department.Fixtures) { const auto E=MarketPlanogram::Equipment(F.EquipmentId); Rows+=E.Levels*(E.bDoubleSided?2:1); }
        const int32 Desired=Rows*2;
        while(!Assortment.IsEmpty()&&Items.Num()<Desired) Items.Append(Assortment);
        MarketLayout::Plan(Department,Items,{});
        for(const auto& Block:Department.Placements)
            if(const auto* F=Plan.FindFixture(Block.FixtureId)) if(StaffPlanner::SameCategory(F->CategoryForFace(Block.Face),C)) Plan.Placements.Add(Block);
    }
}
