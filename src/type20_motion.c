/* Original-ROM handler control flow, tested with controlled sweep outcomes.
 * Not yet linked into live gameplay: full collision/entry/perception ownership
 * still needs integration. See check_type20_motion_rom.py and AGENT_HANDOFF.md. */
#include "rt_type20.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
static void install(RT_Enemy *e,const RT_HighAnimationRow *r)
{e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;}
static void selectState(RT_Enemy *e,u8 state)
{
    if(state==4){RT_type20EnterWalk(e,4);return;}
    e->state.action=e->state.spawnAction=state|(e->state.action&3);
    e->state.row=0;install(e,rtHighStateRows[0]+(state>>2));
}
static void advance(RT_Enemy *e,u8 sequence,u8 amount)
{
    e->state.row+=amount;
    install(e,rtHighCycleRows+rtHighCycleOffsets[sequence]+e->state.row);
}
static RT_EnemyResult motion(RT_Enemy *e,const RT_Type20Motion *c,u8 horizontal,u8 *hit)
{
    RT_EnemySweep result;u8 code;u16 scratch=0;
    if(horizontal){
        result=RT_highEnemyHorizontal(e,c->camera,c->shadowXLow,c->probe,c->probeContext,&code);
        if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    }
    result=RT_highEnemyVertical(e,c->camera,c->accelerationY,c->shadowYLow,c->probe,c->probeContext,&scratch);
    if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    *hit=result==RT_ENEMY_SWEEP_HIT;return RT_ENEMY_CONTINUE;
}
static RT_EnemyResult step(RT_Enemy *e,const RT_Type20Motion *c,u8 state)
{
    if(!e || !c || !c->camera || e->state.kind!=32)return RT_ENEMY_UNAVAILABLE;
    u8 row=e->state.row,sequence,limit;
    switch(state){
    case 0x18:case 0x1c:case 0x20:case 0x24:case 0x44:case 0x94:case 0xa0:case 0xa4:
        sequence=26;limit=2;break;
    case 0x28:sequence=28;limit=4;break;
    case 0x2c:sequence=27;limit=4;break;
    case 0x30:sequence=20;limit=2;break;
    case 0x34:sequence=20;limit=2;break;
    default:return RT_ENEMY_UNAVAILABLE;
    }
    if(row>limit)return RT_ENEMY_UNAVAILABLE;
    if(row==limit){
        if(state==0x34){
            if(--e->state.timer)return RT_ENEMY_CONTINUE;
            e->byte12|=0x20;
            selectState(e,!(e->byte12&1) && (e->state.decisions&1)?4:0x48);
        }else{
            if(--e->state.timer)return RT_ENEMY_CONTINUE;
            if(sequence==26)selectState(e,e->state.decisions&0x31?4:0x38);
            else if(state==0x30)selectState(e,e->state.decisions&1?4:0x38);
            else{
                u8 next=(e->byte12|e->state.decisions)&1?4:0x38;
                e->byte12&=0xfe;selectState(e,next);
            }
        }
        return RT_ENEMY_CONTINUE;
    }
    u8 hit;RT_EnemyResult result=motion(e,c,sequence==26 || state==0x30,&hit);
    if(result!=RT_ENEMY_CONTINUE)return result;
    if(sequence==26){
        if(row?hit:RT_highEnemySettle(e))advance(e,sequence,1);
    }else if(state==0x30 || state==0x34){
        if(!row){
            if(hit)advance(e,20,2);
            else if(!--e->state.timer)advance(e,20,1);
        }else if(hit)advance(e,20,1);
    }else if(state==0x2c){
        if(row==0 || row==2){if(!--e->state.timer)advance(e,27,1);}
        else if(row==1){
            if(RT_highEnemySettle(e)){e->state.attributes=e->state.attributes==0x40?0x80:0x40;advance(e,27,1);}
        }else if(hit)advance(e,27,1);
    }else{
        if(row<2){
            if(e->byte12&0x10)advance(e,28,row?1:2);
            else if(!--e->state.timer)advance(e,28,1);
        }else if(row==2){
            if(RT_highEnemySettle(e)){
                if(!(e->byte12&0x10))selectState(e,0x34);
                else{e->state.attributes=e->state.attributes==0x40?0x80:0x40;advance(e,28,1);}
            }
        }else if(hit)advance(e,28,1);
    }
    return RT_ENEMY_CONTINUE;
}

RT_EnemyResult RT_type20MotionStep(RT_Enemy *e,const RT_Type20Motion *c)
{
    return e?step(e,c,e->state.action&0xfc):RT_ENEMY_UNAVAILABLE;
}
