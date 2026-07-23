#include "search.h"
#include "board.h"
#include "evaluate.h"

static Move bestMove;
static int rootDepth;
static long long counter=0;

int negamax(int depth, int alpha, int beta){
    counter++;
    
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

    int bestScore= -INF;

    for(int i=0;i<moveCount;i++){
        Piece capturedPiece;
        makeTemporaryMove(moveList[i],&capturedPiece);

        toggleTurn();
        int score=-negamax(depth-1,-beta,-alpha);
        
        toggleTurn();
        undoTemporaryMove(moveList[i],capturedPiece);

        if(score>bestScore){
            bestScore=score;

            if(depth==rootDepth){
                bestMove=moveList[i];
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
    negamax(depth,-INF,INF);
    printf("Leaf Evaluations:%lld\n",counter);
    return bestMove;
}