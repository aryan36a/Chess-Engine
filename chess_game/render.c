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
                DrawPiece(piece,BoardDisplayRow(square),BoardDisplayCol(square));
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
    DrawRectangle(BoardDisplayCol(move.from) * TILE_SIZE, BoardDisplayRow(move.from) * TILE_SIZE,
                  TILE_SIZE, TILE_SIZE, fromColor);
    DrawRectangle(BoardDisplayCol(move.to) * TILE_SIZE, BoardDisplayRow(move.to) * TILE_SIZE,
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
    const int x=BOARD_SIZE*TILE_SIZE, w=SIDEBAR_WIDTH, h=BOARD_SIZE*TILE_SIZE;
    const Color panel=(Color){25,28,35,255}, card=(Color){39,44,53,255};
    const Color muted=(Color){155,165,180,255}, bright=(Color){239,242,247,255};
    const Color line=(Color){58,64,76,255};
    DrawRectangle(x,0,w,h,panel);
    DrawRectangle(x,0,w,5,(Color){110,168,255,255});
    DrawText("CHESS / ANALYSIS",x+20,22,13,muted);
    DrawText("Position review",x+20,46,25,bright);
    DrawLine(x+20,82,x+w-20,82,line);

    Color statusColor=thinking?(Color){92,158,239,255}:
        (status&&(strstr(status,"CHECKMATE")||strstr(status,"CHECK")))?(Color){220,91,91,255}:
        (status&&strstr(status,"DRAW"))?(Color){218,174,78,255}:(Color){77,181,132,255};
    DrawRectangleRounded((Rectangle){x+18,98,w-36,58},0.12f,8,card);
    DrawCircle(x+36,119,5,statusColor);
    DrawText(thinking?"Engine thinking":(status?status:"Ready to play"),x+50,109,16,bright);
    DrawText(thinking?"Searching positions...":"You play White against the engine",x+30,133,12,muted);

    DrawRectangleRounded((Rectangle){x+18,170,w-36,58},0.12f,8,card);
    DrawText("NODES SEARCHED",x+30,180,11,muted);
    char nodeText[32]; FormatNodeCount(nodes,nodeText,sizeof(nodeText));
    DrawText(nodeText,x+w-30-MeasureText(nodeText,24),195,24,bright);
    DrawText(thinking?"Live search count":"Latest engine search",x+30,204,11,muted);

    DrawText("EVALUATION",x+20,250,12,muted);
    char scoreText[32]; snprintf(scoreText,sizeof(scoreText),"%+.2f",whiteScore/100.0);
    DrawText(scoreText,x+20,272,34,bright);
    DrawText(whiteScore>35?"White is better":whiteScore< -35?"Black is better":"Position is roughly equal",x+20,312,14,muted);

    const int bx=x+20, by=348, bw=w-40, bh=18;
    float share=0.5f+(float)whiteScore/2000.0f;
    if(share<0.03f) share=0.03f; if(share>0.97f) share=0.97f;
    DrawRectangleRounded((Rectangle){bx,by,bw,bh},0.25f,8,(Color){73,80,93,255});
    DrawRectangleRounded((Rectangle){bx,by,bw*share,bh},0.25f,8,(Color){232,235,241,255});
    DrawText("BLACK",bx,by+28,11,muted);
    DrawText("WHITE",bx+bw-MeasureText("WHITE",11),by+28,11,muted);
    DrawLine(x+20,410,x+w-20,410,line);

    DrawText("CAPTURED PIECES",x+20,428,12,muted);
    DrawText("Taken by you",x+20,455,15,bright);
    DrawCapturedPieceRow("",capturedByWhite,whiteCount,x+20,478,w-40);
    DrawLine(x+20,590,x+w-20,590,line);
    DrawText("Taken by engine",x+20,610,15,bright);
    DrawCapturedPieceRow("",capturedByBlack,blackCount,x+20,633,w-40);
    DrawText("DEPTH LOCKED FOR THIS GAME",x+20,h-44,11,muted);
}
