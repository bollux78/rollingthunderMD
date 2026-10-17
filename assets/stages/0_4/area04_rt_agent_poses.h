#ifndef area04_RT_NATIVE_AGENT_POSES_H
#define area04_RT_NATIVE_AGENT_POSES_H
#include "rt_sprites.h"
#define area04_RT_AGENT_POSE_COUNT 79
/* Variants: Albatross, Leila, alternate Leila, gold-shirt Albatross. Unsupported art is a null pose. */
extern const RT_SpritePose area04_rtAgentPoses[4][area04_RT_AGENT_POSE_COUNT];
/* Rule0 only; other art uses the generic renderer. */
extern const RT_PackedSpritePose area04_rtAgentGraphics[4][area04_RT_AGENT_POSE_COUNT];
#endif
