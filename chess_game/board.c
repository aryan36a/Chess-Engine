#include "board.h"
#include "attacks.h"

Board board;

Move moves[32];

int moveCount=0;
int selectedSquare=-1;
Piece selectedPiece=EMPTY;

Turn currentTurn=WHITE_TURN;

PromotionMenu promotionMenu={false,-1,EMPTY};

static Board temporaryBoard;

bool isWhitePiece(Piece piece){
    return piece>=WHITE_PAWN && piece<=WHITE_KING;
}
bool isBlackPiece(Piece piece){
    return piece>=BLACK_PAWN && piece<=BLACK_KING;
}

void initBoard(void)

{
    // Clear everything
    board.whitePawns   = 0ULL;
    board.whiteKnights = 0ULL;
    board.whiteBishops = 0ULL;
    board.whiteRooks   = 0ULL;
    board.whiteQueens  = 0ULL;
    board.whiteKing    = 0ULL;

    board.blackPawns   = 0ULL;
    board.blackKnights = 0ULL;
    board.blackBishops = 0ULL;
    board.blackRooks   = 0ULL;
    board.blackQueens  = 0ULL;
    board.blackKing    = 0ULL;

    board.whiteKingMoved = false;
    board.blackKingMoved = false;

    board.whiteKingsideRookMoved = false;
    board.whiteQueensideRookMoved = false;

    board.blackKingsideRookMoved = false;
    board.blackQueensideRookMoved = false;

    board.enPassantSquare=-1;

    //White Pieces

    //Pawns(a2-h2)
    for (int i = 48; i <= 55; i++)
        board.whitePawns |= (1ULL << i);

    //Rooks
    board.whiteRooks |= (1ULL << 56);
    board.whiteRooks |= (1ULL << 63);

    //Knights
    board.whiteKnights |= (1ULL << 57);
    board.whiteKnights |= (1ULL << 62);

    //Bishops
    board.whiteBishops |= (1ULL << 58);
    board.whiteBishops |= (1ULL << 61);

    //Queen
    board.whiteQueens |= (1ULL << 59);

    //King
    board.whiteKing |= (1ULL << 60);

    //Black Pieces

    //Pawns (a7-h7)
    for (int i = 8; i <= 15; i++)
        board.blackPawns |= (1ULL << i);

    //Rooks
    board.blackRooks |= (1ULL << 0);
    board.blackRooks |= (1ULL << 7);

    //Knights
    board.blackKnights |= (1ULL << 1);
    board.blackKnights |= (1ULL << 6);

    //Bishops
    board.blackBishops |= (1ULL << 2);
    board.blackBishops |= (1ULL << 5);

    //Queen
    board.blackQueens |= (1ULL << 3);

    //King
    board.blackKing |= (1ULL << 4);

    //Occupied Places
    board.whitePieces =
        board.whitePawns   |
        board.whiteKnights |
        board.whiteBishops |
        board.whiteRooks   |
        board.whiteQueens  |
        board.whiteKing;

    board.blackPieces =
        board.blackPawns   |
        board.blackKnights |
        board.blackBishops |
        board.blackRooks   |
        board.blackQueens  |
        board.blackKing;

    board.occupied =
        board.whitePieces |
        board.blackPieces;
}

bool isBitSet(uint64_t bitboard, int square){
    return (bitboard&(1ULL<<square))!=0;
}

Piece GetPieceAtSquare(int square)
{
    if (isBitSet(board.whitePawns, square))   return WHITE_PAWN;
    if (isBitSet(board.whiteRooks, square))   return WHITE_ROOK;
    if (isBitSet(board.whiteKnights, square)) return WHITE_KNIGHT;
    if (isBitSet(board.whiteBishops, square)) return WHITE_BISHOP;
    if (isBitSet(board.whiteQueens, square))  return WHITE_QUEEN;
    if (isBitSet(board.whiteKing, square))    return WHITE_KING;

    if (isBitSet(board.blackPawns, square))   return BLACK_PAWN;
    if (isBitSet(board.blackRooks, square))   return BLACK_ROOK;
    if (isBitSet(board.blackKnights, square)) return BLACK_KNIGHT;
    if (isBitSet(board.blackBishops, square)) return BLACK_BISHOP;
    if (isBitSet(board.blackQueens, square))  return BLACK_QUEEN;
    if (isBitSet(board.blackKing, square))    return BLACK_KING;

    return EMPTY;
}

