@echo off
setlocal
set "BLENDER_EXE=C:\Program Files\Blender Foundation\Blender 5.2\blender.exe"
if not exist "%BLENDER_EXE%" (
  echo Blender 5.2 bulunamadi: %BLENDER_EXE%
  exit /b 1
)

echo Blender magazasi varliklari uretiliyor...
"%BLENDER_EXE%" --background --python "%~dp0Tools\Blender\create_store_kit.py" -- "%~dp0."
set "BLENDER_RESULT=%ERRORLEVEL%"
if not "%BLENDER_RESULT%"=="0" exit /b %BLENDER_RESULT%

call "%~dp0IMPORT_ENVIRONMENT.cmd"
set "IMPORT_RESULT=%ERRORLEVEL%"
if not "%IMPORT_RESULT%"=="0" exit /b %IMPORT_RESULT%

echo BLENDER MAGAZA KITI HAZIR.
