[CmdletBinding()]
param(
    [ValidateRange(2, 5000)][int]$Games = 200,
    [ValidateRange(5, 5)][int]$Depth = 5,
    [string]$ConfigPath = (Join-Path $PSScriptRoot 'config.psd1')
)

$ErrorActionPreference = 'Stop'
if (-not (Test-Path $ConfigPath)) { throw "Copy config.example.psd1 to config.psd1 and set the executable paths." }
$config = Import-PowerShellDataFile $ConfigPath
foreach ($key in @('EnginePath','CuteChessPath','StockfishPath','ResultsDirectory')) {
    if (-not $config.ContainsKey($key) -or [string]::IsNullOrWhiteSpace([string]$config[$key])) { throw "Missing config key: $key" }
}
foreach ($key in @('EnginePath','CuteChessPath','StockfishPath')) { if (-not (Test-Path $config[$key])) { throw "File not found ($key): $($config[$key])" } }
$engine = (Resolve-Path $config.EnginePath).Path
$cute = (Resolve-Path $config.CuteChessPath).Path
$stockfish = (Resolve-Path $config.StockfishPath).Path
$results = $config.ResultsDirectory
New-Item -ItemType Directory -Force -Path $results | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$pgn = Join-Path $results "gauntlet-$stamp.pgn"
$log = Join-Path $results "gauntlet-$stamp.log"

# Fail early if the configured engine is still the GUI build or lacks a UCI handshake.
$handshake = @('uci','isready','quit') | & $engine 2>&1 | Out-String
if ($LASTEXITCODE -ne 0 -or $handshake -notmatch '(?m)^uciok\s*$' -or $handshake -notmatch '(?m)^readyok\s*$') {
    throw "UCI handshake failed; tournament not started. Output:`n$handshake"
}

$rounds = [Math]::Ceiling($Games / 2)
Write-Host "Starting $Games games (approximately; $rounds paired rounds), depth $Depth"
Write-Host "PGN: $pgn"
$args = @(
    '-engine', "name=AryanChess cmd=$engine proto=uci",
    '-engine', "name=Stockfish cmd=$stockfish proto=uci",
    '-each', "depth=$Depth",
    '-rounds', [string]$rounds,
    '-repeat',
    '-pgnout', $pgn
)
& $cute @args 2>&1 | Tee-Object -FilePath $log
if ($LASTEXITCODE -ne 0) { throw "Cute Chess failed with exit code $LASTEXITCODE. See $log" }
Write-Host "Finished. Review PGN and log; results are relative to this Stockfish build and test settings."