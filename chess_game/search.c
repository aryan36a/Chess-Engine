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
    int moveCount=generateAllMoves(currentTurn==WHITE_TURN,moveList);

    if(moveCount==0){
        if(isKingInCheck(currentTurn==WHITE_TURN)){
            return -INF+depth;
        }
        return 0;
    }

    int bestScore=-INF;
    int selectedMoveScore=-INF;
    int bestChaos=-1;

    for(int i=0;i<moveCount;i++){
        Piece capturedPiece;
        makeTemporaryMove(moveList[i],&capturedPiece);

        toggleTurn();
        // Root candidates need exact scores for a meaningful chaos tie-break.
        // Internal nodes still use the normal alpha-beta window.
        int score=(depth==rootDepth)
            ? -negamax(depth-1,-INF,INF)
            : -negamax(depth-1,-beta,-alpha);

        // Measure the position after each candidate root move, while its
        // board state is still active. Chaos is only a root-level tie-breaker.
        int candidateChaos=-1;
        if(depth==rootDepth){
            candidateChaos=calculateChaosMetric();
        }

        toggleTurn();
        undoTemporaryMove(moveList[i],capturedPiece);

        if(score>bestScore){
            bestScore=score;
        }

        if(depth==rootDepth){
            bool candidateWithinWindow=(score>=bestScore-CHAOS_SCORE_WINDOW);
            bool selectedOutsideWindow=(selectedMoveScore<bestScore-CHAOS_SCORE_WINDOW);

            if(candidateWithinWindow &&
               (bestMove.from<0 || selectedOutsideWindow || candidateChaos>bestChaos)){
                bestMove=moveList[i];
                selectedMoveScore=score;
                bestChaos=candidateChaos;
            }
        }

        if(score>alpha){
            alpha=score;
        }
        if(alpha>=beta){
            break;
        }
    }

    return bestScore;
}

Move findBestMove(int depth){
    bestMove.from=-1;
    bestMove.to=-1;
    rootDepth=depth;
    counter=0;
    __atomic_store_n(&liveNodeCount, 0ULL, __ATOMIC_RELAXED);
    negamax(depth,-INF,INF);
    printf("Leaf Evaluations:%lld\n",counter);
    return bestMove;
}