@echo off
cd /d "%~dp0"
node Tools\ImageBlaster\market-world.mjs export
if errorlevel 1 goto failed
echo Dokulu model Saved\ImageBlaster\UnrealImport klasorunde.
pause
exit /b 0
:failed
echo Model aktarimi tamamlanmadi. Yukaridaki aciklamayi oku.
pause
exit /b 1
