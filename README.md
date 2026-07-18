<div align="center">

# ♟️ ChessEngine-C

### A high-performance chess engine and GUI, built from scratch in C

*Bitboard-driven move generation meets clean, minimal graphics — no shortcuts, no bloat, just chess.*

<br>

[![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Raylib](https://img.shields.io/badge/Graphics-Raylib-orange?style=for-the-badge&logo=raylib&logoColor=white)](https://www.raylib.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey?style=for-the-badge)](#-building-from-source)
[![Status](https://img.shields.io/badge/Status-Active%20Development-brightgreen?style=for-the-badge)](#-future-plans)
[![Stars](https://img.shields.io/github/stars/yourusername/chess-engine-c?style=for-the-badge&color=yellow)](https://github.com/yourusername/chess-engine-c/stargazers)

<br>

[Features](#-features) •
[Demo](#-demo) •
[Building](#-building-from-source) •
[Architecture](#-architecture) •
[Roadmap](#-future-plans) •
[Contributing](#-contributing)

</div>

---

## 📖 Project Showcase

**ChessEngine-C** is a complete chess implementation written entirely in **C**, rendered with **Raylib**, and engineered around **bitboards** — the same core representation technique used by high-performance engines like Stockfish. Rather than modeling the board as an 8×8 array of structs, every piece type and color is represented as a 64-bit integer, where each bit corresponds to a square on the board. This is not an aesthetic choice; it's a performance one.

Bitboards allow the engine to answer questions like *"which squares can this rook attack?"* or *"is the king currently in check?"* using single bitwise operations — AND, OR, XOR, and shifts — instead of iterating over arrays. On modern hardware, this means move generation, attack detection, and occupancy checks execute in a handful of CPU cycles rather than nested loops over board coordinates. The result is a rules engine that stays fast even as move-generation complexity grows (castling, en passant, pin detection, discovered checks).

This project was built with three guiding principles: **correctness** (every legal chess rule is enforced, including the notoriously tricky edge cases), **clean architecture** (a strict separation between the rules engine and the rendering layer, so the core logic has zero dependency on graphics code), and **performance** (bitboard arithmetic and precomputed attack tables over brute-force iteration). It's a systems-level project disguised as a game.

---

## ✨ Features

### ♜ Gameplay

| Feature | Description |
|---|---|
| ✅ Legal move generation | Fully validated legal moves, not just pseudo-legal |
| ✅ Check detection | Real-time detection of checks against either king |
| ✅ Checkmate | Correctly identifies terminal checkmate positions |
| ✅ Stalemate | Detects no-legal-move draw scenarios |
| ✅ Castling | Kingside & queenside, with full rook/king safety validation |
| ✅ En Passant | Correctly tracked and expires after one turn |
| ✅ Promotion | Full promotion to Queen, Rook, Bishop, or Knight |
| ✅ Piece captures | Standard and special captures fully supported |
| ✅ Turn system | Strict alternating turn enforcement |

### ⚙️ Engine

| Feature | Description |
|---|---|
| 🧮 Bitboards | 64-bit integer board representation per piece type/color |
| ⚡ Fast move generation | Bitwise move generation using shifts and masks |
| 🧱 Occupancy masks | Precomputed occupancy for blockers and sliding pieces |
| 🎯 Attack generation | Precomputed attack tables for knights, kings, and pawns |

### 🎨 Graphics

| Feature | Description |
|---|---|
| 🖼️ Raylib rendering | Lightweight, cross-platform 2D rendering |
| 🖱️ Mouse interaction | Click-to-select and click-to-move piece control |
| 💡 Highlighted legal moves | Visual overlay of all legal destinations for a selected piece |
| 🎯 Piece selection | Clear selection state with visual feedback |
| 👑 Promotion UI | In-game overlay for choosing a promotion piece |

---

## 🎬 Demo

<div align="center">

**Gameplay Preview**

`[ 📽️ GIF PLACEHOLDER — assets/demo/gameplay.gif ]`

<br>

**Screenshots**

| Main Board | Legal Move Highlights | Promotion UI |
|:---:|:---:|:---:|
| `[ screenshot placeholder ]` | `[ screenshot placeholder ]` | `[ screenshot placeholder ]` |

</div>

> [!TIP]
> Replace the placeholders above with actual GIFs/screenshots stored under `assets/demo/` once available. Short clips (5–10s) tend to render best on GitHub's README preview.

---

## 🎮 Controls

| Input | Action |
|---|---|
| `Left Click` (on a piece) | Select the piece and reveal legal moves |
| `Left Click` (on a highlighted square) | Move the selected piece |
| `Left Click` (elsewhere) | Deselect the current piece |
| `Left Click` (promotion overlay) | Choose promotion piece (Q / R / B / N) |
| `Esc` | Exit the application |
| `R` *(planned)* | Reset the board to the starting position |

---

## 📂 Folder Structure

```
chess-engine-c/
│
├── src/                 # Core source files (.c)
│   ├── main.c           # Entry point & game loop
│   ├── board.c          # Bitboard board representation & state
│   ├── movegen.c        # Pseudo-legal & legal move generation
│   ├── attacks.c        # Precomputed attack table generation
│   ├── game.c           # Turn management, check/mate/stalemate logic
│   └── render.c         # Raylib rendering & input handling
│
├── include/             # Public headers (.h)
│   ├── board.h
│   ├── movegen.h
│   ├── attacks.h
│   ├── game.h
│   └── render.h
│
├── assets/              # Textures, piece sprites, fonts, demo media
│   ├── pieces/          # Piece sprite images
│   ├── sounds/          # (planned) move/capture sound effects
│   └── demo/            # GIFs & screenshots used in this README
│
├── CMakeLists.txt       # Cross-platform build configuration
├── LICENSE              # MIT License
└── README.md            # You are here
```

| Folder | Purpose |
|---|---|
| `src/` | All engine and application logic — the actual C implementation |
| `include/` | Header files declaring public interfaces for each module |
| `assets/` | Non-code resources: sprites, sounds, and README media |
| `CMakeLists.txt` | Build system entry point, used across all platforms |

---

## 🛠️ Building From Source

This project uses **CMake** as its primary build system, with Raylib as the only external dependency.

### 📦 Dependency: Raylib

Raylib must be installed (or fetched) before building. Below are the fastest paths per platform.

<details>
<summary><strong>🪟 Windows</strong></summary>

**Prerequisites:** [CMake](https://cmake.org/download/), [MinGW-w64](https://www.mingw-w64.org/) or Visual Studio, [Git](https://git-scm.com/)

```powershell
# Clone the repository
git clone https://github.com/yourusername/chess-engine-c.git
cd chess-engine-c

# Configure with CMake (Raylib fetched automatically via FetchContent, if configured)
cmake -S . -B build -G "MinGW Makefiles"

# Build
cmake --build build

# Run
.\build\ChessEngineC.exe
```

> [!NOTE]
> If Raylib isn't fetched automatically, install it manually via [raylib's Windows setup guide](https://github.com/raysan5/raylib/wiki/Working-on-Windows) and set `RAYLIB_PATH` in `CMakeLists.txt`.

</details>

<details>
<summary><strong>🐧 Linux</strong></summary>

**Prerequisites:** `gcc`, `cmake`, `git`, and Raylib's build dependencies.

```bash
# Install build tools and Raylib dependencies (Debian/Ubuntu)
sudo apt update
sudo apt install build-essential cmake git \
    libasound2-dev libx11-dev libxrandr-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev

# Clone and build Raylib (if not installed via package manager)
git clone https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP
sudo make install
cd ../..

# Clone this repository
git clone https://github.com/yourusername/chess-engine-c.git
cd chess-engine-c

# Configure and build
cmake -S . -B build
cmake --build build

# Run
./build/ChessEngineC
```

</details>

<details>
<summary><strong>🍎 macOS</strong></summary>

**Prerequisites:** [Homebrew](https://brew.sh/), CMake, Git

```bash
# Install dependencies
brew install cmake git raylib

# Clone the repository
git clone https://github.com/yourusername/chess-engine-c.git
cd chess-engine-c

# Configure and build
cmake -S . -B build
cmake --build build

# Run
./build/ChessEngineC
```

</details>

### 🔧 Plain GCC / Clang (no CMake)

For a quick manual build without CMake, once Raylib is installed on your system:

```bash
# Linux / macOS
gcc src/*.c -Iinclude -o chess_engine \
    -lraylib -lm -lpthread -ldl -lrt -lX11

# Run
./chess_engine
```

```bash
# Using Clang
clang src/*.c -Iinclude -o chess_engine \
    -lraylib -lm -lpthread -ldl -lrt -lX11

./chess_engine
```

> [!IMPORTANT]
> On macOS, drop `-lrt` and `-lX11`, and instead link the required frameworks:
> `-framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL`

---

## 🏗️ Architecture

This section walks through how the engine actually works under the hood, from board memory layout to the render loop.

### ♟️ Board Representation

The board is **not** stored as an 8×8 array. Instead, it's represented as a set of `uint64_t` bitboards — one per piece type, per color (12 total: white/black × pawn, knight, bishop, rook, queen, king), plus combined occupancy boards for "all white pieces," "all black pieces," and "all pieces." Each bit position (0–63) maps directly to a board square.

```
Example: White Pawns bitboard at the starting position

8  0 0 0 0 0 0 0 0
7  1 1 1 1 1 1 1 1   ← bit set for every pawn
6  0 0 0 0 0 0 0 0
5  0 0 0 0 0 0 0 0
4  0 0 0 0 0 0 0 0
3  0 0 0 0 0 0 0 0
2  0 0 0 0 0 0 0 0
1  0 0 0 0 0 0 0 0
   a b c d e f g h
```

### 🧮 Bitboards

Because each board fits in a single 64-bit integer, operations that would otherwise require loops become **single CPU instructions**:

- **Union of pieces:** `white_pieces | black_pieces`
- **Is a square occupied?** `(occupancy >> square) & 1`
- **Capture detection:** `attacks & enemy_occupancy`

This is the same fundamental technique used in production-grade engines, and it's the reason this project can perform move generation and check detection without scanning the board square-by-square.

### ⚡ Move Generation

Move generation happens in two phases:

1. **Pseudo-legal generation** — for each piece type, precomputed attack tables (for knights, kings, pawns) or on-the-fly sliding attack calculation (for bishops, rooks, queens) produce all geometrically possible moves, ignoring check.
2. **Legal filtering** — each pseudo-legal move is applied to a copy of the board, after which the engine checks whether the moving side's king is left in check. Illegal moves (including moves that expose a discovered check) are filtered out.

### 🛡️ Legal Move Filtering

Special rules are layered on top of the base move generator:

- **Castling** requires that the king is not currently in check, does not pass through an attacked square, and that neither the king nor rook has previously moved.
- **En passant** is tracked via a single "en passant target square" that is set for exactly one ply after a double pawn push, then cleared.
- **Promotion** is triggered when a pawn move's destination rank is the final rank for its color, prompting the promotion UI.

### 🖥️ Rendering

Rendering is handled entirely by **Raylib**, in a dedicated module isolated from game logic. The board and pieces are drawn each frame based on the current bitboard state — the renderer has no knowledge of chess rules, it simply reads board state and draws it.

### 🖱️ Input

Mouse clicks are translated from screen-space pixel coordinates into board-space square indices. A simple state machine tracks whether a piece is currently selected, and dispatches either a "select" or "attempt move" action depending on current state.

### 🔁 Game Loop

The engine follows Raylib's standard real-time loop pattern:

```
while (!WindowShouldClose()) {
    HandleInput();     // Translate mouse clicks into game actions
    UpdateGameState();  // Apply moves, check for checkmate/stalemate
    BeginDrawing();
    RenderBoard();      // Draw board, pieces, highlights, UI
    EndDrawing();
}
```

---

## 🚀 Future Plans

> [!NOTE]
> This project is under active development. The core rules engine is complete and correct; the items below represent the next phase of work, focused on AI and protocol compatibility.

- [ ] PGN import/export support
- [ ] Minimax search algorithm
- [ ] Alpha-Beta pruning
- [ ] Transposition tables (Zobrist hashing)
- [ ] Opening book integration
- [ ] UCI (Universal Chess Interface) protocol support
- [ ] Stockfish compatibility mode
- [ ] Move history & undo/redo
- [ ] Sound effects for moves/captures
- [ ] Configurable themes & board skins

---

## 💡 Technical Highlights

This project was built as a demonstration of core systems-programming and computer-science fundamentals, applied to a genuinely hard problem domain:

| Concept | Where It Shows Up |
|---|---|
| **Bit manipulation** | Bitboard-based board state, attack masks, occupancy tracking, all implemented via AND/OR/XOR/shift operations |
| **Data structures** | Move lists, precomputed attack tables, board state snapshots for legality checking |
| **Rendering** | Real-time 2D graphics pipeline built on Raylib, including sprite drawing and UI overlays |
| **Event handling** | Mouse-driven input translated into discrete, validated game actions |
| **State machines** | Piece selection state, turn state, check/checkmate/stalemate terminal states |
| **Game loops** | A tight, deterministic update-render cycle running every frame |
| **Memory management** | Manual memory handling in C with no garbage collector — careful ownership of board copies during move validation |

---

## 🤔 Why This Project?

Building a chess engine is one of the most well-respected "hard problems" in software engineering for a reason — it looks simple on the surface and turns into a deep systems challenge the moment you try to implement it correctly.

Chess has a small rule set, but an enormous number of subtle edge cases: pins, discovered checks, en passant timing, castling through check, promotion. Encoding these correctly forces careful state modeling, defensive validation, and rigorous testing. Doing it in **C**, with **bitboards**, adds a performance dimension on top of correctness: you have to think about memory layout, cache behavior, and bitwise algorithms instead of relying on high-level abstractions.

This project exists to demonstrate exactly that combination — the discipline to model a genuinely complex rule system correctly, and the low-level engineering skill to make it fast.

---

## 🤝 Contributing

Contributions are welcome and appreciated! Whether it's a bug fix, a new feature, or a documentation improvement, here's how to get involved:

1. **Fork** the repository
2. **Create a branch** for your change
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **Make your changes**, following the existing code style (consistent naming, header/implementation separation)
4. **Test thoroughly** — verify move generation correctness, especially around castling, en passant, and promotion
5. **Commit** with a clear, descriptive message
   ```bash
   git commit -m "Add: short description of the change"
   ```
6. **Push** to your fork and **open a Pull Request**

> [!TIP]
> For larger changes (e.g. adding a search algorithm or UCI support), please open an issue first to discuss the approach before submitting a PR.

### Reporting Bugs

Please include:
- Steps to reproduce
- Expected vs. actual behavior
- Board position (FEN, if applicable) where the issue occurs

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2026 [Your Name]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software...
```

---

<div align="center">

**⭐ If you found this project interesting, consider giving it a star — it helps a lot!**

Made with ♟️ and a lot of bitwise operators.

</div>
