#pragma once

#include "CoreMinimal.h"

// Background market data for the Rakipler page (G-074): the country's grocery chains and the world's largest
// retailers. M24: the game has its own economy and no calendar year on screen; the figures are the game world's,
// keyed by the game's internal calendar (MarketCalendar::StartYear = year 1). The player's national share comes
// from MarketCompany. Research notes: claude/rakipler_pazar_payi_arastirma.md (not shown to the player).
namespace MarketRetail
{
    struct FPoint { int32 Year; float Value; };

    struct FChain
    {
        const TCHAR* Name;
        const TCHAR* Kind;
        const TCHAR* LogoKey;          // Content/Brands/<key>/logo.png ("" = badge only)
        TArray<FPoint> Share;          // % of Turkish grocery retail
        TArray<FPoint> Stores;         // stores in Turkey
        const TCHAR* Note;
        int32 ClosedYear = 0;          // left the market (sold / merged): 0 = still there
    };

    struct FGlobal
    {
        const TCHAR* Name;
        const TCHAR* Kind;
        TArray<FPoint> Revenue;        // billion USD a year
        const TCHAR* Note;
    };

    const TArray<FChain>& National();
    const TArray<FGlobal>& Global();
    // Value on a date: linear between anchor years, held flat before the first and after the last.
    float At(const TArray<FPoint>& Points, float Year);
    // Share of "bakkal ve pazar" + other chains = what the listed chains and the player do not hold.
    float OthersShare(float Year, float PlayerShare);
}