bool isLegalMove(int square){
    for(int i=0;i<moveCount;i++){
        if(moves[i].to==square){
            return true;
        }
    }
    return false;
}

//Update Occupancy
void updateOccupancy(void)
{
    board.whitePieces =
        board.whitePawns   |
        board.whiteKnights |
        board.whiteBishops |
        board.whiteRooks   |
        board.whiteQueens  |
        board.whiteKing;

    board.blackPieces =
        board.blackPawns   |
        board.blackKnights |
        board.blackBishops |
        board.blackRooks   |
        board.blackQueens  |
        board.blackKing;

    board.occupied =
        board.whitePieces |
        board.blackPieces;
}

//Toggle Turn
void toggleTurn(void){
    if(currentTurn==WHITE_TURN){
        currentTurn=BLACK_TURN;
    }else{
        currentTurn=WHITE_TURN;
    }
}

//General Moves

//Move in a line(h+v)
//Direction Array
const int straightRow[4]={-1,1,0,0};
const int straightCol[4]={0,0,-1,1};
const int diagonalRow[4]={-1,-1,1,1};
const int diagonalCol[4]={-1,1,-1,1};
void generateStraightMoves(int square,const int rowDir[],const int colDir[],int directionCount){
    
    int row=square/8;
    int col=square%8;
    uint64_t friendlyPieces;
    uint64_t enemyPieces;
    if(selectedPiece<=WHITE_KING){
        friendlyPieces=board.whitePieces;
        enemyPieces=board.blackPieces;
    }else{
        friendlyPieces=board.blackPieces;
        enemyPieces=board.whitePieces;
    }
    for(int dir=0;dir<directionCount;dir++){
        int newRow=row;
        int newCol=col;

        while(1){
            newRow+=rowDir[dir];
            newCol+=colDir[dir];

            if(newRow<0||newRow>7||newCol<0||newCol>7){
                break;
            }
            int destination=newRow*8+newCol;
            if(isBitSet(friendlyPieces,destination)){
                break;
            }
            moves[moveCount].from=square;
            moves[moveCount].to=destination;
            moveCount++;

            if(isBitSet(enemyPieces,destination)){
                break;
            }
        }
    }
}

void promotePawn(int square, Piece promotedPiece){
    switch(promotedPiece){
        case WHITE_QUEEN:
            board.whitePawns&=~(1ULL<<square);
            board.whiteQueens|=(1ULL<<square);
            break;
        case WHITE_ROOK:
            board.whitePawns&=~(1ULL<<square);
            board.whiteRooks|=(1ULL<<square);
            break;
        case WHITE_BISHOP:
            board.whitePawns&=~(1ULL<<square);
            board.whiteBishops|=(1ULL<<square);
            break;
        case WHITE_KNIGHT:
            board.whitePawns&=~(1ULL<<square);
            board.whiteKnights|=(1ULL<<square);
            break;
        case BLACK_QUEEN:
            board.blackPawns&=~(1ULL<<square);
            board.blackQueens|=(1ULL<<square);
            break;
        case BLACK_ROOK:
            board.blackPawns&=~(1ULL<<square);
            board.blackRooks|=(1ULL<<square);
            break;
        case BLACK_BISHOP:
            board.blackPawns&=~(1ULL<<square);
            board.blackBishops|=(1ULL<<square);
            break;
        case BLACK_KNIGHT:
            board.blackPawns&=~(1ULL<<square);
            board.blackKnights|=(1ULL<<square);
            break;
        default:
            return;
    }
    updateOccupancy();
}

