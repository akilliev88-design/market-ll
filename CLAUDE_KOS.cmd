@echo off
setlocal
cd /d "%~dp0"
if not exist "Saved\Claude" mkdir "Saved\Claude"
echo BASLADI %DATE% %TIME% > "Saved\Claude\son.log"
call "Saved\Claude\is.cmd" >> "Saved\Claude\son.log" 2>&1
set "KOS_RESULT=%ERRORLEVEL%"
echo CLAUDE_KOS_BITTI %DATE% %TIME% >> "Saved\Claude\son.log"
type "Saved\Claude\son.log"
echo.
echo Ayrintili kayit: Saved\Claude\son.log
if not "%~1"=="/q" pause
exit /b %KOS_RESULT%
