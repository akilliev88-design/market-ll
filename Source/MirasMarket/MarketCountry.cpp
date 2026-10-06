#include "MarketCountry.h"
#include "MarketCalendar.h"
#include "MarketEconomy.h"
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
    FString& DefaultIdRef()
    {
        static FString Id = TEXT("tr");
        return Id;
    }

    TArray<FGiantRow>& GiantsRef()
    {
        static TArray<FGiantRow> Rows;
        return Rows;
    }

    // D4: continent names of the pack file (root "continents": id -> name).
    TMap<FString, FString>& ContinentNamesRef()
    {
        static TMap<FString, FString> Names;
        return Names;
    }

    FProfile& ActiveRef()
    {
        // D3: until a campaign sets its country the default pack is active (the Turkish rules are its data).
        static FProfile Profile = Default();
        return Profile;
    }

    int32& ActiveSeedRef()
    {
        static int32 Seed = 0;
        return Seed;
    }

    ECharacter CharacterOf(const FString& Text)
    {
        if (Text == TEXT("istikrarli") || Text == TEXT("stable")) return ECharacter::Stable;
        if (Text == TEXT("oynak") || Text == TEXT("volatile")) return ECharacter::Volatile;
        return ECharacter::HighInflation;
    }

    void ApplyEconomy(const FProfile& P, int32 Seed)
    {
        // A pack with a hand-made curve (Turkey; Y1: pack data) uses it (karar A06); every other pack gets a generated
        // curve.
        if (P.bBuiltinCurve) MarketPrices::ClearEconomy();
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
        O->TryGetStringField(TEXT("continent"), P.Continent);
        const TSharedPtr<FJsonObject>* World = nullptr;
        if (O->TryGetObjectField(TEXT("world"), World))
        {
            double X = -1.0, Y = -1.0;
            if ((*World)->TryGetNumberField(TEXT("x"), X) && (*World)->TryGetNumberField(TEXT("y"), Y)) { P.WorldX = static_cast<float>(X); P.WorldY = static_cast<float>(Y); }
            (*World)->TryGetStringField(TEXT("label"), P.WorldLabel);
        }
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
        P.bBuiltinCurve = false; // D3: only a pack that asks for it keeps the hand-made curve
        const TSharedPtr<FJsonObject>* Economy = nullptr;
        if (O->TryGetObjectField(TEXT("economy"), Economy) && Economy && Economy->IsValid())
        {
            FString Character;
            if ((*Economy)->TryGetStringField(TEXT("character"), Character)) P.Character = CharacterOf(Character);
            (*Economy)->TryGetNumberField(TEXT("inflationMean"), P.InflationMean);
            (*Economy)->TryGetNumberField(TEXT("inflationVol"), P.InflationVol);
            (*Economy)->TryGetNumberField(TEXT("loanSpread"), P.LoanSpread);
            // Y1 (M60): the hand-made curve, [inflation, loan] per campaign year.
            const TArray<TSharedPtr<FJsonValue>>* Curve = nullptr;
            if ((*Economy)->TryGetArrayField(TEXT("curve"), Curve))
                for (const TSharedPtr<FJsonValue>& Year : *Curve)
                {
                    const TArray<TSharedPtr<FJsonValue>>* Pair = nullptr;
                    if (Year.IsValid() && Year->TryGetArray(Pair) && Pair->Num() == 2)
                    {
                        P.CurveInflation.Add((*Pair)[0]->AsNumber());
                        P.CurveLoan.Add((*Pair)[1]->AsNumber());
                    }
                }
            const TArray<TSharedPtr<FJsonValue>>* After = nullptr;
            if ((*Economy)->TryGetArrayField(TEXT("curveAfter"), After) && After->Num() == 2)
            {
                P.CurveAfterInflation = (*After)[0]->AsNumber();
                P.CurveAfterLoan = (*After)[1]->AsNumber();
            }
            P.bBuiltinCurve = P.CurveInflation.Num() > 0;
            double Number = 0.0;
            if ((*Economy)->TryGetNumberField(TEXT("wageFactor"), Number)) P.WageFactor = static_cast<float>(Number);
            if ((*Economy)->TryGetNumberField(TEXT("rentFactor"), Number)) P.RentFactor = static_cast<float>(Number);
            if ((*Economy)->TryGetNumberField(TEXT("groceryPerPersonDay"), Number) && Number > 0.0) P.GroceryPerPersonDay = Number * 100.0; // B1 (#45)
            if ((*Economy)->TryGetNumberField(TEXT("employerSocialRate"), Number)) P.EmployerSocialRate = FMath::Clamp(static_cast<float>(Number), 0.f, 0.6f); // B3
            if ((*Economy)->TryGetNumberField(TEXT("severanceDaysPerYear"), Number)) P.SeveranceDaysPerYear = FMath::Clamp(FMath::RoundToInt32(Number), 0, 90);
        }
        const TSharedPtr<FJsonObject>* Company = nullptr;
        if (O->TryGetObjectField(TEXT("company"), Company) && Company && Company->IsValid()) // M65
        {
            (*Company)->TryGetStringArrayField(TEXT("legalForms"), P.LegalForms);
            double Number = 0.0;
            if ((*Company)->TryGetNumberField(TEXT("setupCost"), Number) && Number > 0.0) P.SetupCost = Number * 100.0;
            if ((*Company)->TryGetNumberField(TEXT("dividendWithholding"), Number)) P.DividendWithholding = FMath::Clamp(static_cast<float>(Number), 0.f, 0.5f);
        }
        const TSharedPtr<FJsonObject>* Habits = nullptr;
        if (O->TryGetObjectField(TEXT("habits"), Habits) && Habits && Habits->IsValid())
        {
            double Number = 0.0;
            if ((*Habits)->TryGetNumberField(TEXT("weeklyShopShare"), Number)) P.WeeklyShopShare = static_cast<float>(Number);
            if ((*Habits)->TryGetNumberField(TEXT("cardShare"), Number)) P.CardShare = static_cast<float>(Number);
            (*Habits)->TryGetBoolField(TEXT("sundayClosed"), P.bSundayClosed);
            const TArray<TSharedPtr<FJsonValue>>* Cards = nullptr; // D3
            if ((*Habits)->TryGetArrayField(TEXT("cardShareByYear"), Cards))
                for (const TSharedPtr<FJsonValue>& V : *Cards) if (V.IsValid()) P.CardShareByYear.Add(FMath::Clamp(static_cast<float>(V->AsNumber()), 0.f, 1.f));
        }
        const TSharedPtr<FJsonObject>* Calendar = nullptr; // D3
        if (O->TryGetObjectField(TEXT("calendar"), Calendar) && Calendar && Calendar->IsValid())
        {
            (*Calendar)->TryGetBoolField(TEXT("showHolidayNames"), P.bShowHolidayNames);
            const TSharedPtr<FJsonObject>* School = nullptr;
            if ((*Calendar)->TryGetObjectField(TEXT("school"), School) && School && School->IsValid())
            {
                auto ReadDay = [&School](const TCHAR* Field, FSchoolDay& Out)
                {
                    const TSharedPtr<FJsonObject>* Row = nullptr;
                    if (!(*School)->TryGetObjectField(Field, Row) || !Row || !Row->IsValid()) return;
                    (*Row)->TryGetNumberField(TEXT("month"), Out.Month);
                    (*Row)->TryGetNumberField(TEXT("weekday"), Out.Weekday);
                    (*Row)->TryGetNumberField(TEXT("n"), Out.Nth);
                    Out.Month = FMath::Clamp(Out.Month, 0, 12);
                    Out.Weekday = FMath::Clamp(Out.Weekday, 0, 6);
                    Out.Nth = FMath::Clamp(Out.Nth, 1, 5);
                };
                ReadDay(TEXT("start"), P.SchoolStart);
                ReadDay(TEXT("end"), P.SchoolEnd);
            }
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
                HO->TryGetStringField(TEXT("table"), Day.Table); // D3
                HO->TryGetNumberField(TEXT("fastDays"), Day.FastDays);
                Day.FastDays = FMath::Clamp(Day.FastDays, 0, 40);
                HO->TryGetBoolField(TEXT("butcherPeak"), Day.bButcherPeak);
                if (Day.Rule == EHolidayRule::Lunar && Day.Table.IsEmpty()) OutErrors.Add(P.Id + TEXT(": ") + Day.Name + TEXT(" ay takvimi tablosu yok."));
                P.Holidays.Add(Day);
            }
        const TSharedPtr<FJsonObject>* Names = nullptr;
        if (O->TryGetObjectField(TEXT("names"), Names) && Names && Names->IsValid())
        {
            (*Names)->TryGetStringArrayField(TEXT("first"), P.FirstNames);
            (*Names)->TryGetStringArrayField(TEXT("last"), P.LastNames);
            const TSharedPtr<FJsonObject>* Pool = nullptr; // D3
            if ((*Names)->TryGetObjectField(TEXT("staff"), Pool) && Pool && Pool->IsValid())
            {
                (*Pool)->TryGetStringArrayField(TEXT("first"), P.StaffFirst);
                (*Pool)->TryGetStringArrayField(TEXT("last"), P.StaffLast);
            }
            if ((*Names)->TryGetObjectField(TEXT("managers"), Pool) && Pool && Pool->IsValid())
            {
                (*Pool)->TryGetStringArrayField(TEXT("first"), P.ManagerFirst);
                (*Pool)->TryGetStringArrayField(TEXT("last"), P.ManagerLast);
            }
        }
        O->TryGetBoolField(TEXT("realChainNames"), P.bRealChainNames); // D3
        O->TryGetStringArrayField(TEXT("regionalSuffixes"), P.RegionalSuffixes);
        O->TryGetStringArrayField(TEXT("localSuffixes"), P.LocalSuffixes); // E2
        O->TryGetStringArrayField(TEXT("playerFormats"), P.PlayerFormats); // M54
        const TSharedPtr<FJsonObject>* Firm = nullptr;
        if (O->TryGetObjectField(TEXT("firmWords"), Firm) && Firm && Firm->IsValid())
        {
            (*Firm)->TryGetStringField(TEXT("wholesale"), P.WholesaleWord);
            (*Firm)->TryGetStringField(TEXT("cashCarry"), P.CashCarryWord);
        }
        const TArray<TSharedPtr<FJsonValue>>* Roster = nullptr;
        if (O->TryGetArrayField(TEXT("roster"), Roster))
            for (const TSharedPtr<FJsonValue>& R : *Roster)
            {
                const TSharedPtr<FJsonObject> RO = R.IsValid() ? R->AsObject() : nullptr;
                if (!RO.IsValid()) continue;
                FRosterRow Row;
                RO->TryGetStringField(TEXT("id"), Row.Id);
                RO->TryGetStringField(TEXT("name"), Row.Name);
                RO->TryGetStringField(TEXT("boss"), Row.Boss);
                RO->TryGetStringField(TEXT("archetype"), Row.Archetype);
                RO->TryGetNumberField(TEXT("stores"), Row.Stores);
                double Number = 0.0;
                if (RO->TryGetNumberField(TEXT("price"), Number)) Row.PriceIndex = static_cast<float>(Number);
                if (RO->TryGetNumberField(TEXT("service"), Number)) Row.Service = static_cast<float>(Number);
                if (RO->TryGetNumberField(TEXT("aggression"), Number)) Row.Aggression = static_cast<float>(Number);
                if (RO->TryGetNumberField(TEXT("ambition"), Number)) Row.Ambition = static_cast<float>(Number);
                RO->TryGetStringField(TEXT("region"), Row.HomeRegion);
                if (Row.Id.IsEmpty() || Row.Name.IsEmpty() || Row.Stores <= 0) { OutErrors.Add(P.Id + TEXT(": eksik zincir satiri atlandi.")); continue; }
                P.Roster.Add(Row);
            }
        O->TryGetStringArrayField(TEXT("banks"), P.Banks); // M30
        const TSharedPtr<FJsonObject>* Online = nullptr;
        if (O->TryGetObjectField(TEXT("online"), Online) && Online && Online->IsValid()) // M32
        {
            (*Online)->TryGetStringField(TEXT("platform"), P.PlatformName);
            double Plateau = 0.0;
            if ((*Online)->TryGetNumberField(TEXT("plateau"), Plateau)) P.OnlinePlateau = FMath::Clamp(static_cast<float>(Plateau), 0.01f, 0.4f);
        }
        const TSharedPtr<FJsonObject>* Climate = nullptr;
        if (O->TryGetObjectField(TEXT("climate"), Climate) && Climate && Climate->IsValid()) // M35
        {
            const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
            if ((*Climate)->TryGetArrayField(TEXT("temperature"), Values) && Values->Num() == 12) for (const TSharedPtr<FJsonValue>& V : *Values) P.ClimateTemperature.Add(FMath::RoundToInt32(V->AsNumber()));
            if ((*Climate)->TryGetArrayField(TEXT("rain"), Values) && Values->Num() == 12) for (const TSharedPtr<FJsonValue>& V : *Values) P.ClimateRain.Add(FMath::Clamp(FMath::RoundToInt32(V->AsNumber()), 0, 100));
        }
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
        if (FFileHelper::LoadFileToString(Json, *FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("ulkeler.json"))))
        {
            Parse(Json, Profiles, Errors);
            ParseGiants(Json, GiantsRef(), Errors);
            // D3: the default country of the file.
            TSharedPtr<FJsonObject> Root;
            const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
            FString Id;
            if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
            {
                if (Root->TryGetStringField(TEXT("defaultCountry"), Id) && !Id.IsEmpty()) DefaultIdRef() = Id;
                const TArray<TSharedPtr<FJsonValue>>* Continents = nullptr;
                if (Root->TryGetArrayField(TEXT("continents"), Continents))
                    for (const TSharedPtr<FJsonValue>& Value : *Continents)
                    {
                        const TSharedPtr<FJsonObject> O = Value.IsValid() ? Value->AsObject() : nullptr;
                        FString Key, Name;
                        if (O.IsValid() && O->TryGetStringField(TEXT("id"), Key) && O->TryGetStringField(TEXT("name"), Name) && !Key.IsEmpty()) ContinentNamesRef().Add(Key, Name);
                    }
            }
        }
        for (const FString& Error : Errors) UE_LOG(LogTemp, Warning, TEXT("MirasMarket countries: %s"), *Error);
        if (!Profiles.ContainsByPredicate([](const FProfile& P) { return P.Id == DefaultIdRef(); }))
        {
            FProfile Bare; // no pack for the default country: the bare prototype (built-in curve, no holidays)
            Bare.Id = DefaultIdRef();
            Profiles.Insert(Bare, 0);
        }
    }
    return Profiles;
}

