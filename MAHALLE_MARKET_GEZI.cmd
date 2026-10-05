@echo off
cd /d "%~dp0"
if not exist "Content\Stores\Handmade\Neighborhood\SM_HandmadeNeighborhood.uasset" (
  echo Blender marketinin Unreal aktarimi eksik.
  pause
  exit /b 1
)
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0MirasMarket.uproject" "/Engine/Maps/Entry?game=/Script/MirasMarket.MarketArtTrialGameMode" -game -HandmadeNeighborhood -ResX=1600 -ResY=900
