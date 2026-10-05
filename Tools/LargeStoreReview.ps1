param([string]$EngineRoot='C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference='Stop'
$marketRoot=Split-Path $PSScriptRoot -Parent
$editor=Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$report=@()
'[]' | Set-Content -LiteralPath (Join-Path $marketRoot 'Saved/LargeStoreReview.json') -Encoding UTF8
foreach($id in @('kucuk_01','buyuk_01','hiper_01')) {
    $log=Join-Path $marketRoot "Saved/Logs/LargeStore_$id.log"
    $started=Get-Date
    & $editor (Join-Path $marketRoot 'MirasMarket.uproject') '/Engine/Maps/Entry?game=/Script/MirasMarket.MarketLargeStoreTrialGameMode' -game "-LargeStore=$id" -LargeStoreCapture -RenderOffscreen -unattended -nosound -nop4 -ResX=1600 -ResY=900 "-abslog=$log"
    if($LASTEXITCODE -ne 0 -or !(Select-String -LiteralPath $log -SimpleMatch 'LARGE_STORE_PASSED:' -Quiet)) {throw "Magaza kontrolu basarisiz: $id"}
    if(Select-String -LiteralPath $log -Pattern 'Failed to compile Material|Missing input texture' -Quiet) {throw "Malzeme kontrolu basarisiz: $id"}
    foreach($n in 1..8) {
        $shot=Get-Item (Join-Path $marketRoot ('Saved/Screenshots/LargeStores/{0}/{1:00}.png' -f $id,$n))
        if($shot.Length -eq 0 -or $shot.LastWriteTime -lt $started) {throw 'Yeni goruntu eksik'}
    }
    $report+=@{store=$id;passed=$true;walk=$true;staticTransforms=$true;views=8}
    $report | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $marketRoot 'Saved/LargeStoreReview.json') -Encoding UTF8
}
