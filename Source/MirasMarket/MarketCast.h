#pragma once

#include "CoreMinimal.h"

// Karar M30 (Mustafa 01.10.2026): the game starts wherever the player wants, so nobody and nothing is tied to the
// prototype's town. The people and firms around the family shop are named from the active country pack
// (Config/ulkeler.json "names", "banks") and the campaign's seed, the same for the whole campaign:
//  - the father's wholesaler and its salesman, the cheaper cash-and-carry and its owner,
//  - the family-run market on the same street and its owner,
//  - the country's banks (local, commercial, investment, development).
// Independent of the world (MarketCountry::Active / ActiveSeed), tested (MirasMarket.Cast.*).
namespace MarketCast
{
    enum class ERole : uint8 { Salesman = 0, Wholesaler, CashCarry, CashCarryOwner, RivalShop, RivalOwner, Count };
    // "Mehmet Kaya" (first and last name, distinct for every role in a campaign).
    FString Person(ERole Role);
    FString FirstName(ERole Role);
    FString LastName(ERole Role);

    FString Salesman();        // the wholesaler's man (first name, as the family calls him)
    FString Wholesaler();      // "Kaya G\u0131da Da\u011f\u0131t\u0131m"
    FString CashCarry();       // "Demir Toptan"
    FString CashCarryOwner();  // "Ali Demir"
    FString RivalShop();       // "\u015eahin Market"
    FString RivalOwner();      // "Kadir \u015eahin"

    // The country's banks: 0 local, 1 commercial, 2 investment, 3 development.
    FString Bank(int32 Index);
}
