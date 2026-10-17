#ifndef RT_NATIVE_HIGH_ENEMY_MOTION_H
#define RT_NATIVE_HIGH_ENEMY_MOTION_H
#include "rt_enemies.h"
/* Typed native motion for source high families20/24/28/2C. Coordinates and
 * velocityY are downward world units; comparisons retain source signed16
 * camera-local wrap. Probe offsets use native right/down cell directions.
 * REMOVED reports the boundary only: caller owns pool counts and retirement.
 * Shadow phase bytes are explicit until the high-family scene owns them. */
u8 RT_highEnemyGround(RT_Enemy *enemy,const RT_WorldCamera *camera,
    RT_EnemyProbe probe,void *context,u8 *selector);
RT_EnemySweep RT_highEnemyHorizontal(RT_Enemy *enemy,const RT_WorldCamera *camera,
    u8 shadowXLow,RT_EnemyProbe probe,void *context,u8 *hitCode);
RT_EnemySweep RT_highEnemyVertical(RT_Enemy *enemy,const RT_WorldCamera *camera,
    s16 accelerationY,u8 shadowYLow,RT_EnemyProbe probe,void *context,u16 *result);
/* Source upward speed0000..000F becomes native downward speed0..-15. */
u8 RT_highEnemySettle(RT_Enemy *enemy);
#endif
