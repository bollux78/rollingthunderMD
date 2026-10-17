#include "rt_enemy_update.h"
/* Rare per-actor service: keep it out of the hot pool traversal. */
__attribute__((noinline))
RT_EnemyResult RT_enemyUpdateDoor(RT_Enemy *e,RT_StagePool *objects,
    const RT_WorldCamera *camera,const RT_Agent *target,u8 visibleActors,
    u8 workspaceFlags)
{
    if(!e || !objects || !objects->level || !camera || !target ||
        e->parent>=RT_NATIVE_STAGE_CAPACITY)return RT_ENEMY_UNAVAILABLE;
    u8 state=e->state.action&0xfc;
    if(state!=0x58 && state!=0x5c &&
        !(state==0x80 && (e->state.spawnAction&0xfc)==0x5c))return RT_ENEMY_UNAVAILABLE;
    RT_Anchor *door=objects->objects+e->parent;
    u16 definition=objects->definitions[e->parent];
    if(door->kind>2 || definition>=objects->level->eventCount)return RT_ENEMY_UNAVAILABLE;
    if(!RT_enemyPerceive(e,target))return RT_ENEMY_UNAVAILABLE;
    if(state==0x58)return RT_doorEnemyEnterStep(&e->state,door)?RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
    u8 selector=0;
    RT_DoorEnemyResult result=RT_doorEnemyStep(&e->state,door,
        objects->level->events[definition].parameter,target->body.x,visibleActors,&selector);
    if(result==RT_DOOR_ENEMY_UNHANDLED)return RT_ENEMY_UNAVAILABLE;
    if(result!=RT_DOOR_ENEMY_SELECT)return RT_ENEMY_CONTINUE;
    return RT_enemySelect(e,camera,selector,workspaceFlags)?RT_ENEMY_SELECT:RT_ENEMY_UNAVAILABLE;
}
