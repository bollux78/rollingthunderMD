#include "rt_enemies.h"
/* Acceleration is downward-positive, like velocityY. The caller owns the
 * mutable acceleration used by states that reverse gravity. */
RT_EnemySweep RT_enemySweepVertical(RT_Enemy *enemy,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context)
{
    if(!enemy || !camera || !probe || enemy->state.kind==RT_ENEMY_FREE)
        return RT_ENEMY_SWEEP_UNAVAILABLE;
    RT_DoorEnemy *a=&enemy->state;
    s32 initial=a->y,target=initial+enemy->velocityY;
    s32 origin=((s32)camera->y+256)*16;
    u8 ascending=enemy->velocityY<0,attribute=a->attributes,state=a->action&0xfc;
    if((ascending && target<=origin-0xf80) || (!ascending && target>origin+0x380)){
        a->kind=RT_ENEMY_FREE;return RT_ENEMY_SWEEP_REMOVED;
    }
    if(!ascending || state==0x28){
        u16 phase=(u16)(-initial)&0x70;
        s32 position=initial+(ascending?-(s16)((0-phase)&0x70):(s16)phase);
        while(ascending?position>=target:position<=target){
            u8 code,flags,hit=0;a->y=position;
            if(!probe(0,ascending?-3:1,&code,&flags,context)){
                a->y=initial;a->attributes=attribute;return RT_ENEMY_SWEEP_UNAVAILABLE;
            }
            if(ascending)hit=flags&8;
            else if((flags&6) && (state!=0x2c || (flags&4))){
                a->attributes=flags&0xc0;hit=(code&0xee)!=0x20;
            }
            if(hit){a->y=position+((u16)(-position)&15);enemy->byte12|=2;return RT_ENEMY_SWEEP_HIT;}
            position+=ascending?-128:128;
        }
    }
    a->y=target;enemy->velocityY=(s16)((u16)enemy->velocityY+(u16)accelerationY);
    return RT_ENEMY_SWEEP_CLEAR;
}
