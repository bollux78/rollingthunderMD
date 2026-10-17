#include "rt_enemies.h"
const RT_EnemyEntry rtEnemyRecoveryEntries[8]={
 {8,0x97,0x6473},{8,0xa0,0x648c},{4,0xf0,0x64ab},
 {10,0x83,0x64dd},{8,0xf7,0x64f6},{4,0xf0,0x64ab},
 {8,0x11,0x6098},{8,0x11,0x6b65}
};
RT_EnemyResult RT_enemyRecoveryStep(RT_Enemy *e,u8 fallbackSelector,u8 *selector)
{
    if(!e || !selector)return RT_ENEMY_UNAVAILABLE;
    RT_DoorEnemy *a=&e->state;u8 state=a->action&0xfc;
    if((state!=0x0c && state!=0x10 && state!=0x14) || (state==0x10 && a->row>1))
        return RT_ENEMY_UNAVAILABLE;
    *selector=0;if(--a->timer)return RT_ENEMY_CONTINUE;
    if(state==0x0c){*selector=e->byte12&8?0x1c:0x10;return RT_ENEMY_SELECT;}
    if(state==0x14){
        *selector=!(a->decisions&1)?0x70:!(e->byte12&1)?4:e->traits&0x4c?0x18:fallbackSelector;
        return RT_ENEMY_SELECT;
    }
    if(a->row==1){
        if(!(a->decisions&1) || !--e->delay){*selector=0x14;return RT_ENEMY_SELECT;}
        a->row=0;
    }else a->row=1;
    a->timer=8;a->flags=0x11;a->pose=214+a->row;return RT_ENEMY_CONTINUE;
}
RT_EnemyResult RT_enemyRecoilStep(RT_Enemy *e,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context,u8 *selector)
{
    if(!e || !selector)return RT_ENEMY_UNAVAILABLE;
    RT_DoorEnemy *a=&e->state;u8 state=a->action&0xfc,row=a->row,ignored;
    if((state!=0x78 && state!=0x7c) || row>2)return RT_ENEMY_UNAVAILABLE;
    *selector=0;
    if(row==2){if(--a->timer)return RT_ENEMY_CONTINUE;*selector=0x10;return RT_ENEMY_SELECT;}
    RT_EnemySweep movement=RT_enemySweepHorizontal(e,camera,probe,context,&ignored);
    if(movement==RT_ENEMY_SWEEP_REMOVED)return RT_ENEMY_REMOVED;
    if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    movement=RT_enemySweepVertical(e,camera,accelerationY,probe,context);
    if(movement==RT_ENEMY_SWEEP_REMOVED)return RT_ENEMY_REMOVED;
    if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(!row){if(--a->timer)return RT_ENEMY_CONTINUE;}
    else if(movement==RT_ENEMY_SWEEP_CLEAR)return RT_ENEMY_CONTINUE;
    a->row=row+1;u8 index=(state==0x7c?3:0)+a->row;
    const RT_EnemyEntry *entry=rtEnemyRecoveryEntries+index;
    a->timer=entry->timer;a->flags=entry->flags;a->pose=208+index;
    return RT_ENEMY_CONTINUE;
}
