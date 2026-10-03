@echo off
rem Claude Code: CLAUDE_KOS.cmd bu dosyayi calistirir. Bu dosya sabit kalir; adimlar adim.cmd icinde
rem (git pull calisan bir .cmd dosyasini degistirirse komut yorumlayicisi bozulur).
cd /d "%~dp0..\.."
echo === GIT ===
git fetch origin
git checkout akis-cc2
git pull --ff-only origin akis-cc2
if errorlevel 1 (echo GIT_PULL_BASARISIZ & exit /b 1)
git log -1 --oneline
echo GIT_TAMAM
call "Saved\Claude\adim.cmd"