bool MarketCountry::ParseGiants(const FString& Json, TArray<FGiantRow>& OutGiants, TArray<FString>& OutErrors)
{
    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid() || !Root->TryGetArrayField(TEXT("giants"), Rows)) return false;
    for (const TSharedPtr<FJsonValue>& Value : *Rows)
    {
        const TSharedPtr<FJsonObject> O = Value.IsValid() ? Value->AsObject() : nullptr;
        FGiantRow Row;
        if (!O.IsValid() || !O->TryGetStringField(TEXT("id"), Row.Id) || Row.Id.IsEmpty()) { OutErrors.Add(TEXT("Kimliksiz dev zincir atlandi.")); continue; }
        O->TryGetStringField(TEXT("name"), Row.Name);
        O->TryGetStringField(TEXT("home"), Row.Home);
        O->TryGetStringField(TEXT("pack"), Row.Pack);
        O->TryGetStringField(TEXT("archetype"), Row.Archetype);
        double Number = 0.0;
        if (O->TryGetNumberField(TEXT("revenueB"), Number)) Row.RevenueB = static_cast<float>(Number);
        if (O->TryGetNumberField(TEXT("growth"), Number)) Row.Growth = static_cast<float>(Number);
        OutGiants.Add(Row);
    }
    return OutGiants.Num() > 0;
}

