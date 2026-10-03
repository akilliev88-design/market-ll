@echo off
rem Claude Code: CLAUDE_KOS.cmd bu dosyayi calistirir. Bu dosya sabit kalir; adimlar adim.cmd icinde
rem (git pull calisan bir .cmd dosyasini degistirirse komut yorumlayicisi bozulur).
cd /d "%~dp0..\.."
echo === GIT ===
rem GitHub deposunun bu bilgisayardaki adi (origin ya da cloud): adresinde akilliev88-design/market-ll gecen.
set "REMOTE="
for /f "tokens=1" %%r in ('git remote -v ^| findstr /i "akilliev88-design/market-ll"') do if not defined REMOTE set "REMOTE=%%r"
if not defined REMOTE (echo GITHUB_UZAK_DEPO_BULUNAMADI & git remote -v & exit /b 1)
echo Uzak depo: %REMOTE%
git fetch %REMOTE%
git checkout akis-cc2 2>nul || git checkout -b akis-cc2 --track %REMOTE%/akis-cc2
git pull --ff-only %REMOTE% akis-cc2
if errorlevel 1 (echo GIT_PULL_BASARISIZ & exit /b 1)
git log -1 --oneline
echo GIT_TAMAM
call "Saved\Claude\adim.cmd"
