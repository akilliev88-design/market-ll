@echo off
setlocal
set "storeTourId=mahalle_01"
if exist "%~dp0Saved\StoreTours\last.txt" set /p storeTourId=<"%~dp0Saved\StoreTours\last.txt"
if not "%~1"=="" set "storeTourId=%~1"
start "Miras Market - Magaza gezisi" "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0MirasMarket.uproject" -game -windowed -ResX=1600 -ResY=900 "-MirasStoreTour=%storeTourId%"
endlocal
