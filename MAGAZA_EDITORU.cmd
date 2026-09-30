@echo off
setlocal
set "MARKET_EDITOR=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
if not exist "%MARKET_EDITOR%" exit /b 1
start "" "%MARKET_EDITOR%" "%~dp0MirasMarket.uproject" -MirasStoreEditor
