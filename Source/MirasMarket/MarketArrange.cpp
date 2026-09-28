// In-game shelf arranging (R while the shop is closed).
// Aim the crosshair at any shelf level: a ghost of the product in hand shows exactly where it would go
// (green strip = it fits, red = why not). Left click / E puts it there; the same product can be placed
// as many times as wanted. Aiming at an existing block selects it: DEL/right click removes it, F picks it
// up to move, C copies it into the hand, +/-, Y, U, arrows change it. Wheel / TAB / Q change the product.
// Every change uses the same rules as the Raf Plani editor (PlanogramEdit.h), is saved to
// Config/planograms.json at once and rebuilds the shelf stock on screen.

#include "MarketGame.h"
#include "ProductCatalog.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    const FLinearColor GhostOk(0.25f, 0.95f, 0.35f);
    const FLinearColor GhostBad(0.95f, 0.22f, 0.15f);
    const FLinearColor HoverColor(1.f, 0.72f, 0.20f);
    const FLinearColor MovingColor(0.30f, 0.60f, 1.f);

    FString GapText(float GapCm)
    {
        return GapCm <= 0.f ? FString(TEXT("aral\u0131k 0 (dip dibe)")) : FString::Printf(TEXT("aral\u0131k %.0f cm"), GapCm);
    }

    // Snap distance while aiming: the block may slide into the nearest gap, but never jumps far away.
    float AimShift(float WidthCm) { return WidthCm * .5f + 15.f; }
}

int32 AMarketGameMode::NearbyFixture(FString* OutFace) const
{
    const APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Pawn) return INDEX_NONE;
    int32 Result = INDEX_NONE;
    float Best = 170.f; // cm from the fixture's centre line (gondola half depth ~45 cm + aisle)
    for (int32 I = 0; I < Planogram.Fixtures.Num(); ++I)
    {
        const FPlanogramFixture& Fixture = Planogram.Fixtures[I];
        const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture.EquipmentId);
        const FVector Local = FTransform(FRotator(0.f, Fixture.Yaw, 0.f), Fixture.Location).InverseTransformPosition(Pawn->GetActorLocation());
        if (FMath::Abs(Local.X) > Spec.UsableWidthCm * .5f + 40.f) continue;
        if (Local.Y > 0.f && !Spec.bDoubleSided) continue; // behind a wall shelf
        const float Distance = FMath::Abs(Local.Y);
        if (Distance >= Best) continue;
        Best = Distance;
        Result = I;
        if (OutFace) *OutFace = Local.Y > 0.f ? TEXT("back") : TEXT("front"); // customer side of the front face is -Y
    }
    return Result;
}

void AMarketGameMode::UpdateArrangeAim()
{
    bArrangeAim = false;
    ArrangeHover = INDEX_NONE;
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;
    FVector Eye;
    FRotator View;
    PC->GetPlayerViewPoint(Eye, View);
    const FVector Dir = View.Vector();
    float BestT = 450.f; // reach: 4.5 m
    for (int32 I = 0; I < Planogram.Fixtures.Num(); ++I)
    {
        const FPlanogramFixture& Fixture = Planogram.Fixtures[I];
        const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture.EquipmentId);
        const FTransform Xf(FRotator(0.f, Fixture.Yaw, 0.f), Fixture.Location);
        const FVector O = Xf.InverseTransformPosition(Eye);
        const FVector D = Xf.InverseTransformVectorNoScale(Dir);
        for (int32 Side = 0; Side < (Spec.bDoubleSided ? 2 : 1); ++Side)
        {
            // Front face products stand behind the plane y = FrontY (negative), seen from y < FrontY.
            const float PlaneY = Side == 0 ? Spec.FrontY : -Spec.FrontY;
            const bool bOnThisSide = Side == 0 ? O.Y < PlaneY : O.Y > PlaneY;
            if (!bOnThisSide || FMath::Abs(D.Y) < KINDA_SMALL_NUMBER) continue;
            const float T = (PlaneY - O.Y) / D.Y;
            if (T <= 0.f || T >= BestT) continue;
            const FVector Hit = O + D * T;
            const float Half = Spec.UsableWidthCm * .5f;
            const int32 Last = FMath::Clamp(Spec.Levels - 1, 0, MarketPlanogram::MaxLevels - 1);
            if (FMath::Abs(Hit.X) > Half + 20.f) continue;
            if (Hit.Z < Spec.LevelTopZ[0] - 12.f || Hit.Z > Spec.LevelTopZ[Last] + Spec.LevelClearanceCm[Last] + 8.f) continue;
            int32 Level = 0;
            for (int32 L = 0; L <= Last; ++L) if (Hit.Z >= Spec.LevelTopZ[L] - 4.f) Level = L;
            BestT = T;
            bArrangeAim = true;
            AimFixture = I;
            AimFace = Side == 0 ? TEXT("front") : TEXT("back");
            AimLevel = Level;
            AimX = FMath::Clamp(Hit.X, -Half, Half);
        }
    }
    if (!bArrangeAim) { ArrangePlan = MarketPlanogramEdit::FBlockPlan(); ArrangePlan.Reason = TEXT("Bir raf seviyesine ni\u015fan al."); return; }
    const FString FixtureId = Planogram.Fixtures[AimFixture].Id;
    if (ArrangeMoving == INDEX_NONE)
        ArrangeHover = MarketPlanogram::BlockAt(Planogram, Products, FixtureId, AimFace, AimLevel, AimX);
    FPlanogramPlacement Wish = Planogram.Placements.IsValidIndex(ArrangeMoving) ? Planogram.Placements[ArrangeMoving] : ArrangeHand;
    Wish.FixtureId = FixtureId;
    Wish.Face = AimFace;
    Wish.Level = AimLevel;
    const int32 ProductIndex = MarketCatalog::IndexOfProduct(Products, Wish.ProductId);
    const float Width = Products.IsValidIndex(ProductIndex) ? MarketPlanogram::BlockWidthCm(Products[ProductIndex], Wish) : 10.f;
    ArrangePlan = MarketPlanogramEdit::PlanBlock(Planogram, Products, Wish, AimX, ArrangeMoving, AimShift(Width));
}

