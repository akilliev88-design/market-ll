@echo off
cd /d "%~dp0"
echo Mevcut dokulu model islemi tamamlaninca Unreal aktarimi ve kontrol goruntuleri alinir.
echo Unreal Editor kapali olmali. Bu pencereyi islem bitene kadar acik tut.
powershell -NoProfile -ExecutionPolicy Bypass -File Tools\ImageBlaster\FinalizeGeneratedStore.ps1
pause
