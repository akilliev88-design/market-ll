#include "MarketCountry.h"
#include "MarketEras.h"
#include "MarketPrices.h"
#include "MarketMap.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace MarketCountry
{
    FProfile& ActiveRef()
    {
        static FProfile Profile;
        return Profile;
    }

    ECharacter CharacterOf(const FString& Text)
    {
        if (Text == TEXT("istikrarli") || Text == TEXT("stable")) return ECharacter::Stable;
        if (Text == TEXT("oynak") || Text == TEXT("volatile")) return ECharacter::Volatile;
        return ECharacter::HighInflation;
    }

    void ApplyEconomy(const FProfile& P, int32 Seed)
    {
        // Turkey keeps the prototype's own curve (karar A06); every other pack gets a generated curve.
        if (P.Id == TEXT("tr")) MarketPrices::ClearEconomy();
        else MarketPrices::SetEconomy(P.InflationMean, P.InflationVol, P.LoanSpread, P.Character == ECharacter::Volatile || P.Character == ECharacter::HighInflation, Seed);
        // B4: the country's eras without a campaign shift (MarketEras::Activate puts the campaign's plan in).
        MarketEras::ActivateNominal(static_cast<MarketEras::ECharacter>(static_cast<uint8>(P.Character)));
    }
}

bool MarketCountry::Parse(const FString& Json, TArray<FProfile>& OutProfiles, TArray<FString>& OutErrors)
{
    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()) { OutErrors.Add(TEXT("ulkeler.json okunamadi.")); return false; }
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    if (!Root->TryGetArrayField(TEXT("countries"), Rows)) { OutErrors.Add(TEXT("ulkeler.json icinde 'countries' yok.")); return false; }
    for (const TSharedPtr<FJsonValue>& Value : *Rows)
    {
        const TSharedPtr<FJsonObject> O = Value.IsValid() ? Value->AsObject() : nullptr;
        FProfile P;
        if (!O.IsValid() || !O->TryGetStringField(TEXT("id"), P.Id) || P.Id.IsEmpty()) { OutErrors.Add(TEXT("Kimliksiz ulke atlandi.")); continue; }
        O->TryGetStringField(TEXT("name"), P.Name);
        O->TryGetStringField(TEXT("nameEn"), P.NameEn);
        O->TryGetNumberField(TEXT("displayScale"), P.DisplayScale);
        O->TryGetNumberField(TEXT("fxPerWorld"), P.FxPerWorld);
        if (!(P.DisplayScale > 0.0)) { OutErrors.Add(P.Id + TEXT(": displayScale gecersiz.")); P.DisplayScale = 1.0; }
        const TSharedPtr<FJsonObject>* Currency = nullptr;
        if (O->TryGetObjectField(TEXT("currency"), Currency) && Currency && Currency->IsValid())
        {
            (*Currency)->TryGetStringField(TEXT("code"), P.CurrencyCode);
            (*Currency)->TryGetStringField(TEXT("symbol"), P.CurrencySymbol);
            (*Currency)->TryGetStringField(TEXT("name"), P.CurrencyName);
            (*Currency)->TryGetBoolField(TEXT("symbolBefore"), P.bSymbolBefore);
            FString Mark;
            if ((*Currency)->TryGetStringField(TEXT("decimal"), Mark) && Mark.Len() == 1) P.DecimalMark = Mark[0];
        }
        const TSharedPtr<FJsonObject>* Economy = nullptr;
        if (O->TryGetObjectField(TEXT("economy"), Economy) && Economy && Economy->IsValid())
        {
            FString Character;
            if ((*Economy)->TryGetStringField(TEXT("character"), Character)) P.Character = CharacterOf(Character);
            (*Economy)->TryGetNumberField(TEXT("inflationMean"), P.InflationMean);
            (*Economy)->TryGetNumberField(TEXT("inflationVol"), P.InflationVol);
            (*Economy)->TryGetNumberField(TEXT("loanSpread"), P.LoanSpread);
            double Number = 0.0;
            if ((*Economy)->TryGetNumberField(TEXT("wageFactor"), Number)) P.WageFactor = static_cast<float>(Number);
            if ((*Economy)->TryGetNumberField(TEXT("rentFactor"), Number)) P.RentFactor = static_cast<float>(Number);
            if ((*Economy)->TryGetNumberField(TEXT("groceryPerPersonDay"), Number) && Number > 0.0) P.GroceryPerPersonDay = Number * 100.0; // B1 (#45)
            if ((*Economy)->TryGetNumberField(TEXT("employerSocialRate"), Number)) P.EmployerSocialRate = FMath::Clamp(static_cast<float>(Number), 0.f, 0.6f); // B3
            if ((*Economy)->TryGetNumberField(TEXT("severanceDaysPerYear"), Number)) P.SeveranceDaysPerYear = FMath::Clamp(FMath::RoundToInt32(Number), 0, 90);
        }
        const TSharedPtr<FJsonObject>* Habits = nullptr;
        if (O->TryGetObjectField(TEXT("habits"), Habits) && Habits && Habits->IsValid())
        {
            double Number = 0.0;
            if ((*Habits)->TryGetNumberField(TEXT("weeklyShopShare"), Number)) P.WeeklyShopShare = static_cast<float>(Number);
            if ((*Habits)->TryGetNumberField(TEXT("cardShare"), Number)) P.CardShare = static_cast<float>(Number);
            (*Habits)->TryGetBoolField(TEXT("sundayClosed"), P.bSundayClosed);
        }
        const TSharedPtr<FJsonObject>* Traditional = nullptr;
        if (O->TryGetObjectField(TEXT("traditional"), Traditional) && Traditional && Traditional->IsValid())
        {
            (*Traditional)->TryGetStringField(TEXT("grocer"), P.GrocerName);
            (*Traditional)->TryGetStringField(TEXT("market"), P.MarketName);
            (*Traditional)->TryGetNumberField(TEXT("marketWeekday"), P.MarketWeekday);
            P.MarketWeekday = FMath::Clamp(P.MarketWeekday, 0, 6);
        }
        const TSharedPtr<FJsonObject>* Chains = nullptr;
        if (O->TryGetObjectField(TEXT("chains"), Chains) && Chains && Chains->IsValid())
            for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*Chains)->Values)
                if (Pair.Value.IsValid()) P.Chains.Add(Pair.Key, Pair.Value->AsString());
        const TArray<TSharedPtr<FJsonValue>>* Holidays = nullptr;
        if (O->TryGetArrayField(TEXT("holidays"), Holidays))
            for (const TSharedPtr<FJsonValue>& H : *Holidays)
            {
                const TSharedPtr<FJsonObject> HO = H.IsValid() ? H->AsObject() : nullptr;
                if (!HO.IsValid()) continue;
                FHoliday Day;
                HO->TryGetStringField(TEXT("name"), Day.Name);
                HO->TryGetNumberField(TEXT("month"), Day.Month);
                HO->TryGetNumberField(TEXT("day"), Day.Day);
                HO->TryGetBoolField(TEXT("lunar"), Day.bLunar);
                FString Text;
                if (HO->TryGetStringField(TEXT("rule"), Text))
                    Day.Rule = Text == TEXT("easter") ? EHolidayRule::Easter : Text == TEXT("nth") ? EHolidayRule::Nth
                        : Text == TEXT("lunar") ? EHolidayRule::Lunar : EHolidayRule::Fixed;
                if (Day.bLunar) Day.Rule = EHolidayRule::Lunar;
                Day.bLunar = Day.Rule == EHolidayRule::Lunar;
                if (HO->TryGetStringField(TEXT("kind"), Text)) Day.Kind = Text == TEXT("feast") ? EHolidayKind::Feast : EHolidayKind::National;
                HO->TryGetNumberField(TEXT("days"), Day.Days);
                Day.Days = FMath::Clamp(Day.Days, 1, 4);
                HO->TryGetNumberField(TEXT("offset"), Day.Offset);
                HO->TryGetNumberField(TEXT("weekday"), Day.Weekday);
                Day.Weekday = FMath::Clamp(Day.Weekday, 0, 6);
                HO->TryGetNumberField(TEXT("n"), Day.Nth);
                Day.Nth = FMath::Clamp(Day.Nth, 1, 5);
                P.Holidays.Add(Day);
            }
        const TSharedPtr<FJsonObject>* Names = nullptr;
        if (O->TryGetObjectField(TEXT("names"), Names) && Names && Names->IsValid())
        {
            (*Names)->TryGetStringArrayField(TEXT("first"), P.FirstNames);
            (*Names)->TryGetStringArrayField(TEXT("last"), P.LastNames);
        }
        O->TryGetStringArrayField(TEXT("relatives"), P.Relatives);
        const TArray<TSharedPtr<FJsonValue>>* Cities = nullptr;
        if (O->TryGetArrayField(TEXT("provinces"), Cities) || O->TryGetArrayField(TEXT("cities"), Cities))
        {
            for (const TSharedPtr<FJsonValue>& C : *Cities)
            {
                const TSharedPtr<FJsonObject> CO = C.IsValid() ? C->AsObject() : nullptr;
                if (!CO.IsValid()) continue;
                FCity City;
                CO->TryGetStringField(TEXT("id"), City.Id);
                CO->TryGetStringField(TEXT("name"), City.Name);
                CO->TryGetNumberField(TEXT("populationK"), City.PopulationK);
                double Number = 0.0;
                // -1 = not given: Resolve derives it from the population.
                City.Income = CO->TryGetNumberField(TEXT("income"), Number) ? static_cast<float>(Number) : -1.f;
                City.Rent = CO->TryGetNumberField(TEXT("rent"), Number) ? static_cast<float>(Number) : -1.f;
                City.Competition = CO->TryGetNumberField(TEXT("competition"), Number) ? static_cast<float>(Number) : -1.f;
                // G-089: an optional map position (depot distances).
                double MapX = 0.0, MapY = 0.0;
                if (CO->TryGetNumberField(TEXT("cx"), MapX) && CO->TryGetNumberField(TEXT("cy"), MapY))
                {
                    City.MapX = static_cast<float>(MapX);
                    City.MapY = static_cast<float>(MapY);
                    City.bOnMap = true;
                }
                if (!City.Id.IsEmpty()) P.Cities.Add(City);
            }
        }
        else if (!O->TryGetStringField(TEXT("provinces"), P.CitiesFile)) O->TryGetStringField(TEXT("cities"), P.CitiesFile);
        O->TryGetStringField(TEXT("map"), P.CitiesFile);
        O->TryGetNumberField(TEXT("referencePopK"), P.ReferencePopK);
        O->TryGetStringField(TEXT("referenceProvince"), P.ReferenceProvince);
        double MapKm = 0.0;
        if (O->TryGetNumberField(TEXT("kmPerMapUnit"), MapKm) && MapKm > 0.0) P.MapKm = static_cast<float>(MapKm);
        auto ReadRegions = [&O](const TCHAR* Field, TArray<FRegion>& Out)
        {
            const TArray<TSharedPtr<FJsonValue>>* List = nullptr;
            if (!O->TryGetArrayField(Field, List)) return;
            for (const TSharedPtr<FJsonValue>& R : *List)
            {
                const TSharedPtr<FJsonObject> RO = R.IsValid() ? R->AsObject() : nullptr;
                if (!RO.IsValid()) continue;
                FRegion Region;
                RO->TryGetStringField(TEXT("id"), Region.Id);
                RO->TryGetStringField(TEXT("name"), Region.Name);
                RO->TryGetStringField(TEXT("region"), Region.Parent);
                double Income = 1.0;
                if (RO->TryGetNumberField(TEXT("income"), Income)) Region.Income = static_cast<float>(Income);
                RO->TryGetStringArrayField(TEXT("provinces"), Region.Provinces);
                if (!Region.Id.IsEmpty()) Out.Add(Region);
            }
        };
        ReadRegions(TEXT("regions"), P.Regions);
        ReadRegions(TEXT("subregions"), P.SubRegions);
        Resolve(P);
        OutProfiles.Add(P);
    }
    return OutProfiles.Num() > 0;
}

