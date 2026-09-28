#include "StaffPlanner.h"
#include "PlanogramEdit.h"
#include "ProductCatalog.h"

namespace
{
    FString CategoryKey(const FString& Category)
    {
        return MarketCatalog::FoldTurkish(Category.TrimStartAndEnd()).ToLower();
    }

    // How good a planned block is: facings reached, next to the same brand, tidy (touching a neighbour or the
    // shelf edge instead of leaving a sliver), at eye level, front face first.
    float ScoreSpot(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FMarketProduct& Product,
        const FPlanogramPlacement& Block, int32 Wanted)
    {
        const FPlanogramEquipment Spec = MarketPlanogram::EquipmentFor(Planogram, Block.FixtureId);
        const float Width = MarketPlanogram::BlockWidthCm(Product, Block);
        const float Left = Block.XCm - Width * .5f;
        const float Right = Block.XCm + Width * .5f;
        const float Touch = 3.f;
        float Score = FMath::Min(Block.Facings, Wanted) * 10.f;
        bool bBrandBeside = false, bBrandInRow = false;
        bool bTidy = FMath::Abs(Left + Spec.UsableWidthCm * .5f) <= Touch || FMath::Abs(Spec.UsableWidthCm * .5f - Right) <= Touch;
        for (const int32 Index : MarketPlanogram::RowBlocks(Planogram, Products, Block.FixtureId, Block.Face, Block.Level))
        {
            const FPlanogramPlacement& Other = Planogram.Placements[Index];
            const FMarketProduct* OtherProduct = MarketCatalog::FindProduct(Products, Other.ProductId);
            if (!OtherProduct) continue;
            const float OtherCenter = MarketPlanogram::PlacementCenterX(Planogram, Products, Other);
            const float OtherHalf = MarketPlanogram::BlockWidthCm(*OtherProduct, Other) * .5f;
            const bool bBeside = FMath::Abs(Left - (OtherCenter + OtherHalf)) <= Touch || FMath::Abs((OtherCenter - OtherHalf) - Right) <= Touch;
            const bool bSameBrand = !Product.Brand.IsEmpty() && OtherProduct->Brand.Equals(Product.Brand, ESearchCase::IgnoreCase);
            bTidy |= bBeside;
            bBrandBeside |= bBeside && bSameBrand;
            bBrandInRow |= bSameBrand;
        }
        if (bBrandBeside) Score += 40.f;
        else if (bBrandInRow) Score += 15.f;
        if (bTidy) Score += 6.f;
        const int32 Level = FMath::Clamp(Block.Level, 0, Spec.Levels - 1);
        Score -= FMath::Abs(Spec.LevelTopZ[Level] - 95.f) / 8.f; // eye/hand level sells best
        if (Block.Face != TEXT("back")) Score += 1.f;
        return Score;
    }

    int32 WidenableBlock(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FString& ProductId)
    {
        for (int32 Index = 0; Index < Planogram.Placements.Num(); ++Index)
        {
            if (Planogram.Placements[Index].ProductId != ProductId) continue;
            FMarketPlanogram Trial = Planogram;
            FString Message;
            if (MarketPlanogramEdit::ChangeFacings(Trial, Products, Index, 1, Message)) return Index;
        }
        return INDEX_NONE;
    }
}

bool StaffPlanner::SameCategory(const FString& A, const FString& B)
{
    const FString KeyA = CategoryKey(A);
    return !KeyA.IsEmpty() && KeyA == CategoryKey(B);
}

int32 StaffPlanner::WantedFacings(const FMarketPlanogram& Planogram, const FMarketProduct& Product, const FPlanogramPlacement& Block)
{
    const int32 Depth = FMath::Max(1, MarketPlanogram::PhysicalDepth(Planogram, Product, Block));
    const int32 Target = FMath::Max(12, Product.CaseUnits);
    return FMath::Clamp(FMath::DivideAndRoundUp(Target, Depth), 2, 6);
}

