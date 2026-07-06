#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>

#include "board.h"
#include "attacks.h"

//General Attcks
bool slidingPieceAttacks(int square,const int rowDir[],const int colDir[],int directionCount,Piece piece1, Piece piece2){
    int row=square/8;
    int col=square%8;

    for(int dir=0;dir<directionCount;dir++){
        int newRow=row;
        int newCol=col;

        while(1){
            newRow+=rowDir[dir];
            newCol+=colDir[dir];

            if(newRow<0||newRow>7||newCol<0||newCol>7){
                break;
            }

            Piece piece=GetPieceAtSquare(newRow*8+newCol);

            if(piece==EMPTY){
                continue;
            }
            if(piece==piece1||piece==piece2){
                return true;
            }
            break;
        }
    }
    return false;
}
//Attack detection
//Helper Function
//Pawn Attacks
bool pawnAttacks(int square, bool byWhite){
    int row=square/8;
    int col=square%8;

    if(byWhite){
        if(row<7){
            if(col>0){
                Piece piece=GetPieceAtSquare((row+1)*8+(col-1));
                if(piece==WHITE_PAWN){
                    return true;
                }
            }
            if(col<7){
                Piece piece=GetPieceAtSquare((row+1)*8+(col+1));
                if(piece==WHITE_PAWN){
                    return true;
                }
            }
        }
    }else{
            if(row>0){
                if(col>0){
                    Piece piece=GetPieceAtSquare((row-1)*8+(col-1));
                    if(piece==BLACK_PAWN){
                        return true;
                    }
                }
                if(col<7){
                    Piece piece=GetPieceAtSquare((row-1)*8+(col+1));
                    if(piece==BLACK_PAWN){
                        return true;
                }
            }
        }
    }
    return false;
}


//Knight Attacks
bool knightAttacks(int square,bool byWhite){

    const int rowDir[8]={-2,-2,-1,-1,1,1,2,2};
    const int colDir[8]={-1,1,-2,2,-2,2,-1,1};

    int row=square/8;
    int col=square%8;

    for(int i=0;i<8;i++){
        int newRow=row+rowDir[i];
        int newCol=col+colDir[i];

        if(newRow<0||newRow>7||newCol<0||newCol>7){
            continue;
        }

        Piece piece=GetPieceAtSquare(newRow*8+newCol);
        if(byWhite){
            if(piece==WHITE_KNIGHT){
                return true;
            }
        }else{
            if(piece==BLACK_KNIGHT){
                return true;
            }
        }
    }
    return false;
}

//king attacks
bool kingAttacks(int square, bool byWhite){

    const int rowDir[8]={-1,1,0,0,-1,-1,1,1};
    const int colDir[8]={0,0,-1,1,-1,1,-1,1};

    int row=square/8;
    int col=square%8;

    for(int i=0;i<8;i++){
        int newRow=row+rowDir[i];
        int newCol=col+colDir[i];

        if(newRow<0||newRow>7||newCol<0||newCol>7){
            continue;
        }

        Piece piece=GetPieceAtSquare(newRow*8+newCol);

        if(byWhite){
            if(piece==WHITE_KING){
                return true;
            }
        }else{
            if(piece==BLACK_KING){
                return true;
            }
        }
    }
    return false;
}

//Rook Attacks+Queen Straights
bool rookAttacks(int square,bool byWhite){
    const int rowDir[4]={-1,1,0,0};
    const int colDir[4]={0,0,-1,1};

    if(byWhite){
        return slidingPieceAttacks(square,rowDir,colDir,4,WHITE_ROOK,WHITE_QUEEN);
    }else{
        return slidingPieceAttacks(square,rowDir,colDir,4,BLACK_ROOK,BLACK_QUEEN);
    }
}

//Bishop Attacks+Queen Diagonals
bool bishopAttacks(int square,bool byWhite){
    const int rowDir[4]={-1,-1,1,1};
    const int colDir[4]={-1,1,-1,1};

    if(byWhite){
        return slidingPieceAttacks(square,rowDir,colDir,4,WHITE_BISHOP,WHITE_QUEEN);
    }else{
        return slidingPieceAttacks(square,rowDir,colDir,4,BLACK_BISHOP,BLACK_QUEEN);
    }
}