@echo off
setlocal
rem Editor hedefini derler; ciktiyi Saved\Logs\DERLE_son.log dosyasina yazar.
rem Kullanim: DERLE.cmd       (sonunda bekler)
rem           DERLE.cmd /q    (beklemeden cikar; ajanlar icin)
set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
if not exist "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" (
  echo Unreal Engine 5.8 bulunamadi: %UE_ROOT%
  if not "%~1"=="/q" pause
  exit /b 1
)
if not exist "%~dp0Saved\Logs" mkdir "%~dp0Saved\Logs"
echo Derleniyor... Unreal Editor aciksa once kapat.
call "%UE_ROOT%\Engine\Build\BatchFiles\Build.bat" MarketSimEditor Win64 Development "-Project=%~dp0MarketSim.uproject" -WaitMutex -NoHotReloadFromIDE -NoUBA > "%~dp0Saved\Logs\DERLE_son.log" 2>&1
set "RESULT=%ERRORLEVEL%"
findstr /i /c:": error" /c:"Result:" "%~dp0Saved\Logs\DERLE_son.log"
if not "%RESULT%"=="0" (
  echo.
  echo DERLEME BASARISIZ. Ayrinti: Saved\Logs\DERLE_son.log
  if not "%~1"=="/q" pause
  exit /b %RESULT%
)
echo DERLEME TAMAM.
if not "%~1"=="/q" pause
exit /b 0
