#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include <stdbool.h>

#define INF 1000000
#define SEARCH_MAX_ROOT_CANDIDATES 256
#define SEARCH_MAX_PV_LENGTH 128

typedef struct {
    Move move;
    int score;              /* From the side-to-move perspective. */
    int depth;
    int bound;              /* 0 exact, 1 lower bound, 2 upper bound. */
    Piece captured;
    int materialEval;       /* White perspective, after the root move. */
    int positionalEval;     /* White perspective, after the root move. */
    bool givesCheck;
    bool isPromotion;
    char pv[SEARCH_MAX_PV_LENGTH];
    bool selected;
} SearchRootCandidate;

typedef struct {
    SearchRootCandidate candidates[SEARCH_MAX_ROOT_CANDIDATES];
    int candidateCount;
    int completedDepth;
    Move selectedMove;
    int selectedScore;
    int highestScore;
    int rootMaterialEval;
    int rootPositionalEval;
    Turn sideToMove;
    unsigned long long nodes;
} SearchRootAnalysis;

// Moves within this many centipawns of the best evaluation may be chosen by chaos.
#define CHAOS_SCORE_WINDOW 35

int negamax(int depth, int alpha,int beta);
Move findBestMove(int depth);
unsigned long long GetSearchNodeCount(void);
int GetSearchScore(void);
void GetSearchRootAnalysis(SearchRootAnalysis *out);

#endif