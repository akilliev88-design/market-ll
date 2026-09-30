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
    FLinearColor Hex(const TCHAR* Code, float Alpha = 1.f);
    // "sut" -> "Sut", "icecek" -> "Icecek" with the Turkish dotted capital I.
    FString Title(const FString& Text);
    FString Initials(const FString& Name);
    FLinearColor RivalColor(int32 Rival);
    // Price of one rival for one product today; false = the rival's shelf is empty.
    bool RivalShelfPrice(const AMarketGameMode& G, int32 Product, int32 Rival, int64& OutPrice);
    // Cheapest open rival with the product on its shelf (INDEX_NONE = nobody has it today).
    int32 CheapestRival(const AMarketGameMode& G, int32 Product, int64& OutPrice);
    double BuyChanceOf(const AMarketGameMode& G, int32 Product);
    FString ProblemText(const AMarketGameMode& G, const MarketDemand::FProblem& Problem);
    FString RivalsToday(const AMarketGameMode& G, int32 OnlyRival = INDEX_NONE);
}
