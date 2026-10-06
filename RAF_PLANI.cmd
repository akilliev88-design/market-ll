@echo off
setlocal
rem Unreal Editor'u acar ve Raf Plani Editoru penceresini otomatik getirir.
set "MARKET_EDITOR=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
if not exist "%MARKET_EDITOR%" (
  echo Unreal Engine 5.8 bulunamadi.
  pause
  exit /b 1
)
if not exist "%~dp0Binaries\Win64\UnrealEditor-MarketSimStudio.dll" (
  echo Raf Plani Editoru henuz derlenmedi. Once DERLE.cmd calistir.
  pause
  exit /b 1
)
start "" "%MARKET_EDITOR%" "%~dp0MarketSim.uproject" -SimPlanogram
