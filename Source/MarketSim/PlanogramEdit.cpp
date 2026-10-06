#include "PlanogramEdit.h"
#include "ProductCatalog.h"

namespace
{
    FString NameOf(const TArray<FMarketProduct>& Products, const FString& Id)
    {
        const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Id);
        return Product && !Product->RealName.IsEmpty() ? Product->RealName : Id;
    }

    bool ValidIndex(const FMarketPlanogram& Planogram, int32 Index, FString& OutMessage)
    {
        if (Planogram.Placements.IsValidIndex(Index)) return true;
        OutMessage = TEXT("\u00d6nce raftaki bir \u00fcr\u00fcn blo\u011funu se\u00e7.");
        return false;
    }

    // Replaces block Index with Block after checking it; the plan stays unchanged when it does not fit.
    bool Replace(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, const FPlanogramPlacement& Block, FString& OutMessage)
    {
        FMarketPlanogram Work = Planogram;
        Work.Placements[Index] = Block;
        FString Reason;
        if (!MarketPlanogram::BlockFits(Work, Products, Index, &Reason)) { OutMessage = Reason; return false; }
        Planogram = MoveTemp(Work);
        return true;
    }
}

int32 MarketPlanogramEdit::NextOrientation(const FMarketProduct& Product, int32 Orientation)
{
    const int32 Next = (Orientation + 1) % 3;
    return Next == 2 && !MarketPlanogram::CanLayOnSide(Product) ? 0 : Next;
}

int32 MarketPlanogramEdit::MaxStack(const FMarketPlanogram& Planogram, const FMarketProduct& Product, const FPlanogramPlacement& Block)
{
    const FPlanogramEquipment Spec = MarketPlanogram::EquipmentFor(Planogram, Block.FixtureId);
    const int32 Level = FMath::Clamp(Block.Level, 0, FMath::Max(0, Spec.Levels - 1));
    return MarketPlanogram::MaxStackFor(Product, Block.Orientation, Spec.LevelClearanceCm[Level]);
}

const TCHAR* MarketPlanogramEdit::OrientationName(int32 Orientation)
{
    return Orientation == 1 ? TEXT("yana d\u00f6n\u00fck") : (Orientation == 2 ? TEXT("yan yat\u0131k") : TEXT("dik, \u00f6nden"));
}

FString MarketPlanogramEdit::CapacityText(const FPlanogramPlacement& Block)
{
    return FString::Printf(TEXT("\u00f6nde %d \u00d7 derinlik %d \u00d7 %d kat = %d adet"), Block.Facings, Block.Depth, Block.Stack, MarketPlanogram::Capacity(Block));
}

MarketPlanogramEdit::FBlockPlan MarketPlanogramEdit::PlanBlock(const FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products,
    const FPlanogramPlacement& Wish, float DesiredX, int32 IgnoreIndex, float MaxShiftCm, bool bAllowFewerFacings)
{
    FBlockPlan Plan;
    Plan.Block = Wish;
    const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Wish.ProductId);
    if (!Product) { Plan.Reason = TEXT("\u00dcr\u00fcn katalogda yok."); return Plan; }
    const FPlanogramFixture* Fixture = Planogram.FindFixture(Wish.FixtureId);
    if (!Fixture) { Plan.Reason = TEXT("Bir rafa ni\u015fan al."); return Plan; }
    const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture->EquipmentId);
    if (Wish.Level < 0 || Wish.Level >= Spec.Levels) { Plan.Reason = FString::Printf(TEXT("Bu reyonda %d seviye var."), Spec.Levels); return Plan; }
    if (Wish.Face == TEXT("back") && !Spec.bDoubleSided) { Plan.Reason = TEXT("Bu reyonun arka y\u00fcz\u00fc yok."); return Plan; }

    FPlanogramPlacement& B = Plan.Block;
    if (B.Orientation == 2 && !MarketPlanogram::CanLayOnSide(*Product)) B.Orientation = 0;
    B.Facings = FMath::Clamp(B.Facings, 1, MarketPlanogram::MaxFacings);
    const int32 StackLimit = MaxStack(Planogram, *Product, B);
    if (B.Stack > StackLimit) { Plan.Note = FString::Printf(TEXT("bu rafa en fazla %d kat s\u0131\u011far"), StackLimit); B.Stack = StackLimit; }
    B.Stack = FMath::Max(1, B.Stack);
    B.Depth = MarketPlanogram::PhysicalDepth(Planogram, *Product, B);
    B.bHasX = true;
    B.OffsetCm = 0.f;

    const int32 Wanted = B.Facings;
    for (int32 Facings = Wanted; Facings >= (bAllowFewerFacings ? 1 : Wanted); --Facings)
    {
        B.Facings = Facings;
        const float Width = MarketPlanogram::BlockWidthCm(*Product, B);
        float X = 0.f;
        if (!MarketPlanogram::FindFreeX(Planogram, Products, B.FixtureId, B.Face, B.Level, Width, DesiredX, IgnoreIndex, X, B.GapCm)) continue;
        if (FMath::Abs(X - DesiredX) > MaxShiftCm + KINDA_SMALL_NUMBER) continue;
        B.XCm = X;
        Plan.bOk = true;
        if (Facings < Wanted)
        {
            const FString Fewer = FString::Printf(TEXT("yer olmad\u0131\u011f\u0131 i\u00e7in \u00f6nde %d yerine %d"), Wanted, Facings);
            Plan.Note = Plan.Note.IsEmpty() ? Fewer : Plan.Note + TEXT(", ") + Fewer;
        }
        return Plan;
    }
    B.Facings = Wanted;
    B.XCm = DesiredX;
    const float Gap = MarketPlanogram::WidestGapCm(Planogram, Products, B.FixtureId, B.Face, B.Level, IgnoreIndex);
    FPlanogramPlacement One = B;
    One.Facings = 1;
    const float OneWidth = MarketPlanogram::BlockWidthCm(*Product, One);
    const float Need = OneWidth + 2.f * (MarketPlanogram::ProductGapCm + B.GapCm);
    Plan.Reason = Gap + KINDA_SMALL_NUMBER < OneWidth
        ? FString::Printf(TEXT("Bu rafta yer yok: 1 adet i\u00e7in %.0f cm gerekir, en geni\u015f bo\u015fluk %.0f cm."), Need, FMath::Max(0.f, Gap))
        : TEXT("Burada yer yok; ni\u015fan\u0131 bir bo\u015flu\u011fa kayd\u0131r.");
    return Plan;
}

