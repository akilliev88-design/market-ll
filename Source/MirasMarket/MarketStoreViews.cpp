#include "MarketStoreViews.h"
#include "MarketEconomy.h"
#include "MarketBranches.h"
#include "MarketStoreKit.h"
#include "MarketStart.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MarketStoreViewsLocal
{
    struct FCatalogBox
    {
        bool bLoaded = false;
        TArray<MarketStoreViews::FView> Views;
    };

    FCatalogBox& Box()
    {
        static FCatalogBox Catalog;
        return Catalog;
    }

    // Read magazalar.json into our own copy (MarketStoreKit::Parse leaves the kit's loaded templates alone).
    void LoadFromFile(TArray<MarketStoreViews::FView>& Out)
    {
        Out.Reset();
        FString Json;
        if (!FFileHelper::LoadFileToString(Json, *(FPaths::ProjectConfigDir() / TEXT("magazalar.json")))) return;
        TArray<FStoreTemplate> Templates;
        TArray<FString> Errors;
        if (!MarketStoreKit::Parse(Json, Templates, Errors)) return;
        for (const FStoreTemplate& Store : Templates)
        {
            if (Store.Id.IsEmpty() || Store.Format.IsEmpty()) continue;
            MarketStoreViews::FView View;
            View.Id = Store.Id;
            View.Format = Store.Format;
            View.Name = Store.Name.IsEmpty() ? Store.Id : Store.Name;
            View.Measures.ShelfFrontM = static_cast<float>(Store.Stats.ShelfFrontM);
            View.Measures.CoolerM = static_cast<float>(Store.Stats.CoolerM);
            View.Measures.FreezerM = static_cast<float>(Store.Stats.FreezerM);
            View.Measures.ProduceM2 = static_cast<float>(Store.Stats.ProduceM2);
            View.Measures.Counters = Store.Stats.Counters.Num();
            View.Measures.Checkouts = Store.Stats.Checkouts;
            View.Measures.SelfCheckouts = Store.Stats.SelfCheckouts;
            View.Measures.BackroomPallets = Store.Stats.BackroomPallets;
            View.Measures.SalesAreaM2 = static_cast<float>(Store.SalesAreaM2);
            Out.Add(View);
        }
    }

    // Country and province of a site key: abroad one view per country and type (03_MAGAZA_AGI.md section 5).
    FString KeyProvince(const FMarketState& State, const FString& Country, const FString& Province)
    {
        const FString C = Country.IsEmpty() ? State.CountryId : Country;
        return C == State.CountryId ? Province : FString();
    }

    void CopyInto(FMarketBranch& Branch, const FString& Id, const MarketStoreAssign::FStoreMeasures& M)
    {
        Branch.StoreView = Id;
        Branch.ViewShelfM = M.ShelfFrontM;
        Branch.ViewCoolerM = M.CoolerM;
        Branch.ViewFreezerM = M.FreezerM;
        Branch.ViewProduceM2 = M.ProduceM2;
        Branch.ViewCounters = M.Counters;
        Branch.ViewCheckouts = M.Checkouts;
        Branch.ViewSelfCheckouts = M.SelfCheckouts;
        Branch.ViewPallets = M.BackroomPallets;
        Branch.ViewAreaM2 = M.SalesAreaM2;
    }
}

const TArray<MarketStoreViews::FView>& MarketStoreViews::Catalog()
{
    MarketStoreViewsLocal::FCatalogBox& Box = MarketStoreViewsLocal::Box();
    if (!Box.bLoaded)
    {
        MarketStoreViewsLocal::LoadFromFile(Box.Views);
        Box.bLoaded = true;
    }
    return Box.Views;
}

void MarketStoreViews::SetCatalog(const TArray<FView>& Views)
{
    MarketStoreViewsLocal::FCatalogBox& Box = MarketStoreViewsLocal::Box();
    Box.Views = Views;
    Box.bLoaded = true;
}

void MarketStoreViews::ResetCatalog()
{
    MarketStoreViewsLocal::FCatalogBox& Box = MarketStoreViewsLocal::Box();
    Box.Views.Reset();
    Box.bLoaded = false;
}

const MarketStoreViews::FView* MarketStoreViews::Find(const FString& Id)
{
    return Catalog().FindByPredicate([&Id](const FView& V) { return V.Id == Id; });
}

TArray<FString> MarketStoreViews::IdsFor(const FString& Format)
{
    TArray<FString> Ids;
    for (const FView& V : Catalog()) if (V.Format == Format) Ids.Add(V.Id);
    if (Ids.Num() == 0 && MarketBranches::BaseFormat(Format) != Format) return IdsFor(MarketBranches::BaseFormat(Format)); // M54
    Ids.Sort();
    return Ids;
}

