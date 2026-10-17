/* Original CPU1 stationary handlers BF37/BF63/C228/C292. The wider
 * family remains quarantined until its movement/decision audit is complete. */
#include "rt_type20.h"
#include "rt_high_animation.h"
static void install(RT_Enemy *e,const RT_HighAnimationRow *row)
{
    e->state.timer=row->timer;e->state.flags=row->flags;e->state.pose=row->pose;
}
static void walk(RT_Enemy *e)
{
    RT_type20EnterWalk(e,4);
}
static void cycle(RT_Enemy *e,u8 sequence)
{
    install(e,rtHighCycleRows+rtHighCycleOffsets[sequence]+e->state.row);
}
RT_EnemyResult RT_type20SimpleStep(RT_Enemy *e)
{
    if(!e || e->state.kind!=32)return RT_ENEMY_UNAVAILABLE;
    u8 state=e->state.action&0xfc,row=e->state.row;
    if(state==0x54){
        if(!--e->state.timer)walk(e);
        return RT_ENEMY_CONTINUE;
    }
    if(state>=0x60 && state<=0x6c){
        if(row>7 || --e->state.timer)return RT_ENEMY_CONTINUE;
        if(row==7){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        if(!row)e->byte4=0x1e;
        e->state.row++;cycle(e,21);return RT_ENEMY_CONTINUE;
    }
    if((state!=0x38 && state!=0x48) || row>2)return RT_ENEMY_UNAVAILABLE;
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    if(state==0x48){
        if(row==2){e->byte12&=0xfe;walk(e);return RT_ENEMY_CONTINUE;}
        if(!row){e->state.action^=3;e->state.spawnAction=e->state.action;}
        e->state.row++;cycle(e,23);return RT_ENEMY_CONTINUE;
    }
    if(!row){
        e->state.action^=3;e->state.spawnAction=e->state.action;
        e->state.row++;cycle(e,22);return RT_ENEMY_CONTINUE;
    }
    if(row==1){
        u8 flags=e->state.decisions;
        u8 select54=(flags&0x20) && ((flags&1) ||
            ((e->state.action&2)?!(flags&4):(flags&4)));
        if(select54){
            /* B88E -> B5AE installs state54; BF63 runs next actor pass. */
            e->state.action=e->state.spawnAction=0x54|(e->state.action&3);
            e->state.row=0;install(e,rtHighStateRows[0]+(0x54>>2));
            return RT_ENEMY_CONTINUE;
        }
        e->state.row++;cycle(e,22);return RT_ENEMY_CONTINUE;
    }
    walk(e);return RT_ENEMY_CONTINUE;
}
