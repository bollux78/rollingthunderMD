#ifndef RT_NATIVE_ENEMIES_H
#define RT_NATIVE_ENEMIES_H
#include "rt_door_enemy.h"
#include "rt_enemy_templates.h"
#include "rt_stage_pool.h"
#include "rt_terrain_window.h"
#include "rt_projectiles.h"
#define RT_ENEMY_CAPACITY 39
#define RT_ENEMY_FREE 255
typedef struct {
    RT_DoorEnemy state;
    const RT_EnemyTemplate *definition;
    s16 velocityX,velocityY;
    u8 traits,variation,byte4,byte6,byte12,byte14,parent,delay;
} RT_Enemy;
typedef struct {
    RT_Enemy slots[RT_ENEMY_CAPACITY];
    u8 doorChildren[RT_NATIVE_STAGE_CAPACITY]; /* slot+1; zero means no link */
    u8 count;
    /* Mutable family36/40 acceleration; every constructor resets its slot.
     * Separate storage preserves the 36-byte native actor layout. */
    s16 accelerationY[RT_ENEMY_CAPACITY];
    u8 scoreOwner[RT_ENEMY_CAPACITY]; /* Last player bullet; reset on slot reuse. */
} RT_EnemyPool;
typedef struct {u8 timer,flags;u16 graphicsKey;} RT_EnemyEntry;
extern const RT_EnemyEntry rtEnemyEntries[48];
/* B429 transitions for door exit, idle/turn, slopes and jump/fall:
 * 00/04/08/0C/10/14/1C/20/24/28/2C/30/34/38/3C/44/48/4C/50/54/94.
 * Preserves direction when selector low bits are zero. Entry poses12..59
 * map to rtEnemyEntries. Other selectors reject without modifying state. */
/* Camera-aware entry resolves standing-attack bounds and70/74 redirects. */
u8 RT_enemySelect(RT_Enemy *enemy,const RT_WorldCamera *camera,
    u8 selector,u8 workspaceFlags);
u8 RT_enemySelectAfterDoor(RT_Enemy *enemy,u8 selector);
/* Non-door state80 admission consumes this entire actor pass, without changing
 * visibility/counts. Supports original common grid selectors00/04/08/3C;
 * kind12 admits only06. Unsupported requests reject without mutation. */
u8 RT_enemyActivateGrid(RT_Enemy *enemy,const RT_WorldCamera *camera);
typedef enum {RT_ENEMY_CONTINUE,RT_ENEMY_SELECT,RT_ENEMY_UNAVAILABLE,RT_ENEMY_REMOVED} RT_EnemyResult;
typedef u8 (*RT_EnemyProbe)(s16 cellX,s16 cellY,u8 *code,u8 *flags,void *context);
typedef struct {
    const RT_Enemy *enemy;
    const RT_WorldStage *stage;
    const RT_WorldCamera *camera;
    const RT_TerrainWindow *window;
} RT_EnemyTerrainContext;
u8 RT_enemyTerrainProbe(s16 cellX,s16 cellY,u8 *code,u8 *flags,void *context);
/* Scene-validated context/enemy/stage/camera and non-null output pointers.
 * The published window itself is still checked before reading. */
u8 RT_enemyTerrainProbeReady(s16 cellX,s16 cellY,u8 *code,u8 *flags,void *context);
extern const RT_EnemyEntry rtEnemyPauseEntries[17];
/* States08/3C. Probe offsets are native cells (right/down positive), not
 * arcade addresses. Pause poses60..76 index rtEnemyPauseEntries. */
RT_EnemyResult RT_enemyPauseStep(RT_Enemy *enemy,s32 playerX,u8 projectileCount,
    RT_EnemyProbe probe,void *context,u8 *selector);
extern const RT_EnemyEntry rtEnemyIdleEntries[14];
/* State38 idle poses77..85; state48 turn poses86..90; state54 turn exit. */
RT_EnemyResult RT_enemyIdleStep(RT_Enemy *enemy,u8 *selector);
RT_EnemyResult RT_enemyTurnStep(RT_Enemy *enemy,u8 *selector);
RT_EnemyResult RT_enemyRangedUpdate(RT_Enemy *enemy,RT_ProjectilePool *pool,
    const RT_WorldCamera *camera,u8 workspaceFlags,RT_WeaponEvent *event);
