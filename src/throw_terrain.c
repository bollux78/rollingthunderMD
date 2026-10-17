#include "rt_enemy_throw.h"
u8 RT_throwTerrainProbe(const RT_Projectile *object,const RT_ThrowMotion *motion,
    u8 *code,u8 *flags,void *context)
{
    const RT_ThrowSceneContext *scene=context;
    if(!object || !motion || !code || !flags || !scene || !scene->terrain ||
       !scene->terrain->ready)return 0;
    const RT_TerrainMaterial *material=RT_terrainWindowProbe(scene->terrain,object->x,object->y,1,0);
    if(!material)return 0;
    *code=material->probeCode;*flags=material->collision[motion->attributes>>6];return 1;
}
u8 RT_throwSceneHit(const RT_Projectile *object,const RT_ThrowMotion *motion,
    u8 explosion,void *context)
{
    RT_ThrowSceneContext *scene=context;
    return scene?RT_throwHitPlayers(object,motion,explosion,&scene->players):0;
}
