#ifndef RT_NATIVE_AGENT_POSES_H
#define RT_NATIVE_AGENT_POSES_H
#include "rt_sprites.h"
#define RT_AGENT_POSE_COUNT 79
/* Variants: Albatross, Leila, alternate Leila, gold-shirt Albatross. Unsupported art is a null pose. */
extern const RT_SpritePose rtAgentPoses[4][RT_AGENT_POSE_COUNT];
/* Rule0 only; other art uses the generic renderer. */
extern const RT_PackedSpritePose rtAgentGraphics[4][RT_AGENT_POSE_COUNT];
#endif
