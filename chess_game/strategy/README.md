# Phase 1: Opponent Awareness and Strategic Goals

This is an isolated, opt-in analysis layer. It does not modify GUI, search,
UCI entry points, or existing legacy source files. Current build targets do
not include these files yet.

## Modules
- `threat_analysis.c/.h`: checks, attacked/hanging pieces, queen attacks,
  king-zone pressure, and castling availability.
- `opponent_model.c/.h`: exponentially weighted move tendencies and
  explainable heuristic intention estimates.
- `goal_manager.c/.h`: a priority-sorted shortlist of strategic goals.

## Intended call sequence
```c
ThreatReport threats;
OpponentIntent intents[8];
StrategicGoal goals[8];
ThreatAnalysis_Analyze(&threats);
int intentCount = OpponentModel_InferIntentions(
    &threats, ourSideIsWhite, &opponentProfile, intents, 8);
int goalCount = GoalManager_Update(
    ourSideIsWhite, &threats, intents, intentCount, goals, 8);
```
Initialize the profile once per game. After an opponent move, record the
moving/captured piece values from the pre-move position, plus whether the move
gave check or castled. Reset the profile for a new opponent/game unless
cross-game adaptation is intended.

## Integration boundaries
- These modules read the current global board only. They do not call move
  generation, mutate `moves[]`/`moveCount`, or alter `currentTurn`.
- They are deliberately not wired into `search.c`, `uci.c`, `uci_main.c`,
  or `main.c`, preserving recent UCI and GUI behavior.
- Goals are hints, not proof of a plan. Initially use them for diagnostics
  and move-ordering experiments, never as a replacement for legal-move
  validation or tactical search.
- Attack maps can count pinned pieces as attackers; confirm forcing lines with
  search.
- Castling availability is inferred from existing king/rook moved flags,
  occupancy, and attacked transit squares.

## Limitations
This phase does not search hypothetical moves, prove mating threats, find
multi-ply queen traps, or learn from large datasets. It provides an explainable
baseline for those later features.
