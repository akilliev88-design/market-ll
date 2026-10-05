#include "MarketStoreDressing.h"
#include "MarketStoreKit.h"
#include "ProductCatalog.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDesktopStorePackingTest,"MirasMarket.Stores.DesktopPreviewPacking",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDesktopStorePackingTest::RunTest(const FString&)
{
    TArray<FString> Errors;TArray<FMarketProduct> Products;
    if(!TestTrue(TEXT("Load authored stores"),MarketStoreKit::Load(Errors)))return false;
    if(!TestTrue(TEXT("Load catalogue"),MarketCatalog::LoadFile(MarketCatalog::DefaultPath(),Products,Errors)))return false;
    for(const TCHAR* Id:{TEXT("kucuk_01"),TEXT("buyuk_01"),TEXT("hiper_01")})
    {
        const auto* S=MarketStoreKit::Find(Id);if(!S){AddError(TEXT("Store missing"));continue;}
        auto Plan=MarketStoreKit::ToPlanogram(*S);MarketStoreDressing::FillPreview(Plan,Products);
        TestTrue(TEXT("Authored preview has products"),Plan.Placements.Num()>0);
        TArray<FString> Warnings;MarketPlanogram::FindOverflows(Plan,Products,Warnings);
        TestEqual(TEXT("No overflowing shelf rows"),Warnings.Num(),0);
        for(const auto& B:Plan.Placements)
        {
            const auto* F=Plan.FindFixture(B.FixtureId);const auto* P=MarketCatalog::FindProduct(Products,B.ProductId);
            if(!F||!P){AddError(TEXT("Missing placement product or fixture"));continue;}
            const auto E=MarketPlanogram::Equipment(F->EquipmentId);const float Width=MarketPlanogram::BlockWidthCm(*P,B);
            TestEqual(TEXT("Placement belongs to face category"),P->Category,F->CategoryForFace(B.Face));
            TestTrue(TEXT("Block inside physical shelf ends"),FMath::Abs(B.XCm)+Width*.5f<=E.UsableWidthCm*.5f+.01f);
            TestTrue(TEXT("Package fits shelf height"),E.LevelClearanceCm[B.Level]<=0||MarketPlanogram::NominalHeightCm(*P)<=E.LevelClearanceCm[B.Level]);
            TestTrue(TEXT("Depth stays inside usable shelf"),B.Depth<=MarketPlanogram::PhysicalDepth(Plan,*P,B));
        }
    }
    return true;
}
