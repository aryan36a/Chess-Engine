#include<stdio.h>
#include<raylib.h>
#include<stdint.h>
#include "board.h"

#define BOARD_SIZE 8
#define TILE_SIZE 128

Texture2D pieceTextures[13];

//Prototypes
void LoadPieceTextures(void);
void UnloadPieceTextures(void);
void DrawPiece(Piece piece,int row,int col);
void DrawBoardPieces(void);
void toggleTurn(void);

int main(void){

    InitWindow(BOARD_SIZE*TILE_SIZE,BOARD_SIZE*TILE_SIZE,"CHESS");

    LoadPieceTextures();
    initBoard();

    //Hover Colour
    Color hover=(Color){144,238,144,150};
    Color selected=(Color){173,216,255,150};

    int sRow=-1;
    int sCol=-1;

    while(!WindowShouldClose()){

        Vector2 mouse=GetMousePosition();

        BeginDrawing();

        ClearBackground(RAYWHITE);

        //Render Board
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

        //Mouse Input
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){

            sRow=mouse.y/TILE_SIZE;
            sCol=mouse.x/TILE_SIZE;

            int clickedSquare=sRow*8+sCol;

            if(selectedSquare!=-1){

                if(isLegalMove(clickedSquare)){

                    switch(selectedPiece){

                        case WHITE_PAWN:
                            movePiece(&board.whitePawns,selectedSquare,clickedSquare);
                            toggleTurn();
                            break;
                        case BLACK_PAWN:
                            movePiece(&board.blackPawns,selectedSquare,clickedSquare);
                            toggleTurn();                            
                            break;
                        case WHITE_KNIGHT:
                            movePiece(&board.whiteKnights,selectedSquare,clickedSquare);
                            toggleTurn();            
                            break;
                        case BLACK_KNIGHT:
                            movePiece(&board.blackKnights,selectedSquare,clickedSquare);
                            toggleTurn();
                            break;
                        case WHITE_ROOK:
                            movePiece(&board.whiteRooks,selectedSquare,clickedSquare);
                            if(selectedSquare==56){
                                board.whiteQueensideRookMoved=true;
                            }else if(selectedSquare==63){
                                board.whiteKingsideRookMoved=true;
                            }
                            toggleTurn();
                            break;
                        case BLACK_ROOK:
                            movePiece(&board.blackRooks,selectedSquare,clickedSquare);
                            if(selectedSquare==0){
                                board.blackQueensideRookMoved=true;
                            }else if(selectedSquare==7){
                                board.blackKingsideRookMoved=true;
                            }
                            toggleTurn();
                            break;
                        case WHITE_BISHOP:
                            movePiece(&board.whiteBishops,selectedSquare,clickedSquare);
                            toggleTurn();
                            break;
                        case BLACK_BISHOP:
                            movePiece(&board.blackBishops,selectedSquare,clickedSquare);
                            toggleTurn();
                            break;
                        case WHITE_QUEEN:
                            movePiece(&board.whiteQueens,selectedSquare,clickedSquare);
                            toggleTurn();
                            break;
                        case BLACK_QUEEN:
                            movePiece(&board.blackQueens,selectedSquare,clickedSquare);
                            toggleTurn();
                            break;
                        case WHITE_KING:
                            movePiece(&board.whiteKing,selectedSquare,clickedSquare);
                            performCastle(selectedSquare,clickedSquare);
                            board.whiteKingMoved=true;
                            toggleTurn();
                            break;
                        case BLACK_KING:
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
                }else if(currentTurn==WHITE_TURN&& isBlackPiece(piece)){
                    selectedSquare=-1;
                    selectedPiece=EMPTY;
                    moveCount=0;
                    printf("Wrong Colour\n");
                }else if(currentTurn==BLACK_TURN&& isWhitePiece(piece)){
                    selectedSquare=-1;
                    selectedPiece=EMPTY;
                    moveCount=0;
                    printf("Wrong Colour\n");
                }else{
                    selectedSquare=clickedSquare;
                    selectedPiece=piece;
                 
                    if(piece==WHITE_PAWN){
                       generateWhitePawnMoves(selectedSquare);
                    }else if(piece==BLACK_PAWN){
                        generateBlackPawnMoves(selectedSquare);
                    }else if(piece==WHITE_KNIGHT||piece==BLACK_KNIGHT){
                        generateKnightMoves(selectedSquare);
                    }else if(piece==WHITE_ROOK||piece==BLACK_ROOK){
                        generateRookMoves(selectedSquare);
                    }else if(piece==WHITE_BISHOP||piece==BLACK_BISHOP){
                        generateBishopMoves(selectedSquare);
                    }else if(piece==WHITE_QUEEN||piece==BLACK_QUEEN){
                        generateQueenMoves(selectedSquare);
                    }else if(piece==WHITE_KING||piece==BLACK_KING){
                        generateKingMoves(selectedSquare);
                    }
                }
            }
        }

        //Display Selected Square
        if(selectedSquare!=-1){

            int row=selectedSquare/8;
            int col=selectedSquare%8;

            DrawRectangle(
                col*TILE_SIZE,
                row*TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE,
                selected
            );
        }

        DrawBoardPieces();

        //Legal Move Indicators
        for(int i=0;i<moveCount;i++){

            int row=moves[i].to/8;
            int col=moves[i].to%8;

            DrawCircle(
                col*TILE_SIZE+TILE_SIZE/2,
                row*TILE_SIZE+TILE_SIZE/2,
                15,
                GREEN
            );
        }

        DrawFPS(10,10);

        EndDrawing();
    }

    UnloadPieceTextures();

    CloseWindow();

    return 0;
}

//Texture Loading
void LoadPieceTextures(void){

    pieceTextures[WHITE_PAWN]   = LoadTexture("../assets/pieces/white-pawn.png");
    pieceTextures[WHITE_KNIGHT] = LoadTexture("../assets/pieces/white-knight.png");
    pieceTextures[WHITE_BISHOP] = LoadTexture("../assets/pieces/white-bishop.png");
    pieceTextures[WHITE_ROOK]   = LoadTexture("../assets/pieces/white-rook.png");
    pieceTextures[WHITE_QUEEN]  = LoadTexture("../assets/pieces/white-queen.png");
    pieceTextures[WHITE_KING]   = LoadTexture("../assets/pieces/white-king.png");

    pieceTextures[BLACK_PAWN]   = LoadTexture("../assets/pieces/black-pawn.png");
    pieceTextures[BLACK_KNIGHT] = LoadTexture("../assets/pieces/black-knight.png");
    pieceTextures[BLACK_BISHOP] = LoadTexture("../assets/pieces/black-bishop.png");
    pieceTextures[BLACK_ROOK]   = LoadTexture("../assets/pieces/black-rook.png");
    pieceTextures[BLACK_QUEEN]  = LoadTexture("../assets/pieces/black-queen.png");
    pieceTextures[BLACK_KING]   = LoadTexture("../assets/pieces/black-king.png");
}

//Texture Unloading
void UnloadPieceTextures(void){

    for(int i=WHITE_PAWN;i<=BLACK_KING;i++){
        UnloadTexture(pieceTextures[i]);
    }
}

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

void toggleTurn(void){
    if(currentTurn==WHITE_TURN){
        currentTurn=BLACK_TURN;
    }else{
        currentTurn=WHITE_TURN;
    }
}