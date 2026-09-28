#include "MarketLayout.h"
#include "MarketGoods.h"
#include "PlanogramEdit.h"

namespace MarketLayout
{
    using MarketGoods::EGroup;

    bool IsNonFood(EGroup Group)
    {
        return Group == EGroup::Household || Group == EGroup::PersonalCare || Group == EGroup::Paper;
    }

    struct FFaceSlot
    {
        FString FixtureId;
        FString Face;
        float Distance = 0.f;   // from the entrance (fixture Y)
        FString Category;       // assigned aisle
    };

    FPlanogramFixture MakeFixture(const TCHAR* Id, const TCHAR* Equipment, float X, float Y, float Yaw)
    {
        FPlanogramFixture F;
        F.Id = Id; F.EquipmentId = Equipment; F.Location = FVector(X, Y, 0.f); F.Yaw = Yaw;
        return F;
    }

    // Facings for one case at the product's physical depth on this fixture, then scaled by demand.
    int32 WantedFacings(const FMarketPlanogram& Planogram, const FMarketProduct& Product, const FPlanogramPlacement& Block, float DemandScale)
    {
        const int32 Depth = FMath::Max(1, MarketPlanogram::PhysicalDepth(Planogram, Product, Block));
        const int32 OneCase = FMath::DivideAndRoundUp(FMath::Max(12, Product.CaseUnits), Depth);
        return FMath::Clamp(FMath::RoundToInt32(OneCase * FMath::Clamp(DemandScale, 0.6f, 2.f)), 1, 6);
    }
}

int32 MarketLayout::AisleRank(const FString& Category)
{
    switch (MarketGoods::Classify(Category))
    {
    case EGroup::Drinks: return 0;
    case EGroup::IceCream: return 1;
    case EGroup::Snacks: return 2;
    case EGroup::Sweets: return 3;
    case EGroup::TeaCoffee: return 4;
    case EGroup::Staples: return 5;
    case EGroup::OilSauce: return 6;
    case EGroup::Dairy: return 7;      // at the back: shoppers walk past everything else
    case EGroup::Other: return 8;
    case EGroup::Paper: return 10;
    case EGroup::PersonalCare: return 11;
    case EGroup::Household: return 12;
    default: return 9;
    }
}

TArray<int32> MarketLayout::LevelPreference(const FPlanogramEquipment& Equipment, const FMarketProduct& Product)
{
    const int32 Levels = FMath::Clamp(Equipment.Levels, 1, MarketPlanogram::MaxLevels);
    TArray<int32> Order;
    const float Volume = MarketPlanogram::NominalWidthCm(Product) * MarketPlanogram::NominalDepthCm(Product) * MarketPlanogram::NominalHeightCm(Product);
    const double Margin = Product.BasePrice > 0 ? static_cast<double>(Product.BasePrice - Product.Cost) / Product.BasePrice : 0.0;
    if (Volume > 4000.f || MarketPlanogram::NominalHeightCm(Product) > 28.f)
    {
        for (int32 L = 0; L < Levels; ++L) Order.Add(L);               // heavy and big: bottom up
        return Order;
    }
    // Eye and hand level are the upper middle shelves; the best spot goes to margin or to small, light packages.
    const int32 Top = Levels - 1;
    const bool bPrime = Margin >= 0.33 || MarketPlanogram::NominalHeightCm(Product) < 10.f;
    const TArray<int32> Prime = bPrime ? TArray<int32>{ Top, Top - 1, Top - 2, 0 } : TArray<int32>{ Top - 1, Top - 2, Top, 0 };
    for (const int32 L : Prime) if (L >= 0 && L < Levels) Order.AddUnique(L);
    for (int32 L = 0; L < Levels; ++L) Order.AddUnique(L);
    return Order;
}

FMarketPlanogram MarketLayout::Fixtures(const FString& Format)
{
    FMarketPlanogram Plan;
    if (Format == TEXT("kucuk"))
    {
        Plan.Fixtures.Add(MakeFixture(TEXT("gondol_1"), TEXT("gondola_double_1200"), -130.f, 400.f, 0.f));
        Plan.Fixtures.Add(MakeFixture(TEXT("gondol_2"), TEXT("gondola_double_1200"), 130.f, 400.f, 0.f));
        Plan.Fixtures.Add(MakeFixture(TEXT("duvar_1"), TEXT("wall_shelf_2400"), -420.f, 250.f, 90.f));
        return Plan;
    }
    const bool bBig = Format == TEXT("buyuk");
    // The family shop's arrangement: gondolas in rows, wall shelves on both sides.
    const int32 Rows = bBig ? 2 : 1;
    int32 Index = 0;
    for (int32 Row = 0; Row < Rows; ++Row)
        for (const float X : { -260.f, 0.f, 260.f, bBig ? 520.f : -99999.f })
        {
            if (X < -9999.f) continue;
            Plan.Fixtures.Add(MakeFixture(*FString::Printf(TEXT("gondol_%d"), ++Index), TEXT("gondola_double_1200"), X, 550.f + Row * 310.f, 0.f));
        }
    Plan.Fixtures.Add(MakeFixture(*FString::Printf(TEXT("gondol_%d"), ++Index), TEXT("gondola_double_1200"), 0.f, 550.f + Rows * 310.f, 0.f));
    const int32 Walls = bBig ? 3 : 3;
    for (int32 W = 0; W < Walls; ++W)
    {
        Plan.Fixtures.Add(MakeFixture(*FString::Printf(TEXT("duvar_sol_%d"), W + 1), TEXT("wall_shelf_2400"), -596.f, 180.f + W * 250.f, 90.f));
        Plan.Fixtures.Add(MakeFixture(*FString::Printf(TEXT("duvar_sag_%d"), W + 1), TEXT("wall_shelf_2400"), 596.f, 180.f + W * 250.f, -90.f));
    }
    return Plan;
}

