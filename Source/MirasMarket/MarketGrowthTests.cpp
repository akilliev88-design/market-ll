#include "MarketBranches.h"
#include "MarketEconomy.h"
#include "MarketManagers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace MarketGrowthTest
{
    // A company with Count open branches whose leases were signed DaysAgo days ago.
    void AddShops(FMarketState& S, int32 Count, int32 DaysAgo)
    {
        for (int32 I = 0; I < Count; ++I)
        {
            FMarketBranch& B = S.Branches.AddDefaulted_GetRef();
            B.Name = FString::Printf(TEXT("Test %d"), S.Branches.Num());
            B.Format = TEXT("mahalle");
            B.Stage = static_cast<uint8>(MarketBranches::EStage::Open);
            B.OpenedDay = FMath::Max(1, S.Day - DaysAgo + 8);
            B.SignedDay = FMath::Max(1, S.Day - DaysAgo);
        }
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMarketGrowthStrainTest, "MirasMarket.Branches.GrowthStrain", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FMarketGrowthStrainTest::RunTest(const FString& Parameters)
{
    using namespace MarketGrowthTest;
    FMarketState S; S.Initialize(TArray<FMarketProduct>());
    S.Day = 2000;
    AddShops(S, 4, 30);
    TestEqual(TEXT("Four leases in a year"), MarketBranches::SignedLastYear(S), 4);
    TestEqual(TEXT("No strain for a calm pace"), MarketBranches::GrowthStrain(S), 0.f);
    TestTrue(TEXT("No warning"), MarketBranches::GrowthStrainText(S).IsEmpty());

    // Old leases do not count; a rush of new ones does.
    AddShops(S, 10, 500);
    TestEqual(TEXT("Old leases left out"), MarketBranches::SignedLastYear(S), 4);
    AddShops(S, 16, 60);
    const int32 Capacity = MarketBranches::GrowthCapacity(S);
    TestEqual(TEXT("Capacity: base and half the shops"), Capacity, MarketBranches::StrainBase + 30 / 2);
    // 20 leases against 23: calm enough. Twenty more in the last months (40 against 33): strained.
    AddShops(S, 20, 20);
    const float Strain = MarketBranches::GrowthStrain(S);
    TestTrue(TEXT("Fast growth strains the management"), Strain > 0.f && Strain <= 1.f);
    TestFalse(TEXT("The menu warns"), MarketBranches::GrowthStrainText(S).IsEmpty());
    TestTrue(TEXT("One more lease strains more"), MarketBranches::GrowthStrain(S, 1) > Strain);

    // Province managers widen what the company can follow.
    for (int32 I = 0; I < 4; ++I)
    {
        FMarketManager& M = S.Management.Managers.AddDefaulted_GetRef();
        M.Level = static_cast<uint8>(MarketManagers::ELevel::Province);
        M.Name = FString::Printf(TEXT("Il mudur %d"), I);
    }
    TestEqual(TEXT("Managers add capacity"), MarketBranches::GrowthCapacity(S), Capacity + 10 + 4 * MarketBranches::StrainPerManager);
    TestTrue(TEXT("Less strain with managers"), MarketBranches::GrowthStrain(S) < Strain);
    return true;
}

#endif
