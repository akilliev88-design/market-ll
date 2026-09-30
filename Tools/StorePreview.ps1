param(
    [string]$StoreId = '',
    [switch]$Benchmark,
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)
$ErrorActionPreference = 'Stop'
$storeProjectRoot = Split-Path -Parent $PSScriptRoot
if (Get-Process UnrealEditor -ErrorAction SilentlyContinue) { throw 'Close Unreal Editor before store preview.' }
$storeDocument = Get-Content -LiteralPath (Join-Path $storeProjectRoot 'Config\magazalar.json') -Raw -Encoding UTF8 | ConvertFrom-Json
$storeIds = @($storeDocument.stores.id)
if ($StoreId) {
    if ($storeIds -notcontains $StoreId) { throw "Unknown store: $StoreId" }
    $storeIds = @($StoreId)
}
foreach ($previewStoreId in $storeIds) {
    $previewLog = Join-Path $storeProjectRoot "Saved\Logs\G088_$(if ($Benchmark) {'Benchmark'} else {'Preview'})_$previewStoreId.log"
    $previewArgs = @((Join-Path $storeProjectRoot 'MirasMarket.uproject'), '-game', '-RenderOffscreen', '-unattended', '-nosound', '-nop4', '-ResX=1280', '-ResY=720', "-MirasStorePreview=$previewStoreId", "-abslog=$previewLog")
    if ($Benchmark) { $previewArgs += '-MirasStoreBenchmark' }
    & (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe') @previewArgs
    if ($LASTEXITCODE -ne 0 -or -not (Select-String -LiteralPath $previewLog -SimpleMatch 'MirasStorePreview PASSED' -Quiet)) { throw "Store preview failed: $previewStoreId" }
    if (-not $Benchmark) {
        $previewShots = Get-ChildItem -LiteralPath (Join-Path $storeProjectRoot "Saved\Screenshots\Stores\$previewStoreId") -Filter '*.png'
        if ($previewShots.Count -lt 5) { throw "Missing screenshots: $previewStoreId" }
    }
    Select-String -LiteralPath $previewLog -Pattern 'MirasStorePreview FPS|MirasStoreKit built' | ForEach-Object Line
}
