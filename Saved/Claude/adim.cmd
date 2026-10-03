@echo off
rem Claude Code (akis-cc2): bir sonraki derleme turunun adimlari. Cikti Saved\Claude\son.log'a gider.
cd /d "%~dp0..\.."
echo === DERLE ===
call DERLE.cmd /q
if errorlevel 1 (echo DERLE_BASARISIZ & goto ozet)
echo DERLE_TAMAM
echo === TEST ===
call TEST.cmd /q
if errorlevel 1 (echo TEST_BASARISIZ) else (echo TEST_TAMAM)
:ozet
echo === OZET ===
powershell -NoProfile -ExecutionPolicy Bypass -File "Saved\Claude\ozet.ps1"
echo === SMOKE ===
if exist "Saved\Logs\DERLE_son.log" findstr /c:"Result: Succeeded" "Saved\Logs\DERLE_son.log" >nul
if errorlevel 1 (echo SMOKE_ATLANDI_DERLEME_YOK & exit /b 0)
powershell -NoProfile -ExecutionPolicy Bypass -File SmokeTest.ps1 > "Saved\Logs\SMOKE_son.log" 2>&1
if errorlevel 1 (echo SMOKE_BASARISIZ & powershell -NoProfile -Command "Get-Content 'Saved\Logs\SMOKE_son.log' -Tail 30") else (echo SMOKE_TAMAM)
exit /b 0
