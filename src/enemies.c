#include <genesis.h>
extern u32 rtSpriteBirthEpoch;
#include "rt_enemies.h"
void RT_enemiesInitialize(RT_EnemyPool *pool)
{
    if(!pool)return;
    pool->count=0;
    for(u16 i=0;i<RT_ENEMY_CAPACITY;i++){
        pool->slots[i]=(RT_Enemy){0};pool->slots[i].state.kind=RT_ENEMY_FREE;
        pool->accelerationY[i]=0;
        pool->scoreOwner[i]=0;
    }
    for(u16 i=0;i<RT_NATIVE_STAGE_CAPACITY;i++)pool->doorChildren[i]=0;
}
u8 RT_enemiesSpawnDoor(RT_EnemyPool *pool,RT_StagePool *stagePool,u8 parent,
    u8 stage,u8 worldXLow,u8 counter,u8 *slot)
{
    if(!pool || !stagePool || !slot || parent>=RT_NATIVE_STAGE_CAPACITY)return 0;
    RT_Anchor *door=stagePool->objects+parent;
    if(door->kind>2 || !door->visible)return 0;
    const RT_EnemyTemplate *definition=RT_doorEnemyTemplate(stage,worldXLow,counter);
    if(!definition)return 0;
    u16 index;
    for(index=0;index<RT_ENEMY_CAPACITY;index++)if(pool->slots[index].state.kind==RT_ENEMY_FREE)break;
    if(index==RT_ENEMY_CAPACITY)return 0;
    RT_Enemy *enemy=pool->slots+index;
    enemy->definition=definition;
    pool->accelerationY[index]=definition->accelerationY;
    pool->scoreOwner[index]=0;
    enemy->state.x=door->x+0x100;enemy->state.y=door->y+0x3f0;
    enemy->velocityX=enemy->velocityY=0;
    rtSpriteBirthEpoch++;
    enemy->state.kind=definition->kind;enemy->state.hidden=1;enemy->state.action=0x80;
    enemy->state.spawnAction=definition->spawnAction;
    enemy->state.attributes=definition->attributes;
    enemy->state.row=enemy->state.decisions=0;
    /* Source construction retains timer/animation flags/descriptor until
     * its selector initializer runs; do not invent reset behavior on reuse. */
    enemy->traits=definition->traits;enemy->variation=definition->variation;
    enemy->byte4=definition->initialByte4;enemy->byte6=definition->initialByte6;
    enemy->byte12=enemy->byte14=0;enemy->parent=parent;
    pool->doorChildren[parent]=index+1;pool->count++;
    door->flags|=2;door->spawnDelay=0x7f;*slot=index;
    return 1;
}
u8 RT_enemiesStepDoorActors(RT_EnemyPool *pool,RT_StagePool *stagePool,
    const RT_WorldCamera *camera,s32 playerX,u8 highVisible,
    RT_EnemySelection *selections,u8 *selectionCount,u8 *visibleCount)
{
    if(!pool || !stagePool || !stagePool->level || !camera || !selections ||
        !selectionCount || !visibleCount)return 0;
    *selectionCount=*visibleCount=0;
    s32 left=(s32)camera->x*16-0x300,right=(s32)camera->x*16+0x1500;
    s32 origin=((s32)camera->y+256)*16;
    for(u16 i=0;i<RT_ENEMY_CAPACITY;i++){
        RT_Enemy *enemy=pool->slots+i;RT_DoorEnemy *state=&enemy->state;
        if(state->kind==RT_ENEMY_FREE)continue;
        u8 action=state->action&0xfc;
        if((state->kind!=4 && state->kind!=8) ||
            (action!=0x58 && action!=0x5c &&
             !(action==0x80 && (state->spawnAction&0xfc)==0x5c)))return 0;
        state->hidden=state->x<left || (action==0x80?state->x>=right:state->x>right) ||
            state->y<=origin-0x1000 || state->y>origin+0x400;
        if(state->hidden)continue;
        (*visibleCount)++;
        if(enemy->parent>=RT_NATIVE_STAGE_CAPACITY)return 0;
        RT_Anchor *door=stagePool->objects+enemy->parent;
        if(door->kind>2)return 0;
        u16 definition=stagePool->definitions[enemy->parent];
        if(definition>=stagePool->level->eventCount)return 0;
        if(action==0x58){if(!RT_doorEnemyEnterStep(state,door))return 0;continue;}
        u8 selector;
        RT_DoorEnemyResult result=RT_doorEnemyStep(state,door,
            stagePool->level->events[definition].parameter,playerX,
            (u8)(highVisible+*visibleCount),&selector);
        if(result==RT_DOOR_ENEMY_UNHANDLED)return 0;
        if(result==RT_DOOR_ENEMY_HIDE)(*visibleCount)--;
        if(result==RT_DOOR_ENEMY_SELECT)
            selections[(*selectionCount)++]=(RT_EnemySelection){i,selector};
    }
    return 1;
}
