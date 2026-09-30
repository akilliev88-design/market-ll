#pragma once

#include "CoreMinimal.h"

struct FMarketProduct;

// Demand groups of the catalog's categories. Calendar, customer segments, promotions and freshness all speak in
// these groups, so a new category only has to be mapped once (Classify). Independent of the world.
namespace MarketGoods
{
    enum class EGroup : uint8
    {
        Dairy,        // s\u00fct, yo\u011furt, ayran, peynir
        Drinks,       // i\u00e7ecek
        TeaCoffee,    // \u00e7ay-kahve
        Sweets,       // bisk\u00fcvi-\u00e7ikolata
        Snacks,       // at\u0131\u015ft\u0131rmal\u0131k
        Staples,      // makarna-bakliyat, temel g\u0131da
        OilSauce,     // ya\u011f-sal\u00e7a
        Household,    // temizlik
        PersonalCare, // ki\u015fisel bak\u0131m
        Paper,        // ka\u011f\u0131t
        IceCream,     // dondurma
        Other,
        Count
    };

    // Lower case, Turkish letters folded to ASCII (\u00e7->c, \u011f->g, \u0131/\u0130->i, \u00f6->o, \u015f->s, \u00fc->u).
    FString Fold(const FString& Text);
    EGroup Classify(const FString& Category);
    // Turkish name for reports, e.g. "i\u00e7ecek".
    FString GroupName(EGroup Group);
    // Days a product of this group stays sellable on the shelf (0 = does not spoil in the game).
    int32 ShelfLifeDays(EGroup Group);
    // G-078: the product's own shelf life from the catalog, else its group's.
    int32 ShelfLifeDays(const FMarketProduct& Product);
}
