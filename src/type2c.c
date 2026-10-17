#include "rt_type2c.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type2cSimpleStep(RT_Enemy *e,const RT_Type2cEntryContext *c)
{
    if(!e || !c || e->state.kind!=44)return RT_ENEMY_UNAVAILABLE;
    u8 state=e->state.action&0xfc,row=e->state.row,sequence;
    if(state==0x54){if(row>4)return RT_ENEMY_UNAVAILABLE;sequence=13;}
    else if(state==0x18 || state==0x1c || state==0x78 || state==0x7c || state==0x94 || state==0x98){
        if(row>6)return RT_ENEMY_UNAVAILABLE;
        sequence=14;
    }else if(state==0x38 || state==0x70 || state==0x74){
        if(row>6)return RT_ENEMY_UNAVAILABLE;
        sequence=19;
    }else if(state==0x48){
        if(row>8)return RT_ENEMY_UNAVAILABLE;
        sequence=7;
    }else return RT_ENEMY_UNAVAILABLE;
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    if(sequence==19){
        if(row==1 || row==4){e->state.action^=3;e->state.spawnAction=e->state.action;}
        if(row==2 && (e->state.decisions&0x20)){
            u8 flags=e->state.decisions;e->byte12&=0xfb;
            if((flags&1) || ((e->state.action&2)?!(flags&4):(flags&4)))
                return RT_type2cSelect(e,0x54,c)==RT_TYPE2C_ENTRY_INSTALLED?
                    RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
        }
    }else if(sequence==7){
        if(row==1){e->state.action^=3;e->state.spawnAction=e->state.action;}
        if(row==8)e->byte12&=0xfe;
    }
    if(row==(state==0x54?4:state==0x48?8:6)){
        return RT_type2cSelect(e,state==0x54?0x18:4,c)==RT_TYPE2C_ENTRY_INSTALLED?
            RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
    }
    e->state.row++;
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[sequence]+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
