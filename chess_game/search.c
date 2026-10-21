#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "search.h"
#include "board.h"
#include "evaluate.h"

#define TT_BITS 16
#define TT_SIZE (1u << TT_BITS)
#define MAX_SEARCH_DEPTH 8
#define MATE_SCORE (INF-1000)

typedef struct {
    uint64_t key;
    int score;
    int depth;
    int bound; /* 0 exact, 1 lower bound, 2 upper bound */
    Move best;
    unsigned char valid;
} TTEntry;

static _Thread_local TTEntry transpositionTable[TT_SIZE];
static _Thread_local Move killerMoves[128][2];
static _Thread_local int historyHeuristic[2][64][64];
static _Thread_local long long localNodes;
static unsigned long long liveNodeCount;
static volatile LONG publishedSearchScore;
static SearchRootAnalysis lastRootAnalysis;

unsigned long long GetSearchNodeCount(void){
    return __atomic_load_n(&liveNodeCount, __ATOMIC_RELAXED);
}

int GetSearchScore(void){
    return (int)InterlockedCompareExchange(&publishedSearchScore, 0, 0);
}

void GetSearchRootAnalysis(SearchRootAnalysis *out){
    if (out) *out = lastRootAnalysis;
}

static uint64_t positionKey(void){
    /* Deterministic key independent of thread-local Zobrist initialization. */
    uint64_t h=1469598103934665603ULL;
    const uint64_t pieces[]={
        board.whitePawns,board.whiteKnights,board.whiteBishops,board.whiteRooks,
        board.whiteQueens,board.whiteKing,board.blackPawns,board.blackKnights,
        board.blackBishops,board.blackRooks,board.blackQueens,board.blackKing
    };
    for(unsigned i=0;i<sizeof(pieces)/sizeof(pieces[0]);i++){
        h^=pieces[i]; h*=1099511628211ULL;
    }
    h^=(uint64_t)currentTurn; h*=1099511628211ULL;
    h^=(uint64_t)(board.enPassantSquare+1); h*=1099511628211ULL;
    h^=(uint64_t)board.whiteKingMoved | ((uint64_t)board.blackKingMoved<<1)
      |((uint64_t)board.whiteKingsideRookMoved<<2)
      |((uint64_t)board.whiteQueensideRookMoved<<3)
      |((uint64_t)board.blackKingsideRookMoved<<4)
      |((uint64_t)board.blackQueensideRookMoved<<5);
    h*=1099511628211ULL;
    return h ? h : 1ULL;
}

static int pieceValue(Piece p){
    switch(p){
        case WHITE_PAWN: case BLACK_PAWN:return 100;
        case WHITE_KNIGHT: case BLACK_KNIGHT:return 320;
        case WHITE_BISHOP: case BLACK_BISHOP:return 330;
        case WHITE_ROOK: case BLACK_ROOK:return 500;
        case WHITE_QUEEN: case BLACK_QUEEN:return 900;
        case WHITE_KING: case BLACK_KING:return 20000;
        default:return 0;
    }
}

static bool sameMove(Move a,Move b){ return a.from==b.from && a.to==b.to; }

static int moveOrderScore(Move m, Move ttMove, int ply){
    if(ttMove.from>=0 && sameMove(m,ttMove)) return 2000000;
    Piece attacker=GetPieceAtSquare(m.from), victim=GetPieceAtSquare(m.to);
    if((attacker==WHITE_PAWN || attacker==BLACK_PAWN) &&
       m.to==board.enPassantSquare && m.from%8!=m.to%8){
        victim=(attacker==WHITE_PAWN)?BLACK_PAWN:WHITE_PAWN;
    }
    if(victim!=EMPTY) return 1000000+pieceValue(victim)*10-pieceValue(attacker);
    if(ply<128 && sameMove(m,killerMoves[ply][0])) return 900000;
    if(ply<128 && sameMove(m,killerMoves[ply][1])) return 899000;
    int side=(currentTurn==WHITE_TURN)?0:1;
    return historyHeuristic[side][m.from][m.to];
}

