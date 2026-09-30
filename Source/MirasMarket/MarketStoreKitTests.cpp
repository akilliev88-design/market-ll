#include "MarketStoreKit.h"
#include "MarketLayout.h"
#include "ProductCatalog.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/Font.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStoreTemplatesTest,"MirasMarket.Stores.PhaseA",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStoreTemplatesTest::RunTest(const FString&)
{
    TArray<FString> Errors;
    TestTrue(TEXT("Load store contract"),MarketStoreKit::Load(Errors)); for(const auto& E:Errors) AddError(E);
    TArray<FMarketProduct> Products; MarketCatalog::LoadFile(MarketCatalog::DefaultPath(),Products,Errors);
    Products.RemoveAll([](const auto& P){return !P.bActive;});
    TSet<FString> AllIds;
    for(const TCHAR* Format:{TEXT("mahalle"),TEXT("kucuk"),TEXT("buyuk"),TEXT("hiper")})
    {
        const auto Ids=MarketStoreKit::TemplatesFor(Format); TestTrue(TEXT("Format has template"),Ids.Num()>0);
        for(const FString& Id:Ids)
        {
            TestFalse(TEXT("Unique global store id"),AllIds.Contains(Id)); AllIds.Add(Id);
            const auto* S=MarketStoreKit::Find(Id); if(!S) { AddError(Id); continue; }
            auto Plan=MarketStoreKit::ToPlanogram(*S); TestTrue(TEXT("Fixtures supplied"),Plan.Fixtures.Num()>0);
            const auto Result=MarketLayout::Plan(Plan,Products,{});
            TestEqual(Id+TEXT(" layout fits all active products"),Result.Placed,Products.Num());
            TArray<FString> Warnings; MarketPlanogram::FindOverflows(Plan,Products,Warnings); TestEqual(TEXT("No overflow"),Warnings.Num(),0);
            auto Assigned=MarketStoreKit::ToPlanogram(*S); MarketStoreKit::Fill(Assigned,Products);
            TSet<FString> Placed; for(const auto& B:Assigned.Placements) Placed.Add(B.ProductId);
            TestEqual(Id+TEXT(" assigned departments fit all active products"),Placed.Num(),Products.Num());
            if (const auto* Double = Assigned.Fixtures.FindByPredicate([](const auto& F) { return MarketPlanogram::Equipment(F.EquipmentId).bDoubleSided; }))
            {
                TMap<FString,FString> Overrides; Overrides.Add(Double->Id+TEXT("/front"),FString()); Overrides.Add(Double->Id+TEXT("/back"),Products[0].Category);
                auto Changed=MarketStoreKit::ToPlanogram(*S,Overrides);
                TestTrue(TEXT("Empty front override retained"),Changed.FindFixture(Double->Id)->CategoryForFace(TEXT("front")).IsEmpty());
                TestEqual(TEXT("Independent back override retained"),Changed.FindFixture(Double->Id)->CategoryForFace(TEXT("back")),Products[0].Category);
                MarketStoreKit::Fill(Changed,Products);
                for (const auto& B:Changed.Placements)
                    if (const auto* P=MarketCatalog::FindProduct(Products,B.ProductId))
                        TestEqual(TEXT("Refill respects the chosen face category"),Changed.FindFixture(B.FixtureId)->CategoryForFace(B.Face),P->Category);
            }
            auto Bad=*S; Bad.SalesAreaM2=1; Errors.Reset(); TestFalse(TEXT("Reject band"),MarketStoreKit::Validate(Bad,Errors));
            if(!S->Obstacles.IsEmpty())
            {
                Bad=*S; Bad.Fixtures[0].Location=Bad.Obstacles[0].At; Errors.Reset();
                TestFalse(TEXT("Reject a fixture inside a structural column"),MarketStoreKit::Validate(Bad,Errors));
                Bad=*S; Bad.SalesAreaM2+=9; Errors.Reset();
                TestFalse(TEXT("Area follows recessed floor polygon"),MarketStoreKit::Validate(Bad,Errors));
            }
            Bad=*S; Bad.Fixtures.Append(S->Fixtures); Errors.Reset(); TestFalse(TEXT("Reject duplicate fixtures"),MarketStoreKit::Validate(Bad,Errors));
            Bad=*S; Bad.Fixtures.RemoveAll([](const auto& F){return MarketPlanogram::Equipment(F.EquipmentId).Family==TEXT("cooler");}); Errors.Reset(); TestFalse(TEXT("Reject missing dairy"),MarketStoreKit::Validate(Bad,Errors));
        }
    }
    TArray<FStoreTemplate> Parsed; Errors.Reset(); TestFalse(TEXT("Reject broken JSON"),MarketStoreKit::Parse(TEXT("{broken"),Parsed,Errors));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStoreRandomDressingTest,"MirasMarket.Stores.RandomDressing",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStoreRandomDressingTest::RunTest(const FString&)
{
    TArray<FString> Errors; TestTrue(TEXT("Load tour stores"),MarketStoreKit::Load(Errors));
    TArray<FMarketProduct> Products; MarketCatalog::LoadFile(MarketCatalog::DefaultPath(),Products,Errors);
    for(auto& P:Products) P.bActive=true;
    const auto CatalogBefore=Products;
    for(const TCHAR* Format:{TEXT("mahalle"),TEXT("kucuk"),TEXT("buyuk"),TEXT("hiper")})
        for(const auto& Id:MarketStoreKit::TemplatesFor(Format))
        {
            const auto* S=MarketStoreKit::Find(Id); auto A=MarketStoreKit::ToPlanogram(*S),B=A,C=A;
            MarketStoreKit::FillRandom(A,Products,17); MarketStoreKit::FillRandom(B,Products,17); MarketStoreKit::FillRandom(C,Products,29);
            TestEqual(Id+TEXT(" seeded fill is reproducible"),MarketPlanogram::Serialize(A),MarketPlanogram::Serialize(B));
            TestTrue(Id+TEXT(" another press changes assortment"),MarketPlanogram::Serialize(A)!=MarketPlanogram::Serialize(C));
            TArray<FString> Warnings; MarketPlanogram::FindOverflows(A,Products,Warnings); TestEqual(Id+TEXT(" random blocks fit packages"),Warnings.Num(),0);
            for(const auto& Block:A.Placements)
                if(const auto* P=MarketCatalog::FindProduct(Products,Block.ProductId))
                    TestEqual(TEXT("Random fill keeps department"),A.FindFixture(Block.FixtureId)->CategoryForFace(Block.Face),P->Category);
        }
    for(int32 I=0;I<Products.Num();++I)
    {
        TestEqual(TEXT("Test fill keeps real brand"),Products[I].Brand,CatalogBefore[I].Brand);
        TestEqual(TEXT("Test fill keeps price"),Products[I].BasePrice,CatalogBefore[I].BasePrice);
        TestEqual(TEXT("Test fill keeps cost"),Products[I].Cost,CatalogBefore[I].Cost);
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTurkishSignsTest,"MirasMarket.Stores.TurkishAndFaceCategories",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTurkishSignsTest::RunTest(const FString&)
{
    TestEqual(TEXT("Turkish dotted I"),MarketCatalog::UpperTurkish(TEXT("i\u00e7ecek")),FString(TEXT("\u0130\u00c7ECEK")));
    TestEqual(TEXT("Turkish dotless I"),MarketCatalog::UpperTurkish(TEXT("kuru g\u0131da")),FString(TEXT("KURU GIDA")));
    TestEqual(TEXT("All Turkish glyphs"),MarketCatalog::UpperTurkish(TEXT("\u00e7\u011f\u0131i\u00f6\u015f\u00fc")),FString(TEXT("\u00c7\u011eI\u0130\u00d6\u015e\u00dc")));
    auto* Font=LoadObject<UFont>(nullptr,TEXT("/Game/Stores/Fonts/F_PlexTurkish.F_PlexTurkish"));
    TestNotNull(TEXT("Offline Plex font asset"),Font);
    if(Font)
    {
        TestTrue(TEXT("Font uses offline cache"),Font->FontCacheType==EFontCacheType::Offline);
        for(TCHAR C:FString(TEXT("AZaz09\u00c7\u011e\u0130\u00d6\u015e\u00dc\u00e7\u011f\u0131\u00f6\u015f\u00fc\u20ba")))
        {
            const int32 Index=Font->RemapChar(C);
            TestTrue(FString::Printf(TEXT("Glyph %04x"),int32(C)),Font->Characters.IsValidIndex(Index)&&Font->Characters[Index].USize>0);
        }
    }
    FMarketPlanogram P; FPlanogramFixture F; F.Id=TEXT("gondol_1"); F.Category=TEXT("s\u00fct"); P.Fixtures.Add(F);
    TestEqual(TEXT("Old save inherits both faces"),F.CategoryForFace(TEXT("back")),F.Category);
    P.Fixtures[0].FaceCategories.Add(TEXT("back"),FString());
    TArray<FString> Errors; FMarketPlanogram Reloaded;
    TestTrue(TEXT("Round trip per-face categories"),MarketPlanogram::Parse(MarketPlanogram::Serialize(P),Reloaded,Errors));
    TestTrue(TEXT("Empty override means uncategorised"),Reloaded.Fixtures[0].CategoryForFace(TEXT("back")).IsEmpty());
    TestEqual(TEXT("Front remains dairy"),Reloaded.Fixtures[0].CategoryForFace(TEXT("front")),F.Category);
    return true;
}
