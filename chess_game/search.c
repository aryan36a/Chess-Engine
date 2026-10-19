#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "search.h"
#include "board.h"
#include "evaluate.h"

static _Thread_local Move bestMove;
static _Thread_local int rootDepth;
static _Thread_local long long counter=0;
static unsigned long long liveNodeCount=0;

unsigned long long GetSearchNodeCount(void){
    return __atomic_load_n(&liveNodeCount, __ATOMIC_RELAXED);
}

int negamax(int depth, int alpha, int beta){
    counter++;
    __atomic_add_fetch(&liveNodeCount, 1ULL, __ATOMIC_RELAXED);
    if(depth==0){
        int score=evaluate();
        return (currentTurn==WHITE_TURN)?score:-score;
    }
    Move moveList[256];
    int count=generateAllMoves(currentTurn==WHITE_TURN,moveList);
    if(count==0){
        if(isKingInCheck(currentTurn==WHITE_TURN)) return -INF+depth;
        return 0;
    }
    int bestScore=-INF, selectedMoveScore=-INF, bestChaos=-1;
    for(int i=0;i<count;i++){
        Piece capturedPiece;
        makeTemporaryMove(moveList[i],&capturedPiece);
        toggleTurn();
        int score=(depth==rootDepth)?-negamax(depth-1,-INF,INF)
                                    :-negamax(depth-1,-beta,-alpha);
        int candidateChaos=(depth==rootDepth)?calculateChaosMetric():-1;
        toggleTurn();
        undoTemporaryMove(moveList[i],capturedPiece);
        if(score>bestScore) bestScore=score;
        if(depth==rootDepth){
            bool within=(score>=bestScore-CHAOS_SCORE_WINDOW);
            bool selectedOutside=(selectedMoveScore<bestScore-CHAOS_SCORE_WINDOW);
            if(within&&(bestMove.from<0||selectedOutside||candidateChaos>bestChaos)){
                bestMove=moveList[i]; selectedMoveScore=score; bestChaos=candidateChaos;
            }
        }
        if(score>alpha) alpha=score;
        if(alpha>=beta) break;
    }
    return bestScore;
}

typedef struct {
    Board snapshot;
    Turn turn;
    Move rootMoves[256];
    int moveCount;
    int depth;
    volatile LONG nextIndex;
    int scores[256];
    int chaos[256];
} ParallelRootTask;

static DWORD WINAPI parallelRootWorker(LPVOID argument){
    ParallelRootTask *task=(ParallelRootTask*)argument;
    board=task->snapshot;
    currentTurn=task->turn;
    rootDepth=task->depth;
    counter=0;
    initZobrist();
    for(;;){
        LONG index=InterlockedIncrement(&task->nextIndex)-1;
        if(index>=task->moveCount) break;
        board=task->snapshot;
        currentTurn=task->turn;
        Piece capturedPiece;
        Move move=task->rootMoves[index];
        makeTemporaryMove(move,&capturedPiece);
        toggleTurn();
        task->scores[index]=-negamax(task->depth-1,-INF,INF);
        toggleTurn();
        task->chaos[index]=calculateChaosMetric();
        undoTemporaryMove(move,capturedPiece);
    }
    return 0;
}

Move findBestMove(int depth){
    bestMove.from=-1; bestMove.to=-1;
    rootDepth=depth; counter=0;
    __atomic_store_n(&liveNodeCount,0ULL,__ATOMIC_RELAXED);

    Move rootMoves[256];
    int count=generateAllMoves(currentTurn==WHITE_TURN,rootMoves);
    if(count<=0) return bestMove;
    if(depth<=1 || count==1){
        int bestScore=-INF, bestChaos=-1;
        for(int i=0;i<count;i++){
            Piece capturedPiece;
            makeTemporaryMove(rootMoves[i],&capturedPiece);
            toggleTurn();
            int score=-negamax(depth-1,-INF,INF);
            toggleTurn();
            int chaos=calculateChaosMetric();
            undoTemporaryMove(rootMoves[i],capturedPiece);
            if(bestMove.from<0 || score>bestScore+CHAOS_SCORE_WINDOW ||
               (score>=bestScore-CHAOS_SCORE_WINDOW && chaos>bestChaos)){
                bestMove=rootMoves[i]; bestScore=score; bestChaos=chaos;
            }
        }
        printf("Leaf Evaluations:%lld\n",counter);
        return bestMove;
    }

    ParallelRootTask task={0};
    task.snapshot=board; task.turn=currentTurn; task.moveCount=count; task.depth=depth;
    for(int i=0;i<count;i++) task.rootMoves[i]=rootMoves[i];

    SYSTEM_INFO info;
    GetSystemInfo(&info);
    int workerCount=(int)info.dwNumberOfProcessors;
    if(workerCount<1) workerCount=1;
    if(workerCount>4) workerCount=4;
    if(workerCount>count) workerCount=count;

    HANDLE workers[4];
    int started=0;
    for(int i=0;i<workerCount;i++){
        workers[started]=CreateThread(NULL,0,parallelRootWorker,&task,0,NULL);
        if(workers[started]!=NULL) started++;
    }
    if(started==0){
        // Fallback to a sequential root search if thread creation fails.
        rootDepth=depth;
        int bestScore=-INF,bestChaos=-1;
        for(int i=0;i<count;i++){
            board=task.snapshot; currentTurn=task.turn;
            Piece capturedPiece;
            makeTemporaryMove(rootMoves[i],&capturedPiece);
            toggleTurn();
            int score=-negamax(depth-1,-INF,INF);
            toggleTurn();
            int chaos=calculateChaosMetric();
            undoTemporaryMove(rootMoves[i],capturedPiece);
            if(bestMove.from<0 || score>bestScore+CHAOS_SCORE_WINDOW ||
               (score>=bestScore-CHAOS_SCORE_WINDOW && chaos>bestChaos)){
                bestMove=rootMoves[i]; bestScore=score; bestChaos=chaos;
            }
        }
    }else{
        WaitForMultipleObjects(started,workers,TRUE,INFINITE);
        for(int i=0;i<started;i++) CloseHandle(workers[i]);
        int bestScore=-INF;
        for(int i=0;i<count;i++) if(task.scores[i]>bestScore) bestScore=task.scores[i];
        int chosenScore=-INF, chosenChaos=-1;
        for(int i=0;i<count;i++){
            if(task.scores[i]>=bestScore-CHAOS_SCORE_WINDOW &&
               (bestMove.from<0 || task.chaos[i]>chosenChaos)){
                bestMove=rootMoves[i]; chosenScore=task.scores[i]; chosenChaos=task.chaos[i];
            }
        }
    }
    printf("Parallel root search: %d workers, depth %d\n",started,depth);
    printf("Leaf Evaluations:%lld (per-worker counters are local)\n",counter);
    return bestMove;
}
