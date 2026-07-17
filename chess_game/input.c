#include<stdio.h>
#include<raylib.h>

#include "board.h"
#include "input.h"

#define TILE_SIZE 128

void toggleTurn(void);
void detectCheckAndStaleMate(void);


void HandleMouseInput(Vector2 mouse){

    if(promotionMenu.active){
        HandlePromotionClick(mouse);
        return;
    }

    int sRow=mouse.y/TILE_SIZE;
    int sCol=mouse.x/TILE_SIZE;

    int clickedSquare=sRow*8+sCol;

    if(selectedSquare!=-1){

        if(isLegalMove(clickedSquare)){

            Move move={selectedSquare,clickedSquare};
            makeMove(move);

            if(selectedPiece==WHITE_PAWN&&clickedSquare<8){
                promotionMenu.active=true;
                promotionMenu.square=clickedSquare;
                promotionMenu.pawn=WHITE_PAWN;
            }else if(selectedPiece==BLACK_PAWN&&clickedSquare>=56){
                promotionMenu.active=true;
                promotionMenu.square=clickedSquare;
                promotionMenu.pawn=BLACK_PAWN;
            }else{
                toggleTurn();
                detectCheckAndStaleMate();
            }
        }

        selectedSquare=-1;
        selectedPiece=EMPTY;
        moveCount=0;
    }
    else{

        Piece piece=GetPieceAtSquare(clickedSquare);

        if(piece==EMPTY){
            selectedSquare=-1;
            selectedPiece=EMPTY;
            moveCount=0;
            return;
        }

        if(currentTurn==WHITE_TURN && isBlackPiece(piece)){
            selectedSquare=-1;
            selectedPiece=EMPTY;
            moveCount=0;
            printf("Wrong Colour\n");
            return;
        }

        if(currentTurn==BLACK_TURN && isWhitePiece(piece)){
            selectedSquare=-1;
            selectedPiece=EMPTY;
            moveCount=0;
            printf("Wrong Colour\n");
            return;
        }

        selectedSquare=clickedSquare;
        selectedPiece=piece;
        generateMoves(selectedSquare);
    }
}

void HandlePromotionClick(Vector2 mouse){
    Rectangle queenBox={320,200,128,128};
    Rectangle rookBox={420,200,128,128};
    Rectangle bishopBox={520,200,128,128};
    Rectangle knightBox={620,200,128,128};

    if(CheckCollisionPointRec(mouse,queenBox)){
        if(promotionMenu.pawn==WHITE_PAWN){
            promotePawn(promotionMenu.square,WHITE_QUEEN);
        }else{
            promotePawn(promotionMenu.square,BLACK_QUEEN);
        }
    }else if(CheckCollisionPointRec(mouse,rookBox)){
        if(promotionMenu.pawn==WHITE_PAWN){
            promotePawn(promotionMenu.square,WHITE_ROOK);
        }else{
            promotePawn(promotionMenu.square,BLACK_ROOK);
        }
    }else if(CheckCollisionPointRec(mouse,bishopBox)){
        if(promotionMenu.pawn==WHITE_PAWN){
            promotePawn(promotionMenu.square,WHITE_BISHOP);
        }else{
            promotePawn(promotionMenu.square,BLACK_BISHOP);
        }
    }else if(CheckCollisionPointRec(mouse,knightBox)){
        if(promotionMenu.pawn==WHITE_PAWN){
            promotePawn(promotionMenu.square,WHITE_KNIGHT);
        }else{
            promotePawn(promotionMenu.square,BLACK_KNIGHT);
        }
    }
    else{
        return;
    }

    promotionMenu.active=false;

    toggleTurn();
    detectCheckAndStaleMate();

    selectedSquare=-1;
    selectedPiece=EMPTY;
    moveCount=0;


}

void detectCheckAndStaleMate(void){

    bool white=(currentTurn==WHITE_TURN);

    if(isCheckMate(white)){
        printf("Checkmate!\n");
    }
    else if(isStaleMate(white)){
        printf("Stalemate!\n");
    }
}