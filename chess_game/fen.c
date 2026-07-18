#include<ctype.h>
#include<string.h>
#include "fen.h"
#include "board.h"

static void placePiece(char piece, int square)
{
    switch(piece)
    {
        case 'P': board.whitePawns   |= 1ULL << square; break;
        case 'N': board.whiteKnights |= 1ULL << square; break;
        case 'B': board.whiteBishops |= 1ULL << square; break;
        case 'R': board.whiteRooks   |= 1ULL << square; break;
        case 'Q': board.whiteQueens  |= 1ULL << square; break;
        case 'K': board.whiteKing    |= 1ULL << square; break;

        case 'p': board.blackPawns   |= 1ULL << square; break;
        case 'n': board.blackKnights |= 1ULL << square; break;
        case 'b': board.blackBishops |= 1ULL << square; break;
        case 'r': board.blackRooks   |= 1ULL << square; break;
        case 'q': board.blackQueens  |= 1ULL << square; break;
        case 'k': board.blackKing    |= 1ULL << square; break;
    }
}

bool loadFEN(const char *fen){
    if(fen==NULL){
        return false;
    }

    clearBoard();

    int square=0;

    while(*fen && *fen!=' '){
        if(*fen=='/'){
            fen++;
            continue;
        }

        if(isdigit((unsigned char)*fen)){
            square+=*fen-'0';
            fen++;
            continue;
        }

        if(square>=64){
            return false;
        }

        placePiece(*fen,square);
        square++;
        fen++;
    }

    if(square!=64){
        return false;
    }

    if(*fen==' '){
        fen++;
    }

    if(*fen=='w'){
        currentTurn=WHITE_TURN;
    }
    else if(*fen=='b'){
        currentTurn=BLACK_TURN;
    }
    else{
        return false;
    }

    fen++;

    if(*fen==' '){
        fen++;
    }

    board.whiteKingMoved=true;
    board.blackKingMoved=true;

    board.whiteKingsideRookMoved=true;
    board.whiteQueensideRookMoved=true;

    board.blackKingsideRookMoved=true;
    board.blackQueensideRookMoved=true;

    while(*fen && *fen!=' '){
        switch(*fen){
            case 'K':
                board.whiteKingMoved=false;
                board.whiteKingsideRookMoved=false;
                break;

            case 'Q':
                board.whiteKingMoved=false;
                board.whiteQueensideRookMoved=false;
                break;

            case 'k':
                board.blackKingMoved=false;
                board.blackKingsideRookMoved=false;
                break;

            case 'q':
                board.blackKingMoved=false;
                board.blackQueensideRookMoved=false;
                break;

            case '-':
                break;

            default:
                return false;
        }

        fen++;
    }

    if(*fen==' '){
        fen++;
    }

    if(*fen=='-'){
        board.enPassantSquare=-1;
        fen++;
    }
    else{
        int file=*fen-'a';
        fen++;

        int rank='8'-*fen;
        fen++;

        board.enPassantSquare=rank*8+file;
    }

    if(*fen==' '){
        fen++;
    }

    board.halfMoveClock=0;

    while(isdigit((unsigned char)*fen)){
        board.halfMoveClock=board.halfMoveClock*10+(*fen-'0');
        fen++;
    }

    if(*fen==' '){
        fen++;
    }

    while(isdigit((unsigned char)*fen)){
        fen++;
    }
    
    updateOccupancy();
    
    board.positionHistory[0]=generateHash();
    board.historyCount=1;

    return true;
}