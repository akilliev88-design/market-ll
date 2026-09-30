#pragma once

#include "CoreMinimal.h"

// Background market data for the Rakipler page (G-074): Turkish grocery retail and the world's largest retailers.
// The chains follow their real history (anchor years, linear in between); the player's national share comes from
// MarketCompany. Sources and estimates: claude/rakipler_pazar_payi_arastirma.md (USDA GAIN 2014/2024, Bloomberg HT
// 2025, Deloitte Global Powers of Retailing). The 2011 figures are estimates worked back from 2013.
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
    const TCHAR* Source();
}
