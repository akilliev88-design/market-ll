@echo off
rem Claude (Cowork) 04.10.2026: CLAUDE_KOS.cmd bu dosyayi calistirir. Adimlar adim.cmd icinde.
rem Yerel dal GitHub'dakinden ayrismissa (ff olmuyorsa) Saved\Claude\m69_git.py guvenli birlestirmeyi dener.
cd /d "%~dp0..\.."
echo === GIT ===
set "REMOTE="
for /f "tokens=1" %%r in ('git remote -v ^| findstr /i "akilliev88-design/market-ll"') do if not defined REMOTE set "REMOTE=%%r"
if not defined REMOTE (echo GITHUB_UZAK_DEPO_BULUNAMADI & git remote -v & exit /b 1)
echo Uzak depo: %REMOTE%
git fetch %REMOTE%
git checkout akis-cc2 2>nul || git checkout -b akis-cc2 --track %REMOTE%/akis-cc2
git merge --ff-only %REMOTE%/akis-cc2 >nul 2>nul
if errorlevel 1 (
  python "Saved\Claude\m69_git.py" %REMOTE%
  if errorlevel 1 (echo GIT_BIRLESTIRME_BASARISIZ & exit /b 1)
)
git log -1 --oneline
echo GIT_TAMAM
call "Saved\Claude\adim.cmd"