bool AMarketGameMode::ArrangeCommand(FName Action)
{
    const bool bArrangeKey = Action.ToString().StartsWith(TEXT("Arrange"));
    if (Action == "Arrange")
    {
        if (bArrange) { ExitArrange(TEXT("Raf d\u00fczeni kaydedildi; m\u00fc\u015fteriler raflar\u0131 b\u00f6yle g\u00f6recek.")); return true; }
        if (bOpen) { Notify(TEXT("Raflar\u0131 dizmek i\u00e7in \u00f6nce O ile marketi kapat.")); return true; }
        if (Products.Num() == 0) return true;
        bArrange = true;
        RefreshShelfItems(); // arrange view: every block drawn full
        ArrangeMoving = INDEX_NONE;
        Selected = FMath::Clamp(Selected, 0, Products.Num() - 1);
        ArrangeHand = FPlanogramPlacement();
        ArrangeHand.ProductId = Products[Selected].Id;
        ArrangeHand.Facings = 2;
        TickArrange();
        Notify(TEXT("RAF D\u00dcZEN\u0130: bir rafa ni\u015fan al; ye\u015fil \u00f6nizleme uygunsa sol t\u0131k veya E ile koy. Sa\u011fdaki panel her \u015feyi g\u00f6sterir."));
        return true;
    }
    if (!bArrange) return bArrangeKey; // arrange-only keys do nothing outside the mode
    if (Action == "ToggleShop") { Notify(TEXT("\u00d6nce R ile raf d\u00fczenini bitir.")); return true; }

    UpdateArrangeAim(); // act on what is under the crosshair right now
    const int32 Target = ArrangeHover; // an existing block under the crosshair (never while moving)
    const FString TargetProduct = Planogram.Placements.IsValidIndex(Target) ? Planogram.Placements[Target].ProductId : FString();
    const FString AimFixtureId = bArrangeAim ? Planogram.Fixtures[AimFixture].Id : FString();
    FString Text;
    bool bChanged = false;
    FString Changed; // product whose shelf changed

    if (Action == "ArrangePlace" || Action == "Interact")
    {
        if (!bArrangeAim) Text = TEXT("\u00d6nce bir raf seviyesine ni\u015fan al.");
        else if (Planogram.Placements.IsValidIndex(ArrangeMoving))
        {
            Changed = Planogram.Placements[ArrangeMoving].ProductId;
            const int32 ProductIndex = MarketCatalog::IndexOfProduct(Products, Changed);
            const float Width = Products.IsValidIndex(ProductIndex) ? MarketPlanogram::BlockWidthCm(Products[ProductIndex], Planogram.Placements[ArrangeMoving]) : 10.f;
            bChanged = MarketPlanogramEdit::MoveBlock(Planogram, Products, ArrangeMoving, AimFixtureId, AimFace, AimLevel, AimX, AimShift(Width), Text);
            if (bChanged) ArrangeMoving = INDEX_NONE;
        }
        else
        {
            FPlanogramPlacement Wish = ArrangeHand;
            Wish.FixtureId = AimFixtureId; Wish.Face = AimFace; Wish.Level = AimLevel;
            const int32 ProductIndex = MarketCatalog::IndexOfProduct(Products, Wish.ProductId);
            const float Width = Products.IsValidIndex(ProductIndex) ? MarketPlanogram::BlockWidthCm(Products[ProductIndex], Wish) : 10.f;
            Changed = Wish.ProductId;
            bChanged = MarketPlanogramEdit::AddBlock(Planogram, Products, Wish, AimX, AimShift(Width), Text);
        }
    }
    else if (Action == "ArrangeNext" || Action == "NextProduct" || Action == "ArrangePrev" || Action == "PrevProduct")
    {
        if (ArrangeMoving != INDEX_NONE) Text = TEXT("Ta\u015f\u0131rken \u00fcr\u00fcn de\u011fi\u015fmez: \u00f6nce t\u0131klay\u0131p b\u0131rak veya F ile iptal et.");
        else
        {
            const bool bNext = Action == "ArrangeNext" || Action == "NextProduct";
            Selected = (Selected + (bNext ? 1 : Products.Num() - 1)) % Products.Num();
            ArrangeHand.ProductId = Products[Selected].Id;
            ArrangeHand.Orientation = 0;
            ArrangeHand.Stack = 1;
            Text = TEXT("Elindeki \u00fcr\u00fcn: ") + ProductName(Selected);
        }
    }
    else if (Action == "ArrangeGrab")
    {
        if (ArrangeMoving != INDEX_NONE) { ArrangeMoving = INDEX_NONE; Text = TEXT("Ta\u015f\u0131ma iptal edildi; blok yerinde kald\u0131."); }
        else if (Target == INDEX_NONE) Text = TEXT("Ta\u015f\u0131mak i\u00e7in bir \u00fcr\u00fcn blo\u011funa ni\u015fan al, sonra F.");
        else
        {
            ArrangeMoving = Target;
            const int32 ProductIndex = MarketCatalog::IndexOfProduct(Products, TargetProduct);
            Text = FString::Printf(TEXT("%s al\u0131nd\u0131: yeni yere ni\u015fan al\u0131p sol t\u0131k / E ile b\u0131rak. F iptal."),
                Products.IsValidIndex(ProductIndex) ? *ProductName(ProductIndex) : *TargetProduct);
        }
    }
    else if (Action == "ArrangePick")
    {
        if (Target == INDEX_NONE) Text = TEXT("Kopyalamak i\u00e7in bir \u00fcr\u00fcn blo\u011funa ni\u015fan al, sonra C.");
        else
        {
            const FPlanogramPlacement& Block = Planogram.Placements[Target];
            ArrangeHand.ProductId = Block.ProductId;
            ArrangeHand.Facings = Block.Facings;
            ArrangeHand.Orientation = Block.Orientation;
            ArrangeHand.Stack = Block.Stack;
            ArrangeHand.GapCm = Block.GapCm;
            const int32 ProductIndex = MarketCatalog::IndexOfProduct(Products, Block.ProductId);
            if (ProductIndex != INDEX_NONE) Selected = ProductIndex;
            Text = FString::Printf(TEXT("Eline al\u0131nd\u0131: %s, \u00f6nde %d, %s. Bo\u015f bir yere ni\u015fan al\u0131p koy."),
                ProductIndex != INDEX_NONE ? *ProductName(ProductIndex) : *Block.ProductId, Block.Facings, MarketPlanogramEdit::OrientationName(Block.Orientation));
        }
    }
    else if (Action == "ArrangeRemove")
    {
        if (Planogram.Placements.IsValidIndex(ArrangeMoving))
        {
            Changed = Planogram.Placements[ArrangeMoving].ProductId;
            bChanged = MarketPlanogramEdit::RemoveBlock(Planogram, Products, ArrangeMoving, Text);
            ArrangeMoving = INDEX_NONE;
        }
        else if (Target == INDEX_NONE) Text = TEXT("Kald\u0131rmak i\u00e7in bir \u00fcr\u00fcn blo\u011funa ni\u015fan al.");
        else { Changed = TargetProduct; bChanged = MarketPlanogramEdit::RemoveBlock(Planogram, Products, Target, Text); }
    }
    else if (Action == "PriceUp" || Action == "PriceDown" || Action == "ArrangeTurn" || Action == "ArrangeStack")
    {
        const bool bUp = Action == "PriceUp";
        if (Target != INDEX_NONE)
        {
            Changed = TargetProduct;
            if (Action == "ArrangeTurn") bChanged = MarketPlanogramEdit::CycleOrientation(Planogram, Products, Target, Text);
            else if (Action == "ArrangeStack") bChanged = MarketPlanogramEdit::ChangeStack(Planogram, Products, Target, 1, true, Text);
            else bChanged = MarketPlanogramEdit::ChangeFacings(Planogram, Products, Target, bUp ? 1 : -1, Text);
        }
        else if (ArrangeMoving != INDEX_NONE) Text = TEXT("Ta\u015f\u0131rken de\u011fi\u015ftirilemez: \u00f6nce b\u0131rak.");
        else
        {
            // Nothing under the crosshair: the setting of the product in hand changes (the ghost shows it).
            const int32 ProductIndex = MarketCatalog::IndexOfProduct(Products, ArrangeHand.ProductId);
            if (Action == "ArrangeTurn")
            {
                if (Products.IsValidIndex(ProductIndex))
                    ArrangeHand.Orientation = MarketPlanogramEdit::NextOrientation(Products[ProductIndex], ArrangeHand.Orientation);
            }
            else if (Action == "ArrangeStack")
            {
                if (!Products.IsValidIndex(ProductIndex)) return true;
                FPlanogramPlacement Probe = ArrangeHand;
                if (bArrangeAim) { Probe.FixtureId = AimFixtureId; Probe.Level = AimLevel; }
                const int32 Limit = bArrangeAim ? MarketPlanogramEdit::MaxStack(Planogram, Products[ProductIndex], Probe) : 8;
                ArrangeHand.Stack = ArrangeHand.Stack + 1 > Limit ? 1 : ArrangeHand.Stack + 1;
            }
            else
                ArrangeHand.Facings = FMath::Clamp(ArrangeHand.Facings + (bUp ? 1 : -1), 1, MarketPlanogram::MaxFacings);
            Text = FString::Printf(TEXT("Elindeki: \u00f6nde %d, %d kat, %s."), ArrangeHand.Facings, ArrangeHand.Stack, MarketPlanogramEdit::OrientationName(ArrangeHand.Orientation));
        }
    }
    else if (Action == "ArrangeGapDown" || Action == "ArrangeGapUp")
    {
        const float Delta = Action == "ArrangeGapUp" ? 1.f : -1.f;
        if (Target != INDEX_NONE) { Changed = TargetProduct; bChanged = MarketPlanogramEdit::ChangeGap(Planogram, Products, Target, Delta, Text); }
        else if (ArrangeMoving != INDEX_NONE) Text = TEXT("Ta\u015f\u0131rken de\u011fi\u015ftirilemez: \u00f6nce b\u0131rak.");
        else
        {
            ArrangeHand.GapCm = FMath::Clamp(ArrangeHand.GapCm + Delta, 0.f, MarketPlanogram::MaxBlockGapCm);
            Text = ArrangeHand.GapCm <= 0.f ? FString(TEXT("Elindeki: yan\u0131ndakilerle dip dibe konur (aral\u0131k 0)."))
                : FString::Printf(TEXT("Elindeki: yan\u0131ndakilerle aras\u0131 en az %.0f cm b\u0131rak\u0131l\u0131r."), ArrangeHand.GapCm);
        }
    }
    else if (Action == "ArrangeLeft" || Action == "ArrangeRight")
    {
        if (Target == INDEX_NONE) Text = TEXT("Kayd\u0131rmak i\u00e7in bir \u00fcr\u00fcn blo\u011funa ni\u015fan al (sol/sa\u011f ok 5 cm).");
        else { Changed = TargetProduct; bChanged = MarketPlanogramEdit::Nudge(Planogram, Products, Target, Action == "ArrangeLeft" ? -5.f : 5.f, Text); }
    }
    else if (Action == "ArrangeUp" || Action == "ArrangeDown")
    {
        if (Target == INDEX_NONE) Text = TEXT("Ba\u015fka rafa almak i\u00e7in bir \u00fcr\u00fcn blo\u011funa ni\u015fan al (yukar\u0131/a\u015fa\u011f\u0131 ok).");
        else
        {
            const FPlanogramPlacement Block = Planogram.Placements[Target];
            const int32 Levels = MarketPlanogram::EquipmentFor(Planogram, Block.FixtureId).Levels;
            const int32 NewLevel = Block.Level + (Action == "ArrangeUp" ? 1 : -1);
            if (NewLevel < 0 || NewLevel >= Levels) Text = Action == "ArrangeUp" ? TEXT("Bu en \u00fcst raf.") : TEXT("Bu en alt raf.");
            else
            {
                Changed = TargetProduct;
                bChanged = MarketPlanogramEdit::MoveBlock(Planogram, Products, Target, Block.FixtureId, Block.Face, NewLevel,
                    MarketPlanogram::PlacementCenterX(Planogram, Products, Block), 40.f, Text);
            }
        }
    }
    else
        return bArrangeKey; // F-keys, movement, save etc. keep their normal meaning

    ArrangeApplied(bChanged, Text, Changed);
    return true;
}

