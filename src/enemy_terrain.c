#include "rt_enemies.h"
#ifdef __m68k__
#include "rt_enemy_terrain_68000.h"
#else
u8 RT_enemyTerrainProbeReady(s16 cellX,s16 cellY,u8 *code,u8 *flags,void *context)
{
    const RT_EnemyTerrainContext *probe=context;
    const RT_TerrainMaterial *material=RT_terrainWindowProbe(probe->window,
        probe->enemy->state.x,probe->enemy->state.y,cellX,cellY);
    if(!material)return 0;
    *code=material->probeCode;
    *flags=material->collision[(probe->enemy->state.attributes&0xc0)>>6];
    return 1;
}
#endif

u8 RT_enemyTerrainProbe(s16 cellX,s16 cellY,u8 *code,u8 *flags,void *context)
{
    const RT_EnemyTerrainContext *probe=context;
    if(!probe || !probe->enemy || !probe->stage || !probe->camera || !code || !flags)return 0;
    return RT_enemyTerrainProbeReady(cellX,cellY,code,flags,context);
}
