#include<raylib.h>
#include<stdio.h>
#include<string.h>
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

static void DrawCapturedPieceRow(const char *label, const Piece pieces[],
                                 int count, int x, int y, int maxWidth){
    DrawText(label, x, y, 16, (Color){205, 211, 222, 255});
    int iconSize = 25;
    int gap = 2;
    int perRow = maxWidth / (iconSize + gap);
    if(perRow < 1) perRow = 1;
    for(int i = 0; i < count; ++i){
        int col = i % perRow;
        int row = i / perRow;
        Rectangle source = {0, 0, (float)pieceTextures[pieces[i]].width,
                            (float)pieceTextures[pieces[i]].height};
        Rectangle dest = {(float)(x + col * (iconSize + gap)),
                          (float)(y + 22 + row * (iconSize + 2)),
                          (float)iconSize, (float)iconSize};
        DrawTexturePro(pieceTextures[pieces[i]], source, dest,
                       (Vector2){0, 0}, 0.0f, WHITE);
    }
}

static void FormatNodeCount(unsigned long long nodes, char *out, size_t capacity){
    if(nodes >= 1000000000ULL){
        snprintf(out, capacity, "%.2fB", (double)nodes / 1000000000.0);
    }else if(nodes >= 1000000ULL){
        snprintf(out, capacity, "%.2fM", (double)nodes / 1000000.0);
    }else if(nodes >= 1000ULL){
        snprintf(out, capacity, "%.1fK", (double)nodes / 1000.0);
    }else{
        snprintf(out, capacity, "%llu", nodes);
    }
}

void DrawEvaluationSidebar(int whiteScore, const Piece capturedByWhite[],
                           int whiteCount, const Piece capturedByBlack[],
                           int blackCount, const char *status, bool thinking,
                           unsigned long long nodes){
    const int x = BOARD_SIZE * TILE_SIZE;
    const int panelWidth = SIDEBAR_WIDTH;
    const Color panel = (Color){29, 32, 39, 255};
    const Color card = (Color){41, 45, 54, 255};
    const Color muted = (Color){158, 167, 181, 255};
    const Color text = (Color){239, 242, 247, 255};
    const int barX = x + 18;
    const int barY = 244;
    const int barW = 24;
    const int barH = 300;

    DrawRectangle(x, 0, panelWidth, BOARD_SIZE * TILE_SIZE, panel);
    DrawRectangle(x, 0, panelWidth, 5, (Color){110, 168, 255, 255});
    DrawText("YOUR GAME", x + 18, 22, 13, muted);
    DrawText("Position review", x + 18, 43, 22, text);
    DrawLine(x + 18, 78, x + panelWidth - 18, 78, (Color){62, 68, 80, 255});

    Color statusColor = thinking ? (Color){67, 133, 215, 255}
                      : (status && (strstr(status, "CHECKMATE") || strstr(status, "CHECK")))
                          ? (Color){176, 65, 70, 255}
                      : (status && strstr(status, "DRAW"))
                          ? (Color){163, 123, 48, 255}
                          : (Color){52, 128, 94, 255};
    DrawRectangleRounded((Rectangle){x + 16, 92, panelWidth - 32, 54},
                         0.18f, 8, card);
    DrawCircle(x + 31, 119, 5, statusColor);
    const char *statusText = (thinking ? "Engine is thinking..." :
                              (status ? status : "Ready to play"));
    DrawText(statusText, x + 44, 108, 15, text);
    if(thinking){
        DrawText("Searching candidate moves", x + 18, 155, 13, muted);
    }else if(status && strstr(status, "CHECKMATE")){
        DrawText("Game over", x + 18, 155, 13, muted);
    }else if(status && strstr(status, "CHECK")){
        DrawText("King in danger - respond carefully", x + 18, 155, 13, muted);
    }else if(status && strstr(status, "DRAW")){
        DrawText("The game has ended in a draw", x + 18, 155, 13, muted);
    }else{
        DrawText("You play White against the engine", x + 18, 155, 13, muted);
    }

    DrawRectangleRounded((Rectangle){x + 16, 180, panelWidth - 32, 48},
                         0.14f, 8, card);
    char nodeText[32];
    FormatNodeCount(nodes, nodeText, sizeof(nodeText));
    DrawText("AI predictions", x + 28, 188, 14, muted);
    DrawText(nodeText, x + panelWidth - 92, 185, 22, text);
    DrawText(thinking ? "positions searched - live" : "nodes in latest search",
             x + 28, 207, 11, muted);

    DrawText("EVALUATION", x + 18, 238, 12, muted);
    float whiteShare = 0.5f + (float)whiteScore / 2000.0f;
    if(whiteShare < 0.03f) whiteShare = 0.03f;
    if(whiteShare > 0.97f) whiteShare = 0.97f;
    int whiteHeight = (int)(barH * whiteShare);
    DrawRectangleRounded((Rectangle){barX, barY, barW, barH}, 0.18f, 8,
                         (Color){78, 84, 96, 255});
    DrawRectangle(barX, barY + barH - whiteHeight, barW, whiteHeight,
                  (Color){232, 235, 241, 255});
    DrawRectangleLinesEx((Rectangle){barX, barY, barW, barH}, 1,
                         (Color){101, 109, 123, 255});

    char scoreText[32];
    if(whiteScore > 0){
        snprintf(scoreText, sizeof(scoreText), "+%.2f", whiteScore / 100.0);
    }else if(whiteScore < 0){
        snprintf(scoreText, sizeof(scoreText), "%.2f", whiteScore / 100.0);
    }else{
        snprintf(scoreText, sizeof(scoreText), "0.00");
    }
    DrawText(scoreText, x + 58, 270, 31, text);
    DrawText("White's advantage", x + 58, 306, 14, muted);
    DrawText("BLACK", x + 58, 363, 12, muted);
    DrawText("WHITE", x + 58, 513, 12, muted);
    DrawText("Evaluation is in pawns", x + 58, 543, 12, muted);

    DrawLine(x + 18, 576, x + panelWidth - 18, 576, (Color){62, 68, 80, 255});
    DrawText("CAPTURED PIECES", x + 18, 593, 12, muted);
    DrawCapturedPieceRow("Taken by you", capturedByWhite, whiteCount,
                         x + 18, 616, panelWidth - 36);
    DrawCapturedPieceRow("Taken by engine", capturedByBlack, blackCount,
                         x + 18, 785, panelWidth - 36);
}
