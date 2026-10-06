param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
$trialRoot = Split-Path $PSScriptRoot -Parent
$trialOutput = Join-Path $trialRoot 'Saved/Screenshots/ArtTrial'
$trialLog = Join-Path $trialRoot 'Saved/Logs/ArtTrial.log'
$trialStarted = Get-Date
& (Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe') (Join-Path $trialRoot 'MarketSim.uproject') '/Engine/Maps/Entry?game=/Script/MarketSim.MarketArtTrialGameMode' -game -RenderOffscreen -unattended -nosound -nop4 -ResX=1600 -ResY=900 -ArtTrialCapture "-abslog=$trialLog"
if ($LASTEXITCODE -ne 0 -or -not (Select-String -LiteralPath $trialLog -SimpleMatch 'ART_TRIAL_PASSED' -Quiet)) { throw 'Sanat denemesi basarisiz; ArtTrial.log dosyasini incele.' }
foreach ($trialIndex in 1..3) {
    $trialShot = Get-Item -LiteralPath (Join-Path $trialOutput ('{0:00}.png' -f $trialIndex))
    if ($trialShot.LastWriteTime -lt $trialStarted -or $trialShot.Length -eq 0) { throw 'Yeni ekran goruntusu olusmadi.' }
}
Select-String -LiteralPath $trialLog -Pattern 'ART_TRIAL_READY|ART_TRIAL_FRAME|ART_TRIAL_PASSED' | ForEach-Object Line
