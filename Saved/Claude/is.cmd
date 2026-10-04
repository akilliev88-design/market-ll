@echo off
rem Codex: yerel kodu dogrula; Git birlestirme/push ve ic ad tasimasi ayri istir.
cd /d "%~dp0..\.."
echo === YEREL DOGRULAMA ===
echo GitHub ile otomatik birlestirme ve gonderim yapilmayacak.
tasklist /FI "IMAGENAME eq UnrealEditor.exe" /NH 2>nul | findstr /I /C:"UnrealEditor.exe" >nul
if not errorlevel 1 (
  echo UNREAL_EDITOR_ACIK: Editoru kapatip yeniden calistir.
  exit /b 1
)
echo === DERLE ===
call DERLE.cmd /q
if errorlevel 1 goto derle_hata
findstr /C:"Result: Succeeded" "Saved\Logs\DERLE_son.log" >nul 2>nul
if errorlevel 1 goto derle_hata
echo === TEST ===
call TEST.cmd /q
if errorlevel 1 goto test_hata
echo === SMOKE ===
powershell -NoProfile -ExecutionPolicy Bypass -File SmokeTest.ps1 > "Saved\Logs\SMOKE_son.log" 2>&1
if errorlevel 1 goto smoke_hata
echo DERLE_TEST_SMOKE_TAMAM
exit /b 0
:derle_hata
echo DERLE_BASARISIZ: Saved\Logs\DERLE_son.log
goto hata
:test_hata
echo TEST_BASARISIZ: Saved\Logs\TEST_son.log
goto hata
:smoke_hata
echo SMOKE_BASARISIZ: Saved\Logs\SMOKE_son.log
:hata
exit /b 1
