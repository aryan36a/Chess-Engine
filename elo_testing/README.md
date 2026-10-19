# Elo testing backend

## Status

The tournament runner can be used once this project has a standalone UCI executable. The current application launches Raylib GUI and is not yet a UCI engine, so Cute Chess cannot run it as-is. The UCI mode still needs to be implemented and smoke-tested.

## Requirements (Windows)
- UCI build of this project (planned path: `../build/chess-uci.exe`)
- Cute Chess CLI (`cutechess-cli.exe`)
- Stockfish UCI executable (`stockfish.exe`)
- PowerShell

Downloads: https://cutechess.com/ and https://stockfishchess.org/download/

## Setup
1. Copy `config.example.psd1` to `config.psd1` and edit the executable paths.
2. Run the UCI handshake test: `@('uci','isready','quit') | & $config.EnginePath`.
3. After the executable returns `uciok` and `readyok` without opening a GUI, run `./run-gauntlet.ps1 -Games 200 -Depth 4`.

Outputs are written to `results/` as PGN and log files. Keep the exact Stockfish version and test settings with the results.

## Requirements before trusting the rating
The engine must support `uci`, `isready`, `ucinewgame`, `position startpos`, `position fen`, `go depth N`, and `quit`. It must parse coordinate moves (including promotion), emit a valid `bestmove` for each `go`, and keep diagnostic output off stdout. The current search prints diagnostics with `printf`, which must be suppressed or redirected in UCI mode. Search depth is currently capped at 8.

Start with a few hundred games, alternate colors, use varied/reproducible openings, and test against several opponents or strength settings. A short match gives a noisy relative estimate, not an absolute FIDE rating.