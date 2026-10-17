#include "rt_enemies.h"
const RT_EnemyEntry rtEnemyShortEntries[3]={{6,0x16,0x6117},{6,0x13,0x6130},{4,1,0x6143}};
RT_EnemyResult RT_enemyHopStep(RT_Enemy *e,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context,u8 *selector)
{
    if(!e || !selector)return RT_ENEMY_UNAVAILABLE;
    RT_DoorEnemy *a=&e->state;u8 state=a->action&0xfc,row=a->row,ignored;
    if((state!=0x20 && state!=0x24 && state!=0x30 && state!=0x44) ||
       (state==0x30 && row>1) || (state==0x44 && row>2))return RT_ENEMY_UNAVAILABLE;
    *selector=0;
    if((state==0x30 && row==1) || (state==0x44 && row==2)){
        if(--a->timer)return RT_ENEMY_CONTINUE;
        e->velocityY=0;
        if(state==0x44)*selector=4;
        else if((e->byte12|a->decisions)&1){e->byte12&=0xfd;*selector=4;}
        else *selector=0x38;
        return RT_ENEMY_SELECT;
    }
    RT_EnemySweep movement;
    if(state!=0x20){
        movement=RT_enemySweepHorizontal(e,camera,probe,context,&ignored);
        if(movement==RT_ENEMY_SWEEP_REMOVED)return RT_ENEMY_REMOVED;
        if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    }
    movement=RT_enemySweepVertical(e,camera,accelerationY,probe,context);
    if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    const RT_EnemyEntry *entry;
    if(state==0x44){
        if((!row && !e->velocityY) || (row==1 && movement!=RT_ENEMY_SWEEP_CLEAR)){
            a->row=row+1;a->pose=191+a->row;entry=rtEnemyShortEntries+a->row;
            a->timer=entry->timer;a->flags=entry->flags;
        }
        return movement==RT_ENEMY_SWEEP_REMOVED?RT_ENEMY_REMOVED:RT_ENEMY_CONTINUE;
    }
    if(movement==RT_ENEMY_SWEEP_REMOVED)return RT_ENEMY_REMOVED;
    if(state==0x30){
        if(movement==RT_ENEMY_SWEEP_CLEAR)return RT_ENEMY_CONTINUE;
        a->row=1;a->pose=193;entry=rtEnemyShortEntries+2;
    }else{
        if(movement==RT_ENEMY_SWEEP_HIT){*selector=4;return RT_ENEMY_SELECT;}
        if(--a->timer)return RT_ENEMY_CONTINUE;
        a->row=row+1;a->pose=127+(a->row&63);entry=rtEnemyHopEntries+(a->row&63);
    }
    a->timer=entry->timer;a->flags=entry->flags;return RT_ENEMY_CONTINUE;
}