//White Pawn Moves
void generateWhitePawnMoves(int square){
    int file=square%8;
    moveCount=0;

    if(file>0){
        int captureLeft=square-9;

        if(captureLeft>=0 && isBitSet(board.blackPieces,captureLeft)){
            moves[moveCount].from=square;
            moves[moveCount].to=captureLeft;
            moveCount++;
        }
    }

    if(file<7){
        int captureRight=square-7;

        if(captureRight>=0 && isBitSet(board.blackPieces,captureRight)){
            moves[moveCount].from=square;
            moves[moveCount].to=captureRight;
            moveCount++;
        }
    }

    if(board.enPassantSquare!=-1){
        if(square-9==board.enPassantSquare){
            moves[moveCount].from=square;
            moves[moveCount].to=board.enPassantSquare;
            moveCount++;
        }
        if(square-7==board.enPassantSquare){
            moves[moveCount].from=square;
            moves[moveCount].to=board.enPassantSquare;
            moveCount++;
        }
    }

    int oneStep=square-8;
    if(oneStep >=0 && !isBitSet(board.occupied,oneStep)){
        moves[moveCount].from=square;
        moves[moveCount].to=oneStep;
        moveCount++;

        //First Move (two places)
        if(square>=48 && square<=55){
            int twoStep=square-16;
            if(!isBitSet(board.occupied,twoStep)){
                moves[moveCount].from=square;
                moves[moveCount].to=twoStep;
                moveCount++;
            }
        }

    }

    filterLegalMoves();
}

//Black Pawn Moves
void generateBlackPawnMoves(int square){
    int file=square%8;
    moveCount=0;

    if(file>0){
        int captureLeft=square+9;

        if(captureLeft>=0 && isBitSet(board.whitePieces,captureLeft)){
            moves[moveCount].from=square;
            moves[moveCount].to=captureLeft;
            moveCount++;
        }
    }

    if(file<7){
        int captureRight=square+7;

        if(captureRight>=0 && isBitSet(board.whitePieces,captureRight)){
            moves[moveCount].from=square;
            moves[moveCount].to=captureRight;
            moveCount++;
        }
    }

    if(board.enPassantSquare!=-1){
        if(square+9==board.enPassantSquare){
            moves[moveCount].from=square;
            moves[moveCount].to=board.enPassantSquare;
            moveCount++;
        }
        if(square+7==board.enPassantSquare){
            moves[moveCount].from=square;
            moves[moveCount].to=board.enPassantSquare;
            moveCount++;
        }
    }

    int oneStep=square+8;
    if(oneStep >=0 && !isBitSet(board.occupied,oneStep)){
        moves[moveCount].from=square;
        moves[moveCount].to=oneStep;
        moveCount++;

        //First Move (two places)
        if(square>=8 && square<=15){
            int twoStep=square+16;
            if(!isBitSet(board.occupied,twoStep)){
                moves[moveCount].from=square;
                moves[moveCount].to=twoStep;
                moveCount++;
            }
        }

    }

    filterLegalMoves();
}

//Knight Moves
//Offset Array for Knights
const int kinghtRowOffset[8]={-2,-2,-1,-1,1,1,2,2};
const int knightColOffset[8]={-1,1,-2,2,-2,2,-1,1};
void generateKnightMoves(int square){
    moveCount=0;

    int row=square/8;
    int col=square%8;

    for(int i=0;i<8;i++){
        int newRow=row+kinghtRowOffset[i];
        int newCol=col+knightColOffset[i];

        if(newRow<0||newRow>7||newCol<0||newCol>7){
            continue;
        }
        int destination=newRow*8+newCol;
        
        if(selectedPiece==WHITE_KNIGHT){
            if(isBitSet(board.whitePieces,destination)){
                continue;
            }
        }else{
            if(isBitSet(board.blackPieces,destination)){
                continue;
            }
        }
        
    moves[moveCount].from=square;
    moves[moveCount].to=destination;
    moveCount++;
}

    filterLegalMoves();
}

