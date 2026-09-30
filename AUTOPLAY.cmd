@echo off
setlocal
rem Three normal-player profiles. Overrides: AUTOPLAY.cmd -Years=1 -Seeds=1
set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
if not exist "%~dp0Saved\Logs" mkdir "%~dp0Saved\Logs"
"%UE_ROOT%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "%~dp0MirasMarket.uproject" -run=MirasAutoPlay %* -unattended -nop4 -nosound -NullRHI "-abslog=%~dp0Saved\Logs\AutoPlay.log"
exit /b %ERRORLEVEL%
