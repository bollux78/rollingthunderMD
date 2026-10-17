#include <genesis.h>
extern u32 rtSpriteBirthEpoch;
#include "rt_enemy_spawns.h"
u8 RT_enemiesSpawnGridWithFamilies(RT_EnemyPool *pool,const RT_EnemySpawn *spawn,
    u8 *slot,RT_Type10Effects *effects,u8 highFamilies)
{
    if(!pool || !spawn || !slot || spawn->definition>=rtGridEnemyTemplateCount)return 0;
    const RT_EnemyTemplate *d=rtGridEnemyTemplates+spawn->definition;
    u8 high=d->kind>=32 && d->kind<=44 && !(d->kind&3) &&
        (highFamilies&(1u<<((d->kind-32)>>2)));
    if(d->kind!=4 && d->kind!=8 && d->kind!=12 && !(d->kind==16 && effects) && !high)return 0;
    u16 i;
    for(i=0;i<RT_ENEMY_CAPACITY;i++)if(pool->slots[i].state.kind==RT_ENEMY_FREE)break;
    if(i==RT_ENEMY_CAPACITY)return 0;
    rtSpriteBirthEpoch++;
    RT_Enemy *e=pool->slots+i;
    pool->accelerationY[i]=d->accelerationY;
    pool->scoreOwner[i]=0;
    e->definition=d;e->state.x=(s32)spawn->x*16;e->state.y=(s32)spawn->y*16;
    e->velocityX=e->velocityY=0;e->state.kind=d->kind;e->state.hidden=1;
    e->state.action=0x80;e->state.spawnAction=d->spawnAction;e->state.attributes=d->attributes;
    e->traits=d->traits;e->variation=d->variation;e->byte4=d->initialByte4;e->byte6=d->initialByte6;
    e->byte12=e->byte14=e->state.decisions=0;e->parent=255;
    /* Source leaves timer/row/pose/delay uninitialized until admission. */
    if(effects){effects[i].scriptRequest=0;effects[i].sharedCommand=0;effects[i].sound=0;}
    pool->count++;*slot=i;return 1;
}
u8 RT_enemiesSpawnGridWithType10(RT_EnemyPool *pool,const RT_EnemySpawn *spawn,
    u8 *slot,RT_Type10Effects *effects)
{return RT_enemiesSpawnGridWithFamilies(pool,spawn,slot,effects,0);}

u8 RT_enemiesSpawnGrid(RT_EnemyPool *pool,const RT_EnemySpawn *spawn,u8 *slot)
{
    return RT_enemiesSpawnGridWithType10(pool,spawn,slot,0);
}