bool StaffPlanner::PlanNewBlock(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 ProductIndex,
    FPlanogramPlacement& OutBlock, FString& OutReason)
{
    if (!Products.IsValidIndex(ProductIndex)) { OutReason = TEXT("\u00dcr\u00fcn katalogda yok."); return false; }
    const FMarketProduct& Product = Products[ProductIndex];
    const FString Name = Product.RealName.IsEmpty() ? Product.Id : Product.RealName;
    bool bHasShelf = false, bFound = false;
    float BestScore = -1.0e9f;
    for (const FPlanogramFixture& Fixture : Planogram.Fixtures)
    {
        if (!SameCategory(Fixture.Category, Product.Category)) continue;
        bHasShelf = true;
        const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture.EquipmentId);
        const float Half = Spec.UsableWidthCm * .5f;
        for (int32 FaceIndex = 0; FaceIndex < (Spec.bDoubleSided ? 2 : 1); ++FaceIndex)
        {
            const FString Face = FaceIndex == 0 ? TEXT("front") : TEXT("back");
            for (int32 Level = 0; Level < Spec.Levels; ++Level)
            {
                FPlanogramPlacement Wish;
                Wish.ProductId = Product.Id; Wish.FixtureId = Fixture.Id; Wish.Face = Face; Wish.Level = Level;
                const int32 Wanted = WantedFacings(Planogram, Product, Wish);
                Wish.Facings = Wanted;
                // Aim at both edges of every block (lines up next to it) and at points along the shelf.
                TArray<float> Aims;
                for (int32 Step = 0; Step <= 4; ++Step) Aims.Add(-Half + Spec.UsableWidthCm * Step / 4.f);
                for (const int32 Index : MarketPlanogram::RowBlocks(Planogram, Products, Fixture.Id, Face, Level))
                {
                    const FPlanogramPlacement& Other = Planogram.Placements[Index];
                    const FMarketProduct* OtherProduct = MarketCatalog::FindProduct(Products, Other.ProductId);
                    if (!OtherProduct) continue;
                    const float Center = MarketPlanogram::PlacementCenterX(Planogram, Products, Other);
                    const float OtherHalf = MarketPlanogram::BlockWidthCm(*OtherProduct, Other) * .5f;
                    Aims.Add(Center - OtherHalf - 1.f);
                    Aims.Add(Center + OtherHalf + 1.f);
                }
                for (const float Aim : Aims)
                {
                    const MarketPlanogramEdit::FBlockPlan Plan = MarketPlanogramEdit::PlanBlock(Planogram, Products, Wish, Aim);
                    if (!Plan.bOk) continue;
                    const float Score = ScoreSpot(Planogram, Products, Product, Plan.Block, Wanted);
                    if (Score > BestScore + 0.01f) { BestScore = Score; OutBlock = Plan.Block; bFound = true; }
                }
            }
        }
    }
    if (!bFound)
    {
        OutReason = bHasShelf
            ? FString::Printf(TEXT("%s i\u00e7in \"%s\" reyonlar\u0131nda yer kalmad\u0131."), *Name, *Product.Category)
            : FString::Printf(TEXT("%s i\u00e7in reyon yok (kategori \"%s\"). Raf Plan\u0131'nda bu kategoride bir reyon a\u00e7."), *Name, *Product.Category);
    }
    return bFound;
}