void AMarketGameMode::ArrangeApplied(bool bChanged, const FString& Text, const FString& ProductId)
{
    FString Result = Text;
    if (bChanged)
    {
        FString Error;
        if (!CommitPlan(Error)) Result += TEXT("  (Kaydedilemedi: ") + Error + TEXT(")");
        // Test mode: the changed product's shelf is stocked at once so the new layout is visible.
        if (bTestMode && !ProductId.IsEmpty())
        {
            const int32 Index = MarketCatalog::IndexOfProduct(Products, ProductId);
            if (Index != INDEX_NONE) { State.FillShelfFree(Index); RefreshLabels(); }
        }
    }
    if (!Result.IsEmpty()) Notify(Result);
    TickArrange();
}

void AMarketGameMode::TickArrange()
{
    if (!bArrange) return;
    if (bOpen) { ExitArrange(FString()); return; }
    if (!Planogram.Placements.IsValidIndex(ArrangeMoving)) ArrangeMoving = INDEX_NONE;
    UpdateArrangeAim();
    UpdateGhost();
    UpdateArrangeView();
}

void AMarketGameMode::ExitArrange(const FString& Text)
{
    bArrange = false;
    ArrangeMoving = INDEX_NONE;
    bArrangeAim = false;
    ArrangeHover = INDEX_NONE;
    ClearGhost();
    RefreshShelfItems(); // back to real stock
    if (!Text.IsEmpty()) Notify(Text);
}