FString MarketStoreViews::ViewFor(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format)
{
    const FString C = Country.IsEmpty() ? State.CountryId : Country;
    return MarketStoreAssign::Resolve(State.StoreViews, C, MarketStoreViewsLocal::KeyProvince(State, C, Province), Format, IdsFor(Format));
}

MarketStoreAssign::FStoreMeasures MarketStoreViews::MeasuresFor(const FMarketState& State, const FString& Country, const FString& Province, const FString& Format)
{
    const FView* View = Find(ViewFor(State, Country, Province, Format));
    return View ? MarketStoreAssign::Normalized(View->Measures, Format) : MarketStoreAssign::Nominal(Format);
}

FString MarketStoreViews::AssignTo(FMarketState& State, FMarketBranch& Branch)
{
    const FString C = Branch.Country.IsEmpty() ? State.CountryId : Branch.Country;
    const FString Province = MarketStoreViewsLocal::KeyProvince(State, C, Branch.Province.IsEmpty() ? MarketStart::HomeProvince(State) : Branch.Province);
    const FString Id = MarketStoreAssign::Assign(State.StoreViews, C, Province, Branch.Format, IdsFor(Branch.Format));
    const FView* View = Find(Id);
    if (!View) return FString();
    MarketStoreViewsLocal::CopyInto(Branch, View->Id, MarketStoreAssign::Normalized(View->Measures, Branch.Format));
    return View->Id;
}

FString MarketStoreViews::PreviewTo(const FMarketState& State, FMarketBranch& Branch)
{
    const FView* View = Find(ViewFor(State, Branch.Country, Branch.Province.IsEmpty() ? MarketStart::HomeProvince(State) : Branch.Province, Branch.Format));
    if (!View) return FString();
    MarketStoreViewsLocal::CopyInto(Branch, View->Id, MarketStoreAssign::Normalized(View->Measures, Branch.Format));
    return View->Id;
}

bool MarketStoreViews::HasMeasures(const FMarketBranch& Branch)
{
    return !Branch.StoreView.IsEmpty();
}

MarketStoreAssign::FStoreMeasures MarketStoreViews::MeasuresOf(const FMarketBranch& Branch)
{
    if (!HasMeasures(Branch)) return MarketStoreAssign::Nominal(Branch.Format);
    MarketStoreAssign::FStoreMeasures M;
    M.ShelfFrontM = Branch.ViewShelfM;
    M.CoolerM = Branch.ViewCoolerM;
    M.FreezerM = Branch.ViewFreezerM;
    M.ProduceM2 = Branch.ViewProduceM2;
    M.Counters = Branch.ViewCounters;
    M.Checkouts = Branch.ViewCheckouts;
    M.SelfCheckouts = Branch.ViewSelfCheckouts;
    M.BackroomPallets = Branch.ViewPallets;
    M.SalesAreaM2 = Branch.ViewAreaM2;
    return MarketStoreAssign::Normalized(M, Branch.Format);
}

FString MarketStoreViews::Describe(const FMarketBranch& Branch)
{
    const MarketStoreAssign::FStoreMeasures M = MeasuresOf(Branch);
    const FView* View = HasMeasures(Branch) ? Find(Branch.StoreView) : nullptr;
    const FString Name = View ? View->Name : HasMeasures(Branch) ? Branch.StoreView : FString(TEXT("standart ma\u011faza"));
    FString Tills = FString::Printf(TEXT("%d kasa"), M.Checkouts);
    if (M.SelfCheckouts > 0) Tills += FString::Printf(TEXT(" + %d self servis"), M.SelfCheckouts);
    return FString::Printf(TEXT("%s \u00b7 %.0f m\u00b2 \u00b7 %.0f m raf \u00b7 %s"), *Name, M.SalesAreaM2, M.ShelfFrontM, *Tills);
}

FString MarketStoreViews::WeeklyHint(const FMarketBranch& Branch, int32 Shoppers)
{
    const MarketStoreAssign::FStoreMeasures M = MeasuresOf(Branch);
    if (Branch.LastQueueLost > 0 && MarketStoreAssign::QueueFactor(M, Branch.Format, static_cast<float>(Shoppers)) < 0.95f)
        return FString::Printf(TEXT(" Kasalar yetmiyor: d\u00fcn %d m\u00fc\u015fteri kuyruktan vazge\u00e7ti."), Branch.LastQueueLost);
    if (MarketStoreAssign::FreshFactor(M, Branch.Format) < 0.85f)
        return TEXT(" So\u011fuk dolap ve manav alan\u0131 dar: taze \u00fcr\u00fcn az sat\u0131yor, fire fazla.");
    return FString();
}

