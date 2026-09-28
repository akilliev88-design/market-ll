@echo off
setlocal
set "BLENDER_EXE=C:\Program Files\Blender Foundation\Blender 5.2\blender.exe"
if not exist "%BLENDER_EXE%" (
  echo Blender 5.2 bulunamadi: %BLENDER_EXE%
  exit /b 1
)

echo Blender magazasi varliklari uretiliyor...
"%BLENDER_EXE%" --background --python "%~dp0Tools\Blender\create_store_kit.py" -- "%~dp0"
if not "%ERRORLEVEL%"=="0" exit /b 1

call "%~dp0IMPORT_ENVIRONMENT.cmd"
if not "%ERRORLEVEL%"=="0" exit /b 1

echo BLENDER MAGAZA KITI HAZIR.