//Rook Moves
void generateRookMoves(int square){
    moveCount=0;
    generateStraightMoves(square,straightRow,straightCol,4);
    filterLegalMoves();
}

//Bishop Moves
void generateBishopMoves(int square){
    moveCount=0;
    generateStraightMoves(square,diagonalRow,diagonalCol,4);
    filterLegalMoves();
}

//Queen Moves
void generateQueenMoves(int square){
    moveCount=0;
    generateStraightMoves(square,straightRow,straightCol,4);
    generateStraightMoves(square,diagonalRow,diagonalCol,4);
    filterLegalMoves();
}

//King Moves
//Castling
void generateCastlingMoves(void){
    if(selectedPiece==WHITE_KING){
        if(!board.whiteKingMoved&&!board.whiteKingsideRookMoved&&isBitSet(board.whiteRooks,63)&&!isBitSet(board.occupied,61)
            &&!isBitSet(board.occupied,62)&&!isSquareAttacked(60,false)&&!isSquareAttacked(61,false)&&!isSquareAttacked(62,false)){
            moves[moveCount].from=60;
            moves[moveCount].to=62;
            moveCount++;
        }
        if(!board.whiteKingMoved&&!board.whiteQueensideRookMoved&&isBitSet(board.whiteRooks,56)&&!isBitSet(board.occupied,57)
            &&!isBitSet(board.occupied,58)&&!isBitSet(board.occupied,59)&&!isSquareAttacked(60,false)&&!isSquareAttacked(59,false)&&!isSquareAttacked(58,false)){
            moves[moveCount].from=60;
            moves[moveCount].to=58;
            moveCount++;
        }
    }
    
    if(selectedPiece==BLACK_KING){
        if(!board.blackKingMoved&&!board.blackKingsideRookMoved&&isBitSet(board.blackRooks,7)&&!isBitSet(board.occupied,5)
            &&!isBitSet(board.occupied,6)&&!isSquareAttacked(4,true)&&!isSquareAttacked(5,true)&&!isSquareAttacked(6,true)){
            moves[moveCount].from=4;
            moves[moveCount].to=6;
            moveCount++;
        }
        if(!board.blackKingMoved&&!board.blackQueensideRookMoved&&isBitSet(board.blackRooks,0)&&!isBitSet(board.occupied,1)
            &&!isBitSet(board.occupied,2)&&!isBitSet(board.occupied,3)&&!isSquareAttacked(4,true)&&!isSquareAttacked(3,true)&&!isSquareAttacked(2,true)){
            moves[moveCount].from=4;
            moves[moveCount].to=2;
            moveCount++;
        }
    }
}

void generateKingMoves(int square){
    moveCount=0;
    //King Direction Array
    const int rowDir[8]={-1,1,0,0,-1,-1,1,1};
    const int colDir[8]={0,0,-1,1,-1,1,-1,1};
    int row=square/8;
    int col=square%8;
    
    uint64_t friendlyPieces;
    uint64_t enemyPieces;
    if(selectedPiece<=WHITE_KING){
        friendlyPieces=board.whitePieces;
        enemyPieces=board.blackPieces;
    }else{
        friendlyPieces=board.blackPieces;
        enemyPieces=board.whitePieces;
    }
    for(int dir=0;dir<8;dir++){
        int newRow=row+rowDir[dir];
        int newCol=col+colDir[dir];

        if(newRow<0||newRow>7||newCol<0||newCol>7){
            continue;
        }
        int destination=newRow*8+newCol;
        
        if(isBitSet(friendlyPieces,destination)){
            continue;;
        }
        moves[moveCount].from=square;
        moves[moveCount].to=destination;
        moveCount++;
    }
    generateCastlingMoves();

    filterLegalMoves();
}

