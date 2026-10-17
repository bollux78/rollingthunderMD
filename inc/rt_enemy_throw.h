#ifndef RT_NATIVE_ENEMY_THROW_H
#define RT_NATIVE_ENEMY_THROW_H
#include "rt_combat.h"
/* Throw-only data indexed by the shared projectile slot. The projectile kind
 * owns validity; reallocation initializes these fields before use. */
typedef struct RT_ThrowMotion {s16 velocityY;u8 attributes,direction;} RT_ThrowMotion;
extern const RT_EnemyEntry rtEnemyThrowEntries[20]; /* poses315..334 */
/* Actor animation, first-free shared allocation, held placement and release.
 * visibleProjectiles is the previous secondary pass's visible count.
 * Caller services the shared projectile pass and streams throw graphics.
 * parent retains the native slot link for interruption/scroll cleanup. */
RT_EnemyResult RT_enemyThrowStep(RT_Enemy *enemy,RT_ProjectilePool *pool,
    RT_ThrowMotion motion[RT_PROJECTILE_CAPACITY],u8 visibleProjectiles,
    RT_WeaponEvent *event,u8 *selector);
typedef u8 (*RT_ThrowProbe)(const RT_Projectile *object,const RT_ThrowMotion *motion,
    u8 *code,u8 *flags,void *context);
typedef u8 (*RT_ThrowHit)(const RT_Projectile *object,const RT_ThrowMotion *motion,
    u8 explosion,void *context);
/* One shared-slot service, including new-object activation. Probe failure
 * preserves source movement already applied and reports unavailable. */
RT_EnemyResult RT_throwMotionStep(RT_ProjectilePool *pool,u8 slot,
    RT_ThrowMotion motion[RT_PROJECTILE_CAPACITY],const RT_WorldCamera *camera,
    RT_ThrowProbe probe,RT_ThrowHit hit,void *context,RT_WeaponEvent *event);
/* Callback context is RT_PlayerTargets. A blast can hit both players in
 * source order. Home invincibility and native retired-player state gate hits. */
u8 RT_throwHitPlayers(const RT_Projectile *object,const RT_ThrowMotion *motion,
    u8 explosion,void *context);
typedef struct {const RT_TerrainWindow *terrain;RT_PlayerTargets players;} RT_ThrowSceneContext;
u8 RT_throwTerrainProbe(const RT_Projectile *object,const RT_ThrowMotion *motion,
    u8 *code,u8 *flags,void *context);
u8 RT_throwSceneHit(const RT_Projectile *object,const RT_ThrowMotion *motion,
    u8 explosion,void *context);
typedef struct {
    const RT_WorldStage *stage;
    const RT_WorldCamera *camera;
    RT_ThrowSceneContext scene;
    RT_ThrowMotion *motion;
    RT_ProjectileHit bulletHit;
    void *bulletContext;
} RT_ProjectilePassContext;
typedef struct {u8 slot,sound;} RT_ProjectileSound;
typedef struct {
    RT_ProjectileSound sounds[RT_PROJECTILE_CAPACITY];
    u8 soundCount,visible,removed,faultSlot;
} RT_ProjectilePassResult;
/* Stable shared slot order. New records activate without motion this refresh.
 * Sounds remain ordered by slot; caller owns delivery. Stops at an unsupported
 * slot, retaining earlier work. visible reproduces the source secondary count. */
u8 RT_projectilesUpdateAll(RT_ProjectilePool *pool,const RT_ProjectilePassContext *context,
    RT_ProjectilePassResult *result);
#endif
