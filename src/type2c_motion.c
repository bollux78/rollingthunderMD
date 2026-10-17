#include "rt_type2c.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
static void advance(RT_Enemy *e,u8 sequence)
{
    e->state.row++;
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[sequence]+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
}
RT_EnemyResult RT_type2cMotionStep(RT_Enemy *e,const RT_Type2cMotionContext *c)
{
    if(!e || !c || !c->entry.camera || e->state.kind!=44)return RT_ENEMY_UNAVAILABLE;
    u8 state=e->state.action&0xfc,row=e->state.row;
    u8 airborne=state==0x20 || state==0x24 || state==0x30 || state==0x44 || state==0x84 || state==0x88 || state==0xa0 || state==0xa4;
    u8 death=state>=0x60 && state<=0x6c;
    if(airborne?row>7:death?row>6:state==0x28?row>23:state==0x34?row>13:state==0xac?row>16:state!=0x2c || row>17)
        return RT_ENEMY_UNAVAILABLE;
    u8 hit=0;
    if(airborne && row>=2 && row<=4){
        u8 code;RT_EnemySweep result=RT_highEnemyHorizontal(e,c->entry.camera,c->shadowXLow,
            c->entry.probe,c->entry.probeContext,&code);
        if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    }
    if(airborne?(row>=2 && row<=4):death?row<2:state==0x28?(row==9 || row==13):state==0x34?row==2:state==0xac?(row==4 || row==6):(row==4 || row==6 || row==7)){
        u16 scratch=0;
        RT_EnemySweep result=RT_highEnemyVertical(e,c->entry.camera,c->accelerationY,
            c->shadowYLow,c->entry.probe,c->entry.probeContext,&scratch);
        if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        hit=result==RT_ENEMY_SWEEP_HIT;
    }
    if(airborne){
        s16 sourceVelocity=(s16)(0-(u16)e->velocityY);
        if(row==2){if(sourceVelocity>32)return RT_ENEMY_CONTINUE;}
        else if(row==3){
            if(!hit && sourceVelocity> -32)return RT_ENEMY_CONTINUE;
            if(hit)e->state.row++; /* CDB8 skips the remaining airborne row. */
        }
        else if(row==4){if(!hit)return RT_ENEMY_CONTINUE;}
        else{
            if(--e->state.timer)return RT_ENEMY_CONTINUE;
            if(row==7)return RT_type2cSelect(e,e->state.decisions&0x31?4:0x38,&c->entry)==RT_TYPE2C_ENTRY_INSTALLED?
                RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
        }
        advance(e,5);return RT_ENEMY_CONTINUE;
    }
    if(death){
        if(row==1 && !hit)return RT_ENEMY_CONTINUE;
        if(--e->state.timer)return RT_ENEMY_CONTINUE;
        if(row==6){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        advance(e,24);return RT_ENEMY_CONTINUE;
    }
    if(state==0xac){
        if(row==4){
            /* D168 tests the source velocity without clearing its low bits. */
            if((u16)(0-(u16)e->velocityY)>15)return RT_ENEMY_CONTINUE;
        }else if(row==6){
            if(!hit)return RT_ENEMY_CONTINUE;
        }else{
            if(--e->state.timer)return RT_ENEMY_CONTINUE;
            if(row==5)e->state.attributes=0x80;
            if(row==16){
                u8 flags=e->state.decisions,next;
                if(flags&1)next=4;
                else if(e->variation&0x40)next=flags&0x20?0x38:flags&8?0x28:0x2c;
                else next=flags&2?6:0x49;
                /* D1B2 ANDA02 operates on perception flags still in A, not
                 * the active action loaded into B. Subsequent BITA04 is clear. */
                return RT_type2cSelect(e,next,&c->entry)==RT_TYPE2C_ENTRY_UNAVAILABLE?
                    RT_ENEMY_UNAVAILABLE:RT_ENEMY_CONTINUE;
            }
        }
        advance(e,25);return RT_ENEMY_CONTINUE;
    }
    if(state==0x28 || state==0x34){
        u8 sequence=state==0x28?6:18;
        if(state==0x28 && row==9){
            if(!RT_highEnemySettle(e))return RT_ENEMY_CONTINUE;
            if(!(e->byte12&0x10)){
                return RT_type2cSelect(e,0x34,&c->entry)==RT_TYPE2C_ENTRY_INSTALLED?
                    RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
            }
        }else if((state==0x28 && row==13) || (state==0x34 && row==2)){
            if(!hit)return RT_ENEMY_CONTINUE;
        }else{
            if(--e->state.timer)return RT_ENEMY_CONTINUE;
            if(state==0x28 && row==10)e->state.attributes=e->state.attributes==0x40?0x80:0x40;
            if(row==(state==0x28?23:13)){
                u8 next;
                if(state==0x34){
                    next=!(e->byte12&1) && (e->state.decisions&1)?4:0x48;
                    e->byte12|=0x20;
                }else{
                    next=(e->byte12|e->state.decisions)&1?4:0x38;e->byte12&=0xfe;
                }
                return RT_type2cSelect(e,next,&c->entry)==RT_TYPE2C_ENTRY_INSTALLED?
                    RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
            }
        }
        advance(e,sequence);return RT_ENEMY_CONTINUE;
    }
    if(row==4){
        if(!RT_highEnemySettle(e))return RT_ENEMY_CONTINUE;
        e->state.y-=0xc0;
    }else if(row==7){
        if(!hit)return RT_ENEMY_CONTINUE;
    }else if(row!=6 || !hit){
        if(--e->state.timer)return RT_ENEMY_CONTINUE;
        if(row==5){e->state.y+=0x1c0;e->state.attributes=e->state.attributes==0x40?0x80:0x40;}
        else if(row==17){
            u8 next=(e->byte12|e->state.decisions)&1?4:0x38;
            e->byte12&=0xfe;
            return RT_type2cSelect(e,next,&c->entry)==RT_TYPE2C_ENTRY_INSTALLED?
                RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
        }
    }
    /* CFE7's hit branch increments a fixed-ROM address; the bus ignores the
     * write. No actor field or emulated address is needed in native runtime. */
    advance(e,4);return RT_ENEMY_CONTINUE;
}
