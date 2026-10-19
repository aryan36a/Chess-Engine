#ifndef INPUT_H
#define INPUT_H

#include<raylib.h>
#include "board.h"

#define MAX_UI_CAPTURED_PIECES 32

void HandleMouseInput(Vector2 mouse);

void HandlePromotionClick(Vector2 mouse);
void UpdateComputerSearch(void);
void GetLastMove(Move *move);
int GetCapturedPieces(bool capturedByWhite, Piece out[], int capacity);

#endif