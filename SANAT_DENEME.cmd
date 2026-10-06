@echo off
setlocal
rem 1/2/3: sabit goruntuler. 0: WASD + fare ile gez. Esc: cik.
set "UE_EDITOR=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
"%UE_EDITOR%" "%~dp0MarketSim.uproject" "/Engine/Maps/Entry?game=/Script/MarketSim.MarketArtTrialGameMode" -game -windowed -ResX=1600 -ResY=900 -nosound -nop4
