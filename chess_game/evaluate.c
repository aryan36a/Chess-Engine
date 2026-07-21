#include "evaluate.h"
#include "board.h"

#define PAWN_VALUE    100
#define KNIGHT_VALUE  320
#define BISHOP_VALUE  330
#define ROOK_VALUE    500
#define QUEEN_VALUE   900

int evaluate(void){
    int whiteScore=0;
    int blackScore=0;

    whiteScore+=countPieces(board.whitePawns)*PAWN_VALUE;
    whiteScore+=countPieces(board.whiteKnights)*KNIGHT_VALUE;
    whiteScore+=countPieces(board.whiteBishops)*BISHOP_VALUE;
    whiteScore+=countPieces(board.whiteRooks)*ROOK_VALUE;
    whiteScore+=countPieces(board.whiteQueens)*QUEEN_VALUE;

    blackScore+=countPieces(board.blackPawns)*PAWN_VALUE;
    blackScore+=countPieces(board.blackKnights)*KNIGHT_VALUE;
    blackScore+=countPieces(board.blackBishops)*BISHOP_VALUE;
    blackScore+=countPieces(board.blackRooks)*ROOK_VALUE;
    blackScore+=countPieces(board.blackQueens)*QUEEN_VALUE;

    return whiteScore-blackScore;
}