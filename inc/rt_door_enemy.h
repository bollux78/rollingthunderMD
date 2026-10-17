#ifndef RT_NATIVE_DOOR_ENEMY_H
#define RT_NATIVE_DOOR_ENEMY_H
#include "rt_anchors.h"
/* Native state for the shared type04/08 door-exit handler. World coordinates
 * replace camera-relative source words. Pose indexes address compiled art;
 * no arcade RAM record, pointer, or instruction state is retained. */
typedef struct {
    s32 x,y;
    u8 kind,action,spawnAction,row,timer,flags,attributes,decisions,hidden;
    u16 pose;
} RT_DoorEnemy;
typedef enum {RT_DOOR_ENEMY_UNHANDLED,RT_DOOR_ENEMY_UPDATED,
    RT_DOOR_ENEMY_HIDE,RT_DOOR_ENEMY_SELECT} RT_DoorEnemyResult;
/* Call before the linked door's service. SELECT must be dispatched by the
 * actor behavior owner; HIDE must update that owner's visibility count.
 * parentAttributes comes from the immutable stage event parameter. */
RT_DoorEnemyResult RT_doorEnemyStep(RT_DoorEnemy *enemy,RT_Anchor *door,
    u8 parentAttributes,s32 playerX,u8 visibleActors,u8 *selector);
/* Type04/08/0C state58. Caller installs the initial row0 pose. Subsequent
 * entry poses use indexes6..10. Row6 is retained, not deleted; the source
 * returns there even without a parent. */
u8 RT_doorEnemyEnterStep(RT_DoorEnemy *enemy,RT_Anchor *door);
#endif
