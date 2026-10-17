#ifndef RT_NATIVE_CAMPAIGN_SCENE_COLLISION_H
#define RT_NATIVE_CAMPAIGN_SCENE_COLLISION_H
#include "rt_campaign_scene_actor.h"
typedef struct {u8 code,flags;} RT_SceneCollisionCell;
extern const RT_SceneCollisionCell rtSceneCollisionRows[][64];
extern const u8 rtSceneCollisionRowIds[32];
/* Fixed transition origin(0,8), priorityC0. Native screen12.4 coordinates and
 * horizontal cell displacement. No tile RAM, source shadow or classifier runs
 * at runtime. Compatible with RT_sceneCastStep; context is unused. */
u8 RT_sceneCollisionProbe(const RT_SceneActor *actor,s16 cellX,
 u8 *code,u8 *flags,void *context);
#endif
