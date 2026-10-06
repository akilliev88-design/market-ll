param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',[switch]$EditableShell)
$ErrorActionPreference = 'Stop'
$tourProjectRoot = Split-Path -Parent $PSScriptRoot
$tourLog = Join-Path $tourProjectRoot 'Saved\Logs\StoreTourTest.log'
$tourExtraArgs = @()
if ($EditableShell) { $tourExtraArgs += '-SimStoreEditableTest' }
& (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe') (Join-Path $tourProjectRoot 'MarketSim.uproject') -game -RenderOffscreen -unattended -nosound -nop4 -ResX=1280 -ResY=720 -SimStoreTour=mahalle_01 -SimStoreTourTest @tourExtraArgs "-abslog=$tourLog"
if ($LASTEXITCODE -ne 0 -or -not (Select-String -LiteralPath $tourLog -SimpleMatch 'SimStoreTour PASSED' -Quiet)) { throw 'Store walking / random-fill test failed.' }
Select-String -LiteralPath $tourLog -SimpleMatch 'SimStoreTour test:' | ForEach-Object Line
