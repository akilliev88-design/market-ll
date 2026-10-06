@echo off
setlocal
set "UE_EDITOR=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0MarketSim.uproject"
set "SCRIPT=%~dp0Tools\MetaHuman\hazirla_animasyon.py"
set "LOG=%~dp0Saved\Logs\METAHUMAN_ANIMASYON_son.log"

if not exist "%UE_EDITOR%" (
  echo Unreal Editor bulunamadi: %UE_EDITOR%
  exit /b 2
)

echo MetaHuman yuruyus animasyonlari hazirlaniyor...
"%UE_EDITOR%" "%PROJECT%" -run=pythonscript "-script=%SCRIPT%" -unattended -nop4 -nosound -NullRHI "-abslog=%LOG%"
if errorlevel 1 (
  echo HATA. Ayrinti: %LOG%
  exit /b 1
)

findstr /C:"SIM_METAHUMAN_ANIMATION_OK=" "%LOG%" >nul
if errorlevel 1 (
  echo HATA: Basari isareti bulunamadi. Ayrinti: %LOG%
  exit /b 1
)

echo Tamamlandi. Animasyonlar: Content\MetaHumans\Animasyon
exit /b 0
