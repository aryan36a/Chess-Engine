#include "opponent_model.h"
#define ALPHA 0.18f
static float clamp01(float x){return x<0?0:(x>1?1:x);}
static void update(float *rate,bool yes){*rate=(1.0f-ALPHA)*(*rate)+ALPHA*(yes?1.0f:0.0f);}
void OpponentModel_Init(OpponentProfile *p){if(p)*p=(OpponentProfile){0};}
void OpponentModel_RecordMove(OpponentProfile *p,Piece moving,Piece captured,bool check,bool castled){
    if(!p)return;
    update(&p->checkRate,check);
    update(&p->captureRate,captured!=EMPTY);
    update(&p->queenDevelopmentRate,moving==WHITE_QUEEN||moving==BLACK_QUEEN);
    update(&p->castlingRate,castled);
    update(&p->kingAttackRate,check);
    if(p->observations<0xFFFFFFFFu)++p->observations;
}
static void add(OpponentIntent *out,int cap,int *n,OpponentIntentType type,float confidence,int sq,Piece piece){
    if(*n>=cap||confidence<=0)return;
    out[(*n)++]=(OpponentIntent){type,clamp01(confidence),sq,piece};
}
int OpponentModel_InferIntentions(const ThreatReport *t,bool ourWhite,const OpponentProfile *p,OpponentIntent *out,int cap){
    if(!t||!out||cap<=0)return 0;
    const SideThreatSummary *ours=ourWhite?&t->white:&t->black,*opp=ourWhite?&t->black:&t->white;
    float checks=p?p->checkRate:0,captures=p?p->captureRate:0,queens=p?p->queenDevelopmentRate:0;
    float castles=p?p->castlingRate:0,attacks=p?p->kingAttackRate:0;
    int n=0;
    if(ours->inCheck||ours->kingZonePressure>=3)
        add(out,cap,&n,INTENT_CHECKING_ATTACK,ours->inCheck?1.0f:clamp01(.25f+.07f*ours->kingZonePressure+.25f*attacks+.15f*checks),-1,EMPTY);
    if(ours->queenAttacked)
        add(out,cap,&n,INTENT_QUEEN_ATTACK,clamp01(.65f+.2f*queens),-1,ourWhite?WHITE_QUEEN:BLACK_QUEEN);
    if(ours->kingZonePressure>=2)
        add(out,cap,&n,INTENT_KING_SIDE_ATTACK,clamp01(.2f+.08f*ours->kingZonePressure+.2f*attacks),-1,EMPTY);
    if(opp->canCastleKingside||opp->canCastleQueenside)
        add(out,cap,&n,INTENT_CASTLING,clamp01(.15f+.55f*castles+.1f*queens),-1,EMPTY);
    if(ours->hangingPieces||ours->attackedPieces>1)
        add(out,cap,&n,INTENT_MATERIAL_HUNT,clamp01(.2f+.12f*ours->hangingPieces+.08f*ours->attackedPieces+.15f*captures),-1,EMPTY);
    for(int i=1;i<n;++i){OpponentIntent v=out[i];int j=i;while(j>0&&out[j-1].confidence<v.confidence){out[j]=out[j-1];--j;}out[j]=v;}
    return n;
}