static void orderMoves(Move list[],int count,Move ttMove,int ply){
    int scores[256];
    for(int i=0;i<count;i++) scores[i]=moveOrderScore(list[i],ttMove,ply);
    for(int i=1;i<count;i++){
        Move m=list[i]; int s=scores[i],j=i-1;
        while(j>=0 && scores[j]<s){
            list[j+1]=list[j]; scores[j+1]=scores[j]; j--;
        }
        list[j+1]=m; scores[j+1]=s;
    }
}

static int negamaxInternal(int depth,int alpha,int beta,int ply){
    localNodes++;
    __atomic_add_fetch(&liveNodeCount,1ULL,__ATOMIC_RELAXED);
    if(depth<=0) return (currentTurn==WHITE_TURN)?evaluate():-evaluate();
    if(ply>=120) return (currentTurn==WHITE_TURN)?evaluate():-evaluate();

    uint64_t key=positionKey();
    TTEntry *entry=&transpositionTable[key & (TT_SIZE-1)];
    Move ttMove={-1,-1};
    int originalAlpha=alpha;
    if(entry->valid && entry->key==key){
        ttMove=entry->best;
        if(entry->depth>=depth){
            if(entry->bound==0) return entry->score;
            if(entry->bound==1 && entry->score>alpha) alpha=entry->score;
            if(entry->bound==2 && entry->score<beta) beta=entry->score;
            if(alpha>=beta) return entry->score;
        }
    }

    Move list[256];
    int count=generateAllMoves(currentTurn==WHITE_TURN,list);
    if(count==0) return isKingInCheck(currentTurn==WHITE_TURN)?(-MATE_SCORE+ply):0;
    orderMoves(list,count,ttMove,ply);

    int best=-INF;
    Move bestLocal={-1,-1};
    for(int i=0;i<count;i++){
        Piece captured;
        makeTemporaryMove(list[i],&captured);
        toggleTurn();
        int score=-negamaxInternal(depth-1,-beta,-alpha,ply+1);
        toggleTurn();
        undoTemporaryMove(list[i],captured);

        if(score>best){ best=score; bestLocal=list[i]; }
        if(score>alpha) alpha=score;
        if(alpha>=beta){
            Piece target=GetPieceAtSquare(list[i].to);
            if(target==EMPTY && ply<128){
                if(!sameMove(list[i],killerMoves[ply][0])){
                    killerMoves[ply][1]=killerMoves[ply][0];
                    killerMoves[ply][0]=list[i];
                }
                int side=(currentTurn==WHITE_TURN)?0:1;
                historyHeuristic[side][list[i].from][list[i].to]+=depth*depth;
                if(historyHeuristic[side][list[i].from][list[i].to]>1000000)
                    historyHeuristic[side][list[i].from][list[i].to]=1000000;
            }
            break;
        }
    }
    entry->key=key; entry->depth=depth; entry->score=best; entry->best=bestLocal; entry->valid=1;
    entry->bound=(best<=originalAlpha)?2:(best>=beta)?1:0;
    return best;
}

int negamax(int depth,int alpha,int beta){
    return negamaxInternal(depth,alpha,beta,0);
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
    SearchRootCandidate candidates[256];
    volatile LONG64 nodes;
} ParallelRootTask;

static DWORD WINAPI parallelRootWorker(LPVOID argument){
    ParallelRootTask *task=(ParallelRootTask*)argument;
    memset(transpositionTable,0,sizeof(transpositionTable));
    memset(killerMoves,0xFF,sizeof(killerMoves));
    memset(historyHeuristic,0,sizeof(historyHeuristic));
    localNodes=0;
    for(;;){
        LONG index=InterlockedIncrement(&task->nextIndex)-1;
        if(index>=task->moveCount) break;
        Move root=task->rootMoves[index];
        int score=0;
        Piece moving=GetPieceAtSquare(root.from);
        /* Iterative deepening per root candidate keeps its TT warm across depths. */
        for(int d=1;d<=task->depth;d++){
            board=task->snapshot; currentTurn=task->turn;
            Piece captured;
            makeTemporaryMove(root,&captured);
            toggleTurn();
            score=-negamaxInternal(d-1,-INF,INF,1);
            toggleTurn();
            undoTemporaryMove(root,captured);
        }
        board=task->snapshot; currentTurn=task->turn;
        Piece captured;
        makeTemporaryMove(root,&captured);
        toggleTurn();
        task->chaos[index]=calculateChaosMetric();
        task->candidates[index].move=root;
        task->candidates[index].captured=captured;
        task->candidates[index].givesCheck=isKingInCheck(currentTurn==WHITE_TURN);
        task->candidates[index].isPromotion=
            (moving==WHITE_PAWN && root.to<8) ||
            (moving==BLACK_PAWN && root.to>=56);
        task->candidates[index].materialEval=evaluateMaterial();
        task->candidates[index].positionalEval=evaluatePositional();
        task->candidates[index].depth=task->depth;
        task->candidates[index].bound=0;
        task->candidates[index].pv[0]='\0';
        toggleTurn();
        undoTemporaryMove(root,captured);
        task->scores[index]=score;
        task->candidates[index].score=score;
    }
    InterlockedAdd64(&task->nodes,(LONG64)localNodes);
    return 0;
}

