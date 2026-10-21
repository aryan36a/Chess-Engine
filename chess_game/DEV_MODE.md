# Developer Intelligence Mode

The Raylib GUI has an opt-in developer HUD. During a game, type **112358**
on the number row or numeric keypad to toggle it. Type the same sequence again
to hide it.

## What it shows

- Opponent move tendencies learned from the moves played in this game.
- Heuristic opponent-intention estimates based on the current threats.
- Current strategic goals and their priority.
- The engine's selected root move, its score from the side-to-move perspective,
  completed depth, and whether the existing chaos tie-break selected it within
  the configured score window.
- Up to five legal root moves ranked by the actual parallel alpha-beta search,
  with measured material/positional terms and reliable capture, check, and
  promotion flags where available. Scores are not an objective claim beyond
  the completed search depth; principal variations are shown only when the
  search supplies them.
- Search nodes and completed root depth, plus static position evaluation from
  White's perspective.

The root analysis is the most recently completed engine search. During a new
search, the previous completed snapshot remains available until the search
finishes; no extra candidate searches are launched by the HUD.

## Decision log

The GUI appends a trace to `engine_dev.log` in the working directory from which
`chess.exe` is launched (normally `chess_game/` if you launch it from that folder). It records observed moves,
opponent-model updates, inferred intentions, strategic goals, actual root
candidate rankings, search stats, and the engine's selected move and score. It
appends across sessions;
the file is excluded from Git.

The trace is position/event based, not a dump of every alpha-beta node. Logging
every node would create enormous files and substantially slow search.

## Scope and limitations

This is GUI-only diagnostics. It does not alter move selection, search, the UCI
protocol, or legal-move validation. The existing strategy modules are
heuristic; confidence values are explainability scores, not statistically
calibrated probabilities. Root scores are limited-depth alpha-beta results, not
proof of objective best play.
