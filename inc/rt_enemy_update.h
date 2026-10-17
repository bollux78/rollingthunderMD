#ifndef RT_NATIVE_ENEMY_UPDATE_H
#define RT_NATIVE_ENEMY_UPDATE_H
#include "rt_combat.h"
struct RT_ThrowMotion;
typedef struct {
    const RT_WorldCamera *camera;
    const RT_StagePool *objects;
    const RT_Agent *target;
    RT_PlayerTargets *targets;
    RT_ProjectilePool *projectiles;
    RT_EnemyProbe probe;
    void *probeContext;
    s16 accelerationY;
    u8 visibleObjects,decisionCounter,objectCounter,workspaceFlags,fallbackSelector;
    u8 *coopAttackPhase;
    struct RT_ThrowMotion *throwMotion;
    u8 visibleProjectiles; /* previous ordered secondary pass */
    const RT_EnemyPerceptionTarget *perception; /* optional per-pass snapshot */
} RT_EnemyUpdateContext;
/* Visible common guard service. Perception precedes exactly one handler;
 * outgoing entry initializes in this call but is not stepped again.
 * Source43 A8/AC transitions require no retained object link.
 * Throw states require shared throwMotion metadata; linked door states use
 * RT_enemyUpdateDoor. Unsupported object-link states remain unavailable.
 * Caller owns visibility, target choice, pool removal and sound delivery. */
RT_EnemyResult RT_enemyUpdate(RT_Enemy *enemy,const RT_EnemyUpdateContext *context,
    RT_WeaponEvent *event);
/* Pass-validated common enemy, context pointers, terrain probe and event. */
RT_EnemyResult RT_enemyUpdateReady(RT_Enemy *enemy,const RT_EnemyUpdateContext *context,
    RT_WeaponEvent *event);
/* Linked door wait/exit/entry service; use before the parent's door animation.
 * Visibility and current ordered visible-actor count come from the scene. */
RT_EnemyResult RT_enemyUpdateDoor(RT_Enemy *enemy,RT_StagePool *objects,
    const RT_WorldCamera *camera,const RT_Agent *target,u8 visibleActors,
    u8 workspaceFlags);
#endif
