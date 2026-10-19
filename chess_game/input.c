#include<stdio.h>
#include<raylib.h>
#include<stdlib.h>
#include <threads.h>
#include <stdatomic.h>

#include"evaluate.h"
#include "board.h"
#include "input.h"
#include "search.h"

#define TILE_SIZE 128

void toggleTurn(void);
void detectCheckAndStaleMate(void);
void startComputerSearch(void);


void HandleMouseInput(Vector2 mouse){

    // The human controls White; ignore input while the engine is thinking.
    if(currentTurn!=WHITE_TURN){
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

        if(isBlackPiece(piece)){
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
    atomic_bool finished;
    thrd_t thread;
    bool running;
    bool threadStarted;
} ComputerSearchTask;

static ComputerSearchTask computerSearch;

static int computerSearchThread(void *arg){
    ComputerSearchTask *task=(ComputerSearchTask *)arg;
    board=task->snapshot;
    currentTurn=task->turn;
    selectedSquare=-1;
    selectedPiece=EMPTY;
    moveCount=0;
    promotionMenu.active=false;
    initZobrist();

    task->result=findBestMove(5);
    atomic_store_explicit(&task->finished,true,memory_order_release);
    return 0;
}

void startComputerSearch(void){
    if(currentTurn!=BLACK_TURN || computerSearch.running){
        return;
    }

    computerSearch.snapshot=board;
    computerSearch.turn=currentTurn;
    computerSearch.result=(Move){-1,-1};
    atomic_store_explicit(&computerSearch.finished,false,memory_order_relaxed);

    if(thrd_create(&computerSearch.thread,computerSearchThread,&computerSearch)!=thrd_success){
        fprintf(stderr,"Could not start engine worker thread; using synchronous fallback.\n");
        computerSearch.result=findBestMove(5);
        atomic_store_explicit(&computerSearch.finished,true,memory_order_release);
        computerSearch.threadStarted=false;
        computerSearch.running=true;
        return;
    }
    computerSearch.threadStarted=true;
    computerSearch.running=true;
}

void UpdateComputerSearch(void){
    if(!computerSearch.running ||
       !atomic_load_explicit(&computerSearch.finished,memory_order_acquire)){
        return;
    }

    if(computerSearch.threadStarted){
        thrd_join(computerSearch.thread,NULL);
        computerSearch.threadStarted=false;
    }
    computerSearch.running=false;
    Move best=computerSearch.result;
    if(best.from<0 || best.from>=64 || best.to<0 || best.to>=64){
        return;
    }

    printf("Computer plays:%d -> %d\n",best.from,best.to);
    makeMove(best);
    finalizePromotionIfNeeded(best);
    toggleTurn();
    if(board.historyCount<MAX_POSITION_HISTORY){
        board.positionHistory[board.historyCount++]=generateHash();
    }
    detectCheckAndStaleMate();
}
