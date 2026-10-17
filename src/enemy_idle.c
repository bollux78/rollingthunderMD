#include "rt_enemies.h"
const RT_EnemyEntry rtEnemyIdleEntries[14]={
 {8,1,0x6182},{8,0x23,0x6195},{12,0x23,0x61a8},{8,0x23,0x61c1},
 {8,0x23,0x61a8},{8,0x23,0x61c1},{8,0x23,0x61a8},{8,0x23,0x6195},{8,1,0x6182},
 {6,1,0x61da},{6,1,0x61ed},{6,1,0x6200},{6,0x23,0x6213},{6,0x24,0x6226}
};
static void load(RT_DoorEnemy *actor,u8 row,u8 base)
{
    const RT_EnemyEntry *entry=rtEnemyIdleEntries+base+row;
    actor->row=row;actor->timer=entry->timer;actor->flags=entry->flags;actor->pose=77+base+row;
}
RT_EnemyResult RT_enemyIdleStep(RT_Enemy *enemy,u8 *selector)
{
    if(!enemy || !selector || (enemy->state.action&0xfc)!=0x38 || enemy->state.row>8)
        return RT_ENEMY_UNAVAILABLE;
    *selector=0;RT_DoorEnemy *actor=&enemy->state;
    if(--actor->timer)return RT_ENEMY_CONTINUE;
    u8 row=actor->row,flags=actor->decisions;
    if(row==8){
        if(flags&0x20){enemy->byte12&=0xfb;*selector=4;}
        else if(!(enemy->byte12&4) && ((enemy->byte12&0x20) || (enemy->variation&3)==1))*selector=4;
        else *selector=flags&8?8:0x3c;
        return RT_ENEMY_SELECT;
    }
    if(row==0 || row==7){actor->action^=3;actor->spawnAction=actor->action;}
    else if(row>=2 && (flags&0x20)){
        enemy->byte12&=0xfb;
        if((flags&1) || (!!(flags&4)!=!!(actor->action&2))){*selector=0x54;return RT_ENEMY_SELECT;}
    }
    load(actor,row+1,0);return RT_ENEMY_CONTINUE;
}
RT_EnemyResult RT_enemyTurnStep(RT_Enemy *enemy,u8 *selector)
{
    if(!enemy || !selector)return RT_ENEMY_UNAVAILABLE;
    RT_DoorEnemy *actor=&enemy->state;u8 state=actor->action&0xfc;
    if(state==0x54){
        *selector=0;if(--actor->timer)return RT_ENEMY_CONTINUE;
        enemy->byte12&=0xfe;*selector=4;return RT_ENEMY_SELECT;
    }
    if(state!=0x48 || actor->row>4)return RT_ENEMY_UNAVAILABLE;
    *selector=0;if(--actor->timer)return RT_ENEMY_CONTINUE;
    if(actor->row==4){enemy->byte12&=0xfe;*selector=4;return RT_ENEMY_SELECT;}
    if(actor->row==2){actor->action^=3;actor->spawnAction=actor->action;}
    load(actor,actor->row+1,9);return RT_ENEMY_CONTINUE;
}
