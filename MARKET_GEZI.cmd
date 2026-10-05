@echo off
cd /d "%~dp0"
if not exist "Content\Stores\Generated\MahalleMarket\mahalle-market-textured\StaticMeshes\mahalle-market-textured.uasset" (
  echo Dokulu marketin Unreal varligi henuz hazir degil.
  pause
  exit /b 1
)
if not exist "AssetInbox\ImageBlaster\MahalleMarket\unreal-manifest.json" (
  echo Modelin Unreal aktarimi henuz yapilmadi.
  pause
  exit /b 1
)
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0MirasMarket.uproject" "/Engine/Maps/Entry?game=/Script/MirasMarket.MarketGeneratedStoreGameMode" -game -ResX=1600 -ResY=900 -log
