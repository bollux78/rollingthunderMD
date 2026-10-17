#ifndef RT_NATIVE_ENEMY_PASS_H
#define RT_NATIVE_ENEMY_PASS_H
#include "rt_enemy_update.h"
#include "rt_type10.h"
struct RT_ThrowMotion;
typedef struct {
    const RT_WorldStage *stage;
    const RT_WorldCamera *camera;
    const RT_TerrainWindow *terrain;
    RT_StagePool *objects;
    RT_PlayerTargets players;
    RT_ProjectilePool *projectiles;
    u8 highVisible,visibleObjects,decisionCounter,objectCounter,workspaceFlags,fallbackSelector;
    u8 *coopAttackPhases; /* optional RT_ENEMY_CAPACITY attack-clock bytes; see rt_enemy_attack_clock.h */
    u32 *actionCoverage; /* optional two-word diagnostic mask, accumulated */
    struct RT_ThrowMotion *throwMotion;
    u8 visibleProjectiles; /* previous ordered secondary pass */
    u8 *admissionDelays; /* optional per-slot Home reinforcement delay */
    RT_Type10Effects *type10Effects; /* optional per-slot family state */
    u8 scenePhase,sceneStep;
    u8 vblankPhase; /* current display refresh phase for high-family choices */
} RT_EnemyPassContext;
typedef struct {u16 scoreBCD;u8 slot,sound,command,player;} RT_EnemyPassEvent;
/* At most one event per prepare/perception/active phase per shared actor. */
#define RT_ENEMY_PASS_EVENT_CAPACITY (RT_ENEMY_CAPACITY*3)
typedef struct {
    RT_EnemyPassEvent events[RT_ENEMY_PASS_EVENT_CAPACITY];
    u8 eventCount,visible,removed,hitEntries,doorTransitions,targetPlayer,faultSlot,faultAction,type10Deaths;
} RT_EnemyPassResult;
/* Preserve phase order and reuse the phase accumulator without losing its slot. */
static inline u8 RT_enemyPassFlush(RT_EnemyPassResult *out,RT_EnemyPassEvent *event)
{
    if(!event->sound && !event->command && !event->scoreBCD)return 1;
    if(out->eventCount>=RT_ENEMY_PASS_EVENT_CAPACITY)return 0;
    out->events[out->eventCount++]=*event;
    event->sound=event->command=0;event->scoreBCD=0;
    return 1;
}
/* Legacy Home target policy: player0 unless dead, then surviving player1.
 * Returns255 when no player is alive. Does not swap or copy player records. */
u8 RT_enemyTargetPlayer(const RT_PlayerTargets *players);
/* Stable native pool pass for currently supported common/door guard states.
 * Caller publishes terrain first and runs door animation afterwards. Events
 * remain ordered by actor slot; scene owns score attribution and audio queues.
 * Failure reports the offending slot/action and may follow earlier mutations.
 * No target is a scene/Continue condition, reported as failure with slot255.
 */
u8 RT_enemiesUpdate(RT_EnemyPool *pool,const RT_EnemyPassContext *context,
    RT_EnemyPassResult *result);
/* High-family phase services the SAME39 slots before the low-family phase.
 * Callback appends ordered events and updates counts; no runtime ROM dispatch. */
typedef u8 (*RT_HighEnemyPass)(RT_EnemyPool *,const RT_EnemyPassContext *,RT_EnemyPassResult *);
u8 RT_enemiesUpdateWithHigh(RT_EnemyPool *pool,const RT_EnemyPassContext *context,
    RT_EnemyPassResult *result,RT_HighEnemyPass highPass);
/* Currently supports kind32; rejects other live high families explicitly. */
u8 RT_type20PoolPass(RT_EnemyPool *pool,const RT_EnemyPassContext *context,
    RT_EnemyPassResult *result);
/* Native kind32/36/40/44 in original shared-slot order, including forward child spawns. */
u8 RT_highFamiliesPoolPass(RT_EnemyPool *pool,const RT_EnemyPassContext *context,
    RT_EnemyPassResult *result);
#endif
