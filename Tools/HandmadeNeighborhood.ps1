param([string]$EngineRoot='C:\Program Files\Epic Games\UE_5.8', [switch]$RebuildAssets)
$ErrorActionPreference='Stop'
$marketRoot=Split-Path $PSScriptRoot -Parent
$editor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$project=Join-Path $marketRoot 'MirasMarket.uproject'
if (Get-Process UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue) { throw 'Once Unreal pencerelerini kapatin.' }
if ($RebuildAssets) {
    & 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' -b --python "$marketRoot/Tools/Blender/create_neighborhood_store.py" -- $marketRoot
    if ($LASTEXITCODE -ne 0) { throw 'Blender uretimi basarisiz.' }
    $importLog=Join-Path $marketRoot 'Saved/Logs/HandmadeImport.log'
    & $editor $project -run=pythonscript "-script=$marketRoot/Tools/import_handmade_neighborhood.py" -unattended -NullRHI "-abslog=$importLog"
    if ($LASTEXITCODE -ne 0 -or !(Select-String -LiteralPath $importLog -Pattern 'HANDMADE_IMPORT_PASSED' -SimpleMatch -Quiet)) { throw 'Unreal aktarimi basarisiz.' }
}
$captureLog=Join-Path $marketRoot 'Saved/Logs/HandmadeCapture.log'
$captureStarted=Get-Date
& $editor $project '/Engine/Maps/Entry?game=/Script/MirasMarket.MarketArtTrialGameMode' -game -HandmadeNeighborhood -ArtTrialCapture -RenderOffscreen -unattended -nosound -nop4 -ResX=1600 -ResY=900 "-abslog=$captureLog"
if ($LASTEXITCODE -ne 0 -or !(Select-String -LiteralPath $captureLog -Pattern 'ART_TRIAL_PASSED' -SimpleMatch -Quiet) -or !(Select-String -LiteralPath $captureLog -Pattern 'grounded=1 entry_clear=1 glass_solid=1' -SimpleMatch -Quiet)) { throw 'Gezinti veya goruntu kontrolu basarisiz.' }
foreach ($n in 1..3) {
    $shot=Get-Item (Join-Path $marketRoot ('Saved/Screenshots/HandmadeNeighborhood/{0:00}.png' -f $n))
    if ($shot.Length -eq 0 -or $shot.LastWriteTime -lt $captureStarted) { throw 'Yeni ekran goruntusu eksik.' }
}
Select-String -LiteralPath $captureLog -Pattern 'HANDMADE_WALK|ART_TRIAL_FRAME|ART_TRIAL_PASSED' | ForEach-Object Line