bool MarketPlanogramEdit::AddBlock(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FPlanogramPlacement& Block,
    float DesiredX, float MaxShiftCm, FString& OutMessage, int32* OutIndex)
{
    const FBlockPlan Plan = PlanBlock(Planogram, Products, Block, DesiredX, INDEX_NONE, MaxShiftCm);
    if (!Plan.bOk) { OutMessage = Plan.Reason; return false; }
    const int32 Index = Planogram.Placements.Add(Plan.Block);
    if (OutIndex) *OutIndex = Index;
    OutMessage = FString::Printf(TEXT("%s rafa kondu: %s."), *NameOf(Products, Block.ProductId), *CapacityText(Plan.Block));
    if (!Plan.Note.IsEmpty()) OutMessage += TEXT(" (") + Plan.Note + TEXT(")");
    return true;
}

bool MarketPlanogramEdit::AddToRowEnd(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, const FString& ProductId,
    const FString& FixtureId, const FString& Face, int32 Level, FString& OutMessage, int32* OutIndex)
{
    FPlanogramPlacement Block;
    Block.ProductId = ProductId; Block.FixtureId = FixtureId; Block.Face = Face; Block.Level = Level; Block.Facings = 2;
    // Just right of the rightmost block (or the left edge of an empty row), wherever the nearest gap is.
    float DesiredX = -MarketPlanogram::EquipmentFor(Planogram, FixtureId).UsableWidthCm * .5f;
    const TArray<int32> Row = MarketPlanogram::RowBlocks(Planogram, Products, FixtureId, Face, Level);
    if (Row.Num() > 0)
    {
        const FPlanogramPlacement& Last = Planogram.Placements[Row.Last()];
        const FMarketProduct* LastProduct = MarketCatalog::FindProduct(Products, Last.ProductId);
        DesiredX = MarketPlanogram::PlacementCenterX(Planogram, Products, Last) + (LastProduct ? MarketPlanogram::BlockWidthCm(*LastProduct, Last) : 0.f);
    }
    return AddBlock(Planogram, Products, Block, DesiredX, 1.0e6f, OutMessage, OutIndex);
}

bool MarketPlanogramEdit::MoveBlock(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index,
    const FString& FixtureId, const FString& Face, int32 Level, float DesiredX, float MaxShiftCm, FString& OutMessage)
{
    if (!ValidIndex(Planogram, Index, OutMessage)) return false;
    FPlanogramPlacement Wish = Planogram.Placements[Index];
    Wish.FixtureId = FixtureId; Wish.Face = Face; Wish.Level = Level;
    const FBlockPlan Plan = PlanBlock(Planogram, Products, Wish, DesiredX, Index, MaxShiftCm);
    if (!Plan.bOk) { OutMessage = Plan.Reason; return false; }
    Planogram.Placements[Index] = Plan.Block;
    OutMessage = FString::Printf(TEXT("%s ta\u015f\u0131nd\u0131: %s."), *NameOf(Products, Wish.ProductId), *CapacityText(Plan.Block));
    if (!Plan.Note.IsEmpty()) OutMessage += TEXT(" (") + Plan.Note + TEXT(")");
    return true;
}

