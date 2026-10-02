@echo off
rem Claude (Cowork) icin: Saved\Claude\is.cmd adimlarini calistirir, ciktiyi Saved\Claude\son.log dosyasina yazar.
cd /d "%~dp0"
if not exist "Saved\Claude" mkdir "Saved\Claude"
echo BASLADI %DATE% %TIME% > "Saved\Claude\son.log"
call "Saved\Claude\is.cmd" >> "Saved\Claude\son.log" 2>&1
echo CLAUDE_KOS_BITTI %DATE% %TIME% >> "Saved\Claude\son.log"
exit
