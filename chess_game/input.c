#include<stdio.h>
#include<raylib.h>
#include<stdlib.h>
#define WIN32_LEAN_AND_MEAN
#define NOGDI
/* Avoid collisions with Raylib's CloseWindow() and ShowCursor(). */
#define CloseWindow Win32ApiCloseWindow
#define ShowCursor Win32ApiShowCursor
#include <windows.h>
#undef CloseWindow
#undef ShowCursor

#include"evaluate.h"
#include "board.h"
#include "input.h"
#include "search.h"

#define TILE_SIZE 128

void toggleTurn(void);
void detectCheckAndStaleMate(void);
void startComputerSearch(void);

static Move lastPlayedMove = {-1, -1};
static Piece capturedByWhite[MAX_UI_CAPTURED_PIECES];
static Piece capturedByBlack[MAX_UI_CAPTURED_PIECES];
static int capturedByWhiteCount = 0;
static int capturedByBlackCount = 0;
static char gameStatus[96] = "Your turn - White";
static bool gameOver = false;
static int engineDepth = 5;
static bool humanPlaysWhite = true;

void SetEngineDepth(int depth){
    if(depth >= 2 && depth <= 8) engineDepth = depth;
}

void SetPlayerColor(bool playWhite){
    humanPlaysWhite = playWhite;
    snprintf(gameStatus, sizeof(gameStatus), "Your turn - %s", playWhite ? "White" : "Black");
    if(currentTurn != (humanPlaysWhite ? WHITE_TURN : BLACK_TURN)){
        startComputerSearch();
    }
}

static void recordMoveForUI(Move move){
    if(move.from < 0 || move.from >= 64 || move.to < 0 || move.to >= 64){
        return;
    }

    Piece movingPiece = GetPieceAtSquare(move.from);
    Piece capturedPiece = GetPieceAtSquare(move.to);
    if((movingPiece == WHITE_PAWN || movingPiece == BLACK_PAWN) &&
       move.to == board.enPassantSquare && move.from % 8 != move.to % 8){
        int capturedSquare = (movingPiece == WHITE_PAWN) ? move.to + 8 : move.to - 8;
        capturedPiece = GetPieceAtSquare(capturedSquare);
    }

    lastPlayedMove = move;
    if(capturedPiece == EMPTY){
        return;
    }

    bool whiteMoved = isWhitePiece(movingPiece);
    Piece *capturedList = whiteMoved ? capturedByWhite : capturedByBlack;
    int *capturedCount = whiteMoved ? &capturedByWhiteCount : &capturedByBlackCount;
    if(*capturedCount < MAX_UI_CAPTURED_PIECES){
        capturedList[(*capturedCount)++] = capturedPiece;
    }
}

void GetLastMove(Move *move){
    if(move != NULL){
        *move = lastPlayedMove;
    }
}

int GetCapturedPieces(bool capturedByWhiteSide, Piece out[], int capacity){
    if(out == NULL || capacity <= 0){
        return 0;
    }
    const Piece *source = capturedByWhiteSide ? capturedByWhite : capturedByBlack;
    int count = capturedByWhiteSide ? capturedByWhiteCount : capturedByBlackCount;
    if(count > capacity){
        count = capacity;
    }
    for(int i = 0; i < count; ++i){
        out[i] = source[i];
    }
    return count;
}

const char *GetGameStatus(void){ return gameStatus; }
bool IsGameOver(void){ return gameOver; }

void HandleMouseInput(Vector2 mouse){

    if(gameOver){ return; }
    // Only accept input when it is the human's turn.
    Turn humanTurn = humanPlaysWhite ? WHITE_TURN : BLACK_TURN;
    if(currentTurn!=humanTurn){
        return;
    }

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
            recordMoveForUI(move);
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

                //printf("Evaluation: %d\n",evaluate());
                board.positionHistory[board.historyCount++] = generateHash();
                detectCheckAndStaleMate();
                startComputerSearch();
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

        if((humanPlaysWhite && !isWhitePiece(piece)) || (!humanPlaysWhite && !isBlackPiece(piece))){
            selectedSquare=-1;
            selectedPiece=EMPTY;
            moveCount=0;
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
    
    //printf("%d\n", evaluate());
    board.positionHistory[board.historyCount++]=generateHash();
    detectCheckAndStaleMate();
    startComputerSearch();

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
    }else if(isFiftyMove()){
        printf("Draw by fifty move rule\n");
    }else if(isInsufficientMaterial()){
        printf("Draw by insufficient meterial\n");
    }else if(isThreeFoldRepetition()){
        printf("Draw by three fold repetition\n");
    }
}

// Background engine search uses its own thread-local board state.
typedef struct {
    Board snapshot;
    Turn turn;
    Move result;
    volatile LONG finished;
    HANDLE thread;
    bool running;
    bool threadStarted;
} ComputerSearchTask;

static ComputerSearchTask computerSearch;

bool IsComputerThinking(void){ return computerSearch.running; }

static DWORD WINAPI computerSearchThread(LPVOID arg){
    ComputerSearchTask *task=(ComputerSearchTask *)arg;
    board=task->snapshot;
    currentTurn=task->turn;
    selectedSquare=-1;
    selectedPiece=EMPTY;
    moveCount=0;
    promotionMenu.active=false;
    initZobrist();

    task->result=findBestMove(engineDepth);
    InterlockedExchange(&task->finished,1);
    return 0;
}

void startComputerSearch(void){
    Turn humanTurn = humanPlaysWhite ? WHITE_TURN : BLACK_TURN;
    if(gameOver || currentTurn==humanTurn || computerSearch.running){
        return;
    }

    computerSearch.snapshot=board;
    computerSearch.turn=currentTurn;
    computerSearch.result=(Move){-1,-1};
    InterlockedExchange(&computerSearch.finished,0);

    computerSearch.thread=CreateThread(NULL,0,computerSearchThread,&computerSearch,0,NULL);
    if(computerSearch.thread==NULL){
        fprintf(stderr,"Could not start engine worker thread; using synchronous fallback.\n");
        computerSearch.result=findBestMove(engineDepth);
        InterlockedExchange(&computerSearch.finished,1);
        computerSearch.threadStarted=false;
        computerSearch.running=true;
        return;
    }
    computerSearch.threadStarted=true;
    computerSearch.running=true;
}

void UpdateComputerSearch(void){
    if(!computerSearch.running ||
       InterlockedCompareExchange(&computerSearch.finished,0,0)==0){
        return;
    }

    if(computerSearch.threadStarted){
        WaitForSingleObject(computerSearch.thread,INFINITE);
        CloseHandle(computerSearch.thread);
        computerSearch.thread=NULL;
        computerSearch.threadStarted=false;
    }
    computerSearch.running=false;
    Move best=computerSearch.result;
    if(best.from<0 || best.from>=64 || best.to<0 || best.to>=64){
        snprintf(gameStatus, sizeof(gameStatus), "Engine could not find a legal move");
        return;
    }

    printf("Computer plays:%d -> %d\n",best.from,best.to);
    recordMoveForUI(best);
    makeMove(best);
    finalizePromotionIfNeeded(best);
    toggleTurn();
    if(board.historyCount<MAX_POSITION_HISTORY){
        board.positionHistory[board.historyCount++]=generateHash();
    }
    detectCheckAndStaleMate();

    // Checkmate/stalemate detection generates moves as a side effect.
    // Do not let those moves appear as a user's current selection.
    selectedSquare=-1;
    selectedPiece=EMPTY;
    moveCount=0;
}
