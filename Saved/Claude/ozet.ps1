# Claude Code: derleme hatalari ve test raporunun kisa ozeti (son.log'a yazilir).
$Root = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$Build = Join-Path $Root 'Saved\Logs\DERLE_son.log'
if (Test-Path $Build) {
    $Errors = Select-String -Path $Build -Pattern ': error|: fatal error' | Select-Object -First 40
    Write-Output ("Derleme hata satiri: " + @($Errors).Count)
    foreach ($E in $Errors) { Write-Output $E.Line.Trim() }
}
$Report = Join-Path $Root 'Saved\TestReports\index.json'
if (-not (Test-Path $Report)) { Write-Output 'Test raporu yok.'; exit 0 }
$R = Get-Content -LiteralPath $Report -Raw | ConvertFrom-Json
Write-Output ("Test: basarili {0}, uyarili {1}, basarisiz {2}, calismadi {3}, rapor {4}" -f $R.succeeded, $R.succeededWithWarnings, $R.failed, $R.notRun, (Get-Item $Report).LastWriteTime)
foreach ($T in $R.tests) {
    foreach ($Entry in $T.entries) { if ($Entry.event.message -like 'OLCUM*') { Write-Output ("OLCUM [" + $T.fullTestPath + "] " + $Entry.event.message) } }
}
foreach ($T in $R.tests) {
    if ($T.state -eq 'Success') { continue }
    Write-Output ("[{0}] {1}" -f $T.state, $T.fullTestPath)
    $N = 0
    foreach ($Entry in $T.entries) {
        if ($Entry.event.type -eq 'Error' -or ($Entry.event.type -eq 'Warning' -and $T.state -ne 'Fail')) {
            Write-Output ("   " + $Entry.event.type + ": " + $Entry.event.message)
            $N++
            if ($N -ge 15) { Write-Output '   ...'; break }
        }
    }
}
