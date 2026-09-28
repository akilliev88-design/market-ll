#include "MarketMotion.h"

namespace MarketMotion
{
    uint32 MotionMix(int32 Seed, int32 Id, uint32 Salt)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), static_cast<uint32>(Id), Salt };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }

    float Unit(uint32 Hash) { return static_cast<float>(Hash % 10000u) / 10000.f; }

    FVector Flat(const FVector& V) { return FVector(V.X, V.Y, 0.f); }

    float Leg(const FVector& A, const FVector& B) { return FVector::Dist2D(A, B); }
}

FString MarketMotion::StyleName(EStyle Style)
{
    switch (Style)
    {
    case EStyle::Shuffle: return TEXT("a\u011f\u0131r ad\u0131ml\u0131");
    case EStyle::Brisk: return TEXT("seri");
    case EStyle::Wander: return TEXT("oyalanan");
    case EStyle::Distracted: return TEXT("telefona dal\u0131k");
    case EStyle::Dart: return TEXT("ko\u015fturan");
    default: return TEXT("sakin");
    }
}

MarketMotion::FGait MarketMotion::MakeGait(MarketCustomers::ESegment Segment, int32 CustomerId, int32 Seed)
{
    using MarketCustomers::ESegment;
    const float A = Unit(MotionMix(Seed, CustomerId, 0x6A17u));
    const float B = Unit(MotionMix(Seed, CustomerId, 0x6A18u));
    FGait G;
    switch (Segment)
    {
    case ESegment::Retired:
        // Most pensioners shuffle; some are sprightly and simply calm.
        G.Style = A < 0.7f ? EStyle::Shuffle : EStyle::Steady;
        G.SpeedScale = G.Style == EStyle::Shuffle ? 0.8f : 0.95f;
        G.PauseChance = 0.08f; G.PauseSeconds = 2.f; G.PersonalSpace = 70.f; G.ChatChance = 0.6f;
        break;
    case ESegment::Family:
        G.Style = A < 0.6f ? EStyle::Wander : EStyle::Steady;
        G.SpeedScale = 0.9f; G.Sway = G.Style == EStyle::Wander ? 12.f : 4.f;
        G.PauseChance = G.Style == EStyle::Wander ? 0.12f : 0.05f; G.PauseSeconds = 1.8f; G.PersonalSpace = 65.f; G.ChatChance = 0.4f; G.Hurry = 0.2f;
        break;
    case ESegment::Worker:
        G.Style = EStyle::Brisk;
        G.SpeedScale = 1.15f; G.PauseChance = 0.01f; G.PersonalSpace = 55.f; G.ChatChance = 0.15f; G.Hurry = 0.6f;
        break;
    case ESegment::Student:
        G.Style = A < 0.5f ? EStyle::Distracted : EStyle::Brisk;
        G.SpeedScale = G.Style == EStyle::Distracted ? 0.85f : 1.1f; G.Sway = G.Style == EStyle::Distracted ? 10.f : 3.f;
        G.PauseChance = G.Style == EStyle::Distracted ? 0.1f : 0.02f; G.PauseSeconds = 1.2f; G.PersonalSpace = 50.f; G.ChatChance = 0.2f; G.Hurry = 0.3f;
        break;
    case ESegment::Trader:
        G.Style = EStyle::Steady;
        G.SpeedScale = 1.f; G.PauseChance = 0.03f; G.PersonalSpace = 60.f; G.ChatChance = 0.5f; G.Hurry = 0.4f;
        break;
    case ESegment::Child:
        G.Style = EStyle::Dart;
        G.SpeedScale = 1.2f; G.Sway = 15.f; G.PauseChance = 0.04f; G.PauseSeconds = 0.8f; G.PersonalSpace = 40.f; G.ChatChance = 0.1f;
        break;
    default:
        break;
    }
    // Every person a little different, so a group never walks in lockstep.
    G.SpeedScale *= 0.92f + 0.16f * B;
    return G;
}

float MarketMotion::Speed(const FGait& Gait, float BaseSpeed, int32 BasketUnits, float DayProgress)
{
    const float Load = FMath::Clamp(1.f - 0.02f * FMath::Max(0, BasketUnits - 4), 0.75f, 1.f);   // a heavy basket
    const float Closing = 1.f + Gait.Hurry * 0.25f * FMath::Clamp((DayProgress - 0.8f) / 0.2f, 0.f, 1.f);
    return BaseSpeed * Gait.SpeedScale * Load * Closing;
}

float MarketMotion::SwayOffset(const FGait& Gait, float Seconds)
{
    return Gait.Sway * FMath::Sin(Seconds * (Gait.Style == EStyle::Dart ? 2.4f : 1.1f));
}

float MarketMotion::RouteLength(const TArray<FVector>& Stops, const TArray<int32>& Order, const FVector& Start, const FVector& End)
{
    float Length = 0.f;
    FVector Here = Start;
    for (int32 Index : Order) { Length += Leg(Here, Stops[Index]); Here = Stops[Index]; }
    return Length + Leg(Here, End);
}

