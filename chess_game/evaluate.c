#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>

#include "evaluate.h"
#include "board.h"

#define PAWN_VALUE    100
#define KNIGHT_VALUE  320
#define BISHOP_VALUE  330
#define ROOK_VALUE    500
#define QUEEN_VALUE   900
#define WHITE 0
#define BLACK 56


 const int pawnTable[64]={
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0

};

const int knightTable[64] = {
   -50,-40,-30,-30,-30,-30,-40,-50,
   -40,-20,  0,  0,  0,  0,-20,-40,
   -30,  0, 10, 15, 15, 10,  0,-30,
   -30,  5, 15, 20, 20, 15,  5,-30,
   -30,  0, 15, 20, 20, 15,  0,-30,
   -30,  5, 10, 15, 15, 10,  5,-30,
   -40,-20,  0,  5,  5,  0,-20,-40,
   -50,-40,-30,-30,-30,-30,-40,-50
};

const int bishopTable[64] = {
   -20,-10,-10,-10,-10,-10,-10,-20,
   -10,  5,  0,  0,  0,  0,  5,-10,
   -10, 10, 10, 10, 10, 10, 10,-10,
   -10,  0, 10, 10, 10, 10,  0,-10,
   -10,  5,  5, 10, 10,  5,  5,-10,
   -10,  0,  5, 10, 10,  5,  0,-10,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -20,-10,-10,-10,-10,-10,-10,-20
};

