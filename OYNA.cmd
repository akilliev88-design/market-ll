@echo off
setlocal
set "MARKET_EDITOR=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
if not exist "%MARKET_EDITOR%" (
  echo Unreal Engine 5.8 bulunamadi. Play.ps1 ile EngineRoot belirtin.
  pause
  exit /b 1
)
if not exist "%~dp0Binaries\Win64\UnrealEditor-MirasMarket.dll" (
  echo Once Build.ps1 ile projeyi derleyin.
  pause
  exit /b 1
)
"%MARKET_EDITOR%" "%~dp0MirasMarket.uproject" -game -windowed -ResX=1600 -ResY=900
