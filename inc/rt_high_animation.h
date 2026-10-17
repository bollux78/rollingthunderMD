#ifndef RT_NATIVE_HIGH_ANIMATION_H
#define RT_NATIVE_HIGH_ANIMATION_H
#include "rt_enemies.h"
/* Generated native pose identifiers, not source runtime addresses. */
#define RT_HIGH_POSE_FIRST 383
#define RT_HIGH_POSE_COUNT 153
#define RT_HIGH_SEQUENCE_COUNT 30
typedef struct {u8 timer,flags;u16 pose;} RT_HighAnimationRow;
extern const RT_HighAnimationRow rtHighStateRows[4][48];
extern const RT_HighAnimationRow rtHighCycleRows[244];
extern const u16 rtHighCycleOffsets[31];
#endif
