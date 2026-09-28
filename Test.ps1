param([string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8')
$ErrorActionPreference = 'Stop'
$testStartedAt = Get-Date
& (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe') "$PSScriptRoot\MirasMarket.uproject" -unattended -nop4 -nosound -NullRHI '-ExecCmds=Automation RunTests MirasMarket' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$PSScriptRoot\Saved\TestReports" -log
if ($LASTEXITCODE -ne 0) { throw 'Otomasyon testleri basarisiz.' }
$reportFile = Get-Item -LiteralPath "$PSScriptRoot\Saved\TestReports\index.json"
if ($reportFile.LastWriteTime -lt $testStartedAt) { throw 'Yeni test raporu olusmadi.' }
$testReport = Get-Content -LiteralPath $reportFile.FullName -Raw | ConvertFrom-Json
if ($testReport.failed -ne 0 -or $testReport.notRun -ne 0 -or $testReport.inProcess -ne 0 -or $testReport.succeeded -lt 15) {
    throw 'Tum testler basariyla tamamlanmadi; Saved/TestReports klasorunu inceleyin.'
}
