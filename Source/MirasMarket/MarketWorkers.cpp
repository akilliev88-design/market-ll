// Shelf staff (reyon gorevlisi) in the game: walking, carrying and doing what StaffPlanner decides.
// A worker: thinks (picks a job) -> walks to the depot -> loads a case -> walks to the block -> places or widens
// the block if that is the job -> puts units on the shelf one by one -> thinks again (or rests by the depot).
#include "MarketGame.h"
#include "ProductCatalog.h"
#include "StaffPlanner.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    constexpr float WorkerWalkSpeed = 150.f;  // cm/s, a little slower than a shopper in a hurry
    constexpr float WorkerLoadSeconds = 1.2f;
    constexpr float WorkerUnitSeconds = 0.3f; // one unit onto the shelf
    constexpr float WorkerThinkSeconds = 1.2f;
    constexpr float BoxPersonLift = 65.f;     // box people are centred, MetaHumans stand on their feet
}

FVector AMarketGameMode::DepotSpot() const { return FVector(-385.f, StoreBack() - 165.f, 0.f); }
FVector AMarketGameMode::WorkerRestSpot(int32 Index) const { return FVector(-320.f + Index * 60.f, StoreBack() - 215.f, 0.f); }

FVector AMarketGameMode::BlockApproachSpot(const FPlanogramPlacement& Placement, float CenterX) const
{
    // 70 cm in front of the block, on the floor (shoppers and workers stand here).
    const FPlanogramFixture* Fixture = Planogram.FindFixture(Placement.FixtureId);
    if (!Fixture) return FVector::ZeroVector;
    const FPlanogramEquipment Spec = MarketPlanogram::Equipment(Fixture->EquipmentId);
    const bool bBack = Placement.Face == TEXT("back");
    const float Front = bBack ? -Spec.FrontY : Spec.FrontY;
    const float FaceSign = bBack ? 1.f : -1.f;
    const FVector Spot = FTransform(FRotator(0, Fixture->Yaw, 0), Fixture->Location).TransformPosition(FVector(CenterX, Front + FaceSign * 70.f, 0));
    return FVector(Spot.X, Spot.Y, 0.f);
}

TArray<FVector> AMarketGameMode::AisleRoute(const FVector& From, const FVector& To) const
{
    // Same open lanes as the shoppers: x = +/-150 runs between the island and the gondolas; the two lanes meet
    // at the front corridor (y = 120) and in the depot aisle behind the last gondola.
    const float FrontY = 120.f;
    const float RearY = StoreBack() - 165.f;
    const float LaneFrom = From.X >= 0.f ? 150.f : -150.f;
    const float LaneTo = To.X >= 0.f ? 150.f : -150.f;
    TArray<FVector> Route;
    Route.Add(FVector(LaneFrom, From.Y, 0.f));
    if (LaneFrom != LaneTo)
    {
        const float ViaFront = FMath::Abs(From.Y - FrontY) + FMath::Abs(To.Y - FrontY);
        const float ViaRear = FMath::Abs(From.Y - RearY) + FMath::Abs(To.Y - RearY);
        const float CrossY = ViaFront <= ViaRear ? FrontY : RearY;
        Route.Add(FVector(LaneFrom, CrossY, 0.f));
        Route.Add(FVector(LaneTo, CrossY, 0.f));
    }
    Route.Add(FVector(LaneTo, To.Y, 0.f));
    Route.Add(FVector(To.X, To.Y, 0.f));
    return Route;
}

bool AMarketGameMode::CommitPlan(FString& OutError)
{
    ++ArrangeVersion;
    const bool bSaved = MarketPlanogram::SaveFile(MarketPlanogram::DefaultPath(), Planogram, OutError);
    RebuildShelfContents();
    return bSaved;
}