TArray<int32> MarketMotion::OrderVisits(const TArray<FVector>& Stops, const FVector& Start, const FVector& End, EStyle Style)
{
    TArray<int32> Order;
    for (int32 I = 0; I < Stops.Num(); ++I) Order.Add(I);
    // A family walks its list as written; a child goes for the first wish (what it came for) and then the rest.
    if (Style == EStyle::Wander || Stops.Num() < 2) return Order;
    const int32 Fixed = Style == EStyle::Dart ? 1 : 0;
    // Nearest shelf next.
    TArray<int32> Result;
    TArray<bool> Used; Used.Init(false, Stops.Num());
    FVector Here = Start;
    for (int32 I = 0; I < Fixed; ++I) { Result.Add(I); Used[I] = true; Here = Stops[I]; }
    while (Result.Num() < Stops.Num())
    {
        int32 Best = INDEX_NONE;
        float BestDistance = 0.f;
        for (int32 I = 0; I < Stops.Num(); ++I)
            if (!Used[I] && (Best == INDEX_NONE || Leg(Here, Stops[I]) < BestDistance)) { Best = I; BestDistance = Leg(Here, Stops[I]); }
        Used[Best] = true;
        Result.Add(Best);
        Here = Stops[Best];
    }
    // 2-opt: reverse a stretch while that shortens the walk.
    for (bool bImproved = true; bImproved;)
    {
        bImproved = false;
        for (int32 I = Fixed; I < Result.Num() - 1; ++I)
            for (int32 J = I + 1; J < Result.Num(); ++J)
            {
                TArray<int32> Candidate = Result;
                for (int32 A = I, B = J; A < B; ++A, --B) Candidate.Swap(A, B);
                if (RouteLength(Stops, Candidate, Start, End) + 0.5f < RouteLength(Stops, Result, Start, End)) { Result = Candidate; bImproved = true; }
            }
    }
    return Result;
}

float MarketMotion::BrowseSeconds(float BaseSeconds, EStyle Style, bool bReturning, bool bOnShelf, float PriceRatio)
{
    float Seconds = BaseSeconds;
    if (bReturning) Seconds *= 0.75f;                 // knows where things are
    if (!bOnShelf) Seconds *= 1.8f;                   // searches the shelf before giving up
    if (PriceRatio > 1.05f) Seconds += FMath::Min(2.5f, (PriceRatio - 1.f) * 10.f); // picks it up, compares
    if (Style == EStyle::Distracted) Seconds *= 1.3f;
    if (Style == EStyle::Dart) Seconds *= 0.6f;
    return FMath::Clamp(Seconds, 0.4f, 8.f);
}

bool MarketMotion::Balks(MarketCustomers::ESegment Segment, int32 QueueAhead, int32 BasketUnits, float Roll)
{
    using MarketCustomers::ESegment;
    // How long a queue this person accepts for a basket of this size.
    const float Tolerance = (Segment == ESegment::Worker ? 2.f : Segment == ESegment::Retired ? 5.f : Segment == ESegment::Child ? 2.f : 3.f)
        + 0.5f * FMath::Min(BasketUnits, 8);
    if (QueueAhead <= Tolerance) return false;
    return Roll < FMath::Clamp((QueueAhead - Tolerance) * 0.3f, 0.f, 0.8f);
}

FVector MarketMotion::Steer(const FVector& Here, const FVector& Target, const TArray<FVector>& Others, float PersonalSpace, float& OutSpeedFactor)
{
    OutSpeedFactor = 1.f;
    FVector Forward = Flat(Target - Here);
    const float Distance = Forward.Size();
    if (Distance < 1.f) return Target;
    Forward /= Distance;
    const FVector Right(-Forward.Y, Forward.X, 0.f);  // keep to the right (Z up, X forward, Y right)
    float Nudge = 0.f;
    for (const FVector& Other : Others)
    {
        const FVector To = Flat(Other - Here);
        const float Ahead = FVector::DotProduct(To, Forward);
        const float Side = FVector::DotProduct(To, Right);
        if (Ahead <= 0.f || Ahead > PersonalSpace * 2.f || FMath::Abs(Side) > PersonalSpace) continue;
        // Someone in front: slow down in the personal space, step aside a little from further away.
        if (Ahead < PersonalSpace) OutSpeedFactor = FMath::Min(OutSpeedFactor, FMath::Max(0.15f, Ahead / PersonalSpace));
        // Someone straight ahead or on the left: we step right; someone on the right: we step left.
        Nudge += (Side > 1.f ? -1.f : 1.f) * (PersonalSpace - FMath::Abs(Side)) * 0.5f;
    }
    if (Nudge == 0.f) return Target;
    // Never leave the aisle: at most 25 cm, and not when the stop is close (shelf, till).
    const float Allowed = FMath::Min(25.f, Distance * 0.3f);
    return Here + Forward * Distance + Right * FMath::Clamp(Nudge, -Allowed, Allowed);
}

bool MarketMotion::Chats(const FGait& A, const FGait& B, float Roll)
{
    return Roll < A.ChatChance * B.ChatChance;
}

float MarketMotion::ChatSeconds(float Roll)
{
    return 3.f + 4.f * FMath::Clamp(Roll, 0.f, 1.f);
}
