#ifndef RT_NATIVE_PROJECTILES_H
#define RT_NATIVE_PROJECTILES_H
#include "rt_agent.h"
#include "rt_terrain_window.h"
#define RT_PROJECTILE_CAPACITY 49
/* Source allocation examines 48 adjacent pairs, including slots 47/48.
 * Native slots preserve that deterministic order without source records. */
typedef enum {RT_PROJECTILE_FREE,RT_PROJECTILE_NEW_SHOT,RT_PROJECTILE_NEW_FLASH,
    RT_PROJECTILE_SHOT,RT_PROJECTILE_FLASH,RT_PROJECTILE_IMPACT,
    RT_PROJECTILE_NEW_THROW_DOWN,RT_PROJECTILE_NEW_THROW_FORWARD,
    RT_PROJECTILE_HELD_DOWN,RT_PROJECTILE_HELD_FORWARD,
    RT_PROJECTILE_THROWN,RT_PROJECTILE_EXPLOSION} RT_ProjectileKind;
enum {RT_PROJECTILE_LEFT=1,RT_PROJECTILE_LAST=2,RT_PROJECTILE_ALTERNATE=4,RT_PROJECTILE_ENEMY=8};
typedef struct {
    s32 x,y;
    s16 velocityX;
    u8 kind,phase,timer,pose,flags,owner,layer,reserved;
} RT_Projectile;
typedef struct {RT_Projectile slots[RT_PROJECTILE_CAPACITY];u8 active,reserved;} RT_ProjectilePool;
typedef struct {s16 velocityX,x,y,flashX,flashY;u8 pose,flashPose,flags,reserved;} RT_ShotTemplate;
typedef struct {u8 sound,command;} RT_WeaponEvent;
extern const RT_ShotTemplate rtShotTemplates[12];
extern const u8 rtEnemyShotCyclePoses[2][3];
extern const u8 rtShotCyclePoses[2][3],rtShotFlashPoses[2][2],rtShotImpactPoses[2];
/* Consume this agent's pending request. A full pool or empty primary weapon
 * still retains the firing animation; ammunition changes only on allocation. */
u8 RT_projectilesRequest(RT_ProjectilePool *pool,RT_Agent *agent,u8 owner,RT_WeaponEvent *event);
/* Called only after bounds/terrain accept the shot. The target system applies
 * its source-derived hit response and returns whether a target was struck.
 * RT_PROJECTILE_ENEMY selects player targets; otherwise select enemy targets. */
typedef u8 (*RT_ProjectileHit)(const RT_Projectile *projectile,void *context);
typedef u8 (*RT_ProjectileTerrain)(const RT_Projectile *shot,void *context);
/* Single ordinary slot service for the unified ordered secondary pass. */
void RT_projectileStepWithTerrain(RT_ProjectilePool *pool,u8 slot,RT_Agent *agents,u8 players,
    const RT_WorldStage *stage,const RT_WorldCamera *camera,RT_ProjectileHit hit,void *context,
    RT_ProjectileTerrain terrain,void *terrainContext);
void RT_projectilesStepWithTerrain(RT_ProjectilePool *pool,RT_Agent *agents,u8 players,
    const RT_WorldStage *stage,const RT_WorldCamera *camera,RT_ProjectileHit hit,void *context,
    RT_ProjectileTerrain terrain,void *terrainContext);
u8 RT_projectilesStepPublished(RT_ProjectilePool *pool,RT_Agent *agents,u8 players,
    const RT_WorldStage *stage,const RT_WorldCamera *camera,const RT_TerrainWindow *window,
    RT_ProjectileHit hit,void *context);
void RT_projectilesStep(RT_ProjectilePool *pool,RT_Agent *agents,u8 players,
    const RT_WorldStage *stage,const RT_WorldCamera *camera,RT_ProjectileHit hit,void *context);
#endif
