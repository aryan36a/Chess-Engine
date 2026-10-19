#include<stdio.h>
#include<string.h>
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

int runUciMode(void);

int main(int argc, char **argv){
    if(argc > 1 && strcmp(argv[1], "--uci") == 0){
        return runUciMode();
    }

    InitWindow(BOARD_SIZE*TILE_SIZE+SIDEBAR_WIDTH,BOARD_SIZE*TILE_SIZE,"CHESS");

    LoadPieceTextures();

    bool depthSelected = false;
    bool colorSelected = false;
    int selectedDepth = 5;
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

        // Startup-only depth selection. It remains locked until the game closes.
        if(!depthSelected){
            const int ow=520, oh=300;
            int ox=(BOARD_SIZE*TILE_SIZE+SIDEBAR_WIDTH-ow)/2;
            int oy=(BOARD_SIZE*TILE_SIZE-oh)/2;
            DrawRectangleRounded((Rectangle){(float)ox,(float)oy,(float)ow,(float)oh},0.06f,12,(Color){29,32,39,255});
            DrawRectangle(ox,oy,ow,5,(Color){110,168,255,255});
            DrawText("CHESS ENGINE",ox+28,oy+24,14,(Color){158,167,181,255});
            DrawText("Choose difficulty",ox+28,oy+52,30,RAYWHITE);
            DrawText("Search depth is fixed for this game.",ox+28,oy+94,16,(Color){158,167,181,255});
            const int depths[7]={2,3,4,5,6,7,8};
            const int bw=58,bh=54,gap=8;
            int sx=ox+(ow-(7*bw+6*gap))/2;
            for(int i=0;i<7;i++){
                Rectangle b={(float)(sx+i*(bw+gap)),(float)(oy+140),(float)bw,(float)bh};
                bool hot=CheckCollisionPointRec(mouse,b);
                DrawRectangleRounded(b,0.16f,8,hot?(Color){75,112,160,255}:(Color){45,51,62,255});
                char label[8]; snprintf(label,sizeof(label),"%d",depths[i]);
                DrawText(label,(int)b.x+(bw-MeasureText(label,22))/2,(int)b.y+15,22,RAYWHITE);
                if(hot && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                    selectedDepth=depths[i]; SetEngineDepth(selectedDepth); depthSelected=true;
                }
            }
            DrawText("Higher depth takes longer to calculate.",ox+28,oy+215,15,(Color){158,167,181,255});
            DrawText("Select a depth to begin",ox+28,oy+248,15,(Color){110,168,255,255});
            EndDrawing();
            continue;
        }

        // After choosing search depth, choose which side the human controls.
        if(depthSelected && !colorSelected){
            const int ow=520, oh=250;
            int ox=(BOARD_SIZE*TILE_SIZE+SIDEBAR_WIDTH-ow)/2;
            int oy=(BOARD_SIZE*TILE_SIZE-oh)/2;
            DrawRectangleRounded((Rectangle){(float)ox,(float)oy,(float)ow,(float)oh},0.06f,12,(Color){29,32,39,255});
            DrawRectangle(ox,oy,ow,5,(Color){110,168,255,255});
            DrawText("CHESS ENGINE",ox+28,oy+24,14,(Color){158,167,181,255});
            DrawText("Choose your colour",ox+28,oy+52,28,RAYWHITE);
            DrawText("The engine will play the opposite side.",ox+28,oy+91,16,(Color){158,167,181,255});
            Rectangle whiteButton={(float)(ox+36),(float)(oy+135),210,58};
            Rectangle blackButton={(float)(ox+274),(float)(oy+135),210,58};
            bool whiteHot=CheckCollisionPointRec(mouse,whiteButton);
            bool blackHot=CheckCollisionPointRec(mouse,blackButton);
            DrawRectangleRounded(whiteButton,0.14f,8,whiteHot?(Color){75,112,160,255}:(Color){45,51,62,255});
            DrawRectangleRounded(blackButton,0.14f,8,blackHot?(Color){75,112,160,255}:(Color){45,51,62,255});
            DrawText("Play White",(int)whiteButton.x+54,(int)whiteButton.y+19,20,RAYWHITE);
            DrawText("Play Black",(int)blackButton.x+54,(int)blackButton.y+19,20,RAYWHITE);
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (whiteHot || blackHot)){
                SetPlayerColor(whiteHot);
                colorSelected=true;
            }
            EndDrawing();
            continue;
        }

        //Render Board
        for(int col=0;col<BOARD_SIZE;col++){
            for(int row=0;row<BOARD_SIZE;row++){

                int boardRow = IsBoardFlipped() ? 7 - row : row;
                int boardCol = IsBoardFlipped() ? 7 - col : col;
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

            int row=BoardDisplayRow(selectedSquare);
            int col=BoardDisplayCol(selectedSquare);

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
                int row=BoardDisplayRow(moves[i].to);
                int col=BoardDisplayCol(moves[i].to);

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
            int row=BoardDisplayRow(kingSquare);
            int col=BoardDisplayCol(kingSquare);
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

            int row=BoardDisplayRow(kingSquare);
            int col=BoardDisplayCol(kingSquare);
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