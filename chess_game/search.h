#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"

#define INF 1000000

int negamax(int depth, int alpha,int beta);
Move findBestMove(int depth);

#endif