//============================================================================
//Move Pieces
void movePiece(uint64_t *bitboard, int from, int to){
    removePiece(to);

    *bitboard &= ~(1ULL << from);
    *bitboard |=  (1ULL << to);

    updateOccupancy();
}

//Castle
void performCastle(int from,int to){
    if(from==60){
        if(to==62){
            board.whiteRooks&=~(1ULL<<63);
            board.whiteRooks|=(1ULL<<61);
        }else if(to==58){
            board.whiteRooks&=~(1ULL<<56);
            board.whiteRooks|=(1ULL<<59);
        }
    }else if(from==4){
        if(to==6){
            board.blackRooks&=~(1ULL<<7);
            board.blackRooks|=(1ULL<<5);
        }else if(to==2){
            board.blackRooks&=~(1ULL<<0);
            board.blackRooks|=(1ULL<<3);
        }
    }
    updateOccupancy();
}

void makeMove(Move move){

    Piece movingPiece = GetPieceAtSquare(move.from);
    
    if(movingPiece!=WHITE_PAWN&&movingPiece!=BLACK_PAWN){
            board.enPassantSquare=-1;
    }
    
    switch(movingPiece){

        case WHITE_PAWN:
            if(move.to==board.enPassantSquare&&move.from%8!=move.to%8){
                removePiece(move.to+8);
            }

            if(move.from-move.to==16){
                board.enPassantSquare=(move.from+move.to)/2;
            }else{
                board.enPassantSquare=-1;
            }

            movePiece(&board.whitePawns, move.from, move.to);
            break;

        case BLACK_PAWN:
            if(move.to==board.enPassantSquare&&move.from%8!=move.to%8){
                removePiece(move.to-8);
            }

            if(move.to-move.from==16){
                board.enPassantSquare=(move.from+move.to)/2;
            }else{
                board.enPassantSquare=-1;
            }

            movePiece(&board.blackPawns, move.from, move.to);
            break;

        case WHITE_KNIGHT:
            movePiece(&board.whiteKnights, move.from, move.to);
            break;

        case BLACK_KNIGHT:
            movePiece(&board.blackKnights, move.from, move.to);
            break;

        case WHITE_BISHOP:
            movePiece(&board.whiteBishops, move.from, move.to);
            break;

        case BLACK_BISHOP:
            movePiece(&board.blackBishops, move.from, move.to);
            break;

        case WHITE_ROOK:
        
            movePiece(&board.whiteRooks, move.from, move.to);

            if(move.from==56){
                board.whiteQueensideRookMoved=true;
            }else if(move.from==63){
                board.whiteKingsideRookMoved=true;
            }
            break;

        case BLACK_ROOK:
            movePiece(&board.blackRooks, move.from, move.to);

            if(move.from==0){
                board.blackQueensideRookMoved=true;
            }else if(move.from==7){
                board.blackKingsideRookMoved=true;
            }
            break;

        case WHITE_QUEEN:
            movePiece(&board.whiteQueens, move.from, move.to);
            break;

        case BLACK_QUEEN:
            movePiece(&board.blackQueens, move.from, move.to);
            break;

        case WHITE_KING:
            movePiece(&board.whiteKing, move.from, move.to);
            performCastle(move.from, move.to);
            board.whiteKingMoved=true;
            break;

        case BLACK_KING:
            movePiece(&board.blackKing, move.from, move.to);
            performCastle(move.from, move.to);
            board.blackKingMoved=true;
            break;

        default:
            break;
    }
}

void makeTemporaryMove(Move move, Piece *capturedPiece){
    Piece movingPiece=GetPieceAtSquare(move.from);

    temporaryBoard=board;

    if(capturedPiece!=NULL){
        if((movingPiece==WHITE_PAWN||movingPiece==BLACK_PAWN)&&move.to==board.enPassantSquare&&move.from%8!=move.to%8){
            if(movingPiece==WHITE_PAWN){
                *capturedPiece=GetPieceAtSquare(move.to+8);
            }else{
                *capturedPiece=GetPieceAtSquare(move.to-8);
            }
        }else{
            *capturedPiece=GetPieceAtSquare(move.to);
        }
    }

    makeMove(move);
}

