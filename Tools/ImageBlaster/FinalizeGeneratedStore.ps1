param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
$marketRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
Set-Location -LiteralPath $marketRoot
$statusFile = Join-Path $marketRoot 'Saved/ImageBlaster/finalize-status.json'
$runLog = Join-Path $marketRoot 'Saved/Logs/GeneratedStoreFinalize.log'
function Status([string]$stage, [string]$detail) {
    @{stage=$stage; detail=$detail; updated=(Get-Date).ToString('o')} | ConvertTo-Json | Set-Content -LiteralPath $statusFile -Encoding utf8
}
function Run([string]$program, [string[]]$arguments, [string]$marker = '', [string]$markerLog = '') {
    & $program @arguments *> $runLog
    if ($LASTEXITCODE -ne 0) { throw "Asama basarisiz: $program. Log: $runLog" }
    $checkLog = if ($markerLog) { $markerLog } else { $runLog }
    if ($marker -and -not (Select-String -LiteralPath $checkLog -SimpleMatch $marker -Quiet)) { throw "Basari kaydi yok: $marker. Log: $checkLog" }
}
try {
    Status 'waiting_mesh' 'World Labs HQ model is being generated; existing operation is reused.'
    Run 'node' @('Tools/ImageBlaster/market-world.mjs', 'export')
    if (Get-Process UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue) { throw 'Unreal is open. Close it and rerun MARKET_AKTARIM_TAMAMLA.cmd.' }
    Status 'inspecting' 'Inspecting downloaded HQ model with Blender.'
    Run 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' @('-b', '-t', '4', '--python', 'Tools/Blender/inspect_generated_market.py', '--', 'Saved/ImageBlaster/UnrealImport/mahalle-market-textured.glb', 'Saved/ImageBlaster/textured-inspection.json') 'GENERATED_MARKET_INSPECTED'
    $editor = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
    $project = Join-Path $marketRoot 'MirasMarket.uproject'
    foreach ($script in @('import_generated_collision.py', 'import_generated_market.py')) {
        Status 'importing' $script
        $importLog = Join-Path $marketRoot ('Saved/Logs/' + $script + '.log')
        Run $editor @($project, '-run=pythonscript', "-script=$marketRoot/Tools/$script", '-unattended', '-nop4', '-nosound', '-NullRHI', "-abslog=$importLog") 'Python script executed successfully' $importLog
    }
    Status 'capturing' 'Checking player ground contact and capturing three review views.'
    $captureLog = Join-Path $marketRoot 'Saved/Logs/GeneratedStoreCapture.log'
    Run $editor @($project, '/Engine/Maps/Entry?game=/Script/MirasMarket.MarketGeneratedStoreGameMode', '-game', '-RenderOffscreen', '-unattended', '-nosound', '-nop4', '-ResX=1600', '-ResY=900', '-GeneratedStoreCapture', "-abslog=$captureLog") 'GENERATED_STORE_PASSED' $captureLog
    foreach ($number in 1..3) {
        $shot = Join-Path $marketRoot ('Saved/Screenshots/GeneratedStore/{0:00}.png' -f $number)
        if (-not (Test-Path -LiteralPath $shot) -or (Get-Item -LiteralPath $shot).Length -eq 0) { throw "Missing screenshot: $shot" }
    }
    Status 'review_ready' 'Model imported and ground check passed. Screenshots need visual review; campaign integration is pending.'
} catch {
    Status 'failed' $_.Exception.Message
    throw
}
