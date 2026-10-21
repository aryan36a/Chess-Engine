#include "goal_manager.h"
static float clamp01(float x){return x<0?0:(x>1?1:x);}
static void add(StrategicGoal *out,int cap,int *n,StrategicGoalType type,float priority,float confidence,int sq,Piece piece){
    if(*n>=cap||priority<=0)return;
    out[(*n)++]=(StrategicGoal){type,clamp01(priority),clamp01(confidence),0.0f,sq,piece,true};
}
static const OpponentIntent *find(const OpponentIntent *a,int n,OpponentIntentType type){
    for(int i=0;a&&i<n;++i)if(a[i].type==type)return &a[i];
    return 0;
}
int GoalManager_Update(bool ourWhite,const ThreatReport *t,const OpponentIntent *intents,int intentCount,StrategicGoal *out,int cap){
    if(!t||!out||cap<=0)return 0;
    const SideThreatSummary *ours=ourWhite?&t->white:&t->black,*opp=ourWhite?&t->black:&t->white;
    const OpponentIntent *check=find(intents,intentCount,INTENT_CHECKING_ATTACK);
    const OpponentIntent *queen=find(intents,intentCount,INTENT_QUEEN_ATTACK);
    const OpponentIntent *castle=find(intents,intentCount,INTENT_CASTLING);
    const OpponentIntent *material=find(intents,intentCount,INTENT_MATERIAL_HUNT);
    int n=0;
    if(ours->inCheck)add(out,cap,&n,GOAL_NEUTRALIZE_KING_THREAT,1.0f,1.0f,-1,EMPTY);
    else if(check&&check->confidence>=.45f)add(out,cap,&n,GOAL_IMPROVE_KING_SAFETY,.45f+.4f*check->confidence,check->confidence,-1,EMPTY);
    if(ours->queenAttacked)add(out,cap,&n,GOAL_SAVE_QUEEN,.9f,1.0f,-1,ourWhite?WHITE_QUEEN:BLACK_QUEEN);
    if(ours->hangingPieces>0)add(out,cap,&n,GOAL_PROTECT_HANGING_PIECE,clamp01(.35f+.12f*ours->hangingPieces),.8f,-1,EMPTY);
    if(material&&material->confidence>=.35f)add(out,cap,&n,GOAL_EXPLOIT_HANGING_PIECE,.3f+.45f*material->confidence,material->confidence,-1,EMPTY);
    if((opp->canCastleKingside||opp->canCastleQueenside)&&castle&&castle->confidence>=.4f)
        add(out,cap,&n,GOAL_PREVENT_CASTLING,.2f+.45f*castle->confidence,castle->confidence,-1,EMPTY);
    /* Opponent queen is a target only when the board says our side attacks it. */
    if(opp->queenAttacked){
        float confidence=queen?queen->confidence:.7f;
        add(out,cap,&n,GOAL_ATTACK_OPPONENT_QUEEN,.25f+.45f*confidence,confidence,-1,ourWhite?BLACK_QUEEN:WHITE_QUEEN);
    }
    for(int i=1;i<n;++i){StrategicGoal v=out[i];int j=i;while(j>0&&out[j-1].priority<v.priority){out[j]=out[j-1];--j;}out[j]=v;}
    return n;
}