const TArray<MarketCountry::FProfile>& MarketCountry::All()
{
    static TArray<FProfile> Profiles;
    static bool bLoaded = false;
    if (!bLoaded)
    {
        bLoaded = true;
        FString Json;
        TArray<FString> Errors;
        if (FFileHelper::LoadFileToString(Json, *FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("ulkeler.json")))) Parse(Json, Profiles, Errors);
        for (const FString& Error : Errors) UE_LOG(LogTemp, Warning, TEXT("MirasMarket countries: %s"), *Error);
        if (!Profiles.ContainsByPredicate([](const FProfile& P) { return P.Id == TEXT("tr"); })) Profiles.Insert(FProfile(), 0);
    }
    return Profiles;
}

const MarketCountry::FProfile* MarketCountry::Find(const FString& Id)
{
    return All().FindByPredicate([&Id](const FProfile& P) { return P.Id == Id; });
}

void MarketCountry::Resolve(FProfile& P)
{
    // Provinces from the map file (Turkey): names, population and the chain density of iller.json.
    if (P.Cities.Num() == 0 && P.CitiesFile == TEXT("iller.json"))
    {
        for (const MarketMapData::FProvince& Map : MarketMapData::Get().Provinces)
        {
            FCity City;
            City.Id = Map.Id;
            City.Name = Map.Name;
            City.PopulationK = Map.PopulationK;
            City.Income = City.Rent = City.Competition = -1.f;
            int32 Chains = 0;
            for (const TPair<FString, int32>& Pair : Map.Stores) Chains += Pair.Value;
            City.ChainDensity = Map.PopulationK > 0 ? Chains * 100.f / Map.PopulationK : -1.f;
            City.MapX = Map.Center.X; // G-089: depot distances
            City.MapY = Map.Center.Y;
            City.bOnMap = true;
            P.Cities.Add(City);
        }
    }
    // Regions of every province.
    for (FCity& City : P.Cities)
        for (const FRegion& Sub : P.SubRegions)
            if (Sub.Provinces.Contains(City.Id)) { City.SubRegion = Sub.Id; City.Region = Sub.Parent; break; }
    // The reference point (1.0) and the derived values.
    float RefPop = static_cast<float>(FMath::Max(1, P.ReferencePopK));
    float RefDensity = -1.f;
    if (const FCity* Ref = P.Cities.FindByPredicate([&P](const FCity& C) { return C.Id == P.ReferenceProvince; }))
    {
        RefPop = static_cast<float>(FMath::Max(1, Ref->PopulationK));
        RefDensity = Ref->ChainDensity;
    }
    for (FCity& City : P.Cities)
    {
        const FRegion* Sub = P.SubRegions.FindByPredicate([&City](const FRegion& R) { return R.Id == City.SubRegion; });
        const float S = FMath::Clamp(FMath::Log2(FMath::Max(1.f, static_cast<float>(City.PopulationK)) / RefPop), -3.f, 6.f);
        if (City.Income <= 0.f) City.Income = FMath::Clamp((Sub ? Sub->Income : 1.f) * (1.f + 0.03f * S), 0.55f, 1.6f);
        if (City.Rent <= 0.f) City.Rent = FMath::Clamp(FMath::Pow(City.Income, 1.5f) * (1.f + 0.1f * S), 0.5f, 1.9f);
        if (City.Competition <= 0.f)
        {
            float Crowd = 1.f + 0.08f * S;
            if (City.ChainDensity >= 0.f && RefDensity > 0.f) Crowd *= 0.5f + 0.5f * City.ChainDensity / RefDensity;
            City.Competition = FMath::Clamp(Crowd, 0.6f, 1.5f);
        }
    }
}

