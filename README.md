# Chess Engine

A simple chess engine written in C using Raylib.

## Features

- Bitboard board representation
- Piece movement
  - Pawns
  - Knights
  - Bishops
  - Rooks
  - Queens
  - Kings
- Capturing
- Turn-based gameplay
- Castling
- Piece highlighting
- Legal move indicators

## Project Structure

```
.
├── assets/
│   └── pieces/
├── board.c
├── board.h
├── input.c
├── input.h
├── main.c
├── render.c
├── render.h
├── texture.c
├── texture.h
```

## Requirements

- GCC
- Raylib 5.x

## Build

```bash
gcc *.c -o chess.exe -lraylib
```

## Run

```bash
./chess.exe
```

## Current Status

Implemented:

- Board rendering
- Bitboard representation
- Move generation
- Move execution
- Castling
- Turn management
- En passant
- Pawn promotion

Planned:

- Check detection
- Checkmate
- Stalemate
- AI opponent

## License

This project is for learning and personal use.
