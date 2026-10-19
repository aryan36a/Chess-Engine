#ifndef RENDER_H
#define RENDER_H

#include<raylib.h>
#include "board.h"

#define BOARD_SIZE 8
#define TILE_SIZE 128
#define SIDEBAR_WIDTH 320

extern Texture2D pieceTextures[13];

//Prototypes
void DrawPiece(Piece piece,int row,int col);
void DrawBoardPieces(void);
void DrawChessBoard(Vector2 mouse,Color hover);
void DrawPromotionMenu(void);
void DrawLastMoveHighlight(Move move);
void DrawEvaluationSidebar(int whiteScore, const Piece capturedByWhite[], int whiteCount,
                           const Piece capturedByBlack[], int blackCount,
                           const char *status, bool thinking, unsigned long long nodes);

#endif