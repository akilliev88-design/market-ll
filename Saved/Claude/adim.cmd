@echo off
rem Claude (Cowork) M69: iki tur. 1) eski hikaye kalintilari temizligi: derle, test, smoke; gecerse commit + push.
rem 2) ic kod adi MarketSim -> MarketSim (Saved\Claude\m69_ic_ad.py): derle, test, smoke; gecerse commit + push.
rem Cikti Saved\Claude\son.log'a gider.
cd /d "%~dp0..\.."
set "GITID=-c user.name=Claude-Cowork -c user.email=noreply@anthropic.com"
echo === TUR1 DERLE ===
if exist "Saved\Logs\DERLE_son.log" del /q "Saved\Logs\DERLE_son.log" >nul 2>nul
call DERLE.cmd /q
findstr /c:"Result: Succeeded" "Saved\Logs\DERLE_son.log" >nul 2>nul
if errorlevel 1 (echo TUR1_DERLE_BASARISIZ & powershell -NoProfile -ExecutionPolicy Bypass -File "Saved\Claude\ozet.ps1" & goto bitti)
echo TUR1_DERLE_TAMAM
echo === TUR1 TEST ===
call TEST.cmd /q
set "T1=%errorlevel%"
if "%T1%"=="0" (echo TUR1_TEST_TAMAM) else (echo TUR1_TEST_BASARISIZ)
powershell -NoProfile -ExecutionPolicy Bypass -File "Saved\Claude\ozet.ps1"
if not "%T1%"=="0" goto bitti
echo === TUR1 SMOKE ===
powershell -NoProfile -ExecutionPolicy Bypass -File SmokeTest.ps1 > "Saved\Logs\SMOKE_son.log" 2>&1
if errorlevel 1 (echo TUR1_SMOKE_BASARISIZ & powershell -NoProfile -Command "Get-Content 'Saved\Logs\SMOKE_son.log' -Tail 30" & goto bitti)
echo TUR1_SMOKE_TAMAM
echo === TUR1 COMMIT ===
git add -u
git %GITID% commit -q -F "Saved\Claude\m69a_mesaj.txt"
git push %REMOTE% akis-cc2
git log -1 --oneline
echo TUR1_COMMIT_TAMAM
echo === TUR2 IC AD ===
python "Saved\Claude\m69_ic_ad.py"
if errorlevel 1 (echo TUR2_ICAD_BASARISIZ & goto bitti)
if exist "Binaries\Win64\UnrealEditor-MarketSim.dll" del /q "Binaries\Win64\UnrealEditor-MarketSim*.*" >nul 2>nul
if exist "Binaries\Win64\UnrealEditor-MarketSimStudio.dll" del /q "Binaries\Win64\UnrealEditor-MarketSimStudio*.*" >nul 2>nul
echo === TUR2 DERLE ===
if exist "Saved\Logs\DERLE_son.log" del /q "Saved\Logs\DERLE_son.log" >nul 2>nul
call DERLE.cmd /q
findstr /c:"Result: Succeeded" "Saved\Logs\DERLE_son.log" >nul 2>nul
if errorlevel 1 (echo TUR2_DERLE_BASARISIZ & powershell -NoProfile -ExecutionPolicy Bypass -File "Saved\Claude\ozet.ps1" & powershell -NoProfile -Command "if (Test-Path Saved\Logs\DERLE_son.log) { Get-Content Saved\Logs\DERLE_son.log -Tail 25 }" & goto bitti)
echo TUR2_DERLE_TAMAM
echo === TUR2 TEST ===
call TEST.cmd /q
set "T2=%errorlevel%"
if "%T2%"=="0" (echo TUR2_TEST_TAMAM) else (echo TUR2_TEST_BASARISIZ)
powershell -NoProfile -ExecutionPolicy Bypass -File "Saved\Claude\ozet.ps1"
if not "%T2%"=="0" goto bitti
echo === TUR2 SMOKE ===
powershell -NoProfile -ExecutionPolicy Bypass -File SmokeTest.ps1 > "Saved\Logs\SMOKE_son.log" 2>&1
if errorlevel 1 (echo TUR2_SMOKE_BASARISIZ & powershell -NoProfile -Command "Get-Content 'Saved\Logs\SMOKE_son.log' -Tail 30" & goto bitti)
echo TUR2_SMOKE_TAMAM
echo === TUR2 COMMIT ===
git add -u
git %GITID% commit -q -F "Saved\Claude\m69c_mesaj.txt"
git push %REMOTE% akis-cc2
git log -2 --oneline
echo TUR2_COMMIT_TAMAM
:bitti
echo === GIT DURUM ===
git status --short | findstr /v /c:"?? Saved" 
exit /b 0