void AMarketGameMode::SyncWorkers()
{
    const int32 Wanted = FMath::Clamp(State.Stockers, 0, FMarketState::MaxStockers);
    while (Workers.Num() > Wanted)
    {
        FMarketWorker& Leaving = Workers.Last();
        if (Leaving.Carton) Leaving.Carton->Destroy();
        if (Leaving.Tag && Leaving.Tag->GetOwner()) Leaving.Tag->GetOwner()->Destroy();
        if (Leaving.Actor) Leaving.Actor->Destroy();
        Workers.Pop();
    }
    while (Workers.Num() < Wanted)
    {
        const int32 Index = Workers.Num();
        FMarketWorker Worker;
        Worker.Name = StaffPlanner::WorkerName(Index);
        const FVector Spot = WorkerRestSpot(Index);
        if (AActor* Human = MarketPeople::Spawn(GetWorld(), People, Spot, 7001 + Index * 97, Worker.Shopper))
        {
            Worker.Actor = Human;
            Worker.bHuman = true;
        }
        else Worker.Actor = SimplePerson(Spot, FLinearColor(.72f, .22f, .12f)); // company terracotta
        // 3D text has no Turkish glyphs: "GOREVLI AYSE".
        Worker.Tag = Label(Spot + FVector(0, 0, 205), FRotator::ZeroRotator,
            MarketCatalog::FoldTurkish(TEXT("G\u00d6REVL\u0130 ") + Worker.Name.ToUpper()), 9.f, FColor(255, 214, 150), true);
        Worker.Tag->SetCullDistance(1600.f);
        Worker.Timer = 0.5f + Index * 0.4f;
        Worker.bResting = true;
        Workers.Add(Worker);
    }
}

void AMarketGameMode::ResetWorkerJobs()
{
    for (FMarketWorker& Worker : Workers) WorkerFinish(Worker);
    Unplaceable.Reset();
    NoRoomTold.Reset();
}

void AMarketGameMode::WorkerGoTo(FMarketWorker& Worker, const FVector& Goal)
{
    const FVector Here = Worker.Actor ? Worker.Actor->GetActorLocation() : Goal;
    Worker.Route = AisleRoute(FVector(Here.X, Here.Y, 0.f), Goal);
}

bool AMarketGameMode::WorkerWalk(FMarketWorker& Worker, float DeltaTime)
{
    const FVector Lift(0, 0, Worker.bHuman ? 0.f : BoxPersonLift);
    const FVector Here = Worker.Actor->GetActorLocation() - Lift;
    while (Worker.Route.Num() > 0 && FVector::Dist2D(Here, Worker.Route[0]) < 4.f) Worker.Route.RemoveAt(0);
    const bool bMoving = Worker.Route.Num() > 0;
    if (bMoving)
    {
        const FVector Next = Worker.Route[0];
        if (Worker.bHuman)
        {
            bool bActuallyMoving = false;
            Worker.Actor->SetActorLocation(MarketPeople::MoveToward(Worker.Shopper, Here, Next, WorkerWalkSpeed,
                DeltaTime, Worker.Facing, bActuallyMoving) + Lift);
        }
        else
        {
            Worker.Facing = (Next - Here).GetSafeNormal2D();
            Worker.Actor->SetActorLocation(FMath::VInterpConstantTo(Here, Next, DeltaTime, WorkerWalkSpeed) + Lift);
        }
    }
    if (Worker.bHuman) MarketPeople::Update(Worker.Actor, People, Worker.Shopper, Worker.Facing, bMoving, MetaHumanYawOffset, DeltaTime);
    return !bMoving;
}

void AMarketGameMode::WorkerFinish(FMarketWorker& Worker)
{
    Worker.Job = StaffPlanner::FJob();
    Worker.Stage = EWorkerStage::Idle;
    Worker.Carry = 0;
    Worker.Retries = 0;
    Worker.Timer = 0.4f;
    if (Worker.Carton) { Worker.Carton->Destroy(); Worker.Carton = nullptr; }
}

void AMarketGameMode::WorkerThink(int32 WorkerIndex)
{
    FMarketWorker& Worker = Workers[WorkerIndex];
    TSet<int32> Busy;
    for (int32 Other = 0; Other < Workers.Num(); ++Other)
        if (Other != WorkerIndex && Workers[Other].Job.Kind != StaffPlanner::EJob::None) Busy.Add(Workers[Other].Job.Product);
    TArray<TPair<int32, FString>> NoRoom;
    // While the player arranges (R) the plan belongs to the player: workers only refill.
    const StaffPlanner::FJob Job = StaffPlanner::ChooseJob(Planogram, Products, State, Busy, Unplaceable, !bArrange, &NoRoom);
    for (const TPair<int32, FString>& Item : NoRoom)
    {
        Unplaceable.Add(Item.Key);
        bool bTold = false;
        NoRoomTold.Add(Item.Key, &bTold);
        if (!bTold) Notify(Worker.Name + TEXT(": ") + Item.Value); // once, not after every plan change
    }
    if (Job.Kind == StaffPlanner::EJob::None)
    {
        if (!Worker.bResting) { WorkerGoTo(Worker, WorkerRestSpot(WorkerIndex)); Worker.bResting = true; }
        return;
    }
    Worker.Job = Job;
    Worker.bResting = false;
    Worker.Retries = 0;
    Worker.Stage = EWorkerStage::ToDepot;
    WorkerGoTo(Worker, DepotSpot());
}

