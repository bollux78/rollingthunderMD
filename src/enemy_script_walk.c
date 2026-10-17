#include "rt_enemies.h"
const RT_EnemyEntry rtEnemyScriptWalkEntries[8]={
    {6,0x53,0x6e70},{6,0x53,0x6e83},{6,0x53,0x6e96},{6,0x53,0x6ea3},
    {6,0x53,0x6eb0},{6,0x53,0x6ec3},{6,0x53,0x6ed6},{6,0x53,0x6ee9}
};
RT_EnemyResult RT_enemyScriptWalkStep(RT_Enemy *e,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,u8 *selector)
{
    if(!e || !camera || !probe || !selector ||
       (e->state.kind!=4 && e->state.kind!=8 && e->state.kind!=12) ||
       (e->state.action&0xfc)!=0xb4 || e->state.row>7)return RT_ENEMY_UNAVAILABLE;
    *selector=0;
    RT_EnemySweep movement=RT_enemySweepHorizontal(e,camera,probe,context,selector);
    if(movement==RT_ENEMY_SWEEP_REMOVED)return RT_ENEMY_REMOVED;
    if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    /* CCAA keeps collision position/latch effects but ignores its selector. */
    if(movement==RT_ENEMY_SWEEP_HIT)*selector=0;
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    u8 row=e->state.row==7?0:e->state.row+1;
    const RT_EnemyEntry *entry=rtEnemyScriptWalkEntries+row;
    e->state.row=row;e->state.timer=entry->timer;e->state.flags=entry->flags;
    e->state.pose=307+row;
    return RT_ENEMY_CONTINUE;
}
