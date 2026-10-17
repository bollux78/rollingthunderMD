#ifndef RT_NATIVE_MOTION_H
#define RT_NATIVE_MOTION_H
#include "rt_world.h"
/* One physical state per entity. Positions and velocities use four fractional
 * bits, x right/y down. Fractions accumulate independently of collision-snapped
 * positions, as required by the original player motion behavior. */
typedef struct {
    s32 x,y;
    s16 velocityX,velocityY,gravity;
    u8 fractionX,fractionY,layer,reserved;
} RT_MotionBody;
typedef enum {RT_MOTION_ORDINARY,RT_MOTION_VAULT,RT_MOTION_DROP} RT_VerticalMode;
typedef enum {RT_MOTION_CLEAR,RT_MOTION_CONTACT,RT_MOTION_FELL_OUT} RT_MotionResult;
/* A probe offset is measured in terrain cells, with positive Y downward.
 * The foot anchor lies on a pixel boundary: sample one subpixel above it. */
const RT_TerrainMaterial *RT_motionProbe(const RT_WorldStage *stage,
    const RT_MotionBody *body,const RT_WorldCamera *camera,s16 dx,s16 dy);
/* Solo follows the original camera dead zone. Locked X is the existing Home
 * co-op movement boundary; shared-camera framing is a separate operation. */
RT_MotionResult RT_motionHorizontal(RT_MotionBody *body,
    const RT_WorldStage *stage,RT_WorldCamera *camera,u8 lockedCamera);
RT_MotionResult RT_motionVertical(RT_MotionBody *body,
    const RT_WorldStage *stage,RT_WorldCamera *camera,RT_VerticalMode mode);
/* Stair animation owns world displacement. Between its steps only the camera
 * moves, retaining the original threshold hysteresis in one per-agent byte. */
void RT_motionStairCamera(const RT_MotionBody *body,const RT_WorldStage *stage,
    RT_WorldCamera *camera,u8 *flags);
#endif
