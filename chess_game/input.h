#ifndef INPUT_H
#define INPUT_H

#include<raylib.h>
#include "board.h"

#define MAX_UI_CAPTURED_PIECES 32

void HandleMouseInput(Vector2 mouse);
void SetEngineDepth(int depth);
void SetPlayerColor(bool humanPlaysWhite);

void HandlePromotionClick(Vector2 mouse);
void UpdateComputerSearch(void);
void GetLastMove(Move *move);
int GetCapturedPieces(bool capturedByWhite, Piece out[], int capacity);
const char *GetGameStatus(void);
bool IsGameOver(void);
bool IsComputerThinking(void);

#endif