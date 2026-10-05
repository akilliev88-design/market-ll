@echo off
cd /d "%~dp0Saved\ImageBlaster\image-blaster\app"
echo Tarayicida http://127.0.0.1:5173/mahalle-market adresini ac.
echo Bu pencereyi kapatmak onizlemeyi durdurur.
call npm.cmd run dev -- --host 127.0.0.1 --port 5173 --strictPort
pause