u8 RT_enemyProjectileRequest(RT_ProjectilePool *pool,const RT_Enemy *enemy,
    u8 lowPose,RT_WeaponEvent *event,u8 *firstSlot);
/* Scene resolves stable native links; a thrown phase is optional. */
enum {RT_RECOIL_THROW_NONE,RT_RECOIL_THROW_68,RT_RECOIL_THROW_69};
typedef struct {RT_Anchor *door;u8 *thrownPhase;u8 doorOwnsEnemy,thrownFamily;} RT_EnemyRecoilLinks;
/* Resolve only native held families. New/airborne/reused slots do not match
 * source68/69 and must not receive a release-phase increment. */
static inline void RT_enemyThrownLinks(const RT_Enemy *enemy,RT_ProjectilePool *pool,
    RT_EnemyRecoilLinks *links)
{
    if(!enemy || !pool || !links || enemy->parent>=RT_PROJECTILE_CAPACITY-1)return;
    RT_Projectile *object=pool->slots+enemy->parent;
    if(object->kind==RT_PROJECTILE_HELD_DOWN)links->thrownFamily=RT_RECOIL_THROW_68;
    else if(object->kind==RT_PROJECTILE_HELD_FORWARD)links->thrownFamily=RT_RECOIL_THROW_69;
    else return;
    links->thrownPhase=&object->phase;
}
u8 RT_enemyRecoilEnter(RT_Enemy *enemy,u8 selector,u8 sourceCounter,
    const RT_EnemyRecoilLinks *links);
/* Object transition A8/AC; linked-object admission is owned by the scene. */
extern const RT_EnemyEntry rtEnemyTimedExitEntries[19]; /* poses288..306 */
RT_EnemyResult RT_enemyTimedExitStep(RT_Enemy *enemy,u8 *selector);
extern const RT_EnemyEntry rtEnemyApproachEntries[4]; /* poses284..287 */
RT_EnemyResult RT_enemyApproachStep(RT_Enemy *enemy,s32 playerX,u8 *selector);
extern const RT_EnemyEntry rtEnemyObjectEntries[16]; /* poses268..283 */
RT_EnemyResult RT_enemyObjectTransitionStep(RT_Enemy *enemy,u8 *selector);
extern const RT_EnemyEntry rtEnemyProximityEntries[17]; /* poses251..267 */
u8 RT_enemyProximityEnter(RT_Enemy *enemy,const RT_Agent *target);
RT_EnemyResult RT_enemyProximityStep(RT_Enemy *enemy,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context,u8 *selector);
extern const RT_EnemyEntry rtEnemyHitEntries[35]; /* poses216..250 */
u8 RT_enemyHitEnter(RT_Enemy *enemy,u8 selector,u8 sourceCounter,
    const RT_EnemyRecoilLinks *links,u16 *scoreBCD,u8 *sound);
RT_EnemyResult RT_enemyHitStep(RT_Enemy *enemy,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context,u8 *selector,u8 *command);
extern const RT_EnemyEntry rtEnemyRecoveryEntries[8]; /* poses208..215 */
RT_EnemyResult RT_enemyRecoveryStep(RT_Enemy *enemy,u8 fallbackSelector,u8 *selector);
RT_EnemyResult RT_enemyRecoilStep(RT_Enemy *enemy,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context,u8 *selector);
extern const RT_EnemyEntry rtEnemyRangedEntries[10]; /* poses194..203 */
/* Shot requests are consumed once by the projectile allocator. SELECT may
 * return selector zero, which must not be treated as no transition. */
RT_EnemyResult RT_enemyRangedStep(RT_Enemy *enemy,u8 workspaceFlags,
    u8 *requestShot,u8 *selector);
