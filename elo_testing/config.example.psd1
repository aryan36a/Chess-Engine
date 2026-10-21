@{
    # Copy to config.psd1 and edit the paths for your machine.
    # Headless build: no --uci argument is needed.
    EnginePath = (Join-Path $PSScriptRoot '..\chess_game\chess-uci.exe')
    CuteChessPath = 'C:\Program Files\Cute Chess\cutechess-cli.exe'
    StockfishPath = 'C:\Engines\Stockfish\stockfish.exe'
    ResultsDirectory = (Join-Path $PSScriptRoot 'results')
}