void undoTemporaryMove(Move move, Piece capturedPiece){
    (void)move;
    (void)capturedPiece;
    board=temporaryBoard;
}

bool leavesKingInCheck(Move move){
    Piece capturedPiece=EMPTY;
    bool white=isWhitePiece(GetPieceAtSquare(move.from));

    makeTemporaryMove(move,&capturedPiece);

    if(isKingInCheck(white)){
        undoTemporaryMove(move,capturedPiece);
        return true;
    }

    undoTemporaryMove(move,capturedPiece);
    return false;
}

void filterLegalMoves(void){
    int legalCount=0;

    for(int i=0;i<moveCount;i++){
        if(!leavesKingInCheck(moves[i])){
            moves[legalCount]=moves[i];
            legalCount++;
        }
    }

    moveCount=legalCount;
}

//Capture Pieces
void removePiece(int square){

    //White Pieces
    if(isBitSet(board.whitePawns,square))
        board.whitePawns &= ~(1ULL<<square);

    else if(isBitSet(board.whiteRooks,square))
        board.whiteRooks &= ~(1ULL<<square);

    else if(isBitSet(board.whiteKnights,square))
        board.whiteKnights &= ~(1ULL<<square);

    else if(isBitSet(board.whiteBishops,square))
        board.whiteBishops &= ~(1ULL<<square);

    else if(isBitSet(board.whiteQueens,square))
        board.whiteQueens &= ~(1ULL<<square);

    else if(isBitSet(board.whiteKing,square))
        board.whiteKing &= ~(1ULL<<square);

    //Black Pieces
    else if(isBitSet(board.blackPawns,square))
        board.blackPawns &= ~(1ULL<<square);

    else if(isBitSet(board.blackRooks,square))
        board.blackRooks &= ~(1ULL<<square);

    else if(isBitSet(board.blackKnights,square))
        board.blackKnights &= ~(1ULL<<square);

    else if(isBitSet(board.blackBishops,square))
        board.blackBishops &= ~(1ULL<<square);

    else if(isBitSet(board.blackQueens,square))
        board.blackQueens &= ~(1ULL<<square);

    else if(isBitSet(board.blackKing,square))
        board.blackKing &= ~(1ULL<<square);

    //Update Occupancy Bitboards
    updateOccupancy();

    if(square==56){
        board.whiteQueensideRookMoved=true;
    }
    if(square==63){
        board.whiteKingsideRookMoved=true;
    }
    if(square==0){
        board.blackQueensideRookMoved=true;
    }
    if(square==7){
        board.blackKingsideRookMoved=true;
    }
}

//Attack Checker
bool isSquareAttacked(int square, bool byWhite){
    if(pawnAttacks(square,byWhite)){
        return true;
    }
    if(knightAttacks(square,byWhite)){
        return true;
    }
    if(bishopAttacks(square,byWhite)){
        return true;
    }
    if(rookAttacks(square,byWhite)){
        return true;
    }
    if(kingAttacks(square,byWhite)){
        return true;
    }
    return false;
}

bool isKingInCheck(bool white){
    //printf("Checking white king...\n");
    int kingSquare=-1;
    if(white){
        for(int i=0;i<64;i++){
            if(isBitSet(board.whiteKing,i)){
                //printf("White king square: %d\n",kingSquare);
                kingSquare=i;
                break;
            }
        }
        return isSquareAttacked(kingSquare,false);
    }else{
        for(int i=0;i<64;i++){
            if(isBitSet(board.blackKing,i)){
                //printf("Black king square: %d\n",kingSquare);
                kingSquare=i;
                break;
            }
        }
        return isSquareAttacked(kingSquare,true);
    }
}