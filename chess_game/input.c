#include<stdio.h>
#include<raylib.h>

#include "board.h"
#include "input.h"

#define TILE_SIZE 128

void toggleTurn(void);


void HandleMouseInput(Vector2 mouse){

    int sRow=mouse.y/TILE_SIZE;
    int sCol=mouse.x/TILE_SIZE;

    int clickedSquare=sRow*8+sCol;

    if(selectedSquare!=-1){

        if(isLegalMove(clickedSquare)){

            switch(selectedPiece){

                case WHITE_PAWN:

                    //Move Piece after enpassant (sp case)
                    if(clickedSquare==board.enPassantSquare&&selectedSquare%8!=clickedSquare%8){
                        removePiece(clickedSquare+8);
                    }

                    //Set enpassant
                    if(selectedSquare-clickedSquare==16){
                        board.enPassantSquare=(selectedSquare+clickedSquare)/2;
                    }else{
                        board.enPassantSquare=-1;
                    }

                    movePiece(&board.whitePawns,selectedSquare,clickedSquare);
                    toggleTurn();
                    break;
                case BLACK_PAWN:

                    //Move Piece after wnpassant(special case)
                    if(clickedSquare==board.enPassantSquare&&selectedSquare%8!=clickedSquare%8){
                        removePiece(clickedSquare-8);
                    }

                    //Set enpassant
                    if(clickedSquare-selectedSquare==16){
                        board.enPassantSquare=(selectedSquare+clickedSquare)/2;
                    }else{
                        board.enPassantSquare=-1;
                    }

                    movePiece(&board.blackPawns,selectedSquare,clickedSquare);
                    toggleTurn();
                    break;

                case WHITE_KNIGHT:
                    board.enPassantSquare=-1;
                    movePiece(&board.whiteKnights,selectedSquare,clickedSquare);
                    toggleTurn();
                    break;

                case BLACK_KNIGHT:
                    board.enPassantSquare=-1;
                    movePiece(&board.blackKnights,selectedSquare,clickedSquare);
                    toggleTurn();
                    break;

                case WHITE_ROOK:
                    board.enPassantSquare=-1;
                    movePiece(&board.whiteRooks,selectedSquare,clickedSquare);

                    if(selectedSquare==56){
                        board.whiteQueensideRookMoved=true;
                    }else if(selectedSquare==63){
                        board.whiteKingsideRookMoved=true;
                    }

                    toggleTurn();
                    break;

                case BLACK_ROOK:
                    board.enPassantSquare=-1;
                    movePiece(&board.blackRooks,selectedSquare,clickedSquare);

                    if(selectedSquare==0){
                        board.blackQueensideRookMoved=true;
                    }else if(selectedSquare==7){
                        board.blackKingsideRookMoved=true;
                    }

                    toggleTurn();
                    break;

                case WHITE_BISHOP:
                    board.enPassantSquare=-1;
                    movePiece(&board.whiteBishops,selectedSquare,clickedSquare);
                    toggleTurn();
                    break;

                case BLACK_BISHOP:
                    board.enPassantSquare=-1;
                    movePiece(&board.blackBishops,selectedSquare,clickedSquare);
                    toggleTurn();
                    break;

                case WHITE_QUEEN:
                    board.enPassantSquare=-1;
                    movePiece(&board.whiteQueens,selectedSquare,clickedSquare);
                    toggleTurn();
                    break;

                case BLACK_QUEEN:
                    board.enPassantSquare=-1;
                    movePiece(&board.blackQueens,selectedSquare,clickedSquare);
                    toggleTurn();
                    break;

                case WHITE_KING:
                    board.enPassantSquare=-1;
                    movePiece(&board.whiteKing,selectedSquare,clickedSquare);
                    performCastle(selectedSquare,clickedSquare);
                    board.whiteKingMoved=true;
                    toggleTurn();
                    break;

                case BLACK_KING:
                    board.enPassantSquare=-1;
                    movePiece(&board.blackKing,selectedSquare,clickedSquare);
                    performCastle(selectedSquare,clickedSquare);
                    board.blackKingMoved=true;
                    toggleTurn();
                    break;

                default:
                    break;
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

        switch(piece){

            case WHITE_PAWN:
                generateWhitePawnMoves(selectedSquare);
                break;

            case BLACK_PAWN:
                generateBlackPawnMoves(selectedSquare);
                break;

            case WHITE_KNIGHT:
            case BLACK_KNIGHT:
                generateKnightMoves(selectedSquare);
                break;

            case WHITE_ROOK:
            case BLACK_ROOK:
                generateRookMoves(selectedSquare);
                break;

            case WHITE_BISHOP:
            case BLACK_BISHOP:
                generateBishopMoves(selectedSquare);
                break;

            case WHITE_QUEEN:
            case BLACK_QUEEN:
                generateQueenMoves(selectedSquare);
                break;

            case WHITE_KING:
            case BLACK_KING:
                generateKingMoves(selectedSquare);
                break;

            default:
                break;
        }
    }
}