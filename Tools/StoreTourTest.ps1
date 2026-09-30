param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
$tourProjectRoot = Split-Path -Parent $PSScriptRoot
$tourLog = Join-Path $tourProjectRoot 'Saved\Logs\StoreTourTest.log'
& (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe') (Join-Path $tourProjectRoot 'MirasMarket.uproject') -game -RenderOffscreen -unattended -nosound -nop4 -ResX=1280 -ResY=720 -MirasStoreTour=mahalle_01 -MirasStoreTourTest "-abslog=$tourLog"
if ($LASTEXITCODE -ne 0 -or -not (Select-String -LiteralPath $tourLog -SimpleMatch 'MirasStoreTour PASSED' -Quiet)) { throw 'Store walking / random-fill test failed.' }
Select-String -LiteralPath $tourLog -SimpleMatch 'MirasStoreTour test:' | ForEach-Object Line
