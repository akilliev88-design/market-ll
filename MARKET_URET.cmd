@echo off
cd /d "%~dp0"
node Tools\ImageBlaster\market-world.mjs generate
if errorlevel 1 goto failed
echo Ortam uretildi. MARKET_ONIZLE.cmd ile gezebilirsin.
pause
exit /b 0
:failed
echo Uretim tamamlanmadi. Yukaridaki aciklamayi oku.
pause
exit /b 1
