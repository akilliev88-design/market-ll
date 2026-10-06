param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
& (Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat') MarketSimEditor Win64 Development "-Project=$PSScriptRoot\MarketSim.uproject" -WaitMutex -NoHotReloadFromIDE -NoUBA
if ($LASTEXITCODE -ne 0) { throw 'Derleme basarisiz.' }