FString AMarketGameMode::ArrangeHint() const
{
    return TEXT("R: Raf d\u00fczenini bitir   \u00b7   ayr\u0131nt\u0131lar sa\u011fdaki panelde");
}

void AMarketGameMode::ClearGhost()
{
    for (const TWeakObjectPtr<UStaticMeshComponent>& Item : GhostItems)
        if (UStaticMeshComponent* Component = Item.Get()) Component->DestroyComponent();
    GhostItems.Reset();
    GhostShapeKey.Reset();
    if (GhostHolder) { GhostHolder->Destroy(); GhostHolder = nullptr; }
    if (GhostStrip) { GhostStrip->Destroy(); GhostStrip = nullptr; }
    if (HoverStrip) { HoverStrip->Destroy(); HoverStrip = nullptr; }
    if (GhostLabel) { if (AActor* LabelActor = GhostLabel->GetOwner()) LabelActor->Destroy(); GhostLabel = nullptr; }
    for (const TObjectPtr<AActor>& Mark : RowMarks) if (Mark) Mark->Destroy();
    RowMarks.Reset();
    RowMarksKey.Reset();
    GhostStripKey.Reset();
    HoverStripKey.Reset();
}

void AMarketGameMode::UpdateGhost()
{
    // A coloured strip on the shelf lip: Key avoids respawning it every frame.
    auto Strip = [this](TObjectPtr<AActor>& Actor, FString& Key, const FPlanogramPlacement* Block, float CenterX, float Width, const FLinearColor& Color, float Lift)
    {
        const FPlanogramFixture* Fixture = Block ? Planogram.FindFixture(Block->FixtureId) : nullptr;
        if (!Fixture)
        {
            if (Actor) { Actor->Destroy(); Actor = nullptr; }
            Key.Reset();
            return;
        }
        const FString NewKey = FString::Printf(TEXT("%s|%s|%d|%.1f|%.1f|%s"), *Block->FixtureId, *Block->Face, Block->Level, CenterX, Width, *Color.ToString());
        if (Actor && NewKey == Key) return;
        if (Actor) Actor->Destroy();
        Key = NewKey;
        const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture->EquipmentId);
        const int32 Level = FMath::Clamp(Block->Level, 0, Spec.Levels - 1);
        const float FaceSign = Block->Face == TEXT("back") ? 1.f : -1.f;
        const FRotator Rotation(0.f, Fixture->Yaw, 0.f);
        const FVector Local(CenterX, FaceSign * (FMath::Abs(Spec.FrontY) + 1.5f + Lift), Spec.LevelTopZ[Level] + 1.f + Lift);
        Actor = Box(FTransform(Rotation, Fixture->Location).TransformPosition(Local), FVector(FMath::Max(4.f, Width), 2.f, 2.2f), Color, false);
        if (Actor) Actor->SetActorRotation(Rotation);
    };

    if (!bArrange) { ClearGhost(); return; }

    // Every block of the aimed row gets a grey strip, so blocks with no stock yet (reserved space) are visible.
    const FString MarksKey = bArrangeAim ? FString::Printf(TEXT("%d|%s|%d|%d"), AimFixture, *AimFace, AimLevel, ArrangeVersion) : FString();
    if (MarksKey != RowMarksKey)
    {
        for (const TObjectPtr<AActor>& Mark : RowMarks) if (Mark) Mark->Destroy();
        RowMarks.Reset();
        RowMarksKey = MarksKey;
        if (bArrangeAim)
            for (const int32 Index : MarketPlanogram::RowBlocks(Planogram, Products, Planogram.Fixtures[AimFixture].Id, AimFace, AimLevel))
            {
                const FPlanogramPlacement& Block = Planogram.Placements[Index];
                const int32 ProductIndex = MarketCatalog::IndexOfProduct(Products, Block.ProductId);
                if (!Products.IsValidIndex(ProductIndex)) continue;
                TObjectPtr<AActor> Mark = nullptr;
                FString Unused;
                Strip(Mark, Unused, &Block, MarketPlanogram::PlacementCenterX(Planogram, Products, Block),
                    MarketPlanogram::BlockWidthCm(Products[ProductIndex], Block), FLinearColor(0.55f, 0.62f, 0.72f), -0.4f);
                if (Mark) RowMarks.Add(Mark);
            }
    }

    // Selected (hovered) or picked-up block.
    const int32 Marked = Planogram.Placements.IsValidIndex(ArrangeMoving) ? ArrangeMoving : ArrangeHover;
    if (Planogram.Placements.IsValidIndex(Marked))
    {
        const FPlanogramPlacement& Block = Planogram.Placements[Marked];
        const int32 ProductIndex = MarketCatalog::IndexOfProduct(Products, Block.ProductId);
        const float Width = Products.IsValidIndex(ProductIndex) ? MarketPlanogram::BlockWidthCm(Products[ProductIndex], Block) : 10.f;
        Strip(HoverStrip, HoverStripKey, &Block, MarketPlanogram::PlacementCenterX(Planogram, Products, Block), Width,
            Marked == ArrangeMoving ? MovingColor : HoverColor, .6f);
    }
    else Strip(HoverStrip, HoverStripKey, nullptr, 0.f, 0.f, HoverColor, 0.f);

    // Ghost of the block a click would create / drop.
    const FPlanogramPlacement& Plan = ArrangePlan.Block;
    const int32 ProductIndex = MarketCatalog::IndexOfProduct(Products, Plan.ProductId);
    if (!bArrangeAim || !Products.IsValidIndex(ProductIndex))
    {
        Strip(GhostStrip, GhostStripKey, nullptr, 0.f, 0.f, GhostOk, 0.f);
        if (GhostHolder) GhostHolder->SetActorHiddenInGame(true);
        if (GhostLabel) GhostLabel->SetVisibility(false);
        return;
    }
    const FMarketProduct& Product = Products[ProductIndex];
    const float Width = MarketPlanogram::BlockWidthCm(Product, Plan);
    Strip(GhostStrip, GhostStripKey, &Plan, Plan.XCm, Width, ArrangePlan.bOk ? GhostOk : GhostBad, 0.f);

    // Product meshes: only the visible front row (with stacking), rebuilt when the block's shape changes.
    if (!GhostHolder)
    {
        GhostHolder = GetWorld()->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator);
        auto* Root = NewObject<USceneComponent>(GhostHolder);
        GhostHolder->SetRootComponent(Root);
        Root->RegisterComponent();
        GhostShapeKey.Reset();
    }
    if (GhostLookProduct != Plan.ProductId)
    {
        GhostLook = LoadProductLook(ProductIndex);
        GhostMesh = GhostLook.Mesh;
        GhostMaterials.Reset();
        for (UMaterialInterface* Material : GhostLook.Materials) GhostMaterials.Add(Material);
        GhostLookProduct = Plan.ProductId;
        GhostShapeKey.Reset();
    }
    TArray<FTransform> Transforms;
    if (ArrangePlan.bOk) BlockSlotTransforms(GhostLook, Product, Plan, Plan.XCm, 4242, 0, true, Transforms, nullptr, nullptr);
    const FString ShapeKey = FString::Printf(TEXT("%s|%d"), *Plan.ProductId, Transforms.Num());
    if (ShapeKey != GhostShapeKey)
    {
        for (const TWeakObjectPtr<UStaticMeshComponent>& Item : GhostItems)
            if (UStaticMeshComponent* Component = Item.Get()) Component->DestroyComponent();
        GhostItems.Reset();
        for (int32 K = 0; K < Transforms.Num(); ++K)
        {
            auto* Item = NewObject<UStaticMeshComponent>(GhostHolder);
            Item->SetupAttachment(GhostHolder->GetRootComponent());
            Item->SetMobility(EComponentMobility::Movable);
            Item->SetStaticMesh(GhostLook.Mesh);
            Item->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Item->SetCastShadow(false);
            for (int32 Slot = 0; Slot < GhostLook.Materials.Num(); ++Slot)
                if (GhostLook.Materials[Slot]) Item->SetMaterial(Slot, GhostLook.Materials[Slot]);
            Item->RegisterComponent();
            GhostItems.Add(Item);
        }
        GhostShapeKey = ShapeKey;
    }
    for (int32 K = 0; K < Transforms.Num() && K < GhostItems.Num(); ++K)
        if (UStaticMeshComponent* Item = GhostItems[K].Get()) Item->SetWorldTransform(Transforms[K]);
    GhostHolder->SetActorHiddenInGame(!ArrangePlan.bOk);

    // Floating label above the ghost: what goes there, or why not.
    const FPlanogramFixture* Fixture = Planogram.FindFixture(Plan.FixtureId);
    if (!Fixture) return;
    const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture->EquipmentId);
    const int32 Level = FMath::Clamp(Plan.Level, 0, Spec.Levels - 1);
    const bool bBack = Plan.Face == TEXT("back");
    const float FaceSign = bBack ? 1.f : -1.f;
    const float Height = MarketPlanogram::OrientedHeightCm(Product, Plan.Orientation) * FMath::Max(1, Plan.Stack);
    const FRotator LabelRotation(0.f, Fixture->Yaw + (bBack ? 90.f : -90.f), 0.f);
    const FVector LabelLocal(Plan.XCm, FaceSign * (FMath::Abs(Spec.FrontY) + 4.f), Spec.LevelTopZ[Level] + FMath::Min(Height, Spec.LevelClearanceCm[Level]) + 3.f);
    const FVector LabelWorld = FTransform(FRotator(0.f, Fixture->Yaw, 0.f), Fixture->Location).TransformPosition(LabelLocal);
    const FString LabelText = ArrangePlan.bOk
        ? FString::Printf(TEXT("%s  \u00b7  \u00f6nde %d  \u00b7  %d adet"), *ProductName(ProductIndex), Plan.Facings, MarketPlanogram::Capacity(Plan))
        : FString(TEXT("Buraya sigmiyor"));
    if (!GhostLabel) GhostLabel = Label(LabelWorld, LabelRotation, MarketCatalog::FoldTurkish(LabelText), 2.6f, FColor::White, true);
    GhostLabel->SetVisibility(true);
    GhostLabel->SetWorldLocationAndRotation(LabelWorld, LabelRotation);
    GhostLabel->SetText(FText::FromString(MarketCatalog::FoldTurkish(LabelText)));
    GhostLabel->SetTextRenderColor(ArrangePlan.bOk ? FColor(150, 255, 160) : FColor(255, 140, 120));
}

