#include "rt_enemies.h"
const RT_EnemyEntry rtEnemyRangedEntries[10]={
 {8,1,0x60a5},{10,0,0x60b8},{10,0,0x60d1},{10,0,0x60b8},{8,1,0x60a5},
 {8,0x11,0x60ea},{10,0x11,0x60fd},{10,0x11,0x610a},{10,0x11,0x60fd},{8,0x11,0x60ea}
};
RT_EnemyResult RT_enemyRangedStep(RT_Enemy *e,u8 workspaceFlags,u8 *requestShot,u8 *selector)
{
    if(!e || !requestShot || !selector)return RT_ENEMY_UNAVAILABLE;
    *requestShot=0;*selector=0;
    RT_DoorEnemy *a=&e->state;u8 state=a->action&0xfc,row=a->row;
    if((state!=0x18 && state!=0x1c) || row>4)return RT_ENEMY_UNAVAILABLE;
    if(--a->timer)return RT_ENEMY_CONTINUE;
    u8 low=state==0x1c;
    if(row==4){
        if(low)e->byte12&=0xf7;
        if(!(a->decisions&1))*selector=0x70;
        else if(low)*selector=workspaceFlags&0x80?0x10:0x14;
        else *selector=workspaceFlags&0x80?0:4;
        return RT_ENEMY_SELECT;
    }
    if(row==1)*requestShot=1;
    if(row==2 && --e->delay)row=0;
    a->row=row+1;u8 index=low*5+a->row;
    const RT_EnemyEntry *entry=rtEnemyRangedEntries+index;
    a->timer=entry->timer;a->flags=entry->flags;a->pose=194+index;
    return RT_ENEMY_CONTINUE;
}
