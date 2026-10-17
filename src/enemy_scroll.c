#include "rt_enemy_spawns.h"
u8 RT_enemiesCullWithProjectiles(RT_EnemyPool *pool,RT_StagePool *objects,RT_ProjectilePool *projectiles,
    const RT_WorldCamera *camera,u8 boundaries)
{
    if(!pool || !camera || boundaries>15)return 255;
    if(!pool->count || !boundaries || boundaries==3 || boundaries==7 || boundaries>=11)return 0;
    u8 remaining=pool->count,removed=0;
    for(u16 i=0;i<RT_ENEMY_CAPACITY;i++){
        RT_Enemy *e=pool->slots+i;RT_DoorEnemy *a=&e->state;
        if(a->kind==RT_ENEMY_FREE)continue;
        if(a->hidden){
            s16 x=(s16)(a->x-(s32)camera->x*16);
            s16 y=(s16)(((s32)camera->y+256)*16-a->y);
            u8 remove=0;
            switch(boundaries){
            case 1:remove=x < -0x700;break;
            case 2:remove=x >= 0x1900;break;
            case 4:remove=y < -0x800;break;
            /* Original AFE5's flag5 repeats flag6's right bound. */
            case 5:case 6:remove=y < -0x800 || x >= 0x1900;break;
            case 8:remove=y >= 0x1800;break;
            case 9:remove=y >= 0x1800 || x < -0x700;break;
            case 10:remove=y >= 0x1800 || x >= 0x1900;break;
            }
            if(remove){
                u8 state=a->action&0xfc;
                if(a->kind<32 && state==0x5c){
                    if(!objects || e->parent>=RT_NATIVE_STAGE_CAPACITY ||
                        objects->objects[e->parent].kind>=3)return 255;
                    if(pool->doorChildren[e->parent]==i+1){
                        objects->objects[e->parent].flags&=0xfd;
                        objects->objects[e->parent].spawnDelay=255;
                    }
                }else if(a->kind<32 && (state==0x84 || state==0x88) && e->parent!=255){
                    /* Throw windup can scroll out before allocating a projectile.
                     * Native parent255 means no link. CPU2 C37D/C389 only
                     * releases a linked held68/69 object, not the windup itself. */
                    if(!projectiles || e->parent>=RT_PROJECTILE_CAPACITY-1)return 255;
                    RT_EnemyRecoilLinks links={0};RT_enemyThrownLinks(e,projectiles,&links);
                    if(links.thrownPhase && ((state==0x84 && links.thrownFamily==RT_RECOIL_THROW_68) ||
                        (state==0x88 && links.thrownFamily==RT_RECOIL_THROW_69)))++*links.thrownPhase;
                }
                a->kind=RT_ENEMY_FREE;pool->count--;removed++;
            }
        }
        if(!--remaining)return removed;
    }
    return 255;
}

u8 RT_enemiesCull(RT_EnemyPool *pool,RT_StagePool *objects,
    const RT_WorldCamera *camera,u8 boundaries)
{
    return RT_enemiesCullWithProjectiles(pool,objects,0,camera,boundaries);
}
