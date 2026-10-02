#include "MarketTuning.h"

namespace MarketTuningLocal
{
    TMap<FString, float>& Knobs()
    {
        static TMap<FString, float> Map;
        return Map;
    }
}

float MarketTuning::Get(const TCHAR* Key, float Default)
{
    const float* Found = MarketTuningLocal::Knobs().Find(Key);
    return Found ? *Found : Default;
}

void MarketTuning::Set(const FString& Key, float Value)
{
    MarketTuningLocal::Knobs().Add(Key.TrimStartAndEnd(), Value);
}

void MarketTuning::Reset()
{
    MarketTuningLocal::Knobs().Reset();
}

int32 MarketTuning::Apply(const FString& List)
{
    TArray<FString> Parts;
    List.ParseIntoArray(Parts, TEXT(","), true);
    int32 Applied = 0;
    for (const FString& Part : Parts)
    {
        FString Key, Value;
        if (!Part.Split(TEXT("="), &Key, &Value) || Key.TrimStartAndEnd().IsEmpty()) continue;
        Set(Key, FCString::Atof(*Value.TrimStartAndEnd()));
        ++Applied;
    }
    return Applied;
}

FString MarketTuning::Describe()
{
    TArray<FString> Lines;
    for (const TPair<FString, float>& Pair : MarketTuningLocal::Knobs()) Lines.Add(FString::Printf(TEXT("%s=%g"), *Pair.Key, Pair.Value));
    Lines.Sort();
    return FString::Join(Lines, TEXT(", "));
}
