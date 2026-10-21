$ErrorActionPreference = "Stop"

# Build the Raylib GUI and include nested modules such as strategy/*.c.
# Keep uci_main.c out because main.c is the GUI entry point (and also dispatches --uci).
$sourceFiles = Get-ChildItem -Path $PSScriptRoot -Recurse -File -Filter "*.c" |
    Where-Object { $_.Name -ne "uci_main.c" } |
    ForEach-Object { $_.FullName }

if (-not $sourceFiles) {
    throw "No C source files found under $PSScriptRoot"
}

$exePath = Join-Path $PSScriptRoot "chess.exe"
& gcc @sourceFiles -o $exePath -lraylib
if ($LASTEXITCODE -ne 0) {
    throw "GCC build failed with exit code $LASTEXITCODE"
}

Write-Host "Build succeeded: $exePath"
