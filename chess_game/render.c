#include<raylib.h>
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