#include<raylib.h>
#include "render.h"
#include "texture.h"

//Draw Piece
void DrawPiece(Piece piece,int row,int col){

    DrawTexture(
        pieceTextures[piece],
        col*TILE_SIZE,
        row*TILE_SIZE,
        WHITE
    );
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