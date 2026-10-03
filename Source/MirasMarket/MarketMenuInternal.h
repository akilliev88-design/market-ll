#pragma once

#include "CoreMinimal.h"
#include "MarketDemand.h"

class AMarketGameMode;

// Helpers shared by the menu's source files (MarketMenuWidget.cpp, MarketMenuPages.cpp). Named namespace: the
// module is a unity build.
namespace MarketMenuUi
{
    FSlateFontInfo MenuFont(bool bBold, int32 Size);
    // 1234.5 TL -> "1.234,50 TL"
    FString Tl(int64 Kurus);
    // C3 (A istek 1): a narrow card's amount: whole lira from 10 000, "1,2 milyon" from a million (Tl stays the full sum).
    FString TlShort(int64 Kurus);
    FLinearColor Hex(const TCHAR* Code, float Alpha = 1.f);
    // "sut" -> "Sut", "icecek" -> "Icecek" with the Turkish dotted capital I.
    FString Title(const FString& Text);
    FString Initials(const FString& Name);
    FLinearColor RivalColor(int32 Rival);
    // E2: the home province's chains the shoppers compare with (slot 0 = most present), at most three.
    int32 RivalCount(const AMarketGameMode* G);
    FString RivalName(const AMarketGameMode* G, int32 Slot);
    FString RivalKind(const AMarketGameMode* G, int32 Slot);
    FString RivalLogoKey(const AMarketGameMode* G, int32 Slot);
    // Price of one rival for one product today; false = no such rival.
    bool RivalShelfPrice(const AMarketGameMode& G, int32 Product, int32 Rival, int64& OutPrice);
    // Cheapest rival with the product (INDEX_NONE = no chain in the province).
    int32 CheapestRival(const AMarketGameMode& G, int32 Product, int64& OutPrice);
    double BuyChanceOf(const AMarketGameMode& G, int32 Product);
    FString ProblemText(const AMarketGameMode& G, const MarketDemand::FProblem& Problem);
    FString RivalsToday(const AMarketGameMode& G, int32 OnlyRival = INDEX_NONE);
}
