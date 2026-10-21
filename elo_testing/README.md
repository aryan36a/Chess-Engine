# Elo testing backend

## Status

The repository includes separate GUI and headless UCI entry points. Use
`chess-uci.exe` in Cute Chess; it needs no command-line arguments and never
opens a Raylib window. The normal `chess.exe` remains the graphical game and
supports `--uci` when launched manually.

## Requirements (Windows)
- `chess_game/chess-uci.exe` from this project
- Cute Chess CLI (`cutechess-cli.exe`)
- Stockfish UCI executable (`stockfish.exe`)
- PowerShell

Downloads: https://cutechess.com/ and https://stockfishchess.org/download/

## Setup
1. Copy `config.example.psd1` to `config.psd1` and edit the executable paths.
   The example points at the repository's `chess_game/chess-uci.exe`; rebuild
   it after changing the engine source.
2. Build the headless executable from the repository root:
   `gcc chess_game/attacks.c chess_game/board.c chess_game/evaluate.c chess_game/fen.c chess_game/search.c chess_game/uci.c chess_game/uci_main.c -o chess_game/chess-uci.exe`
3. Run the UCI handshake test:
   `@('uci','isready','quit') | & $config.EnginePath`.
4. The UCI executable always searches at depth 5. Run
   `./run-gauntlet.ps1 -Games 200 -Depth 5`.

Outputs are written to `results/` as PGN and log files. Keep the exact Stockfish version and test settings with the results.

## Requirements before trusting the rating
The engine must support `uci`, `isready`, `ucinewgame`, `position startpos`, `position fen`, `go depth N`, and `quit`. It must parse coordinate moves (including promotion), emit a valid `bestmove` for each `go`, and keep diagnostic output off stdout. Search depth is currently capped at 8.

Start with a few hundred games, alternate colors, use varied/reproducible openings, and test against several opponents or strength settings. A short match gives a noisy relative estimate, not an absolute FIDE rating.