const MarketCountry::FRegion* MarketCountry::SubRegionOf(const FString& CountryId, const FString& ProvinceId)
{
    const FProfile* Country = Find(CountryId);
    const FCity* City = FindCity(CountryId, ProvinceId);
    return Country && City ? Country->SubRegions.FindByPredicate([City](const FRegion& R) { return R.Id == City->SubRegion; }) : nullptr;
}

const MarketCountry::FRegion* MarketCountry::RegionOf(const FString& CountryId, const FString& ProvinceId)
{
    const FProfile* Country = Find(CountryId);
    const FCity* City = FindCity(CountryId, ProvinceId);
    return Country && City ? Country->Regions.FindByPredicate([City](const FRegion& R) { return R.Id == City->Region; }) : nullptr;
}

int32 MarketCountry::PopulationK(const FString& CountryId)
{
    const FProfile* Country = Find(CountryId);
    int32 Sum = 0;
    if (Country) for (const FCity& City : Country->Cities) Sum += City.PopulationK;
    return Sum;
}

const MarketCountry::FCity* MarketCountry::FindCity(const FString& CountryId, const FString& CityId)
{
    const FProfile* Country = Find(CountryId);
    return Country ? Country->Cities.FindByPredicate([&CityId](const FCity& C) { return C.Id == CityId; }) : nullptr;
}