Move findBestMove(int depth){
    Move best={-1,-1};
    __atomic_store_n(&liveNodeCount,0ULL,__ATOMIC_RELAXED);
    if(depth<1) depth=1;
    if(depth>MAX_SEARCH_DEPTH) depth=MAX_SEARCH_DEPTH;

    Move rootMoves[256];
    int count=generateAllMoves(currentTurn==WHITE_TURN,rootMoves);
    if(count<=0){
        memset(&lastRootAnalysis,0,sizeof(lastRootAnalysis));
        lastRootAnalysis.selectedMove=best;
        lastRootAnalysis.sideToMove=currentTurn;
        InterlockedExchange(&publishedSearchScore,0);
        return best;
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
        HANDLE thread=CreateThread(NULL,0,parallelRootWorker,&task,0,NULL);
        if(thread) workers[started++]=thread;
    }
    if(started>0){
        WaitForMultipleObjects(started,workers,TRUE,INFINITE);
        for(int i=0;i<started;i++) CloseHandle(workers[i]);
    }else{
        /* Sequential fallback */
        parallelRootWorker(&task);
    }

    int bestScore=-INF;
    for(int i=0;i<count;i++) if(task.scores[i]>bestScore) bestScore=task.scores[i];
    InterlockedExchange(&publishedSearchScore,bestScore);
    int chosenChaos=-1;
    for(int i=0;i<count;i++){
        if(task.scores[i]>=bestScore-CHAOS_SCORE_WINDOW &&
           (best.from<0 || task.chaos[i]>chosenChaos)){
            best=task.rootMoves[i]; chosenChaos=task.chaos[i];
        }
    }
    memset(&lastRootAnalysis,0,sizeof(lastRootAnalysis));
    lastRootAnalysis.candidateCount=count;
    lastRootAnalysis.completedDepth=depth;
    lastRootAnalysis.selectedMove=best;
    lastRootAnalysis.selectedScore=bestScore;
    lastRootAnalysis.highestScore=bestScore;
    lastRootAnalysis.rootMaterialEval=evaluateMaterial();
    lastRootAnalysis.rootPositionalEval=evaluatePositional();
    lastRootAnalysis.sideToMove=task.turn;
    lastRootAnalysis.nodes=(unsigned long long)task.nodes;
    for(int i=0;i<count;i++){
        lastRootAnalysis.candidates[i]=task.candidates[i];
        lastRootAnalysis.candidates[i].selected=
            sameMove(task.rootMoves[i],best);
    }
    for(int i=1;i<count;i++){
        SearchRootCandidate value=lastRootAnalysis.candidates[i];
        int j=i;
        while(j>0 &&
              lastRootAnalysis.candidates[j-1].score<value.score){
            lastRootAnalysis.candidates[j]=lastRootAnalysis.candidates[j-1];
            --j;
        }
        lastRootAnalysis.candidates[j]=value;
    }
    for(int i=0;i<count;i++){
        if(lastRootAnalysis.candidates[i].selected)
            lastRootAnalysis.selectedScore=lastRootAnalysis.candidates[i].score;
    }
    fprintf(stderr,"Parallel root search: %d workers, iterative depth 1-%d\n",started,depth);
    fprintf(stderr,"Search nodes: %llu\n",GetSearchNodeCount());
    return best;
}
