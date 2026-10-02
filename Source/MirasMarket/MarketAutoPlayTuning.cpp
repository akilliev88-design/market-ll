#include "MarketAutoPlay.h"
#include "MarketTuning.h"
#include "String/LexFromString.h"

TArray<MarketAutoPlay::FProfile> MarketAutoPlay::TunedProfiles()
{
    TArray<FProfile> Values=Profiles();
    Values[0].ExpansionBuffer=MarketTuning::Get(TEXT("OpenBuffer.Careful"),static_cast<float>(Values[0].ExpansionBuffer));
    Values[1].ExpansionBuffer=MarketTuning::Get(TEXT("OpenBuffer.Balanced"),static_cast<float>(Values[1].ExpansionBuffer));
    return Values;
}

bool MarketAutoPlay::ConfigureTuning(const FString& List,FString& Error)
{
    // Each commandlet starts from defaults. Validate every knob before applying any of them.
    MarketTuning::Reset(); Error.Reset();
    if(List.TrimStartAndEnd().IsEmpty())return true;
    TArray<FString> Parts; List.ParseIntoArray(Parts,TEXT(","),false);
    TSet<FString> Seen;
    for(const auto& Part:Parts)
    {
        FString Key,Text; float Value=0;
        if(!Part.Split(TEXT("="),&Key,&Text)) { Error=TEXT("Tune must contain Key=Value pairs"); return false; }
        Key=Key.TrimStartAndEnd(); Text=Text.TrimStartAndEnd();
        if(Seen.Contains(Key) || !LexTryParseString(Value,*Text) || !FMath::IsFinite(Value))
        { Error=TEXT("Duplicate or invalid Tune value: ")+Part; return false; }
        float Low=0,High=0;
        if(Key==TEXT("BranchCompetition")) {Low=.1f;High=10.f;}
        else if(Key==TEXT("RealWageGrowth")) {Low=0.f;High=.1f;}
        else if(Key==TEXT("RealSpend")) {Low=0.f;High=2.f;}
        else if(Key==TEXT("OpenBuffer.Balanced") || Key==TEXT("OpenBuffer.Careful")) {Low=.1f;High=5.f;}
        else if(Key==TEXT("LossMonthsToClose"))
        {Low=1.f;High=12.f;if(Value!=FMath::RoundToInt(Value)){Error=TEXT("LossMonthsToClose must be an integer");return false;}}
        else {Error=TEXT("Unknown Tune key: ")+Key;return false;}
        if(Value<Low || Value>High) {Error=TEXT("Tune outside supported range: ")+Part;return false;}
        Seen.Add(Key);
    }
    MarketTuning::Apply(List);
    return true;
}