void AMarketGameMode::WorkerLoaded(FMarketWorker& Worker)
{
    const int32 P = Worker.Job.Product;
    if (!State.Stock.IsValidIndex(P) || !Products.IsValidIndex(P)) { WorkerFinish(Worker); return; }
    const FMarketStock& Stock = State.Stock[P];
    const int32 Room = Worker.Job.Kind == StaffPlanner::EJob::Refill ? Stock.Capacity - Stock.Shelf : StaffPlanner::CarryUnits;
    Worker.Carry = FMath::Clamp(FMath::Min(Room, Stock.Warehouse), 0, StaffPlanner::CarryUnits);
    if (Worker.Carry <= 0 && Worker.Job.Kind == StaffPlanner::EJob::Refill) { WorkerFinish(Worker); return; }
    FVector Goal;
    if (Worker.Job.Kind == StaffPlanner::EJob::Refill)
    {
        // The block of the product nearest to the depot door is as good as any: all blocks share the stock.
        if (!ShelfApproach.IsValidIndex(P) || ShelfApproach[P].Num() == 0) { WorkerFinish(Worker); return; }
        Goal = ShelfApproach[P][0];
        Goal.Z = 0.f;
    }
    else Goal = BlockApproachSpot(Worker.Job.Block, Worker.Job.Block.XCm);
    if (Worker.Carry > 0)
    {
        Worker.Carton = SurfaceBox(Worker.Actor->GetActorLocation(), FVector(38.f, 30.f, 24.f), EMarketSurface::Cardboard, false);
    }
    Worker.Stage = EWorkerStage::ToShelf;
    WorkerGoTo(Worker, Goal);
}

void AMarketGameMode::WorkerAtShelf(FMarketWorker& Worker)
{
    const int32 P = Worker.Job.Product;
    if (!State.Stock.IsValidIndex(P) || !Products.IsValidIndex(P)) { WorkerFinish(Worker); return; }
    if (Worker.Job.Kind == StaffPlanner::EJob::Place || Worker.Job.Kind == StaffPlanner::EJob::Widen)
    {
        if (bArrange) return; // the player is arranging: wait here until R mode ends
        FString Outcome;
        const bool bPlace = Worker.Job.Kind == StaffPlanner::EJob::Place;
        const bool bDone = bPlace ? StaffPlanner::TryPlace(Planogram, Products, Worker.Job.Block, Outcome)
                                  : StaffPlanner::TryWiden(Planogram, Products, Worker.Job.Block, Outcome);
        if (!bDone)
        {
            FPlanogramPlacement Again;
            FString Reason;
            if (bPlace && ++Worker.Retries <= 2 && StaffPlanner::PlanNewBlock(Planogram, Products, P, Again, Reason))
            {
                Worker.Job.Block = Again; // the spot was taken meanwhile: walk to the next best one
                WorkerGoTo(Worker, BlockApproachSpot(Again, Again.XCm));
                return;
            }
            if (bPlace)
            {
                Unplaceable.Add(P);
                bool bTold = false;
                NoRoomTold.Add(P, &bTold);
                if (!bTold) Notify(Worker.Name + TEXT(": ") + (Reason.IsEmpty() ? Outcome : Reason));
            }
            WorkerFinish(Worker);
            return;
        }
        FString Error;
        CommitPlan(Error);
        NoRoomTold.Remove(P);
        Notify(Worker.Name + TEXT(": ") + Outcome + (Error.IsEmpty() ? FString() : TEXT("  (Kaydedilemedi: ") + Error + TEXT(")")));
        Worker.Job.Kind = StaffPlanner::EJob::Refill; // the block exists now: fill it
    }
    // Face the shelf while working (a refill goes to the product's first block, see WorkerLoaded).
    FString FixtureId = Worker.Job.Block.FixtureId;
    if (FixtureId.IsEmpty())
        if (const FPlanogramPlacement* First = Planogram.FindPlacement(Products[P].Id)) FixtureId = First->FixtureId;
    if (const FPlanogramFixture* Fixture = Planogram.FindFixture(FixtureId))
    {
        const FVector Here = Worker.Actor->GetActorLocation();
        Worker.Facing = (Fixture->Location - Here).GetSafeNormal2D();
        if (Worker.bHuman && !Worker.Facing.IsNearlyZero())
            Worker.Actor->SetActorRotation(FRotator(0.f, Worker.Facing.Rotation().Yaw - 90.f - Worker.Shopper.MeshYaw + MetaHumanYawOffset, 0.f));
    }
    Worker.Stage = EWorkerStage::Working;
    Worker.Timer = WorkerUnitSeconds;
}

