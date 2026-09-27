@echo off
setlocal
rem Otomasyon testlerini calistirir; sonuc Saved\Logs\TEST_son.log ve Saved\TestReports\index.json
if not exist "%~dp0Saved\Logs" mkdir "%~dp0Saved\Logs"
echo Testler calisiyor... (1-3 dakika, Unreal Editor kapali olmali)
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0Test.ps1" > "%~dp0Saved\Logs\TEST_son.log" 2>&1
if errorlevel 1 (
  echo TESTLER BASARISIZ. Ayrinti: Saved\Logs\TEST_son.log
  if not "%~1"=="/q" pause
  exit /b 1
)
echo TESTLER TAMAM.
if not "%~1"=="/q" pause
exit /b 0
