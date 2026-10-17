#ifndef RT_NATIVE_CAMPAIGN_SCENE_H
#define RT_NATIVE_CAMPAIGN_SCENE_H
#include "rt_scene_players.h"
#include "rt_enemy_spawns.h"
#include "rt_enemy_pass.h"
#include "rt_enemy_throw.h"
#include "rt_campaign_score.h"
typedef struct {
    const RT_WorldStage *stage;
    const RT_EnemySpawnLevel *spawnLevel;
    RT_WorldCamera camera;
    RT_ScenePlayers players;
    RT_StagePool objects;
    RT_EventWindow objectWindow,enemyWindow;
    RT_TerrainWindow terrain;
    RT_EnemyPool enemies;
    RT_ProjectilePool projectiles;
    RT_CoopPopulation population;
    RT_Type10Effects effects[RT_ENEMY_CAPACITY];
    RT_ThrowMotion throwMotion[RT_PROJECTILE_CAPACITY];
    u8 attackPhases[RT_ENEMY_CAPACITY];
    u16 faultEvent;
    u8 ready;
    u8 visibleProjectiles;
    RT_CampaignTimer timer;
    u8 exiting; /* Exiting player mask; owns scripted movement and pauses world. */
    u8 exitPlayer; /* Player who triggered the shared completion. */
    u8 bossProgress; /* CPU2 CE9B death count; 0x80 after the final exit gate. */
    u8 hasHighEnemies; /* Complete authored grid/door spawn closure, not visibility. */
    u8 attackClockEnabled; /* Generated difficulty needs extended walking clock. */
} RT_CampaignScene;
typedef enum {RT_CAMPAIGN_INIT_OK,RT_CAMPAIGN_INIT_INVALID,
    RT_CAMPAIGN_INIT_OBJECTS,RT_CAMPAIGN_INIT_TERRAIN,
    RT_CAMPAIGN_INIT_ENEMIES} RT_CampaignInit;
/* Fixed authored Campaign entry. Survival must use its own generated world;
 * this API rejects that selection. enemySet explicitly selects authored set0/1.
 * Invalid inputs preserve scene; construction errors leave ready=0 and retain
 * faultEvent for diagnostics. Scene storage belongs in static RAM, not stack.
 * Allocation of all families does not establish complete object behavior,
 * rendering, audio, stage transitions, or Arcade life/credit flow. */
RT_CampaignInit RT_campaignSceneInitialize(RT_CampaignScene *scene,
    const RT_SessionConfig *selection,u8 story,u8 area,u8 checkpoint,
    u8 enemySet,u16 credits);
/* Called only after the completion sequence hands off. Source next-area entry
 * restores health and guarantees 50 primary rounds in solo, 75 in co-op,
 * retaining surplus ammo and alternate weapon counts.
 * Home credits/selection persist; dead co-op slots are not granted free lives.
 * Score, Arcade reserves, bonus/display sequence and graphics are caller-owned.
 * Final stage is rejected; it must use the ending route. */
RT_CampaignInit RT_campaignSceneAdvance(RT_CampaignScene *scene,u8 enemySet);
typedef struct {
    RT_HomeAgentStep steps[2];
    u8 sounds[2];
    RT_WeaponEvent weapons[2];
    u8 shots;
} RT_CampaignPlayerPass;
/* One player phase, before enemies/projectiles/damage and death resolution.
 * Uses SGDK pad masks (A jump/B fire); caller supplies both controllers.
 * Does not tick invincibility: the application must do that per real refresh,
 * including skipped presentations. Caller publishes camera/terrain afterward.
 * Continue/game-over must be gated by the application before calling this. */
u8 RT_campaignPlayersStep(RT_CampaignScene *scene,const u16 pads[2],
    RT_CampaignPlayerPass *result);
typedef struct {
    RT_EnemyPassResult enemies;
    RT_ProjectilePassResult projectiles;
    u8 playerHits,enemyHits;
} RT_CampaignBattlePass;
/* Publish an empty phase without touching unconsumed event capacity. */
static inline void RT_campaignBattlePassClear(RT_CampaignBattlePass *result)
{
    result->playerHits=result->enemyHits=0;
    RT_EnemyPassResult *enemies=&result->enemies;
    enemies->eventCount=enemies->visible=enemies->removed=enemies->hitEntries=0;
    enemies->doorTransitions=enemies->type10Deaths=0;
    enemies->targetPlayer=enemies->faultSlot=enemies->faultAction=255;
    result->projectiles.soundCount=result->projectiles.visible=result->projectiles.removed=0;
    result->projectiles.faultSlot=255;
}
/* After player movement, before object service/streaming. Publishes terrain,
 * services high and low enemies, then projectiles and player damage. Event
 * arrays remain ordered for the application audio/score/script consumers.
 * No injected actors/damage; tick and refresh phases supplied by real clocks.
 * No live target skips enemy/projectile service, matching Continue gating. */
u8 RT_campaignBattleStep(RT_CampaignScene *scene,u8 tick,u8 refresh,
    RT_CampaignBattlePass *result);
/* After object services, retire hidden entities and admit ordered authored
 * strips for this update's camera movement. Does not service object behaviors.
 * Failures can follow partial admission and must stop the application's loop. */
u8 RT_campaignScroll(RT_CampaignScene *scene,const RT_WorldCamera *previous);
typedef struct {s16 column,row;u8 slot,kind,replace,sound;} RT_CampaignObjectEvent;
typedef struct {
    RT_CampaignObjectEvent events[RT_NATIVE_STAGE_CAPACITY];
    u32 unservicedKinds;
    u8 count,spawned;
} RT_CampaignObjectPass;
/* Door/fan phase after battle, before scrolling. Returns persistent world-cell
 * tile edits to the renderer, which must retain them across frame skips.
 * Other visible object families are reported in unservicedKinds; a nonzero
 * mask means object coverage is incomplete, not a fully supported scene. */
u8 RT_campaignObjectsStep(RT_CampaignScene *scene,u8 tick,
    RT_CampaignObjectPass *result);
/* visibilityReady requires a successful ObjectsStep at the same camera with
 * unchanged object membership, positions and visibility since that step. */
u8 RT_campaignScrollPrepared(RT_CampaignScene *scene,const RT_WorldCamera *previous,u8 visibilityReady);
#endif
