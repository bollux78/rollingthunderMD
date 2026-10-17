#ifndef RT_NATIVE_CAMPAIGN_TRANSITION_SCENE_H
#define RT_NATIVE_CAMPAIGN_TRANSITION_SCENE_H
#include "rt_stage_pool.h"
/* Transition high actors share native stable slots with the separate low-actor
 * service. Canonical scene coordinates are screen pixels in12.4, Y down;
 * the source scene origin is(0,8). No source record or descriptor pointer lives
 * in the runtime. Kind selects the immutable sprite pose (source type-0x40).
 * Caller owns construction order, render acknowledgement and low actors. */
u8 RT_transitionSceneConstruct(RT_StagePool *pool,u8 selector,u8 *spawned);
/* Service once per accepted transition render request. remaining is decimal;
 * ordinaryCompletion selects the source flow02=6,mode11 head threshold.
 * Returns visible high actors, or255 on invalid/exhausted pool, unchanged.
 * Low actors are neither serviced nor counted as visible by this function. */
u8 RT_transitionSceneStep(RT_StagePool *pool,u16 remaining,u8 ordinaryCompletion);
#endif
