#include<raylib.h>
#include<stdio.h>
#include "render.h"
#include "texture.h"

//Draw Piece
void DrawPiece(Piece piece,int row,int col){

    DrawTexture(pieceTextures[piece],col*TILE_SIZE,row*TILE_SIZE,WHITE);
}

//Draw Pieces
void DrawBoardPieces(void){

    for(int row=0;row<8;row++){
        for(int col=0;col<8;col++){
            int square=row*8+col;
            Piece piece=GetPieceAtSquare(square);
            if(piece!=EMPTY){
                DrawPiece(piece,row,col);
            }
        }
    }
}

//Draw Chess Board
void DrawChessBoard(Vector2 mouse,Color hover){

    for(int col=0;col<BOARD_SIZE;col++){
        for(int row=0;row<BOARD_SIZE;row++){

            Rectangle square={
                col*TILE_SIZE,
                row*TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };

            if((row+col)%2!=0){
                DrawRectangleRec(square,GRAY);
            }else{
                DrawRectangleRec(square,WHITE);
            }

            if(CheckCollisionPointRec(mouse,square)){
                DrawRectangleRec(square,hover);
            }
        }
    }
}

//Promotion Menu
void DrawPromotionMenu(void){
    if(!promotionMenu.active){
        return;
    }
    Rectangle menu={300,180,430,160};

    DrawRectangleRec(menu,DARKGRAY);

    if(promotionMenu.pawn==WHITE_PAWN){
        DrawTexture(pieceTextures[WHITE_QUEEN],320,200,WHITE);
        DrawTexture(pieceTextures[WHITE_ROOK],420,200,WHITE);
        DrawTexture(pieceTextures[WHITE_BISHOP],520,200,WHITE);
        DrawTexture(pieceTextures[WHITE_KNIGHT],620,200,WHITE);
    }else{
        DrawTexture(pieceTextures[BLACK_QUEEN],320,200,WHITE);
        DrawTexture(pieceTextures[BLACK_ROOK],420,200,WHITE);
        DrawTexture(pieceTextures[BLACK_BISHOP],520,200,WHITE);
        DrawTexture(pieceTextures[BLACK_KNIGHT],620,200,WHITE);

    }
}

/* Subtle source/destination markers make the engine's last move easy to follow. */
void DrawLastMoveHighlight(Move move){
    if(move.from < 0 || move.from >= 64 || move.to < 0 || move.to >= 64){
        return;
    }
    const Color fromColor = (Color){70, 170, 190, 105};
    const Color toColor = (Color){245, 210, 65, 125};
    DrawRectangle((move.from % 8) * TILE_SIZE, (move.from / 8) * TILE_SIZE,
                  TILE_SIZE, TILE_SIZE, fromColor);
    DrawRectangle((move.to % 8) * TILE_SIZE, (move.to / 8) * TILE_SIZE,
                  TILE_SIZE, TILE_SIZE, toColor);
}

static void DrawCapturedPieceRow(const char *label, const Piece pieces[],
                                 int count, int x, int y, int maxWidth){
    DrawText(label, x, y, 18, (Color){215, 220, 230, 255});
    int iconSize = 28;
    int gap = 3;
    int perRow = maxWidth / (iconSize + gap);
    if(perRow < 1) perRow = 1;
    for(int i = 0; i < count; ++i){
        int col = i % perRow;
        int row = i / perRow;
        Rectangle source = {0, 0, (float)pieceTextures[pieces[i]].width,
                            (float)pieceTextures[pieces[i]].height};
        Rectangle dest = {(float)(x + col * (iconSize + gap)),
                          (float)(y + 25 + row * (iconSize + 4)),
                          (float)iconSize, (float)iconSize};
        DrawTexturePro(pieceTextures[pieces[i]], source, dest,
                       (Vector2){0, 0}, 0.0f, WHITE);
    }
}

void DrawEvaluationSidebar(int whiteScore, const Piece capturedByWhite[],
                           int whiteCount, const Piece capturedByBlack[],
                           int blackCount){
    const int x = BOARD_SIZE * TILE_SIZE;
    const int panelWidth = SIDEBAR_WIDTH;
    const Color panel = (Color){32, 35, 42, 255};
    const Color muted = (Color){170, 178, 190, 255};
    const int barX = x + 18;
    const int barY = 190;
    const int barW = 28;
    const int barH = 500;

    DrawRectangle(x, 0, panelWidth, BOARD_SIZE * TILE_SIZE, panel);
    DrawText("ENGINE ANALYSIS", x + 18, 24, 20, RAYWHITE);
    DrawText("Evaluation", x + 18, 64, 18, muted);

    /* Scores are white-centric centipawns. Map +/- 10 pawns to the bar ends. */
    float whiteShare = 0.5f + (float)whiteScore / 2000.0f;
    if(whiteShare < 0.03f) whiteShare = 0.03f;
    if(whiteShare > 0.97f) whiteShare = 0.97f;
    int whiteHeight = (int)(barH * whiteShare);
    DrawRectangle(barX, barY, barW, barH - whiteHeight, (Color){75, 80, 91, 255});
    DrawRectangle(barX, barY + barH - whiteHeight, barW, whiteHeight,
                  (Color){235, 237, 240, 255});
    DrawRectangleLines(barX, barY, barW, barH, (Color){100, 105, 115, 255});

    char scoreText[32];
    if(whiteScore > 0){
        snprintf(scoreText, sizeof(scoreText), "+%.2f", whiteScore / 100.0);
    }else if(whiteScore < 0){
        snprintf(scoreText, sizeof(scoreText), "%.2f", whiteScore / 100.0);
    }else{
        snprintf(scoreText, sizeof(scoreText), "0.00");
    }
    DrawText(scoreText, x + 62, 100, 30, RAYWHITE);
    DrawText("White advantage", x + 62, 137, 15, muted);
    DrawText("Black", barX + 36, barY + 8, 15, muted);
    DrawText("White", barX + 36, barY + barH - 24, 15, muted);

    int contentX = x + 62;
    int contentWidth = panelWidth - 78;
    DrawLine(x + 18, 730, x + panelWidth - 18, 730, (Color){75, 80, 91, 255});
    DrawCapturedPieceRow("Captured by White", capturedByWhite, whiteCount,
                         contentX, 748, contentWidth);
    int whiteRows = (whiteCount + (contentWidth / 31) - 1) / (contentWidth / 31);
    if(whiteRows < 1) whiteRows = 1;
    int blackSectionY = 790 + whiteRows * 32;
    if(blackSectionY > 900) blackSectionY = 900;
    DrawCapturedPieceRow("Captured by Black", capturedByBlack, blackCount,
                         contentX, blackSectionY, contentWidth);
}