extern const RT_EnemyEntry rtEnemyHopEntries[64]; /* poses127..190 */
extern const RT_EnemyEntry rtEnemyShortEntries[3]; /* poses191..193 */
RT_EnemyResult RT_enemyHopStep(RT_Enemy *enemy,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context,u8 *selector);
extern const RT_EnemyEntry rtEnemyJumpEntries[18]; /* poses109..126 */
/* Jump28, fall2C and landing34, including ordered vertical collision. */
RT_EnemyResult RT_enemyJumpStep(RT_Enemy *enemy,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context,u8 *selector);
extern const RT_EnemyEntry rtEnemySlopeEntries[12]; /* poses97..108 */
RT_EnemyResult RT_enemySlopeStep(RT_Enemy *enemy,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,u8 *selector);
typedef enum {RT_ENEMY_SWEEP_CLEAR,RT_ENEMY_SWEEP_HIT,
    RT_ENEMY_SWEEP_REMOVED,RT_ENEMY_SWEEP_UNAVAILABLE} RT_EnemySweep;
/* Native downward-positive acceleration; removal cleanup remains caller-owned. */
RT_EnemySweep RT_enemySweepVertical(RT_Enemy *enemy,const RT_WorldCamera *camera,
    s16 accelerationY,RT_EnemyProbe probe,void *context);
extern const RT_EnemyEntry rtEnemyScriptWalkEntries[8]; /* poses307..314 */
RT_EnemyResult RT_enemyScriptWalkStep(RT_Enemy *enemy,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,u8 *selector);
RT_EnemySweep RT_enemySweepHorizontal(RT_Enemy *enemy,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,u8 *selector);
u8 RT_enemyWalkTransitions(RT_Enemy *enemy,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,u8 *selector);
u8 RT_enemyWalkAnimation(RT_Enemy *enemy);
extern const u16 rtEnemyWalkGraphics[6]; /* poses91..96 */
/* NULL co-op phase preserves arcade cadence; a phase pointer enables1.5x
 * ranged decision opportunities only, without accelerating movement. */
u8 RT_enemyWalkingChoice(RT_Enemy *enemy,s32 playerX,u8 sourceCounter,
    u8 *coopAttackPhase);
u8 RT_enemyFindWalkObject(const RT_Enemy *enemy,const RT_StagePool *objects,
    u8 visibleObjects,u8 sourceCounter,u8 *selector,u8 *slot);
/* Ordered movement -> terrain -> objects -> decision -> animation.
 * Removal marks the actor free; pool counts/link cleanup remain caller-owned.
 * Output selections must be dispatched, never silently replaced by walking. */
RT_EnemyResult RT_enemyWalkStep(RT_Enemy *enemy,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,const RT_StagePool *objects,u8 visibleObjects,
    s32 playerX,u8 decisionCounter,u8 objectCounter,u8 *selector,u8 *objectSlot,
    u8 *coopAttackPhase);
void RT_enemiesInitialize(RT_EnemyPool *pool);
/* Source D091 allocation only, invoked after RT_ANCHOR_SPAWN. No selector
 * initialization, AI or rendering is implied. Hidden/non-door parents reject.
 * Failure leaves pool, parent and output untouched. Stage is root*5+segment.
 * Links use stable slot indexes; removal/link side effects remain caller-owned. */
u8 RT_enemiesSpawnDoor(RT_EnemyPool *pool,RT_StagePool *stagePool,u8 parent,
    u8 stage,u8 worldXLow,u8 counter,u8 *slot);
typedef struct {u8 slot,selector;} RT_EnemySelection;
/* Door-only actor pass, in stable slot order, before door animation services.
 * Caller supplies the already serviced high-actor visible count. Selections
 * must be applied before another pass; unsupported actions return0 explicitly.
 * Output storage has RT_ENEMY_CAPACITY entries. This does not apply contact
 * damage, movement, or selectors, and failure can follow earlier mutations. */
u8 RT_enemiesStepDoorActors(RT_EnemyPool *pool,RT_StagePool *stagePool,
    const RT_WorldCamera *camera,s32 playerX,u8 highVisible,
    RT_EnemySelection *selections,u8 *selectionCount,u8 *visibleCount);
#endif
