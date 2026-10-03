#include "MarketStart.h"
#include "MarketCountry.h"
#include "MarketStaff.h"
#include "MarketFinance.h"
#include "MarketOwner.h"

namespace MarketStart
{
    struct FForms { const TCHAR* Key; const TCHAR* Plain; const TCHAR* Genitive; const TCHAR* Ablative; const TCHAR* With; const TCHAR* Mine; };
    const FForms Forms[] =
    {
        { TEXT("baba"), TEXT("baban"), TEXT("baban\u0131n"), TEXT("babandan"), TEXT("babanla"), TEXT("babam\u0131n") },
        { TEXT("teyze"), TEXT("teyzen"), TEXT("teyzenin"), TEXT("teyzenden"), TEXT("teyzenle"), TEXT("teyzemin") },
        { TEXT("dayi"), TEXT("day\u0131n"), TEXT("day\u0131n\u0131n"), TEXT("day\u0131ndan"), TEXT("day\u0131nla"), TEXT("day\u0131m\u0131n") },
        { TEXT("hala"), TEXT("halan"), TEXT("halan\u0131n"), TEXT("halandan"), TEXT("halanla"), TEXT("halam\u0131n") },
        { TEXT("amca"), TEXT("amcan"), TEXT("amcan\u0131n"), TEXT("amcandan"), TEXT("amcanla"), TEXT("amcam\u0131n") },
        { TEXT("buyukanne"), TEXT("b\u00fcy\u00fckannen"), TEXT("b\u00fcy\u00fckannenin"), TEXT("b\u00fcy\u00fckannenden"), TEXT("b\u00fcy\u00fckannenle"), TEXT("b\u00fcy\u00fckannemin") },
    };

    const FForms& FormsOf(const FString& Key)
    {
        for (const FForms& F : Forms) if (Key == F.Key) return F;
        return Forms[0]; // older saves: the prototype's father
    }

    uint32 StartMix(int32 Seed, uint32 Salt, uint32 Extra)
    {
        uint32 Hash = 2166136261u ^ Salt;
        const uint32 Parts[2] = { static_cast<uint32>(Seed), Extra };
        for (uint32 Part : Parts) for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return Hash;
    }
}

const TArray<FString>& MarketStart::RelativeKeys()
{
    // M36 (Mustafa 02.10.2026): our own family, not an aunt or an uncle: father and mother retire and leave the shop
    // to us. The stories speak of the father.
    static const TArray<FString> Keys = { TEXT("baba") };
    return Keys;
}

void MarketStart::Setup(FMarketState& State, const FString& CountryId, const FString& CityId, int32 Seed)
{
    State.CountryId = MarketCountry::FindOrDefault(CountryId).Id;
    State.CityId = CityId;
    if (!MarketCountry::FindCity(State.CountryId, State.CityId)) State.CityId = LegacyProvince(State.CountryId);
    State.RivalSeed = Seed;
    State.RelativeKey = RelativeKeys()[0];
    // The shop comes with its people: one cashier, two shelf stockers (karar L03).
    MarketStaff::AddStartingStaff(State, 1, 2);
    // M37 (Mustafa 02.10.2026: "ba\u015flang\u0131\u00e7taki bor\u00e7, d\u00fckk\u00e2n\u0131n elindeki para ekonomiye g\u00f6re"): the till holds a
    // month of the shop's fixed costs with our salary (people, running costs, the rent to the parents); the father's
    // debt to the wholesaler is a month and a half of them. Both follow the country, the province and the year.
    MarketCountry::SetActive(State.CountryId, Seed); // the country's wages and prices for the numbers below
    State.Owner.SalaryX10 = MarketOwner::StartSalaryX10;
    const int64 Month = MarketFinance::CompanyMonthCost(State) + MarketOwner::CompanyCost(State);
    State.Cash = FMath::Max<int64>(10000, Month / 100 * 100);
    State.StartDebt = FMath::Max<int64>(10000, FMath::RoundToInt64(Month * StartDebtMonths) / 10000 * 10000);
    State.InheritedDebt = State.StartDebt;
    MarketEras::Setup(State); // C3 (B4): this campaign's eras, shifted by the seed
}

int32 MarketStart::StockShelvesPartly(FMarketState& State, int32 Seed)
{
    int32 Moved = 0;
    for (int32 I = 0; I < State.Stock.Num(); ++I)
    {
        FMarketStock& Item = State.Stock[I];
        if (Item.Capacity <= 0 || Item.Warehouse <= 0) continue;
        const float Fill = 0.40f + (StartMix(Seed, 0x5F11u, static_cast<uint32>(I)) % 36u) / 100.f; // 40-75 %
        const int32 Want = FMath::Max(0, FMath::RoundToInt(Item.Capacity * Fill) - Item.Shelf);
        const int32 Take = FMath::Min(Want, Item.Warehouse);
        Item.Shelf += Take;
        Item.Warehouse -= Take;
        Moved += Take;
    }
    return Moved;
}

FString MarketStart::Relative(const FMarketState& State, ECase Case, bool bCapital)
{
    const FForms& F = FormsOf(State.RelativeKey);
    FString Text = Case == ECase::Genitive ? F.Genitive : Case == ECase::Ablative ? F.Ablative : Case == ECase::With ? F.With : Case == ECase::Mine ? F.Mine : F.Plain;
    if (bCapital && Text.Len() > 0) Text[0] = FChar::ToUpper(Text[0]);
    return Text;
}

FString MarketStart::LegacyProvince(const FString& CountryId)
{
    const MarketCountry::FProfile* Country = CountryId.IsEmpty() ? &MarketCountry::Default() : MarketCountry::Find(CountryId);
    if (!Country) return FString();
    if (!Country->ReferenceProvince.IsEmpty() && MarketCountry::FindCity(Country->Id, Country->ReferenceProvince)) return Country->ReferenceProvince;
    return Country->Cities.Num() > 0 ? Country->Cities[0].Id : FString();
}

FString MarketStart::HomeProvince(const FMarketState& State)
{
    return MarketCountry::FindCity(State.CountryId, State.CityId) ? State.CityId : LegacyProvince(State.CountryId);
}

FString MarketStart::PlaceText(const FMarketState& State)
{
    const MarketCountry::FProfile* Country = MarketCountry::Find(State.CountryId);
    const MarketCountry::FCity* City = MarketCountry::FindCity(State.CountryId, HomeProvince(State));
    const FString CountryName = Country ? Country->Name : MarketCountry::Default().Name;
    return City ? City->Name + TEXT(", ") + CountryName : CountryName;
}

FString MarketStart::IntroText(const FMarketState& State)
{
    // M36: father and mother retire; the building stays theirs, the rent is their pension.
    return FString::Printf(TEXT("%s. Annenle baban emekli oldu; marketi art\u0131k sen y\u00f6netiyorsun. Bina onlar\u0131n: her ay kiras\u0131n\u0131 onlara \u00f6dersin. Bir kasiyer, iki reyon g\u00f6revlisi, baban\u0131n toptanc\u0131ya %s borcu ve kasada bir ayl\u0131k gider. Raflar yar\u0131 dolu; depoya bak, eksikleri diz ve O ile a\u00e7."),
        *PlaceText(State), *MarketCountry::Money(State.InheritedDebt));
}