MarketLayout::FResult MarketLayout::Plan(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const TArray<float>& Demand)
{
    FResult Result;
    Planogram.Placements.Reset();
    auto DemandOf = [&Demand](int32 I) { return Demand.IsValidIndex(I) ? FMath::Max(0.05f, Demand[I]) : 1.f; };

    // 1. Aisles and their demand, in the customer path order.
    TArray<FString> Categories;
    TMap<FString, float> CategoryDemand;
    for (int32 I = 0; I < Products.Num(); ++I)
    {
        const FString Key = MarketGoods::Fold(Products[I].Category);
        if (!CategoryDemand.Contains(Key)) Categories.Add(Key);
        CategoryDemand.FindOrAdd(Key) += DemandOf(I);
    }
    Categories.Sort([](const FString& A, const FString& B) { return AisleRank(A) != AisleRank(B) ? AisleRank(A) < AisleRank(B) : A < B; });

    // 2. Faces of the fixtures, nearest to the entrance first.
    TArray<FFaceSlot> Slots;
    for (const FPlanogramFixture& F : Planogram.Fixtures)
    {
        const FPlanogramEquipment Spec = MarketPlanogram::Equipment(F.EquipmentId);
        for (int32 Face = 0; Face < (Spec.bDoubleSided ? 2 : 1); ++Face)
        {
            FFaceSlot Slot;
            Slot.FixtureId = F.Id;
            Slot.Face = Face == 0 ? TEXT("front") : TEXT("back");
            Slot.Distance = F.Location.Y + Face * 1.f;
            Slots.Add(Slot);
        }
    }
    Slots.Sort([](const FFaceSlot& A, const FFaceSlot& B) { return A.Distance != B.Distance ? A.Distance < B.Distance : A.FixtureId < B.FixtureId; });
    if (Slots.Num() == 0 || Categories.Num() == 0) return Result;

    // Food aisles take faces from the entrance, non-food from the far end; faces in proportion to demand.
    TArray<FString> Food, NonFood;
    float FoodDemand = 0.f, NonFoodDemand = 0.f;
    for (const FString& C : Categories)
    {
        if (IsNonFood(MarketGoods::Classify(C))) { NonFood.Add(C); NonFoodDemand += CategoryDemand[C]; }
        else { Food.Add(C); FoodDemand += CategoryDemand[C]; }
    }
    const float TotalDemand = FMath::Max(0.01f, FoodDemand + NonFoodDemand);
    int32 NonFoodSlots = NonFood.Num() > 0 ? FMath::Clamp(FMath::RoundToInt32(Slots.Num() * NonFoodDemand / TotalDemand), NonFood.Num(), FMath::Max(NonFood.Num(), Slots.Num() - Food.Num())) : 0;
    NonFoodSlots = FMath::Min(NonFoodSlots, Slots.Num());
    const int32 FoodSlots = Slots.Num() - NonFoodSlots;
    auto Assign = [&Slots, &CategoryDemand](const TArray<FString>& Group, float GroupDemand, int32 First, int32 Count, bool bReverse)
    {
        int32 Given = 0;
        for (int32 G = 0; G < Group.Num() && Given < Count; ++G)
        {
            const int32 Left = Group.Num() - G - 1;
            const int32 Share = FMath::Clamp(FMath::RoundToInt32(Count * CategoryDemand[Group[G]] / FMath::Max(0.01f, GroupDemand)), 1, FMath::Max(1, Count - Given - Left));
            for (int32 S = 0; S < Share && Given < Count; ++S, ++Given)
            {
                const int32 Index = bReverse ? Slots.Num() - 1 - (First + Given) : First + Given;
                Slots[Index].Category = Group[G];
            }
        }
        // Leftover faces go to the busiest aisle of the group.
        for (; Given < Count && Group.Num() > 0; ++Given) Slots[bReverse ? Slots.Num() - 1 - (First + Given) : First + Given].Category = Group[0];
    };
    Assign(Food, FoodDemand, 0, FoodSlots, false);
    Assign(NonFood, NonFoodDemand, 0, NonFoodSlots, true);
    // Cleaning never shares a gondola with food: a fixture with one non-food face becomes non-food on both.
    for (FFaceSlot& Slot : Slots)
    {
        if (IsNonFood(MarketGoods::Classify(Slot.Category))) continue;
        if (const FFaceSlot* Other = Slots.FindByPredicate([&Slot](const FFaceSlot& S) { return S.FixtureId == Slot.FixtureId && &S != &Slot && IsNonFood(MarketGoods::Classify(S.Category)); }))
            Slot.Category = Other->Category;
    }
    for (FPlanogramFixture& F : Planogram.Fixtures)
        if (const FFaceSlot* Front = Slots.FindByPredicate([&F](const FFaceSlot& S) { return S.FixtureId == F.Id; }))
        {
            F.Category = Front->Category;
            Result.Aisles.Add(F.Id + TEXT(": ") + F.Category);
        }

    // 3-5. Products by aisle, brand blocks, busiest first; each on the best level of its aisle's faces.
    TArray<int32> Order;
    for (int32 I = 0; I < Products.Num(); ++I) Order.Add(I);
    Order.Sort([&Products, &DemandOf](int32 A, int32 B)
    {
        const int32 RankA = AisleRank(Products[A].Category), RankB = AisleRank(Products[B].Category);
        if (RankA != RankB) return RankA < RankB;
        if (Products[A].Brand != Products[B].Brand) return Products[A].Brand < Products[B].Brand;
        return DemandOf(A) > DemandOf(B);
    });
    for (const int32 I : Order)
    {
        const FMarketProduct& Product = Products[I];
        const FString Key = MarketGoods::Fold(Product.Category);
        float AisleAverage = CategoryDemand[Key];
        int32 InAisle = 0;
        for (const FMarketProduct& P : Products) if (MarketGoods::Fold(P.Category) == Key) ++InAisle;
        AisleAverage /= FMath::Max(1, InAisle);
        const float Scale = 0.7f + 0.5f * DemandOf(I) / FMath::Max(0.05f, AisleAverage);
        // Own aisle first, then the other faces from the nearest aisle rank.
        TArray<const FFaceSlot*> Tries;
        for (const FFaceSlot& S : Slots) if (S.Category == Key) Tries.Add(&S);
        TArray<const FFaceSlot*> Others;
        for (const FFaceSlot& S : Slots) if (S.Category != Key && IsNonFood(MarketGoods::Classify(S.Category)) == IsNonFood(MarketGoods::Classify(Key))) Others.Add(&S);
        Others.Sort([&Key](const FFaceSlot& A, const FFaceSlot& B) { return FMath::Abs(AisleRank(A.Category) - AisleRank(Key)) < FMath::Abs(AisleRank(B.Category) - AisleRank(Key)); });
        Tries.Append(Others);
        bool bPlaced = false;
        for (const FFaceSlot* Slot : Tries)
        {
            const FPlanogramEquipment Spec = MarketPlanogram::EquipmentFor(Planogram, Slot->FixtureId);
            for (const int32 Level : LevelPreference(Spec, Product))
            {
                FPlanogramPlacement Block;
                Block.ProductId = Product.Id;
                Block.FixtureId = Slot->FixtureId;
                Block.Face = Slot->Face;
                Block.Level = Level;
                Block.Facings = WantedFacings(Planogram, Product, Block, Scale);
                FString Message;
                // Left to right: aim at the left end, the block snaps to the first free spot.
                if (MarketPlanogramEdit::AddBlock(Planogram, Products, Block, -Spec.UsableWidthCm * 0.5f, 1.0e6f, Message)) { bPlaced = true; break; }
            }
            if (bPlaced) break;
        }
        if (bPlaced) ++Result.Placed;
        else Result.NoRoom.Add(Product.Id);
    }
    return Result;
}

int32 MarketLayout::CopyTemplate(const FMarketPlanogram& Template, FMarketPlanogram& Target, const TArray<FMarketProduct>& Products)
{
    int32 Copied = 0;
    for (const FPlanogramPlacement& Block : Template.Placements)
    {
        const FPlanogramFixture* From = Template.FindFixture(Block.FixtureId);
        const FPlanogramFixture* To = Target.FindFixture(Block.FixtureId);
        if (!From || !To || From->EquipmentId != To->EquipmentId) continue;
        Target.Placements.Add(Block);
        if (MarketPlanogram::BlockFits(Target, Products, Target.Placements.Num() - 1)) ++Copied;
        else Target.Placements.Pop();
    }
    return Copied;
}

TArray<int32> MarketLayout::Capacities(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products)
{
    TArray<int32> Result;
    for (const FMarketProduct& P : Products) Result.Add(MarketPlanogram::ProductCapacity(Planogram, Products, P.Id));
    return Result;
}