bool MarketPlanogramEdit::RemoveBlock(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, FString& OutMessage)
{
    if (!ValidIndex(Planogram, Index, OutMessage)) return false;
    const FString ProductId = Planogram.Placements[Index].ProductId;
    Planogram.Placements.RemoveAt(Index);
    const bool bStillOnShelf = Planogram.FindPlacement(ProductId) != nullptr;
    OutMessage = NameOf(Products, ProductId) + (bStillOnShelf
        ? TEXT(" blo\u011fu kald\u0131r\u0131ld\u0131 (\u00fcr\u00fcn ba\u015fka yerlerde rafta).")
        : TEXT(" raftan kald\u0131r\u0131ld\u0131; tekrar rafa koyana kadar sat\u0131lmaz."));
    return true;
}

bool MarketPlanogramEdit::ChangeFacings(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, int32 Delta, FString& OutMessage)
{
    if (!ValidIndex(Planogram, Index, OutMessage)) return false;
    FPlanogramPlacement Block = Planogram.Placements[Index];
    const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Block.ProductId);
    if (!Product) { OutMessage = TEXT("\u00dcr\u00fcn katalogda yok."); return false; }
    const int32 Target = FMath::Clamp(Block.Facings + Delta, 1, MarketPlanogram::MaxFacings);
    if (Target == Block.Facings) { OutMessage = Delta < 0 ? TEXT("\u00d6nde en az 1 \u00fcr\u00fcn olur.") : TEXT("\u00d6nde adet s\u0131n\u0131rda."); return false; }
    const float Center = MarketPlanogram::PlacementCenterX(Planogram, Products, Block);
    Block.Facings = Target;
    const float Width = MarketPlanogram::BlockWidthCm(*Product, Block);
    float X = 0.f;
    // Grow around the centre; when one side touches a neighbour or the edge, slide up to one facing width.
    if (!MarketPlanogram::FindFreeX(Planogram, Products, Block.FixtureId, Block.Face, Block.Level, Width, Center, Index, X, Block.GapCm) ||
        FMath::Abs(X - Center) > MarketPlanogram::OrientedWidthCm(*Product, Block.Orientation) + MarketPlanogram::ItemGapCm)
    {
        OutMessage = TEXT("Yan\u0131nda yer yok: \u00f6nce kom\u015fu blo\u011fu kayd\u0131r veya k\u00fc\u00e7\u00fclt.");
        return false;
    }
    Block.XCm = X; Block.bHasX = true;
    if (!Replace(Planogram, Products, Index, Block, OutMessage)) return false;
    OutMessage = FString::Printf(TEXT("%s: %s."), *NameOf(Products, Block.ProductId), *CapacityText(Block));
    return true;
}

bool MarketPlanogramEdit::Nudge(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, float DeltaCm, FString& OutMessage)
{
    if (!ValidIndex(Planogram, Index, OutMessage)) return false;
    FPlanogramPlacement Block = Planogram.Placements[Index];
    const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Block.ProductId);
    if (!Product) { OutMessage = TEXT("\u00dcr\u00fcn katalogda yok."); return false; }
    const float Center = MarketPlanogram::PlacementCenterX(Planogram, Products, Block);
    const float Width = MarketPlanogram::BlockWidthCm(*Product, Block);
    float X = 0.f;
    // Nearest free spot to the target; accepted only when it lies in the pushed direction (stops at contact).
    if (!MarketPlanogram::FindFreeX(Planogram, Products, Block.FixtureId, Block.Face, Block.Level, Width, Center + DeltaCm, Index, X, Block.GapCm) ||
        (X - Center) * DeltaCm <= KINDA_SMALL_NUMBER || FMath::Abs(X - Center) > FMath::Abs(DeltaCm) + KINDA_SMALL_NUMBER)
    {
        OutMessage = DeltaCm < 0 ? TEXT("Sola daha fazla kaymaz (kenar veya kom\u015fu blok).") : TEXT("Sa\u011fa daha fazla kaymaz (kenar veya kom\u015fu blok).");
        return false;
    }
    Block.XCm = X; Block.bHasX = true;
    if (!Replace(Planogram, Products, Index, Block, OutMessage)) return false;
    OutMessage = FString::Printf(TEXT("%s kayd\u0131r\u0131ld\u0131 (%+.0f cm)."), *NameOf(Products, Block.ProductId), X - Center);
    return true;
}

