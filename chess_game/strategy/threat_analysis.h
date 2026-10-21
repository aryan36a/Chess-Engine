#ifndef CHESS_STRATEGY_THREAT_ANALYSIS_H
#define CHESS_STRATEGY_THREAT_ANALYSIS_H
#include <stdbool.h>
#include "../board.h"
typedef struct {
    bool inCheck, queenAttacked;
    int attackedPieces, hangingPieces, kingZonePressure;
    bool canCastleKingside, canCastleQueenside;
} SideThreatSummary;
typedef struct { SideThreatSummary white, black; } ThreatReport;
/* Read-only: does not generate moves or modify board, currentTurn, moves[], or moveCount. */
void ThreatAnalysis_Analyze(ThreatReport *report);
#endif