void AMarketGameMode::TickWorkers(float DeltaTime)
{
    if (Workers.Num() != FMath::Clamp(State.Stockers, 0, FMarketState::MaxStockers)) SyncWorkers();
    if (WorkerPlanVersion != ArrangeVersion) { WorkerPlanVersion = ArrangeVersion; Unplaceable.Reset(); } // new plan, new room
    const APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
    for (int32 Index = 0; Index < Workers.Num(); ++Index)
    {
        FMarketWorker& Worker = Workers[Index];
        if (!Worker.Actor) continue;
        const bool bAtGoal = WorkerWalk(Worker, DeltaTime);
        switch (Worker.Stage)
        {
        case EWorkerStage::Idle:
            Worker.Timer -= DeltaTime;
            if (Worker.Timer <= 0.f) { Worker.Timer = WorkerThinkSeconds; WorkerThink(Index); }
            break;
        case EWorkerStage::ToDepot:
            if (bAtGoal) { Worker.Stage = EWorkerStage::Loading; Worker.Timer = WorkerLoadSeconds; }
            break;
        case EWorkerStage::Loading:
            Worker.Timer -= DeltaTime;
            if (Worker.Timer <= 0.f) WorkerLoaded(Worker);
            break;
        case EWorkerStage::ToShelf:
            if (bAtGoal) WorkerAtShelf(Worker);
            break;
        case EWorkerStage::Working:
            Worker.Timer -= DeltaTime;
            if (Worker.Timer > 0.f) break;
            Worker.Timer = WorkerUnitSeconds;
            if (Worker.Carry <= 0 || State.Restock(Worker.Job.Product, 1) == 0) { WorkerFinish(Worker); break; }
            --Worker.Carry;
            RefreshShelfItems();
            if (Worker.Carry <= 0) WorkerFinish(Worker);
            break;
        }
        const FVector Floor = Worker.Actor->GetActorLocation() - FVector(0, 0, Worker.bHuman ? 0.f : BoxPersonLift);
        if (Worker.Carton) // the case is held in front of the chest
        {
            Worker.Carton->SetActorLocationAndRotation(Floor + Worker.Facing * 30.f + FVector(0, 0, Worker.bHuman ? 105.f : 110.f),
                FRotator(0.f, Worker.Facing.Rotation().Yaw, 0.f));
        }
        if (Worker.Tag)
        {
            const FVector TagLocation = Floor + FVector(0, 0, Worker.bHuman ? 200.f : 185.f);
            const float Yaw = Camera ? (Camera->GetCameraLocation() - TagLocation).Rotation().Yaw : 0.f;
            Worker.Tag->SetWorldLocationAndRotation(TagLocation, FRotator(0.f, Yaw, 0.f));
        }
    }
}

FString AMarketGameMode::WorkerSummary() const
{
    FString Out;
    for (const FMarketWorker& Worker : Workers)
    {
        const int32 P = Worker.Job.Product;
        const FString Product = Products.IsValidIndex(P) ? ProductName(P) : FString();
        FString Doing;
        switch (Worker.Stage)
        {
        case EWorkerStage::Idle: Doing = TEXT("bo\u015fta"); break;
        case EWorkerStage::ToDepot:
        case EWorkerStage::Loading: Doing = TEXT("depodan al\u0131yor: ") + Product; break;
        case EWorkerStage::ToShelf:
            Doing = Worker.Job.Kind == StaffPlanner::EJob::Place ? TEXT("rafa koymaya g\u00f6t\u00fcr\u00fcyor: ") + Product
                : Worker.Job.Kind == StaffPlanner::EJob::Widen ? TEXT("blo\u011funu geni\u015fletecek: ") + Product
                : TEXT("raf\u0131na g\u00f6t\u00fcr\u00fcyor: ") + Product;
            break;
        case EWorkerStage::Working: Doing = FString::Printf(TEXT("dolduruyor: %s (%d)"), *Product, Worker.Carry); break;
        }
        Out += (Out.IsEmpty() ? FString() : FString(TEXT("\n"))) + Worker.Name + TEXT(": ") + Doing;
    }
    return Out;
}