StaffPlanner::FJob StaffPlanner::ChooseJob(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FMarketState& State,
    const TSet<int32>& Busy, const TSet<int32>& Unplaceable, bool bAllowPlanEdits, TArray<TPair<int32, FString>>* OutNoRoom)
{
    FJob Job;
    const int32 Count = FMath::Min(Products.Num(), State.Stock.Num());
    const auto Free = [&Busy](int32 Index) { return !Busy.Contains(Index); };

    // 1. Emptiest shelf that is at half or less.
    float Emptiest = 2.f;
    for (int32 I = 0; I < Count; ++I)
    {
        const FMarketStock& S = State.Stock[I];
        if (!Free(I) || S.Capacity <= 0 || S.Warehouse <= 0 || S.Shelf * 2 > S.Capacity) continue;
        const float Ratio = static_cast<float>(S.Shelf) / S.Capacity;
        if (Ratio < Emptiest) { Emptiest = Ratio; Job.Kind = EJob::Refill; Job.Product = I; }
    }
    if (Job.Kind != EJob::None) return Job;

    // 2. Stock in the depot but no place on a shelf.
    if (bAllowPlanEdits)
    {
        for (int32 I = 0; I < Count; ++I)
        {
            const FMarketStock& S = State.Stock[I];
            if (!Free(I) || S.Capacity > 0 || S.Warehouse <= 0 || Unplaceable.Contains(I)) continue;
            FPlanogramPlacement Block;
            FString Reason;
            if (PlanNewBlock(Planogram, Products, I, Block, Reason))
            {
                Job.Kind = EJob::Place; Job.Product = I; Job.Block = Block;
                return Job;
            }
            if (OutNoRoom) OutNoRoom->Add(TPair<int32, FString>(I, Reason));
        }
    }

    // 3. Top up a shelf missing at least a fifth.
    int32 MostMissing = 0;
    for (int32 I = 0; I < Count; ++I)
    {
        const FMarketStock& S = State.Stock[I];
        const int32 Missing = S.Capacity - S.Shelf;
        if (!Free(I) || S.Capacity <= 0 || S.Warehouse <= 0 || Missing < FMath::Max(1, S.Capacity / 5)) continue;
        if (Missing > MostMissing) { MostMissing = Missing; Job.Kind = EJob::Refill; Job.Product = I; }
    }
    if (Job.Kind != EJob::None) return Job;

    // 4. A product whose shelf space cannot take one case while it has more stock than fits.
    if (bAllowPlanEdits)
    {
        for (int32 I = 0; I < Count; ++I)
        {
            const FMarketStock& S = State.Stock[I];
            if (!Free(I) || S.Capacity <= 0 || S.Capacity >= FMath::Max(12, Products[I].CaseUnits) || S.Shelf + S.Warehouse <= S.Capacity) continue;
            const int32 Index = WidenableBlock(Planogram, Products, Products[I].Id);
            if (Index == INDEX_NONE) continue;
            Job.Kind = EJob::Widen; Job.Product = I; Job.Block = Planogram.Placements[Index];
            return Job;
        }
    }
    return Job;
}

int32 StaffPlanner::FindBlock(const FMarketPlanogram& Planogram, const FPlanogramPlacement& Block)
{
    return Planogram.Placements.IndexOfByPredicate([&Block](const FPlanogramPlacement& P)
    {
        return P.ProductId == Block.ProductId && P.FixtureId == Block.FixtureId && P.Face == Block.Face && P.Level == Block.Level
            && FMath::Abs(P.XCm - Block.XCm) <= 2.f;
    });
}

bool StaffPlanner::TryPlace(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Block, FString& OutMessage)
{
    return MarketPlanogramEdit::AddBlock(Planogram, Products, Block, Block.XCm, 5.f, OutMessage);
}

bool StaffPlanner::TryWiden(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Block, FString& OutMessage)
{
    const int32 Index = FindBlock(Planogram, Block);
    if (Index == INDEX_NONE) { OutMessage = TEXT("Blok yerinde yok."); return false; }
    return MarketPlanogramEdit::ChangeFacings(Planogram, Products, Index, 1, OutMessage);
}

FString StaffPlanner::WorkerName(int32 Index)
{
    static const TCHAR* Names[] = { TEXT("Ahmet"), TEXT("Ay\u015fe"), TEXT("Mehmet"), TEXT("Fatma") };
    return Names[FMath::Abs(Index) % UE_ARRAY_COUNT(Names)];
}
