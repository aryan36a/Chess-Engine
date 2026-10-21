#ifndef CHESS_STRATEGY_OPPONENT_MODEL_H
#define CHESS_STRATEGY_OPPONENT_MODEL_H
#include <stdbool.h>
#include "../board.h"
#include "threat_analysis.h"
typedef struct {
    float checkRate, captureRate, queenDevelopmentRate, castlingRate, kingAttackRate;
    unsigned int observations;
} OpponentProfile;
typedef enum {
    INTENT_CHECKING_ATTACK, INTENT_QUEEN_ATTACK, INTENT_KING_SIDE_ATTACK,
    INTENT_CASTLING, INTENT_MATERIAL_HUNT
} OpponentIntentType;
typedef struct {
    OpponentIntentType type;
    float confidence;       /* heuristic estimate in [0,1], not certainty */
    int targetSquare;       /* -1 if unspecified */
    Piece targetPiece;      /* EMPTY if unspecified */
} OpponentIntent;
void OpponentModel_Init(OpponentProfile *profile);
/* Record a move already applied; piece/capture values come from pre-move position. */
void OpponentModel_RecordMove(OpponentProfile *profile, Piece movingPiece,
                              Piece capturedPiece, bool gaveCheck, bool castled);
/* Returns up to capacity intents, sorted by confidence descending. */
int OpponentModel_InferIntentions(const ThreatReport *threats, bool ourSideIsWhite,
                                  const OpponentProfile *profile,
                                  OpponentIntent *out, int capacity);
#endif
