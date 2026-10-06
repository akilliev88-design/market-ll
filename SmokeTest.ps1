param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
& (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe') "$PSScriptRoot\MarketSim.uproject" -game -RenderOffscreen -NullRHI -unattended -nosound -nop4 -SimSmoke "-abslog=$PSScriptRoot\Saved\Logs\GameplaySmoke.log"
if ($LASTEXITCODE -ne 0) { throw 'Oyun oturumu kontrolu basarisiz.' }
if (-not (Select-String -LiteralPath "$PSScriptRoot\Saved\Logs\GameplaySmoke.log" -SimpleMatch 'MarketSim smoke PASSED' -Quiet)) {
    throw 'Oyun oturumu kontrolu tamamlanmadi; logu inceleyin.'
}
