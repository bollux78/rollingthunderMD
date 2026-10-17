#include "rt_enemies.h"
const RT_EnemyEntry rtEnemyJumpEntries[18]={
 {8,0x26,0x6bd1},{8,0x22,0x6be4},{8,0x22,0x6bfd},{8,0x1e,0x6c35},
 {8,0x1f,0x6c48},{8,0x15,0x6c5b},{8,0x14,0x6c74},{8,0x0d,0x6c8d},
 {16,0x19,0x6156},{4,1,0x6143},
 {8,0x1b,0x68b3},{8,0x21,0x68c0},{10,0x1c,0x68d9},{10,0x1d,0x68ec},
 {4,0x21,0x6905},{8,0x21,0x691e},{8,0x1a,0x6937},{8,0,0x694a}
};
static RT_EnemyResult finish(RT_Enemy *e,u8 *selector)
{
    e->velocityY=0;
    if((e->byte12|e->state.decisions)&1){e->byte12&=0xfd;*selector=4;}
    else *selector=0x38;
    return RT_ENEMY_SELECT;
}
RT_EnemyResult RT_enemyJumpStep(RT_Enemy *e,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context,u8 *selector)
{
    if(!e || !selector)return RT_ENEMY_UNAVAILABLE;
    RT_DoorEnemy *a=&e->state;u8 state=a->action&0xfc,row=a->row,next=row+1,index;
    if((state!=0x28 && state!=0x2c && state!=0x34) || row>(state==0x34?1:7))return RT_ENEMY_UNAVAILABLE;
    *selector=0;
    if(state==0x34 && row==1){
        if(--a->timer)return RT_ENEMY_CONTINUE;
        e->byte12|=0x20;*selector=e->byte12&1?0x48:4;return RT_ENEMY_SELECT;
    }
    u8 moving=(state==0x34)||(state==0x28?(row==1 || row==2):(row>=3 && row<=5));
    if(moving){
        RT_EnemySweep movement=RT_enemySweepVertical(e,camera,accelerationY,probe,context);
        if(movement==RT_ENEMY_SWEEP_REMOVED)return RT_ENEMY_REMOVED;
        if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(state==0x34){if(movement==RT_ENEMY_SWEEP_CLEAR)return RT_ENEMY_CONTINUE;}
        else if(movement==RT_ENEMY_SWEEP_HIT){
            if(state==0x2c)next=6;
            else if(row==1)next++;
            else {a->action^=3;a->spawnAction=a->action;}
        }else{
            if(state==0x28 && !e->velocityY){*selector=0x34;return RT_ENEMY_SELECT;}
            if(row==(state==0x28?2:5) || --a->timer)return RT_ENEMY_CONTINUE;
        }
    }else{
        if(--a->timer)return RT_ENEMY_CONTINUE;
        if(state==0x28 && row==0){a->action^=3;a->spawnAction=a->action;}
        else if(row==(state==0x28?4:1))a->attributes=a->attributes==0x40?0x80:0x40;
        if(row==7)return finish(e,selector);
    }
    index=(state==0x28?0:state==0x34?8:10)+next;
    const RT_EnemyEntry *entry=rtEnemyJumpEntries+index;
    a->row=next;a->timer=entry->timer;a->flags=entry->flags;a->pose=109+index;
    return RT_ENEMY_CONTINUE;
}