const TArray<MarketCountry::FGiantRow>& MarketCountry::Giants()
{
    All();
    return GiantsRef();
}

TArray<FString> MarketCountry::Continents()
{
    TArray<FString> List;
    for (const FProfile& P : All()) if (!P.Continent.IsEmpty()) List.AddUnique(P.Continent);
    return List;
}

FString MarketCountry::ContinentOf(const FString& Country)
{
    const FProfile* Pack = Find(Country);
    return Pack ? Pack->Continent : FString();
}

FString MarketCountry::ContinentName(const FString& Id)
{
    All();
    if (const FString* Name = ContinentNamesRef().Find(Id)) return *Name;
    return Id.IsEmpty() ? Id : Id.Left(1).ToUpper() + Id.Mid(1);
}

const MarketCountry::FProfile* MarketCountry::Find(const FString& Id)
{
    return All().FindByPredicate([&Id](const FProfile& P) { return P.Id == Id; });
}

const FString& MarketCountry::DefaultId()
{
    All();
    return DefaultIdRef();
}

const MarketCountry::FProfile& MarketCountry::Default()
{
    const FProfile* Pack = Find(DefaultId());
    check(Pack); // All() always holds the default country
    return *Pack;
}

const MarketCountry::FProfile& MarketCountry::FindOrDefault(const FString& Id)
{
    const FProfile* Pack = Id.IsEmpty() ? nullptr : Find(Id);
    return Pack ? *Pack : Default();
}