void AMarketGameMode::UpdateArrangeView()
{
    ArrangeKeys.Reset();
    auto AddKey = [this](const TCHAR* Key, const TCHAR* Label) { ArrangeKeys.Add(TPair<FString, FString>(FString(Key), FString(Label))); };
    const bool bMoving = Planogram.Placements.IsValidIndex(ArrangeMoving);
    const bool bHover = Planogram.Placements.IsValidIndex(ArrangeHover);

    // Where the crosshair is.
    if (bArrangeAim)
    {
        const FPlanogramFixture& Fixture = Planogram.Fixtures[AimFixture];
        const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture.EquipmentId);
        const float Used = MarketPlanogram::LevelUsedWidthCm(Planogram, Products, Fixture.Id, AimFace, AimLevel);
        const int32 Blocks = MarketPlanogram::RowBlocks(Planogram, Products, Fixture.Id, AimFace, AimLevel).Num();
        ArrangeTitle = FString::Printf(TEXT("%s  \u00b7  %s y\u00fcz  \u00b7  raf %d / %d"), *Fixture.Label,
            AimFace == TEXT("back") ? TEXT("arka") : TEXT("\u00f6n"), AimLevel + 1, Spec.Levels);
        ArrangeRow = FString::Printf(TEXT("Bu raf: %.0f / %.0f cm dolu  \u00b7  %d blok  \u00b7  en geni\u015f bo\u015fluk %.0f cm  \u00b7  y\u00fckseklik %.0f cm"),
            Used, Spec.UsableWidthCm, Blocks, MarketPlanogram::WidestGapCm(Planogram, Products, Fixture.Id, AimFace, AimLevel, ArrangeMoving),
            Spec.LevelClearanceCm[FMath::Clamp(AimLevel, 0, MarketPlanogram::MaxLevels - 1)]);
        ArrangeRowFill = Spec.UsableWidthCm > 0.f ? Used / Spec.UsableWidthCm : 0.f;
    }
    else
    {
        ArrangeTitle = TEXT("Bir rafa ni\u015fan al");
        ArrangeRow = TEXT("Ekran\u0131n ortas\u0131ndaki noktay\u0131 bir raf seviyesinin \u00fcst\u00fcne getir.");
        ArrangeRowFill = 0.f;
    }

    // Product in hand (or the block being moved).
    const FPlanogramPlacement& HandBlock = bMoving ? Planogram.Placements[ArrangeMoving] : ArrangeHand;
    const int32 HandIndex = MarketCatalog::IndexOfProduct(Products, HandBlock.ProductId);
    if (Products.IsValidIndex(HandIndex))
    {
        const FMarketProduct& P = Products[HandIndex];
        const FPlanogramPlacement& Shown = bArrangeAim ? ArrangePlan.Block : HandBlock; // plan = clamped to this shelf
        int32 BlockCount = 0;
        for (const FPlanogramPlacement& Block : Planogram.Placements) if (Block.ProductId == P.Id) ++BlockCount;
        const FMarketStock* Stock = State.Stock.IsValidIndex(HandIndex) ? &State.Stock[HandIndex] : nullptr;
        ArrangeHandTitle = (bMoving ? TEXT("TA\u015eINIYOR: ") : TEXT("EL\u0130NDEK\u0130: ")) + ProductName(HandIndex);
        ArrangeHandText = FString::Printf(TEXT("%s  \u00b7  %s  \u00b7  %.1f \u00d7 %.1f \u00d7 %.1f cm\n%s  \u00b7  %s  \u00b7  %s\nRafta %d blok  \u00b7  raf sto\u011fu %d / %d  \u00b7  depoda %d"),
            P.Brand.IsEmpty() ? TEXT("-") : *P.Brand, P.PackageType.IsEmpty() ? TEXT("ambalaj") : *P.PackageType,
            MarketPlanogram::NominalWidthCm(P), MarketPlanogram::NominalDepthCm(P), MarketPlanogram::NominalHeightCm(P),
            MarketPlanogramEdit::OrientationName(Shown.Orientation),
            bArrangeAim ? *MarketPlanogramEdit::CapacityText(Shown) : *FString::Printf(TEXT("\u00f6nde %d  \u00b7  %d kat"), Shown.Facings, Shown.Stack),
            *GapText(Shown.GapCm), BlockCount, Stock ? Stock->Shelf : 0, Stock ? Stock->Capacity : 0, Stock ? Stock->Warehouse : 0);
    }
    else { ArrangeHandTitle = TEXT("EL\u0130NDE \u00dcR\u00dcN YOK"); ArrangeHandText = TEXT("TAB / Q / tekerlek ile \u00fcr\u00fcn se\u00e7."); }

    // Block under the crosshair.
    if (bHover)
    {
        const FPlanogramPlacement& Block = Planogram.Placements[ArrangeHover];
        const int32 Index = MarketCatalog::IndexOfProduct(Products, Block.ProductId);
        const FMarketStock* Stock = State.Stock.IsValidIndex(Index) ? &State.Stock[Index] : nullptr;
        ArrangeTargetTitle = TEXT("N\u0130\u015eANDAK\u0130 BLOK: ") + (Index != INDEX_NONE ? ProductName(Index) : Block.ProductId);
        ArrangeTargetText = FString::Printf(TEXT("%s  \u00b7  %s\nKonum %+.0f cm  \u00b7  %s  \u00b7  \u00fcr\u00fcn\u00fcn t\u00fcm raf sto\u011fu %d / %d%s"),
            MarketPlanogramEdit::OrientationName(Block.Orientation), *MarketPlanogramEdit::CapacityText(Block),
            MarketPlanogram::PlacementCenterX(Planogram, Products, Block), *GapText(Block.GapCm), Stock ? Stock->Shelf : 0, Stock ? Stock->Capacity : 0,
            Stock && Stock->Shelf == 0 ? TEXT("\n(Rafta stok yok: d\u00fczen modunda dolu g\u00f6steriliyor, oyunda bo\u015f g\u00f6r\u00fcn\u00fcr. R ile \u00e7\u0131k\u0131p E ile raf\u0131 doldur.)") : TEXT(""));
    }
    else { ArrangeTargetTitle.Reset(); ArrangeTargetText.Reset(); }

    // What a click would do.
    bArrangeStatusOk = bArrangeAim && ArrangePlan.bOk;
    if (!bArrangeAim) ArrangeStatus = TEXT("Ni\u015fan rafta de\u011fil.");
    else if (ArrangePlan.bOk)
        ArrangeStatus = (bMoving ? TEXT("\u2713 T\u0131kla: buraya ta\u015f\u0131n\u0131r") : TEXT("\u2713 T\u0131kla: buraya konur")) +
            (ArrangePlan.Note.IsEmpty() ? FString() : TEXT(" (") + ArrangePlan.Note + TEXT(")"));
    else ArrangeStatus = TEXT("\u2717 ") + ArrangePlan.Reason;

    // Keys that do something right now.
    AddKey(TEXT("Sol t\u0131k / E"), bMoving ? TEXT("Ta\u015f\u0131nan blo\u011fu buraya b\u0131rak") : TEXT("Elindekini buraya koy (tekrar tekrar)"));
    if (bMoving)
    {
        AddKey(TEXT("F"), TEXT("Ta\u015f\u0131may\u0131 iptal et"));
        AddKey(TEXT("DEL"), TEXT("Ta\u015f\u0131nan blo\u011fu kald\u0131r"));
    }
    else
    {
        AddKey(TEXT("Tekerlek / TAB / Q"), TEXT("Elindeki \u00fcr\u00fcn\u00fc de\u011fi\u015ftir"));
        if (bHover)
        {
            AddKey(TEXT("Sa\u011f t\u0131k / DEL"), TEXT("Ni\u015fandaki blo\u011fu kald\u0131r"));
            AddKey(TEXT("F"), TEXT("Ni\u015fandaki blo\u011fu ta\u015f\u0131"));
            AddKey(TEXT("C"), TEXT("Ni\u015fandakini eline al (ayn\u0131s\u0131n\u0131 koymak i\u00e7in)"));
            AddKey(TEXT("+ / -"), TEXT("Ni\u015fandakinin \u00f6nde adedi"));
            AddKey(TEXT("Y / U"), TEXT("Ni\u015fandakinin y\u00f6n\u00fc / kat\u0131"));
            AddKey(TEXT("Z / X"), TEXT("Ni\u015fandakinin yan\u0131ndakilerle aral\u0131\u011f\u0131 \u2212 / +"));
            AddKey(TEXT("Sol / Sa\u011f ok"), TEXT("Ni\u015fandakini 5 cm kayd\u0131r"));
            AddKey(TEXT("Yukar\u0131 / A\u015fa\u011f\u0131 ok"), TEXT("Ni\u015fandakini \u00fcst / alt rafa al"));
        }
        else
        {
            AddKey(TEXT("+ / -"), TEXT("Elindekinin \u00f6nde adedi"));
            AddKey(TEXT("Y / U"), TEXT("Elindekinin y\u00f6n\u00fc / kat\u0131"));
            AddKey(TEXT("Z / X"), TEXT("Elindekinin yan\u0131ndakilerle aral\u0131\u011f\u0131 \u2212 / + (0 = dip dibe)"));
        }
    }
    AddKey(TEXT("R"), TEXT("Raf d\u00fczenini bitir (kay\u0131t otomatik)"));
}

void AMarketGameMode::RebuildShelfContents()
{
    for (const TWeakObjectPtr<AActor>& Actor : ShelfContentActors)
        if (AActor* Existing = Actor.Get()) Existing->Destroy();
    ShelfContentActors.Reset();
    ShelfInstances.Reset();
    ShelfSlots.Reset();
    ShelfSingles.Reset();
    ShelfApproach.Reset();
    ShelfLabels.Reset();
    ShelfLabelProduct.Reset();
    BuildShelfContents();
    // New capacities: a smaller or removed block sends its surplus back to the warehouse.
    ApplyCapacities();
    RefreshLabels();
}
