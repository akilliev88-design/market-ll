#pragma once

#include "CoreMinimal.h"
#include "MarketEconomy.h"

// The chain of command for the decisions that matter (karar M33, Mustafa 01.10.2026: "hiyerar\u015fide birbirlerini kontrol
// etsinler; kritik \u015feylerde bir \u00fcst\u00fcnde kim varsa ona sorsunlar"). Independent of the world, tested
// (MarketSim.Command.*).
//  - Everyday things stay below and never reach the player: prices and orders (store managers, MarketManagers),
//    a week's clearance of slow goods (MarketBranches::Clearance: deep cuts need the province manager, who also
//    takes back a weak store manager's mistakes), the online channels of a province (MarketOnline proposals).
//  - Critical things come up the line as a decision card, brought by the country manager, else the region's
//    director, else the sub-region's manager (one who checked it):
//      closing a branch: a province manager proposes it after three months in a row in the red (a branch past its
//      first three months); turned down, he does not ask again for 90 days;
//      opening a branch: a province manager whose shops all earn money and whose province has room proposes a
//      new neighbourhood market when the till can carry it (three times its opening cost); one proposal per
//      province in 180 days.
//      renewing a store (D9b, M46): the oldest store of his province past eight years, when the till holds three
//      times the works; one proposal per province in 180 days.
//  - Without a province manager nobody proposes: the player runs the province himself.
namespace MarketCommand
{
    constexpr int32 LossMonthsToClose = 3;
    constexpr int32 MatureDays = 90;
    constexpr int32 CloseQuietDays = 90;
    constexpr int32 OpenQuietDays = 180;
    constexpr int32 RenewYears = 8;   // D9b (M46): a province manager proposes renewing a store this old (180 days apart)

    // "\u00fclke m\u00fcd\u00fcr\u00fc Ay\u015fe Kaya" who brings a province's proposal up ("" nobody: it comes straight to the player).
    FString Forwarder(const FMarketState& State, const FString& Country, const FString& Province);

    // Month start (after the branches' day): the months in the red, the province managers' proposals.
    void CloseDay(FMarketState& State, const TArray<FMarketProduct>& Products);
    // A "command.*" decision (MarketEvents::Decide).
    bool Resolve(FMarketState& State, const TArray<FMarketProduct>& Products, const FMarketDecision& Decision, int32 Option, FString& OutMessage);
}
