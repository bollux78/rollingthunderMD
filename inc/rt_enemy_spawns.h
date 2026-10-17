#ifndef RT_NATIVE_ENEMY_SPAWNS_H
#define RT_NATIVE_ENEMY_SPAWNS_H
#include "rt_enemies.h"
#include "rt_type10.h"
/* Immutable original actor-grid placement, native downward world pixels.
 * All families are retained, including those not yet implemented at runtime. */
typedef struct {s16 x,y;u16 definition;} RT_EnemySpawn;
typedef struct {
    const RT_EnemySpawn *events;
    const RT_StageEventCell *cells;
    u16 eventCount;
    u8 columns,rows;
} RT_EnemySpawnLevel;
extern const RT_EnemyTemplate rtGridEnemyTemplates[];
extern const u16 rtGridEnemyTemplateCount;
extern const RT_EnemySpawnLevel rtEnemySpawnLevels[2][10];
/* First-free native constructor for common guards. Unsupported families reject
 * before mutation; they must not be silently dropped by a spawn visitor.
 * Scene owns aggregate counts/admission and subsequent state80 activation. */
u8 RT_enemiesSpawnGrid(RT_EnemyPool *pool,const RT_EnemySpawn *spawn,u8 *slot);
/* Extended constructor admits type10 when the caller supplies one effects
 * record per pool slot. Clear that slot's native effects on every successful
 * reuse, including common guards. Failures leave pool, effects and slot intact.
 * This reset is native ownership policy, not an arcade record-field write. */
u8 RT_enemiesSpawnGridWithType10(RT_EnemyPool *pool,const RT_EnemySpawn *spawn,
    u8 *slot,RT_Type10Effects *effects);
/* Retire hidden actors at a camera boundary before admitting its strips.
 * Uses source combined-axis rules (flag5 intentionally differs from objects).
 * Returns removed count or255 on invalid accounting/unavailable links; earlier
 * removals survive failure. Door ownership cleanup is supported, throws pending. */
/* Shared projectile pool permits source68/69 release when a thrower retires. */
u8 RT_enemiesCullWithProjectiles(RT_EnemyPool *pool,RT_StagePool *objects,
    RT_ProjectilePool *projectiles,const RT_WorldCamera *camera,u8 boundaries);
u8 RT_enemiesCull(RT_EnemyPool *pool,RT_StagePool *objects,
    const RT_WorldCamera *camera,u8 boundaries);
/* Admit original records into the common pool with shared high+low population.
 * Unsupported records stop with their event index; earlier allocations remain.
 * fault=65535 indicates no record-specific failure. Caller must surface errors. */
u8 RT_enemiesSpawnWindow(RT_EnemyPool *pool,const RT_EnemySpawnLevel *level,
    const RT_EventWindowRequest *request,u8 highPopulation,u16 *fault);
/* Home 2P additions reuse authored placements. One reinforcement per two
 * accepted originals; capacity shortage is reported, never counted as spawned.
 * Extras wait12 visible updates before service. Pass admissionDelays to the
 * actor pass; clear a slot delay when another constructor reuses that slot. */
typedef struct {u16 originals,extras,capacityMisses;u8 phase;u8 admissionDelays[RT_ENEMY_CAPACITY];} RT_CoopPopulation;
/* Native admission state shared by authored and relocated visitors. Initialize
 * population from the validated pool (plus any separately owned high actors),
 * and fault to65535. The callback owns successful population increments and
 * the co-op phase, including delayed reinforcements. */
typedef struct {
    RT_EnemyPool *pool;u8 population;u16 *fault;RT_CoopPopulation *coop;
    RT_Type10Effects *effects;u8 highFamilies;
} RT_EnemyAdmission;
u8 RT_enemyAdmissionVisit(const RT_EnemySpawn *spawn,u16 index,void *context);
u8 RT_enemiesSpawnWindowCoop(RT_EnemyPool *pool,const RT_EnemySpawnLevel *level,
    const RT_EventWindowRequest *request,u8 highPopulation,u16 *fault,
    RT_CoopPopulation *policy);
/* Extended admission uses the type10 constructor and resets per-slot effects.
 * Caller must supply effects and optional policy admissionDelays to the actor
 * pass as well, which consumes the family-specific reinforcement delays. */
u8 RT_enemiesSpawnWindowWithType10(RT_EnemyPool *pool,const RT_EnemySpawnLevel *level,
    const RT_EventWindowRequest *request,u8 highPopulation,u16 *fault,
    RT_CoopPopulation *policy,RT_Type10Effects *effects);
enum {RT_ENEMY_SPAWN_ERROR,RT_ENEMY_SPAWN_ACCEPTED,RT_ENEMY_SPAWN_FULL};
typedef u8 (*RT_EnemySpawnVisitor)(const RT_EnemySpawn *spawn,u16 index,void *context);
/* Source actor strips span three cells, including LEFT (stage-object LEFT
 * spans four). Population is the shared actor count, updated by the visitor.
 * FULL stops the current cell; ERROR stops publication after prior successes.
 * Center inputs are bounded to the authored grid plus two border cells. */
u8 RT_visitEnemySpawns(const RT_EnemySpawnLevel *level,u8 gridX,u8 gridY,
    RT_EventWindowEdge edge,const u8 *population,RT_EnemySpawnVisitor visit,void *context);
/* Original CPU2 B1E0 shares39 slots across low and high families. The mask
 * opts into constructor groups32/36/40/44 as bits0..3; only enable a group when
 * the caller supplies its service/rendering. No separate high allocation pool. */
u8 RT_enemiesSpawnGridWithFamilies(RT_EnemyPool *pool,const RT_EnemySpawn *spawn,
    u8 *slot,RT_Type10Effects *effects,u8 highFamilies);
u8 RT_enemiesSpawnWindowWithFamilies(RT_EnemyPool *pool,const RT_EnemySpawnLevel *level,
    const RT_EventWindowRequest *request,u16 *fault,RT_CoopPopulation *policy,
    RT_Type10Effects *effects,u8 highFamilies);
#endif