const FString& MarketCountry::MapCountry()
{
    // D3: the pack whose provinces come from the map file (iller.json) is the one drawn on the main screen.
    static FString Id;
    static bool bFound = false;
    if (!bFound)
    {
        bFound = true;
        for (const FProfile& P : All()) if (P.CitiesFile == TEXT("iller.json")) { Id = P.Id; break; }
    }
    return Id;
}

bool MarketCountry::HasMap(const FString& Id)
{
    return !Id.IsEmpty() && Id == MapCountry() && MarketMapData::Get().bLoaded;
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
    // M61b: the reference point (1.0) is the country's median province, no province is named.
    if (P.Cities.Num() > 0)
    {
        TArray<const FCity*> ByPop;
        for (const FCity& City : P.Cities) ByPop.Add(&City);
        ByPop.Sort([](const FCity& A, const FCity& B) { return A.PopulationK == B.PopulationK ? A.Id < B.Id : A.PopulationK < B.PopulationK; });
        const FCity* Middle = ByPop[(ByPop.Num() - 1) / 2];
        P.MedianPopK = FMath::Max(1, Middle->PopulationK);
        P.MedianProvince = Middle->Id;
        TArray<float> Densities;
        for (const FCity& City : P.Cities) if (City.ChainDensity >= 0.f) Densities.Add(City.ChainDensity);
        Densities.Sort();
        P.MedianDensity = Densities.Num() > 0 ? Densities[(Densities.Num() - 1) / 2] : -1.f;
    }
    const float RefPop = static_cast<float>(FMath::Max(1, P.MedianPopK));
    const float RefDensity = P.MedianDensity;
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

int32 MarketCountry::MedianPopK(const FString& CountryId)
{
    const FProfile* Country = Find(CountryId);
    return Country ? FMath::Max(1, Country->MedianPopK) : 500;
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
    SetActiveProfile(FindOrDefault(Id), Seed);
}

void MarketCountry::SetActiveProfile(const FProfile& Profile, int32 Seed)
{
    ActiveRef() = Profile;
    ActiveSeedRef() = Seed;
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

namespace MarketCountry
{
    // B5 (L08): the currencies' random walk and world inflation.
    constexpr double WorldInflation = 0.025;

    double FxWobble(int32 Seed, uint32 A, uint32 B)
    {
        uint32 Hash = 2166136261u;
        const uint32 Parts[3] = { static_cast<uint32>(Seed), A, B };
        for (uint32 Part : Parts)
            for (int32 Byte = 0; Byte < 4; ++Byte) { Hash ^= (Part >> (Byte * 8)) & 0xFFu; Hash *= 16777619u; }
        return ((Hash & 0xFFFFu) / 65535.0 + (Hash >> 16) / 65535.0) - 1.0;
    }

    uint32 FxCountryKey(const FString& Id)
    {
        uint32 Hash = 5381u;
        for (const TCHAR C : Id) Hash = Hash * 33u + static_cast<uint32>(C);
        return Hash;
    }

    double FxWalkSize(ECharacter Character)
    {
        switch (Character)
        {
        case ECharacter::Stable: return 0.03;
        case ECharacter::Volatile: return 0.08;
        default: return 0.06;
        }
    }
}

double MarketCountry::FxRate(const FMarketState& State, const FString& CountryId, int32 GameDay)
{
    const FString Id = CountryId.IsEmpty() ? State.CountryId : CountryId;
    const FProfile* Pack = Find(Id);
    const FProfile& P = Pack ? *Pack : Active();
    const bool bOwn = Id == State.CountryId;
    double Log = FMath::Loge(FMath::Max(0.0001, P.FxPerWorld));
    const int32 Day = FMath::Max(1, GameDay);
    const int32 FirstYear = MarketCalendar::DateOf(1).Year;
    const int32 LastYearOfDay = MarketCalendar::DateOf(Day).Year;
    for (int32 Year = FirstYear; Year <= LastYearOfDay; ++Year)
    {
        const int32 From = FMath::Max(1, MarketCalendar::GameDayOf(Year, 1, 1));
        const int32 To = FMath::Min(Day, MarketCalendar::GameDayOf(Year + 1, 1, 1));
        if (To <= From) continue;
        const double Part = (To - From) / 365.0;
        // Own country: the campaign's price curve (eras included); others: their own economy (E1, MarketPrices).
        const double Inflation = bOwn ? MarketPrices::YearlyInflation(Year) : MarketPrices::YearlyInflation(Id, Year);
        Log += (FMath::Loge(1.0 + Inflation) - FMath::Loge(1.0 + WorldInflation)) * Part;
        Log += FxWalkSize(P.Character) * FxWobble(State.RivalSeed, FxCountryKey(Id), static_cast<uint32>(Year)) * Part;
    }
    if (bOwn)
        for (const MarketEras::FEra& E : MarketEras::PlanOf(State))
            if (E.Kind == MarketEras::EKind::CurrencyShock && Day > E.StartDay)
                Log += 0.25 * E.Strength * FMath::Min(1.0, (Day - E.StartDay) / 10.0); // the jump takes about ten days
    return FMath::Exp(Log);
}

int64 MarketCountry::ToWorld(const FMarketState& State, const FString& CountryId, int64 Internal, int32 GameDay)
{
    const FString Id = CountryId.IsEmpty() ? State.CountryId : CountryId;
    const FProfile* Pack = Find(Id);
    const double Scale = Pack ? Pack->DisplayScale : 1.0;
    return FMath::RoundToInt64(static_cast<double>(Internal) * Scale / FMath::Max(0.0001, FxRate(State, Id, GameDay)));
}

int32 MarketCountry::ActiveSeed()
{
    return ActiveSeedRef();
}
