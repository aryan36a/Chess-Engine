# Developer Intelligence Mode

The Raylib GUI has an opt-in developer HUD. During a game, type **112358**
on the number row or numeric keypad to toggle it. Type the same sequence again
to hide it.

## What it shows

- Opponent move tendencies learned from the moves played in this game.
- Heuristic opponent-intention estimates based on the current threats.
- Current strategic goals and their priority.
- Up to five legal moves for the human side, ranked using a static evaluation,
  check bonus, and centralization heuristic. These are rankings, not calibrated
  probabilities or a claim that the opponent will definitely play them.
- Search activity, node count, and the last root score.

The candidate list is only shown when the human side is to move. While the
engine is thinking, the HUD says that a direct next-move candidate list is not
available rather than pretending to predict a reply to an unplayed engine move.

## Decision log

The GUI appends a trace to `chess_game/engine_dev.log` (relative to the working
directory from which `chess.exe` is launched). It records observed moves,
opponent-model updates, inferred intentions, strategic goals, ranked candidate
moves, search stats, and the engine's selected move. It appends across sessions;
the file is excluded from Git.

The trace is position/event based, not a dump of every alpha-beta node. Logging
every node would create enormous files and substantially slow search.

## Scope and limitations

This is GUI-only diagnostics. It does not alter move selection, search, the UCI
protocol, or legal-move validation. The existing strategy modules are
heuristic; confidence values are explainability scores, not statistically
calibrated probabilities. Candidate move scores are static evaluations, not a
full opponent search.
