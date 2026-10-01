#include "MirasAutoPlayCommandlet.h"
#include "MarketAutoPlay.h"
#include "MarketCalendar.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

UMirasAutoPlayCommandlet::UMirasAutoPlayCommandlet()
{
    IsClient = false; IsServer = false; IsEditor = true;
    LogToConsole = true; ShowErrorCount = true;
}
int32 UMirasAutoPlayCommandlet::Main(const FString& Params)
{
    MarketAutoPlay::FOptions Options;
    int32 Years = 10;
    FParse::Value(*Params, TEXT("Years="), Years);
    FParse::Value(*Params, TEXT("Seeds="), Options.Seeds);
    FParse::Value(*Params, TEXT("Seed="), Options.FirstSeed);
    FParse::Value(*Params, TEXT("Country="), Options.Country);
    FParse::Value(*Params, TEXT("Province="), Options.Province);
    FParse::Value(*Params, TEXT("Style="), Options.StyleIndex);
    if (Years < 1 || Years > 30) { UE_LOG(LogTemp, Error, TEXT("Years must be 1..30")); return 1; }
    Options.Days = MarketCalendar::GameDayOf(MarketCalendar::StartYear + Years, MarketCalendar::StartMonth, MarketCalendar::StartDayOfMonth) - 1;
    FParse::Value(*Params, TEXT("Days="), Options.Days);
    TArray<FMarketProduct> Base; TArray<int32> Capacities; TArray<FString> Errors;
    if (!MarketAutoPlay::LoadInputs(Base, Capacities, Errors))
    { for (const FString& Error : Errors) UE_LOG(LogTemp, Error, TEXT("%s"), *Error); return 1; }
    const MarketAutoPlay::FReport Report = MarketAutoPlay::Run(Options, Base, Capacities);
    const FString Directory = FPaths::ProjectSavedDir() / TEXT("AutoPlay") / FDateTime::Now().ToString(TEXT("%Y%m%d-%H%M%S"));
    if (!MarketAutoPlay::WriteReport(Report, Directory)) return 1;
    UE_LOG(LogTemp, Display, TEXT("AutoPlay report: %s"), *Directory);
    int32 Failures = Report.Errors.Num();
    for (const MarketAutoPlay::FRun& Trial : Report.Runs) Failures += Trial.AuditFailures;
    return Failures > 0 ? 1 : 0;
}
