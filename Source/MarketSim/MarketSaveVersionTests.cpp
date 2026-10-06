#include "MarketGame.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSaveVersionGate, "MarketSim.Save.ExactVersion", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSaveVersionGate::RunTest(const FString& Parameters)
{
    const auto RoundTrip = [&](int32 Version, bool Expected)
    {
        auto* Save = NewObject<UMarketSave>(); Save->State.Version = Version;
        TArray<uint8> Bytes;
        if (!TestTrue(TEXT("Save serialized to memory"), UGameplayStatics::SaveGameToMemory(Save, Bytes))) return;
        auto* Loaded = Cast<UMarketSave>(UGameplayStatics::LoadGameFromMemory(Bytes));
        if (!TestNotNull(TEXT("Save deserialized"), Loaded)) return;
        TestEqual(TEXT("Serialized version is preserved, never silently upgraded"), Loaded->State.Version, Version);
        TestEqual(TEXT("Only current version passes the campaign load gate"), Loaded->State.IsStructurallyValid(), Expected);
    };
    RoundTrip(FMarketState::CurrentVersion - 1, false);
    RoundTrip(FMarketState::CurrentVersion, true);
    RoundTrip(FMarketState::CurrentVersion + 1, false);
    return true;
}
#endif