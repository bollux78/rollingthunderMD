#ifndef RT_NATIVE_SURVIVAL_EVENTS_H
#define RT_NATIVE_SURVIVAL_EVENTS_H
#include "rt_survival_world.h"
#include "rt_enemy_spawns.h"
/* Visit authored Level1 records at their relocated positions, retaining the
 * original definition index for door parameters and enemy templates. Repeated
 * sections intentionally have repeated definition indices. Live instances
 * still own distinct pool slots. Counts and failure semantics match the native
 * visitors; callers must not retain the temporary relocated event pointer. */
u8 RT_survivalVisitStageEvents(const RT_SurvivalWorld *world,
    const RT_StageEventLevel *source,const RT_EventWindowRequest *request,
    u8 *population,RT_StageEventVisitor visit,void *context);
u8 RT_survivalVisitEnemySpawns(const RT_SurvivalWorld *world,
    const RT_EnemySpawnLevel *source,const RT_EventWindowRequest *request,
    const u8 *population,RT_EnemySpawnVisitor visit,void *context);
#endif
