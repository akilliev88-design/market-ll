@echo off
rem Claude Code (akis-cc2): bir sonraki derleme turunun adimlari. Cikti Saved\Claude\son.log'a gider.
cd /d "%~dp0..\.."
echo === DERLE ===
if exist "Saved\Logs\DERLE_son.log" del /q "Saved\Logs\DERLE_son.log" >nul 2>nul
call DERLE.cmd /q
if errorlevel 1 (echo DERLE_BASARISIZ & goto ozet)
rem 03.10.2026: "dosya baska bir islem tarafindan kullaniliyor" gibi durumlarda DERLE.cmd hata vermeden donebiliyor;
rem derleme logunda "Result: Succeeded" yoksa derleme olmamis sayilir ve testler eski surumle calistirilmaz.
findstr /c:"Result: Succeeded" "Saved\Logs\DERLE_son.log" >nul 2>nul
if errorlevel 1 (echo DERLE_BASARISIZ_SONUC_YOK: derleme yapilmadi ya da log kilitli. Unreal Editor ve baska derlemeler kapali mi? & powershell -NoProfile -Command "if (Test-Path Saved\Logs\DERLE_son.log) { Get-Content Saved\Logs\DERLE_son.log -Tail 15 }" & goto ozet)
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
rem D8 (04.10.2026): otomatik oyuncu 30 yil, uc tarz, bir tohum (uzun surebilir). Rapor Saved\AutoPlay\C10\D8\rapor.md
echo === BOT ===
if exist "Saved\AutoPlay\C10\D8" rmdir /s /q "Saved\AutoPlay\C10\D8"
call AUTOPLAY.cmd -Years=30 -Seeds=1 -Experiment=D8 > "Saved\Logs\BOT_son.log" 2>&1
if errorlevel 1 (echo BOT_CIKIS_KODU_1_DENETIM_HATASI_OLABILIR)
powershell -NoProfile -Command "if (Test-Path 'Saved\AutoPlay\C10\D8\rapor.md') { Select-String -Path 'Saved\AutoPlay\C10\D8\rapor.md' -Pattern '^### |^\| [0-9]+ \||yil\)\.|hedef|kasa eksiye|denetim' | ForEach-Object { $_.Line } } else { 'BOT_RAPORU_YOK'; Get-Content 'Saved\Logs\AutoPlay.log' -Tail 20 }"
echo BOT_TAMAM
exit /b 0
