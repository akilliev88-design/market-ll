#include "MarketTelevisionDisplay.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/TextRenderComponent.h"
#include "Engine/PointLight.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTelevisionDisplayTest, "MarketSim.Visuals.TelevisionDisplay", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTelevisionDisplayTest::RunTest(const FString&)
{
    using namespace MarketTelevisionDisplay;
    TMap<FString, FProfile> Profiles; FString Error;
    TestTrue(TEXT("Explicit id mappings"), Parse(TEXT("{\"products\":[{\"id\":\"tv_55\",\"technology\":\"OLED\",\"inches\":55,\"resolution\":\"4K\",\"refreshHz\":120,\"rearLight\":true},{\"id\":\"tv_32\",\"inches\":32}]}"), Profiles, Error));
    TestEqual(TEXT("Actual technology is shown"), Features(Profiles[TEXT("tv_55")]), FString(TEXT("55\" / OLED / 4K / 120 Hz")));
    TestEqual(TEXT("Unknown technology is never guessed"), Features(Profiles[TEXT("tv_32")]), FString(TEXT("32\"")));
    TestTrue(TEXT("Rear lighting explicitly enabled"), Profiles[TEXT("tv_55")].bRearLight);
    TestFalse(TEXT("Rear lighting disabled by default"), Profiles[TEXT("tv_32")].bRearLight);
    TestFalse(TEXT("Other product does not inherit TV specifications"), Profiles.Contains(TEXT("milk_1l")));
    TestFalse(TEXT("Duplicate ids rejected"), Parse(TEXT("{\"products\":[{\"id\":\"tv_55\"},{\"id\":\"tv_55\"}]}"), Profiles, Error));
    TestEqual(TEXT("Failed parse is atomic"), Profiles.Num(), 0);
    TestFalse(TEXT("Negative diagonal rejected"), Parse(TEXT("{\"products\":[{\"id\":\"tv_55\",\"inches\":-1}]}"), Profiles, Error));
    TestFalse(TEXT("Malformed JSON rejected"), Parse(TEXT("{}"), Profiles, Error));
    for (const TCHAR* Id : { TEXT("tv_wall_4800"), TEXT("tv_plinth_2400"), TEXT("tv_island_3000") })
    {
        TestTrue(TEXT("Fixture registered"), MarketPlanogram::IsKnownEquipment(Id));
        const auto Equipment = MarketPlanogram::Equipment(Id);
        TestTrue(TEXT("Fixture has usable shelf"), Equipment.Levels > 0);
        TestFalse(TEXT("Category sign stays on authored panel"), Equipment.bSignOnTop);
    }
    TestTrue(TEXT("Island allows independent sides"), MarketPlanogram::Equipment(TEXT("tv_island_3000")).bDoubleSided);
    for (const int32 Inches : {32, 43, 55, 65, 75})
    {
        auto* Mesh = LoadObject<UStaticMesh>(nullptr, *FString::Printf(TEXT("/Game/Stores/Televisions/SM_Television%d.SM_Television%d"), Inches, Inches));
        if (!TestNotNull(TEXT("Independent TV asset imported"), Mesh)) continue;
        const FVector Size = Mesh->GetBoundingBox().GetSize();
        const float ScreenWidth = Inches * 2.54f * 16.f / FMath::Sqrt(337.f);
        TestTrue(TEXT("Physical screen size matches diagonal"), FMath::IsNearlyEqual(Size.Y, ScreenWidth + 2.2f, .2f));
        TestTrue(TEXT("Product front axis is depth X"), Size.X < 35 && Size.Y > 65);
        TestTrue(TEXT("Product bottom sits on shelf"), FMath::Abs(Mesh->GetBoundingBox().Min.Z) < .1);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTelevisionLabelsTest, "MarketSim.Visuals.TelevisionLabels", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTelevisionLabelsTest::RunTest(const FString&)
{
    using namespace MarketTelevisionDisplay;
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
    FMarketPlanogram Plan;
    FPlanogramFixture Fixture; Fixture.Id = TEXT("review"); Fixture.EquipmentId = TEXT("tv_island_3000"); Fixture.Yaw = 90;
    Plan.Fixtures.Add(Fixture);
    FMarketProduct Product; Product.Id = TEXT("tv_55"); Product.WidthMm = 1240; Product.DepthMm = 280; Product.HeightMm = 770;
    TArray<FMarketProduct> Products = {Product};
    FPlanogramPlacement Placement; Placement.ProductId = Product.Id; Placement.FixtureId = Fixture.Id; Placement.Facings = 1; Placement.Face = TEXT("back"); Placement.bHasX = true;
    FProfile Profile; Profile.Technology = TEXT("OLED"); Profile.bRearLight = true;
    TArray<AActor*> Actors;
    Decorate(World, Plan, Products, Placement, TEXT("Test brand 55"), &Profile, Actors);
    TestEqual(TEXT("Panel, product name, technology and explicit glow"), Actors.Num(), 4);
    int32 Texts = 0, Lights = 0;
    for (auto* Actor : Actors)
    {
        if (const auto* Text = Actor->FindComponentByClass<UTextRenderComponent>())
        {
            ++Texts;
            const auto Local = FTransform(FRotator(0,Fixture.Yaw,0),Fixture.Location).InverseTransformPosition(Text->GetComponentLocation());
            TestTrue(TEXT("Back labels face back aisle even after rotation"), Local.Y > 0);
            TestTrue(TEXT("Text fits allocated product width"), Text->GetTextLocalSize().Y <= 119.f + .1f);
        }
        if (Cast<APointLight>(Actor)) ++Lights;
        Actor->Destroy();
    }
    TestEqual(TEXT("Two separate dynamic text lines"), Texts, 2); TestEqual(TEXT("Only one enabled glow"), Lights, 1);
    Actors.Reset();
    Decorate(World, Plan, Products, Placement, TEXT("Replacement product"), nullptr, Actors);
    TestEqual(TEXT("Replacement without specs has panel and name only"), Actors.Num(), 2);
    for (auto* Actor : Actors) Actor->Destroy();
    GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
    return true;
}
