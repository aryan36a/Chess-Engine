#include<stdio.h>
#include<raylib.h>
#include<stdint.h>
#include "board.h"
#include "render.h"
#include "texture.h"
#include "input.h"
#include "fen.h"
#include "evaluate.h"
#include "search.h"

#define BOARD_SIZE 8
#define TILE_SIZE 128


//Prototypes
void LoadPieceTextures(void);
void UnloadPieceTextures(void);
void DrawPiece(Piece piece,int row,int col);
void DrawBoardPieces(void);
void toggleTurn(void);
void DrawCheckHighlight(void);

int main(void){

    InitWindow(BOARD_SIZE*TILE_SIZE+SIDEBAR_WIDTH,BOARD_SIZE*TILE_SIZE,"CHESS");

    LoadPieceTextures();
    //Init zorbist
    initZobrist();

    initBoard(); //Only comment/disable if you are using FEN generator (Note: Game wont load if you comment this without a fen)
    //loadFEN(""); //Enter your custom FEN here for testing purposes

    //Testing shit here
    Move list[256];
    //printf("white: %d\n",generateAllMoves(true,list));
    //printf("Black: %d\n", generateAllMoves(false, list));
    //printf("Evaluation = %d\n", evaluate());
    //Move best = findBestMove(6);//Change Depth for search here


    //printf("Best Move: %d -> %d\n", best.from, best.to);
    
    //Hover Colour
    Color hover=(Color){144,238,144,150};
    Color selected=(Color){173,216,255,150};

    int sRow=-1;
    int sCol=-1;

    while(!WindowShouldClose()){

        UpdateComputerSearch();
        Vector2 mouse=GetMousePosition();

        BeginDrawing();

        ClearBackground((Color){24, 27, 33, 255});

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
                    DrawRectangleRec(square,(Color){118, 150, 86, 255});
                }else{
                    DrawRectangleRec(square,(Color){238, 238, 210, 255});
                }

                if(CheckCollisionPointRec(mouse,square)){
                    DrawRectangleRec(square,hover);
                }
            }
        }

        //Mouse Input
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
           HandleMouseInput(mouse);
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
        Move lastMove;
        GetLastMove(&lastMove);
        DrawLastMoveHighlight(lastMove);
        DrawCheckHighlight();
        DrawBoardPieces();
        DrawPromotionMenu();

        // Legal moves belong to the selected piece only. Move generation
        // also runs during checkmate/stalemate detection, so moveCount alone
        // does not mean the user has selected a piece.
        if(selectedSquare!=-1){
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
        }

        Piece capturedByWhite[MAX_UI_CAPTURED_PIECES];
        Piece capturedByBlack[MAX_UI_CAPTURED_PIECES];
        int capturedByWhiteCount = GetCapturedPieces(true, capturedByWhite,
                                                     MAX_UI_CAPTURED_PIECES);
        int capturedByBlackCount = GetCapturedPieces(false, capturedByBlack,
                                                      MAX_UI_CAPTURED_PIECES);
        DrawEvaluationSidebar(evaluate(), capturedByWhite, capturedByWhiteCount,
                              capturedByBlack, capturedByBlackCount,
                              GetGameStatus(), IsComputerThinking(), GetSearchNodeCount());

        DrawFPS(10,10);

        EndDrawing();
    }

    UnloadPieceTextures();

    CloseWindow();

    return 0;
}

void DrawCheckHighlight(void){
    if(isKingInCheck(true)){
        int kingSquare=-1;
        for(int i=0;i<64;i++){
            if(isBitSet(board.whiteKing,i)){
                kingSquare=i;
                break;
            }
        }
        if(kingSquare!=-1){
            int row=kingSquare/8;
            int col=kingSquare%8;
            DrawRectangle(
                col*TILE_SIZE,
                row*TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE,
                (Color){255,0,0,120}
            );
        }
    }
    if(isKingInCheck(false)){
        int kingSquare=-1;
        for(int i=0;i<64;i++){
            if(isBitSet(board.blackKing,i)){
                kingSquare=i;
                break;
            }
        }
        if(kingSquare!=-1){

            int row=kingSquare/8;
            int col=kingSquare%8;
            DrawRectangle(
                col*TILE_SIZE,
                row*TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE,
                (Color){255,0,0,120}
            );
        }
    }
}