float MarketCountry::CityCompetition(const FString& CountryId, const FString& CityId)
{
    const FCity* City = CityId.IsEmpty() ? nullptr : FindCity(CountryId, CityId);
    return City ? FMath::Clamp(City->Competition, 0.5f, 2.f) : 1.f;
}

float MarketCountry::CityIncome(const FString& CountryId, const FString& CityId)
{
    const FCity* City = CityId.IsEmpty() ? nullptr : FindCity(CountryId, CityId);
    return City ? FMath::Clamp(City->Income, 0.5f, 2.f) : 1.f;
}

const MarketCountry::FProfile& MarketCountry::Active()
{
    return ActiveRef();
}

void MarketCountry::SetActive(const FString& Id, int32 Seed)
{
    const FProfile* Found = Find(Id.IsEmpty() ? FString(TEXT("tr")) : Id);
    SetActiveProfile(Found ? *Found : FProfile(), Seed);
}

void MarketCountry::SetActiveProfile(const FProfile& Profile, int32 Seed)
{
    ActiveRef() = Profile;
    ApplyEconomy(Profile, Seed);
}

FString MarketCountry::Decorate(const FString& Number)
{
    const FProfile& P = Active();
    FString Text = Number;
    if (P.DecimalMark == TEXT('.'))
    {
        // Turkish style 1.234,50 -> 1,234.50
        Text.ReplaceCharInline(TEXT('.'), TEXT('\x01'));
        Text.ReplaceCharInline(TEXT(','), TEXT('.'));
        Text.ReplaceCharInline(TEXT('\x01'), TEXT(','));
    }
    if (!P.bSymbolBefore) return Text + TEXT(" ") + P.CurrencySymbol;
    return Text.StartsWith(TEXT("-")) ? TEXT("-") + P.CurrencySymbol + Text.Mid(1) : P.CurrencySymbol + Text;
}

FString MarketCountry::Money(int64 Internal)
{
    const double Scale = Active().DisplayScale;
    const int64 Local = FMath::IsNearlyEqual(Scale, 1.0) ? Internal : FMath::RoundToInt64(static_cast<double>(Internal) * Scale);
    const int64 Abs = Local < 0 ? -Local : Local;
    FString Whole = FString::Printf(TEXT("%lld"), static_cast<long long>(Abs / 100));
    for (int32 I = Whole.Len() - 3; I > 0; I -= 3) Whole.InsertAt(I, TEXT('.'));
    return Decorate(FString::Printf(TEXT("%s%s,%02lld"), Local < 0 ? TEXT("-") : TEXT(""), *Whole, static_cast<long long>(Abs % 100)));
}

FString MarketCountry::ChainName(const FString& Archetype)
{
    const FString* Name = Active().Chains.Find(Archetype);
    return Name ? *Name : FString();
}
