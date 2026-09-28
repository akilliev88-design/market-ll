@echo off
setlocal
rem Tek komutla son kontrol: derle, malzemeleri kur, testler, oyun ici smoke, ekran goruntusu.
rem Unreal Editor kapali olmali. Ilk calistirmada shader derlemesi uzun surebilir (10-30 dk).
set "UE_GAME=C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
if not exist "%~dp0Saved\Logs" mkdir "%~dp0Saved\Logs"

echo [1/5] Derleme...
call "%~dp0DERLE.cmd" /q
if errorlevel 1 (set "STEP=Derleme - Saved\Logs\DERLE_son.log" & goto fail)

echo [2/5] Malzemeler...
call "%~dp0GORSEL_HAZIRLA.cmd" /q
if errorlevel 1 (set "STEP=Malzemeler - Saved\Logs\GORSEL_son.log" & goto fail)

echo [3/5] Otomasyon testleri...
call "%~dp0TEST.cmd" /q
if errorlevel 1 (set "STEP=Testler - Saved\Logs\TEST_son.log" & goto fail)

echo [4/5] Oyun ici kontrol (smoke)...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0SmokeTest.ps1" > "%~dp0Saved\Logs\SMOKE_son.log" 2>&1
if errorlevel 1 (set "STEP=Smoke - Saved\Logs\GameplaySmoke.log" & goto fail)

echo [5/5] Ekran goruntusu (pencere acilip kendiliginden kapanir)...
"%UE_GAME%" "%~dp0MirasMarket.uproject" -game -windowed -ResX=1280 -ResY=720 -MirasCapture
echo.
echo HEPSI TAMAM. Goruntuler: Saved\Screenshots\MirasMarket*.png (giris, iki duvar, gondol, dokme reyon)
echo Oynamak icin OYNA.cmd
pause
exit /b 0

:fail
echo.
echo DURDU: %STEP%
pause
exit /b 1