const int rookTable[64] = {
     0,  0,  5, 10, 10,  5,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     5, 10, 10, 10, 10, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

const int queenTable[64] = {
   -20,-10,-10, -5, -5,-10,-10,-20,
   -10,  0,  0,  0,  0,  0,  0,-10,
   -10,  0,  5,  5,  5,  5,  0,-10,
    -5,  0,  5,  5,  5,  5,  0, -5,
     0,  0,  5,  5,  5,  5,  0, -5,
   -10,  5,  5,  5,  5,  5,  0,-10,
   -10,  0,  5,  0,  0,  0,  0,-10,
   -20,-10,-10, -5, -5,-10,-10,-20
};

const int kingTable[64] = {
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -30,-40,-40,-50,-50,-40,-40,-30,
   -20,-30,-30,-40,-40,-30,-30,-20,
   -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

//POP LSB 
static inline int popLSB(uint64_t *bb){
    int sq=__builtin_ctzll(*bb);
    *bb&=*bb-1;
    return sq;
}

//Helper
static int evaluateBitboard(uint64_t pieces,const int table[64],int value,int isWhite){
    int score=0;
    while(pieces){
        int sq=popLSB(&pieces);
        score+=value+table[sq^isWhite];
    }
    return score;
}

int evaluate(void){
    int whiteScore=0;
    int blackScore=0;

    whiteScore+=evaluateBitboard(board.whitePawns,pawnTable,PAWN_VALUE,WHITE);
    whiteScore+=evaluateBitboard(board.whiteKnights,knightTable,KNIGHT_VALUE,WHITE);
    whiteScore+=evaluateBitboard(board.whiteRooks,rookTable,ROOK_VALUE,WHITE);
    whiteScore+=evaluateBitboard(board.whiteBishops,bishopTable,BISHOP_VALUE,WHITE);
    whiteScore+=evaluateBitboard(board.whiteQueens,queenTable,QUEEN_VALUE,WHITE);
    whiteScore+=evaluateBitboard(board.whiteKing,kingTable,0,WHITE);

    blackScore+=evaluateBitboard(board.blackPawns,pawnTable,PAWN_VALUE,BLACK);
    blackScore+=evaluateBitboard(board.blackKnights,knightTable,KNIGHT_VALUE,BLACK);
    blackScore+=evaluateBitboard(board.blackRooks,rookTable,ROOK_VALUE,BLACK);
    blackScore+=evaluateBitboard(board.blackBishops,bishopTable,BISHOP_VALUE,BLACK);
    blackScore+=evaluateBitboard(board.blackQueens,queenTable,QUEEN_VALUE,BLACK);
    blackScore+=evaluateBitboard(board.blackKing,kingTable,0,BLACK);

    return whiteScore-blackScore;
}

// Quantifies tactical complexity rather than positional advantage.
// Used only as a tie-breaker between root moves with near-equal evaluations.
int calculateChaosMetric(void)
{
    Move whiteMoves[256];
    Move blackMoves[256];
    int whiteMobility = generateAllMoves(true, whiteMoves);
    int blackMobility = generateAllMoves(false, blackMoves);
    int captureMoves = 0;
    int attackedPieces = 0;
    int hangingPieces = 0;
    int contestedPieces = 0;

    for (int i = 0; i < whiteMobility; ++i) {
        Piece target = GetPieceAtSquare(whiteMoves[i].to);
        Piece moving = GetPieceAtSquare(whiteMoves[i].from);
        if (isBlackPiece(target) ||
            ((moving == WHITE_PAWN) &&
             whiteMoves[i].to == board.enPassantSquare &&
             whiteMoves[i].from % 8 != whiteMoves[i].to % 8)) {
            ++captureMoves;
        }
    }

    for (int i = 0; i < blackMobility; ++i) {
        Piece target = GetPieceAtSquare(blackMoves[i].to);
        Piece moving = GetPieceAtSquare(blackMoves[i].from);
        if (isWhitePiece(target) ||
            ((moving == BLACK_PAWN) &&
             blackMoves[i].to == board.enPassantSquare &&
             blackMoves[i].from % 8 != blackMoves[i].to % 8)) {
            ++captureMoves;
        }
    }

    for (int square = 0; square < 64; ++square) {
        Piece piece = GetPieceAtSquare(square);
        if (piece == EMPTY) {
            continue;
        }

        bool whitePiece = isWhitePiece(piece);
        bool attackedByWhite = isSquareAttacked(square, true);
        bool attackedByBlack = isSquareAttacked(square, false);
        bool attackedByEnemy = whitePiece ? attackedByBlack : attackedByWhite;
        bool defendedByOwnSide = whitePiece ? attackedByWhite : attackedByBlack;

        if (attackedByEnemy) {
            ++attackedPieces;
            if (!defendedByOwnSide && piece != WHITE_KING && piece != BLACK_KING) {
                ++hangingPieces;
            }
        }
        if (attackedByWhite && attackedByBlack) {
            ++contestedPieces;
        }
    }

    int checks = (isKingInCheck(true) ? 1 : 0) +
                 (isKingInCheck(false) ? 1 : 0);

    return (whiteMobility + blackMobility) * 2
         + captureMoves * 8
         + attackedPieces * 5
         + hangingPieces * 12
         + contestedPieces * 4
         + checks * 20;
}

void printEvaluationBreakdown(void)
{
    printf("White Pawns   %d\n", evaluateBitboard(board.whitePawns, pawnTable, PAWN_VALUE, WHITE));
    printf("Black Pawns   %d\n", evaluateBitboard(board.blackPawns, pawnTable, PAWN_VALUE, BLACK));

    printf("White Knights %d\n", evaluateBitboard(board.whiteKnights, knightTable, KNIGHT_VALUE, WHITE));
    printf("Black Knights %d\n", evaluateBitboard(board.blackKnights, knightTable, KNIGHT_VALUE, BLACK));

    printf("White Bishops %d\n", evaluateBitboard(board.whiteBishops, bishopTable, BISHOP_VALUE, WHITE));
    printf("Black Bishops %d\n", evaluateBitboard(board.blackBishops, bishopTable, BISHOP_VALUE, BLACK));

    printf("White Rooks   %d\n", evaluateBitboard(board.whiteRooks, rookTable, ROOK_VALUE, WHITE));
    printf("Black Rooks   %d\n", evaluateBitboard(board.blackRooks, rookTable, ROOK_VALUE, BLACK));

    printf("White Queens  %d\n", evaluateBitboard(board.whiteQueens, queenTable, QUEEN_VALUE, WHITE));
    printf("Black Queens  %d\n", evaluateBitboard(board.blackQueens, queenTable, QUEEN_VALUE, BLACK));

    printf("White King    %d\n", evaluateBitboard(board.whiteKing, kingTable, 0, WHITE));
    printf("Black King    %d\n", evaluateBitboard(board.blackKing, kingTable, 0, BLACK));

    printf("White queens: %d\n", countPieces(board.whiteQueens));
    printf("Black queens: %d\n", countPieces(board.blackQueens));
}