@{
    # Copy to config.psd1 and edit the paths for your machine.
    EnginePath = (Join-Path $PSScriptRoot '..\build\chess-uci.exe')
    CuteChessPath = 'C:\Program Files\Cute Chess\cutechess-cli.exe'
    StockfishPath = 'C:\Engines\Stockfish\stockfish.exe'
    ResultsDirectory = (Join-Path $PSScriptRoot 'results')
}