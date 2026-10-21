#include "threat_analysis.h"
#include <string.h>
static bool canCastle(bool white, bool kingside) {
    const int kingSq=white?60:4, rookSq=white?(kingside?63:56):(kingside?7:0);
    const int t1=white?(kingside?61:59):(kingside?5:3), t2=white?(kingside?62:58):(kingside?6:2);
    if(white) {
        if(board.whiteKingMoved || (kingside?board.whiteKingsideRookMoved:board.whiteQueensideRookMoved) ||
           !isBitSet(board.whiteKing,kingSq) || !isBitSet(board.whiteRooks,rookSq)) return false;
    } else {
        if(board.blackKingMoved || (kingside?board.blackKingsideRookMoved:board.blackQueensideRookMoved) ||
           !isBitSet(board.blackKing,kingSq) || !isBitSet(board.blackRooks,rookSq)) return false;
    }
    if(isSquareAttacked(kingSq,!white)||isSquareAttacked(t1,!white)||isSquareAttacked(t2,!white)) return false;
    if(isBitSet(board.occupied,t1)||isBitSet(board.occupied,t2)) return false;
    if(!kingside && isBitSet(board.occupied,white?57:1)) return false;
    return true;
}
/* Counts attacked squares in the 3x3 king zone, including the king square. */
static int kingZonePressure(bool attackingWhite) {
    uint64_t king=attackingWhite?board.blackKing:board.whiteKing;
    if(!king) return 0;
    int sq=__builtin_ctzll(king), row=sq/8, col=sq%8, count=0;
    for(int r=row-1;r<=row+1;++r) for(int c=col-1;c<=col+1;++c)
        if(r>=0&&r<8&&c>=0&&c<8&&isSquareAttacked(r*8+c,attackingWhite)) ++count;
    return count;
}
static void analyzeSide(bool white, SideThreatSummary *s) {
    memset(s,0,sizeof(*s));
    s->inCheck=isKingInCheck(white);
    s->canCastleKingside=canCastle(white,true);
    s->canCastleQueenside=canCastle(white,false);
    s->kingZonePressure=kingZonePressure(!white);
    for(int sq=0;sq<64;++sq) {
        Piece p=GetPieceAtSquare(sq);
        if(p==EMPTY||isWhitePiece(p)!=white) continue;
        bool attacked=isSquareAttacked(sq,!white), defended=isSquareAttacked(sq,white);
        if(!attacked) continue;
        ++s->attackedPieces;
        if(!defended&&p!=WHITE_KING&&p!=BLACK_KING) ++s->hangingPieces;
        if(p==(white?WHITE_QUEEN:BLACK_QUEEN)) s->queenAttacked=true;
    }
}
void ThreatAnalysis_Analyze(ThreatReport *report) {
    if(!report) return;
    analyzeSide(true,&report->white);
    analyzeSide(false,&report->black);
}
