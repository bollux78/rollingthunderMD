#include "rt_enemies.h"
/* Source C395/C3DA/C41F rows, compiled IDs216..250. */
const RT_EnemyEntry rtEnemyHitEntries[35]={
{8,151,0x6473},
{8,160,0x648c},
{4,240,0x64ab},
{4,240,0x64c4},
{24,240,0x6567},
{8,240,0x6580},
{8,240,0x6599},
{8,240,0x65b2},
{8,240,0x65c5},
{8,241,0x65d8},
{8,241,0x65df},
{10,131,0x64dd},
{8,247,0x64f6},
{4,240,0x64ab},
{4,240,0x64c4},
{24,240,0x6567},
{8,240,0x6580},
{8,240,0x6599},
{8,240,0x65b2},
{8,240,0x65c5},
{8,241,0x65d8},
{8,241,0x65df},
{8,152,0x6509},
{6,152,0x6522},
{4,152,0x653b},
{24,240,0x6554},
{8,240,0x65e6},
{8,240,0x65f9},
{8,240,0x660c},
{8,240,0x661f},
{8,240,0x662c},
{8,240,0x6639},
{8,240,0x6646},
{8,241,0x6653},
{8,241,0x665a},
};
static void hitRow(RT_Enemy *e,u8 row)
{
    u8 state=e->state.action&0xfc;
    u8 index=(state==0x60?0:state==0x64?11:22)+row;
    const RT_EnemyEntry *r=rtEnemyHitEntries+index;
    e->state.row=row;e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=216+index;
}
/* Rare per-actor service: keep it out of the hot pool traversal. */
__attribute__((noinline))
u8 RT_enemyHitEnter(RT_Enemy *e,u8 selector,u8 counter,
    const RT_EnemyRecoilLinks *links,u16 *scoreBCD,u8 *sound)
{
    u8 state=selector&0xfc;
    if(!e || !scoreBCD || !sound || (state!=0x60 && state!=0x64 && state!=0x68) ||
        (e->state.kind!=4 && e->state.kind!=8 && e->state.kind!=12))return 0;
    RT_DoorEnemy *a=&e->state;u8 prior=a->action&0xfc;
    if(prior==0x5c && (!links || !links->door))return 0;
    if(prior==0x5c && links->doorOwnsEnemy){links->door->flags&=0xfd;links->door->spawnDelay=255;}
    else if(links && links->thrownPhase &&
        ((prior==0x84 && links->thrownFamily==RT_RECOIL_THROW_68) ||
         (prior==0x88 && links->thrownFamily==RT_RECOIL_THROW_69)))++*links->thrownPhase;
    if(!(selector&3))selector+=a->action&3;
    *scoreBCD=e->byte6?0:a->kind==4?0x30:a->kind==8?0x40:0x70;
    u8 variant=e->byte6?!!(counter&2):(counter&2)?2:(counter&4)?1:0;
    *sound=e->byte6?0:variant==0?0x6a:variant==1?0x2e:0x6b;
    if(variant!=2){a->y-=variant?0xb0:0xe0;e->velocityY=-64;e->velocityX=selector&2?-48:48;}
    /* Source preserves old facing in state but uses incoming facing for X. */
    a->action=a->spawnAction=0x60+variant*4+(a->action&3);hitRow(e,0);return 1;
}
RT_EnemyResult RT_enemyHitStep(RT_Enemy *e,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context,u8 *selector,u8 *command)
{
    if(!e || !selector || !command)return RT_ENEMY_UNAVAILABLE;
    u8 state=e->state.action&0xfc,row=e->state.row,ignored;
    if((state!=0x60 && state!=0x64 && state!=0x68) || row>(state==0x68?12:10))return RT_ENEMY_UNAVAILABLE;
    *selector=*command=0;
    RT_EnemySweep movement;
    if(state!=0x68 && row<2){
        if(state==0x64 || e->velocityX){
            movement=RT_enemySweepHorizontal(e,camera,probe,context,&ignored);
            if(movement==RT_ENEMY_SWEEP_REMOVED)return RT_ENEMY_REMOVED;
            if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
            if(state==0x60)e->velocityX=(s16)(e->velocityX+(e->velocityX<0?4:-4));
        }
        movement=RT_enemySweepVertical(e,camera,accelerationY,probe,context);
        if(movement==RT_ENEMY_SWEEP_REMOVED)return RT_ENEMY_REMOVED;
        if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(row==1){
            if(movement==RT_ENEMY_SWEEP_CLEAR)return RT_ENEMY_CONTINUE;
            if(e->byte6){*selector=0x10;return RT_ENEMY_SELECT;}
            hitRow(e,row+1);return RT_ENEMY_CONTINUE;
        }
    }else if(state==0x68 && row==2){
        movement=RT_enemySweepVertical(e,camera,accelerationY,probe,context);
        if(movement==RT_ENEMY_SWEEP_REMOVED)return RT_ENEMY_REMOVED;
        if(movement==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(movement==RT_ENEMY_SWEEP_CLEAR)return RT_ENEMY_CONTINUE;
    }
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    if(row==5){if(state==0x68 && e->byte6){*selector=0x10;return RT_ENEMY_SELECT;}*command=3;}
    if(row==(state==0x68?12:10)){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
    hitRow(e,row+1);return RT_ENEMY_CONTINUE;
}
