@echo off
setlocal
set "UE_EDITOR=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "PROJECT=%~dp0MarketSim.uproject"

if not exist "%UE_EDITOR%" (
  echo Unreal Editor bulunamadi: %UE_EDITOR%
  exit /b 1
)

echo Cevre varliklari Unreal'a aktariliyor...
"%UE_EDITOR%" "%PROJECT%" -run=pythonscript -script="%~dp0Tools\import_environment.py" -unattended -nop4 -nosplash -nullrhi
if not "%ERRORLEVEL%"=="0" exit /b 1

echo Cevre varliklari dogrulaniyor...
"%UE_EDITOR%" "%PROJECT%" -run=pythonscript -script="%~dp0Tools\validate_environment.py" -unattended -nop4 -nosplash -nullrhi
if not "%ERRORLEVEL%"=="0" exit /b 1

echo CEVRE AKTARIMI TAMAM.
