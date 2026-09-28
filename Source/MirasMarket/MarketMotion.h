#pragma once

#include "CoreMinimal.h"
#include "MarketCustomers.h"

// How people move in the shop (G-070, Docs/Kurgu/00_KURGU_KITABI.md \u00a76). Independent of the world, tested
// (MirasMarket.Motion.*). MarketPeople animates MetaHumans; this module decides how a person walks and behaves:
//  - Walking style per person (fixed for a neighbour): a pensioner shuffles slowly and stops to look around,
//    a worker walks briskly, a family wanders and stops often, a student drifts looking at the phone, a child
//    darts. Speed also drops with a heavy basket and rises towards closing time for people in a hurry.
//  - Route: most people put their list in a short walking order (nearest shelf first, then 2-opt);
//    families keep the order they wrote it in; children go straight to what they came for.
//  - At the shelf: a regular knows where things are; an empty shelf means searching before giving up; a price
//    above the rival's means picking the product up and comparing.
//  - Queue: seeing a long queue with a small basket, some people put the basket down and leave.
//  - Crowd: people keep a personal space, keep to the right when someone comes towards them and slow down
//    behind a slower person instead of walking through them.
//  - Neighbours who both shop here often stop for a short chat when they meet.
namespace MarketMotion
{
    enum class EStyle : uint8 { Steady = 0, Shuffle, Brisk, Wander, Distracted, Dart, Count };

    struct FGait
    {
        EStyle Style = EStyle::Steady;
        float SpeedScale = 1.f;       // x the segment's walking speed
        float Sway = 0.f;             // cm of side-to-side drift while walking
        float PauseChance = 0.f;      // per second while shopping: stops and looks around
        float PauseSeconds = 1.5f;
        float PersonalSpace = 60.f;   // cm kept from other people
        float ChatChance = 0.f;       // when meeting another regular
        float Hurry = 0.f;            // 0..1: speeds up towards closing time
    };

    FString StyleName(EStyle Style);
    // Fixed for a neighbour (segment + id + campaign seed): the same person walks the same way every visit.
    FGait MakeGait(MarketCustomers::ESegment Segment, int32 CustomerId, int32 Seed);
    // cm/s now: base x style, slower with a full basket, faster near closing for people in a hurry.
    float Speed(const FGait& Gait, float BaseSpeed, int32 BasketUnits, float DayProgress);
    // Side-to-side drift at walking time Seconds (cm, perpendicular to the walking direction).
    float SwayOffset(const FGait& Gait, float Seconds);

    // Order in which the stops are visited (indices into Stops), from Start, ending at End (the till).
    TArray<int32> OrderVisits(const TArray<FVector>& Stops, const FVector& Start, const FVector& End, EStyle Style);
    float RouteLength(const TArray<FVector>& Stops, const TArray<int32>& Order, const FVector& Start, const FVector& End);

    // Seconds at a shelf. PriceRatio = our price / what the shopper thinks the rival asks (0 = unknown).
    float BrowseSeconds(float BaseSeconds, EStyle Style, bool bReturning, bool bOnShelf, float PriceRatio);

    // Seeing QueueAhead people at the till, leaves the basket (Roll 0..1).
    bool Balks(MarketCustomers::ESegment Segment, int32 QueueAhead, int32 BasketUnits, float Roll);

    // Crowd: returns the point to walk to now (Target nudged to keep to the right of someone coming towards
    // us) and OutSpeedFactor 0.15..1 (slows behind someone inside the personal space ahead).
    FVector Steer(const FVector& Here, const FVector& Target, const TArray<FVector>& Others, float PersonalSpace, float& OutSpeedFactor);

    // Two regulars meet (Roll 0..1). ChatSeconds: 3..7 s from a second roll.
    bool Chats(const FGait& A, const FGait& B, float Roll);
    float ChatSeconds(float Roll);
}
