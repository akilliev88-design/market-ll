param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
$taskLog = Join-Path $PSScriptRoot 'Saved\Logs\StoreEntryReview.log'
& (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe') "$PSScriptRoot\MarketSim.uproject" -game -RenderOffscreen -windowed -ResX=1600 -ResY=900 -unattended -nosound -nop4 -StoreEntryReview "-abslog=$taskLog"
if ($LASTEXITCODE -ne 0) { throw 'Haritadan magazaya giris kontrolu basarisiz.' }
if (-not (Select-String -LiteralPath $taskLog -SimpleMatch 'StoreEntryReview PASSED' -Quiet)) { throw 'Giris kontrolu tamamlanmadi; logu inceleyin.' }
foreach ($taskImage in @('G110\01_finans_acik.png', 'G110\02_finans_kapali.png', 'G119\01_il_karti.png', 'G119\02_magaza.png', 'G119\03_sonraki_magaza.png', 'G119\04_kapali_kayit_harita.png')) {
    if (-not (Test-Path -LiteralPath (Join-Path "$PSScriptRoot\Docs\Images" $taskImage))) { throw "Eksik goruntu: $taskImage" }
}
Write-Host 'G-110/G-119 arayuz ve giris kontrolu basarili.'
