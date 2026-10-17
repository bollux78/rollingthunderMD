#ifndef RT_NATIVE_SURVIVAL_SPAWNS_H
#define RT_NATIVE_SURVIVAL_SPAWNS_H
#include "rt_survival_events.h"
/* Live-pool admission through the same constructors/policy as Campaign.
 * Stage pool retains the original Level1 level for definition metadata.
 * Caller owns camera-window scheduling, retention and retirement. */
u8 RT_survivalSpawnObjects(const RT_SurvivalWorld *world,RT_StagePool *pool,
    const RT_EventWindowRequest *request);
/* World densityTier0..4 schedules0..100% additional authored constructors in
 * quarter steps. Co-op applies its existing1.5x policy on that base; full
 * native pools cap admission. This does not alter attack timing or damage. */
u8 RT_survivalSpawnEnemies(const RT_SurvivalWorld *world,RT_EnemyPool *pool,
    const RT_EnemySpawnLevel *source,const RT_EventWindowRequest *request,
    u16 *fault,RT_CoopPopulation *policy,RT_Type10Effects *effects,u8 highFamilies);
#endif
