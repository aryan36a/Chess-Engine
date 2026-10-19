#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"

#define INF 1000000

// Moves within this many centipawns of the best evaluation may be chosen by chaos.
#define CHAOS_SCORE_WINDOW 35

int negamax(int depth, int alpha,int beta);
Move findBestMove(int depth);

#endif