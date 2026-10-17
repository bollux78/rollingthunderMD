#include "rt_enemies.h"
u8 RT_enemyRecoilEnter(RT_Enemy *enemy,u8 selector,u8 sourceCounter,
    const RT_EnemyRecoilLinks *links)
{
    u8 state=selector&0xfc;
    if(!enemy || (state!=0x78 && state!=0x7c))return 0;
    RT_DoorEnemy *a=&enemy->state;u8 prior=a->action&0xfc;
    if(prior==0x5c && (!links || !links->door))return 0;
    if(prior==0x5c && links->doorOwnsEnemy){
        links->door->flags&=0xfd;links->door->spawnDelay=255;
    }else if(links && links->thrownPhase &&
        ((prior==0x84 && links->thrownFamily==RT_RECOIL_THROW_68) ||
         (prior==0x88 && links->thrownFamily==RT_RECOIL_THROW_69)))++*links->thrownPhase;
    if(!(selector&3))selector+=a->action&3;
    a->spawnAction=a->action=selector;a->y-=sourceCounter&2?0xb0:0xe0;
    enemy->velocityY=-64;enemy->velocityX=a->decisions&4?-48:48;
    u8 index=state==0x7c?3:0;const RT_EnemyEntry *entry=rtEnemyRecoveryEntries+index;
    a->row=0;a->timer=entry->timer;a->flags=entry->flags;a->pose=208+index;return 1;
}
