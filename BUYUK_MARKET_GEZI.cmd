@echo off
cd /d "%~dp0"
set "STORE=buyuk_01"
if not "%~1"=="" set "STORE=%~1"
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0MarketSim.uproject" "/Engine/Maps/Entry?game=/Script/MarketSim.MarketLargeStoreTrialGameMode" -game -LargeStore=%STORE% -ResX=1600 -ResY=900
