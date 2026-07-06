#include<stdio.h>
#include<raylib.h>
#include<stdint.h>
#include "board.h"
#include "render.h"
#include "texture.h"
#include "input.h"

#define BOARD_SIZE 8
#define TILE_SIZE 128


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

        DrawBoardPieces();
        DrawPromotionMenu();

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
