param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
$savedTourRoot = Split-Path -Parent $PSScriptRoot
$savedTourId = 'buyuk_review_' + [Guid]::NewGuid().ToString('N').Substring(0,16)
$savedTourFolder = Join-Path $savedTourRoot 'Saved\StoreTours'
New-Item -ItemType Directory -Path $savedTourFolder -Force | Out-Null
$savedTourFile = Join-Path $savedTourFolder ($savedTourId + '.json')
$savedTourLog = Join-Path $savedTourRoot 'Saved\Logs\SavedStoreTourReview.log'
$savedTourDoc = Get-Content (Join-Path $savedTourRoot 'Config\magazalar.json') -Raw | ConvertFrom-Json
$savedTourStore = $savedTourDoc.stores | Where-Object { $_.id -eq 'buyuk_01' } | Select-Object -First 1
$savedTourStore.id = $savedTourId
$savedTourStore.name = 'Editor kaydi gezi kontrolu'
$savedTourStore | Add-Member -NotePropertyName editableShell -NotePropertyValue $true -Force
$savedTourStore.fixtures = @($savedTourStore.fixtures | Select-Object -First 2)
$savedTourStore.points | Add-Member -NotePropertyName depotDoor -NotePropertyValue @{at=@(200,$savedTourStore.points.backroom.min[1],0);yaw=90} -Force
try {
    [IO.File]::WriteAllText($savedTourFile,(@{schemaVersion=1;stores=@($savedTourStore)} | ConvertTo-Json -Depth 40),[Text.UTF8Encoding]::new($false))
    $savedTourArgs = @('"' + (Join-Path $savedTourRoot 'MarketSim.uproject') + '"','-game','-RenderOffscreen','-windowed','-ResX=1280','-ResY=720','-unattended','-nosound',('-SimStoreTour=' + $savedTourId),'-SimStoreTourSavedTest',('"-abslog=' + $savedTourLog + '"'))
    $savedTourProcess = Start-Process -FilePath (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe') -ArgumentList $savedTourArgs -WindowStyle Hidden -Wait -PassThru
    if ($savedTourProcess.ExitCode -ne 0 -or -not (Select-String -LiteralPath $savedTourLog -SimpleMatch 'SavedStoreTour PASSED' -Quiet)) { throw 'Saved store tour failed.' }
    Write-Output ('SAVED_STORE_TOUR_PASSED: ' + (Join-Path $savedTourRoot ('Saved\Screenshots\Stores\' + $savedTourId + '_SavedTour.png')))
} finally {
    Remove-Item -LiteralPath $savedTourFile -ErrorAction SilentlyContinue
}
