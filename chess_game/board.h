#ifndef BOARD_H
#define BOARD_H

#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>

//Enums
typedef enum{
    EMPTY=0,

    WHITE_PAWN,
    WHITE_ROOK,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_QUEEN,
    WHITE_KING,

    BLACK_PAWN,
    BLACK_ROOK,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_QUEEN,
    BLACK_KING
}Piece;

typedef enum{
    WHITE_TURN,
    BLACK_TURN
} Turn;

//Board Structure
typedef struct
{
    uint64_t whitePawns;
    uint64_t whiteKnights;
    uint64_t whiteBishops;
    uint64_t whiteRooks;
    uint64_t whiteQueens;
    uint64_t whiteKing;

    uint64_t blackPawns;
    uint64_t blackKnights;
    uint64_t blackBishops;
    uint64_t blackRooks;
    uint64_t blackQueens;
    uint64_t blackKing;

    uint64_t whitePieces;
    uint64_t blackPieces;
    uint64_t occupied;

    bool whiteKingMoved;
    bool blackKingMoved;

    bool whiteKingsideRookMoved;
    bool whiteQueensideRookMoved;

    bool blackKingsideRookMoved;
    bool blackQueensideRookMoved;
    
    int enPassantSquare;

}Board;

//Move Structure
typedef struct{
    int from;
    int to;
}Move;

typedef struct{
    bool active;
    int square;
    Piece pawn;
}PromotionMenu;

extern PromotionMenu promotionMenu;

//Global Variables
extern Board board;

extern Move moves[32];

extern int moveCount;
extern int selectedSquare;
extern Turn currentTurn;

extern Piece selectedPiece;

//Board Functions
void initBoard(void);

bool isBitSet(uint64_t bitboard,int square);

Piece GetPieceAtSquare(int square);

bool isLegalMove(int square);

bool isWhitePiece(Piece piece);

bool isBlackPiece(Piece piece);

void toggleTurn(void);

void promotePawn(int square,Piece promotedPiece);

//Piece Move Generation
void generateWhitePawnMoves(int square);
void generateBlackPawnMoves(int square);
void generateKnightMoves(int square);
void generateRookMoves(int square);
void generateBishopMoves(int square);
void generateQueenMoves(int square);
void generateKingMoves(int square);

//Piece Movement
void movePiece(uint64_t *bitboard,int from,int to);
void performCastle(int from,int to);
//Capture
void removePiece(int square);

#endif