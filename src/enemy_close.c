#include "rt_combat.h"
const RT_EnemyEntry rtEnemyCloseEntries[4]={
 {6,0x0e,0x6dda},{6,0,0x6df3},{6,2,0x6e0c},{6,2,0x6e2b}
};
RT_EnemyResult RT_enemyCloseStep(RT_Enemy *enemy,RT_PlayerTargets *targets,u8 *selector)
{
    if(!enemy || !selector || (enemy->state.action&0xfc)!=0x94 || enemy->state.row>3)
        return RT_ENEMY_UNAVAILABLE;
    *selector=0;RT_DoorEnemy *a=&enemy->state;
    if(--a->timer)return RT_ENEMY_CONTINUE;
    if(a->row==3){*selector=4;return RT_ENEMY_SELECT;}
    if(a->row==2){
        if(!targets || !targets->agents || !targets->combat || !targets->players || targets->players>2)
            return RT_ENEMY_UNAVAILABLE;
        *selector=RT_enemyCloseHitPlayers(enemy,targets);
        if(*selector)return RT_ENEMY_SELECT;
    }
    a->row++;const RT_EnemyEntry *entry=rtEnemyCloseEntries+a->row;
    a->timer=entry->timer;a->flags=entry->flags;a->pose=204+a->row;
    return RT_ENEMY_CONTINUE;
}
