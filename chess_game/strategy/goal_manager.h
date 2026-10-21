#ifndef CHESS_STRATEGY_GOAL_MANAGER_H
#define CHESS_STRATEGY_GOAL_MANAGER_H
#include <stdbool.h>
#include "opponent_model.h"
typedef enum {
    GOAL_NEUTRALIZE_KING_THREAT, GOAL_SAVE_QUEEN, GOAL_EXPLOIT_HANGING_PIECE,
    GOAL_PROTECT_HANGING_PIECE, GOAL_ATTACK_OPPONENT_QUEEN,
    GOAL_PREVENT_CASTLING, GOAL_IMPROVE_KING_SAFETY
} StrategicGoalType;
typedef struct {
    StrategicGoalType type;
    float priority, confidence, progress; /* normalized to [0,1] */
    int targetSquare;                     /* -1 if unspecified */
    Piece targetPiece;                    /* EMPTY if unspecified */
    bool active;
} StrategicGoal;
/* Rebuilds a shortlist ordered by priority. Goals never override tactics/legal moves. */
int GoalManager_Update(bool ourSideIsWhite,const ThreatReport *threats,
                       const OpponentIntent *intents,int intentCount,
                       StrategicGoal *out,int capacity);
#endif
