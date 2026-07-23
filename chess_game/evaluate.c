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