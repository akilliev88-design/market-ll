@echo off
setlocal
rem Magaza yuzey kutuphanesini Unreal'a kurar: dokular (AssetInbox\Textures\Miras) + M_MirasSurface + M_MirasAcrylic.
rem Kullanim: GORSEL_HAZIRLA.cmd  veya  GORSEL_HAZIRLA.cmd /q  (beklemeden). Unreal Editor kapali olmali.
set "UE_EDITOR=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0MarketSim.uproject"
set "LOG=%~dp0Saved\Logs\GORSEL_son.log"
if not exist "%UE_EDITOR%" (
  echo Unreal Editor bulunamadi: %UE_EDITOR%
  if not "%~1"=="/q" pause
  exit /b 1
)
if not exist "%~dp0Saved\Logs" mkdir "%~dp0Saved\Logs"
echo Magaza malzemeleri hazirlaniyor... (1-3 dakika)
"%UE_EDITOR%" "%PROJECT%" -run=pythonscript -script="%~dp0Tools\gorsel_malzemeler.py" -unattended -nop4 -nosplash -nullrhi "-abslog=%LOG%"
if errorlevel 1 goto fail
findstr /c:"SIM_MATERIALS_READY" "%LOG%" >nul
if errorlevel 1 goto fail
echo MALZEMELER TAMAM.
if not "%~1"=="/q" pause
exit /b 0
:fail
echo MALZEME HAZIRLAMA BASARISIZ. Ayrinti: Saved\Logs\GORSEL_son.log (SIM_MATERIALS_ERROR satiri)
echo Oyun yine calisir; yuzeyler duz renkle gorunur.
if not "%~1"=="/q" pause
exit /b 1
