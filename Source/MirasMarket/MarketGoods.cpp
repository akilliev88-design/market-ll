#include "MarketGoods.h"
#include "MarketEconomy.h"

FString MarketGoods::Fold(const FString& Text)
{
    FString Out;
    for (const TCHAR C : Text)
    {
        switch (C)
        {
        case TEXT('\u00e7'): case TEXT('\u00c7'): Out += TEXT('c'); break;
        case TEXT('\u011f'): case TEXT('\u011e'): Out += TEXT('g'); break;
        case TEXT('\u0131'): case TEXT('\u0130'): case TEXT('I'): Out += TEXT('i'); break;
        case TEXT('\u00f6'): case TEXT('\u00d6'): Out += TEXT('o'); break;
        case TEXT('\u015f'): case TEXT('\u015e'): Out += TEXT('s'); break;
        case TEXT('\u00fc'): case TEXT('\u00dc'): Out += TEXT('u'); break;
        default: Out += (C >= TEXT('A') && C <= TEXT('Z')) ? static_cast<TCHAR>(C - TEXT('A') + TEXT('a')) : C; break;
        }
    }
    return Out;
}

MarketGoods::EGroup MarketGoods::Classify(const FString& Category)
{
    const FString Key = Fold(Category);
    struct FRule { const TCHAR* Word; EGroup Group; };
    // Order matters: "dondurma" before "sut", "cay-kahve" before anything with "kahve".
    static const FRule Rules[] =
    {
        { TEXT("dondurma"), EGroup::IceCream },
        { TEXT("cay"), EGroup::TeaCoffee }, { TEXT("kahve"), EGroup::TeaCoffee },
        { TEXT("icecek"), EGroup::Drinks }, { TEXT("su"), EGroup::Drinks },
        { TEXT("sut"), EGroup::Dairy }, { TEXT("peynir"), EGroup::Dairy }, { TEXT("yogurt"), EGroup::Dairy },
        { TEXT("biskuvi"), EGroup::Sweets }, { TEXT("cikolata"), EGroup::Sweets }, { TEXT("sekerleme"), EGroup::Sweets },
        { TEXT("atistirmalik"), EGroup::Snacks }, { TEXT("cips"), EGroup::Snacks },
        { TEXT("makarna"), EGroup::Staples }, { TEXT("bakliyat"), EGroup::Staples }, { TEXT("temel"), EGroup::Staples }, { TEXT("un"), EGroup::Staples },
        { TEXT("yag"), EGroup::OilSauce }, { TEXT("salca"), EGroup::OilSauce },
        { TEXT("temizlik"), EGroup::Household }, { TEXT("deterjan"), EGroup::Household },
        { TEXT("bakim"), EGroup::PersonalCare }, { TEXT("kozmetik"), EGroup::PersonalCare },
        { TEXT("kagit"), EGroup::Paper },
    };
    for (const FRule& Rule : Rules)
    {
        const FString Word(Rule.Word);
        // Whole words or hyphen parts only: "su" must not match "sut" or "susam".
        int32 Start = 0;
        while (Start < Key.Len())
        {
            int32 End = Start;
            while (End < Key.Len() && Key[End] != TEXT('-') && Key[End] != TEXT(' ') && Key[End] != TEXT('/')) ++End;
            if (Key.Mid(Start, End - Start) == Word) return Rule.Group;
            Start = End + 1;
        }
    }
    return EGroup::Other;
}

FString MarketGoods::GroupName(EGroup Group)
{
    switch (Group)
    {
    case EGroup::Dairy: return TEXT("s\u00fct \u00fcr\u00fcnleri");
    case EGroup::Drinks: return TEXT("i\u00e7ecek");
    case EGroup::TeaCoffee: return TEXT("\u00e7ay-kahve");
    case EGroup::Sweets: return TEXT("bisk\u00fcvi-\u00e7ikolata");
    case EGroup::Snacks: return TEXT("at\u0131\u015ft\u0131rmal\u0131k");
    case EGroup::Staples: return TEXT("temel g\u0131da");
    case EGroup::OilSauce: return TEXT("ya\u011f-sal\u00e7a");
    case EGroup::Household: return TEXT("temizlik");
    case EGroup::PersonalCare: return TEXT("ki\u015fisel bak\u0131m");
    case EGroup::Paper: return TEXT("ka\u011f\u0131t");
    case EGroup::IceCream: return TEXT("dondurma");
    default: return TEXT("di\u011fer");
    }
}

int32 MarketGoods::ShelfLifeDays(EGroup Group)
{
    switch (Group)
    {
    case EGroup::Dairy: return 7;     // fresh milk and yogurt in the game's short form
    case EGroup::IceCream: return 90;
    case EGroup::Sweets: return 120;
    case EGroup::Snacks: return 90;
    case EGroup::Drinks: return 180;
    default: return 0;
    }
}

int32 MarketGoods::ShelfLifeDays(const FMarketProduct& Product)
{
    return Product.ShelfLifeDays > 0 ? Product.ShelfLifeDays : ShelfLifeDays(Classify(Product.Category));
}

TArray<FString> MarketGoods::Aisles(const TArray<FMarketProduct>& Products)
{
    TArray<FString> Result;
    for (const FMarketProduct& Product : Products)
        if (!Product.Category.IsEmpty()) Result.AddUnique(Product.Category);
    Result.Sort();
    return Result;
}
