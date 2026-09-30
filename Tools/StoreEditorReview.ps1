param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
$storeEditorRoot = Split-Path -Parent $PSScriptRoot
$storeEditorLog = Join-Path $storeEditorRoot 'Saved\Logs\StoreEditorReview.log'
$storeEditorArgs = @('"' + (Join-Path $storeEditorRoot 'MirasMarket.uproject') + '"', '-MirasStoreEditorReview', '-RenderOffscreen', '-unattended', '-nosplash', '-nosound', '-nop4', '"-abslog=' + $storeEditorLog + '"')
$storeEditorProcess = Start-Process -FilePath (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe') -ArgumentList $storeEditorArgs -WindowStyle Hidden -Wait -PassThru
if ($storeEditorProcess.ExitCode -ne 0 -or -not (Select-String -LiteralPath $storeEditorLog -SimpleMatch 'StoreEditor REVIEW PASSED' -Quiet)) { throw 'Store editor review failed.' }
Write-Output 'STORE_EDITOR_REVIEW_PASSED'
