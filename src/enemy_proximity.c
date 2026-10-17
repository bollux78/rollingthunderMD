#include "rt_enemies.h"
/* Original DDB8 rows, including terminal row16. */
const RT_EnemyEntry rtEnemyProximityEntries[17]={
 {3,0xf2,0x6ca0},{3,0xf2,0x6cad},{3,0xf2,0x6cc0},{3,0xf2,0x6cd3},
 {3,0xf2,0x6ce6},{3,0xf2,0x6cf9},{3,0xf2,0x6d0c},{3,0xf2,0x6d1f},
 {3,0x1a,0x6d32},{3,0x1a,0x6d4b},{3,0x1a,0x6d6a},{3,0x1a,0x6d89},
 {3,0x1a,0x6da8},{3,0x1a,0x6dc1},{8,0x21,0x691e},{8,0x1a,0x6937},
 {8,0,0x694a}
};
static void loadRow(RT_Enemy *e,u8 row)
{
    const RT_EnemyEntry *entry=rtEnemyProximityEntries+row;
    e->state.row=row;e->state.timer=entry->timer;e->state.flags=entry->flags;
    e->state.pose=251+row;
}
/* Rare per-actor service: keep it out of the hot pool traversal. */
__attribute__((noinline))
u8 RT_enemyProximityEnter(RT_Enemy *e,const RT_Agent *target)
{
    if(!e || !target || (e->state.spawnAction&0xfc)!=0x8c)return 0;
    RT_DoorEnemy *a=&e->state;
    if(!(a->spawnAction&3))a->spawnAction+=a->action&3;
    u16 dx=(u16)(a->x-target->body.x),dy=(u16)(target->body.y-a->y);
    if((s16)dx<0)dx=(u16)(0-dx);
    if((s16)dy<0)dy=(u16)(0-dy);
    if(dx>=0x500 || dy>=0x400){a->hidden=1;a->action=0x80;return 0;}
    e->velocityY=0;a->action=a->spawnAction;loadRow(e,0);return 1;
}
RT_EnemyResult RT_enemyProximityStep(RT_Enemy *e,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context,u8 *selector)
{
    if(!e || !selector || (e->state.action&0xfc)!=0x8c || e->state.row>15 ||
        (e->state.kind!=4 && e->state.kind!=8 && e->state.kind!=12))return RT_ENEMY_UNAVAILABLE;
    RT_DoorEnemy *a=&e->state;u8 row=a->row,next=row+1;
    RT_EnemySweep movement=RT_ENEMY_SWEEP_CLEAR;*selector=0;
    if(row==14){
        movement=RT_enemySweepVertical(e,camera,accelerationY,probe,context);
        if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(movement==RT_ENEMY_SWEEP_CLEAR)return RT_ENEMY_CONTINUE;
    }else{
        if(--a->timer)return RT_ENEMY_CONTINUE;
        if(row==13){
            u8 code,flags;
            if(!probe || !probe(0,1,&code,&flags,context))return RT_ENEMY_UNAVAILABLE;
            if(flags&2){a->attributes=flags&0xc0;next++;}
        }else if(row==15){
            *selector=a->decisions&1?4:0x70;return RT_ENEMY_SELECT;
        }
    }
    loadRow(e,next);
    return movement==RT_ENEMY_SWEEP_REMOVED?RT_ENEMY_REMOVED:RT_ENEMY_CONTINUE;
}
