param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
$projectFile = Join-Path $PSScriptRoot 'MirasMarket.uproject'
$editorFile = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not (Test-Path -LiteralPath $editorFile)) { throw "UnrealEditor bulunamadi: $editorFile" }
if (-not (Test-Path -LiteralPath (Join-Path $PSScriptRoot 'Binaries\Win64\UnrealEditor-MirasMarket.dll'))) {
    & (Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat') MirasMarketEditor Win64 Development "-Project=$projectFile" -WaitMutex -NoHotReloadFromIDE -NoUBA
    if ($LASTEXITCODE -ne 0) { throw 'Derleme basarisiz.' }
}
# Direct invocation intentionally opens the interactive game requested by the player.
& $editorFile $projectFile -game -windowed -ResX=1600 -ResY=900
