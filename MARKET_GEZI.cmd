@echo off
cd /d "%~dp0"
if not exist "Saved\ImageBlaster\UnrealImport\mahalle-market-textured.glb" (
  echo Dokulu market modeli henuz hazir degil. MARKET_MODEL_AKTAR.cmd ile mevcut isi tamamla.
  pause
  exit /b 1
)
if not exist "Saved\ImageBlaster\unreal-manifest.json" (
  echo Modelin Unreal aktarimi henuz yapilmadi.
  pause
  exit /b 1
)
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0MirasMarket.uproject" "/Engine/Maps/Entry?game=/Script/MirasMarket.MarketGeneratedStoreGameMode" -game -ResX=1600 -ResY=900 -log