bool MarketPlanogramEdit::CycleOrientation(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, FString& OutMessage)
{
    if (!ValidIndex(Planogram, Index, OutMessage)) return false;
    FPlanogramPlacement Block = Planogram.Placements[Index];
    const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Block.ProductId);
    if (!Product) { OutMessage = TEXT("\u00dcr\u00fcn katalogda yok."); return false; }
    const float Center = MarketPlanogram::PlacementCenterX(Planogram, Products, Block);
    Block.Orientation = NextOrientation(*Product, Block.Orientation);
    Block.Stack = FMath::Clamp(Block.Stack, 1, MaxStack(Planogram, *Product, Block));
    Block.Depth = MarketPlanogram::PhysicalDepth(Planogram, *Product, Block);
    const float Width = MarketPlanogram::BlockWidthCm(*Product, Block);
    float X = 0.f;
    if (!MarketPlanogram::FindFreeX(Planogram, Products, Block.FixtureId, Block.Face, Block.Level, Width, Center, Index, X, Block.GapCm) ||
        FMath::Abs(X - Center) > Width * .5f + 5.f)
    {
        OutMessage = TEXT("Bu y\u00f6nde yan\u0131nda yer yok.");
        return false;
    }
    Block.XCm = X; Block.bHasX = true;
    if (!Replace(Planogram, Products, Index, Block, OutMessage)) return false;
    OutMessage = FString::Printf(TEXT("%s: %s, %s."), *NameOf(Products, Block.ProductId), OrientationName(Block.Orientation), *CapacityText(Block));
    return true;
}

bool MarketPlanogramEdit::ChangeStack(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, int32 Delta, bool bWrap, FString& OutMessage)
{
    if (!ValidIndex(Planogram, Index, OutMessage)) return false;
    FPlanogramPlacement Block = Planogram.Placements[Index];
    const FMarketProduct* Product = MarketCatalog::FindProduct(Products, Block.ProductId);
    if (!Product) { OutMessage = TEXT("\u00dcr\u00fcn katalogda yok."); return false; }
    const int32 Limit = MaxStack(Planogram, *Product, Block);
    int32 Target = Block.Stack + Delta;
    if (bWrap && Target > Limit) Target = 1;
    Target = FMath::Clamp(Target, 1, Limit);
    if (Target == Block.Stack)
    {
        OutMessage = Limit == 1 ? TEXT("Bu ambalaj \u00fcst \u00fcste dizilmez veya raf y\u00fcksekli\u011fi yetmiyor.")
                                : FString::Printf(TEXT("Bu rafta en fazla %d kat olur."), Limit);
        return false;
    }
    Block.Stack = Target;
    if (!Replace(Planogram, Products, Index, Block, OutMessage)) return false;
    OutMessage = FString::Printf(TEXT("%s: %s."), *NameOf(Products, Block.ProductId), *CapacityText(Block));
    return true;
}

bool MarketPlanogramEdit::ToggleFace(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, FString& OutMessage)
{
    if (!ValidIndex(Planogram, Index, OutMessage)) return false;
    const FPlanogramPlacement& Block = Planogram.Placements[Index];
    const FString Face = Block.Face == TEXT("back") ? TEXT("front") : TEXT("back");
    const FString FixtureId = Block.FixtureId;
    const int32 Level = Block.Level;
    const float X = MarketPlanogram::PlacementCenterX(Planogram, Products, Block);
    return MoveBlock(Planogram, Products, Index, FixtureId, Face, Level, X, 1.0e6f, OutMessage);
}

bool MarketPlanogramEdit::ChangeGap(FMarketPlanogram& Planogram, const TArray<FMarketProduct>& Products, int32 Index, float DeltaCm, FString& OutMessage)
{
    if (!ValidIndex(Planogram, Index, OutMessage)) return false;
    FPlanogramPlacement Block = Planogram.Placements[Index];
    const float Target = FMath::Clamp(FMath::GridSnap(Block.GapCm + DeltaCm, 0.5f), 0.f, MarketPlanogram::MaxBlockGapCm);
    if (FMath::IsNearlyEqual(Target, Block.GapCm)) { OutMessage = DeltaCm < 0 ? TEXT("Aral\u0131k zaten 0: yan\u0131ndakilerle dip dibe.") : TEXT("Aral\u0131k s\u0131n\u0131rda."); return false; }
    Block.GapCm = Target;
    if (!Replace(Planogram, Products, Index, Block, OutMessage)) { OutMessage = TEXT("Yan\u0131nda bu kadar aral\u0131k i\u00e7in yer yok."); return false; }
    OutMessage = FString::Printf(TEXT("%s: yan\u0131ndaki bloklarla aras\u0131 en az %.1f cm."), *NameOf(Products, Block.ProductId), Target + MarketPlanogram::ProductGapCm);
    return true;
}
