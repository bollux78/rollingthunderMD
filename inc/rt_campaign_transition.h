#ifndef RT_NATIVE_CAMPAIGN_TRANSITION_H
#define RT_NATIVE_CAMPAIGN_TRANSITION_H
#include "rt_world.h"
typedef struct {u16 remaining;u8 divider,ending,pending,complete;} RT_TransitionClock;
typedef enum {RT_TRANSITION_INVALID,RT_TRANSITION_WAIT,RT_TRANSITION_COMPLETE} RT_TransitionResult;
/* Initialize after scene construction. Source origin reset/actor construction
 * and presentation are separate owners. Decimal timer starts at8 units. */
u8 RT_transitionClockBegin(RT_TransitionClock *clock,u8 ending);
/* One scene service may be outstanding. No new actor update until its render
 * is acknowledged; skipped or delayed presentations do not advance the clock. */
u8 RT_transitionFrameRequest(RT_TransitionClock *clock);
RT_TransitionResult RT_transitionFramePresented(RT_TransitionClock *clock);
typedef enum {RT_TRANSITION_AUDIO,RT_TRANSITION_SCENE_COMMAND} RT_TransitionChannel;
typedef struct {u8 channel,command;} RT_TransitionEvent;
typedef struct {RT_TransitionEvent events[2];u8 count;} RT_TransitionCommands;
/* Ordinary transition A466/A4D1 commands, in source order. Caller owns once-only
 * phase dispatch and delivers both events before advancing. Selector5 preserves
 * the existing scene effect on entry. Native selectors0..10 are supported. */
u8 RT_transitionCommandsBegin(u8 selector,u8 *effect,RT_TransitionCommands *commands);
u8 RT_transitionCommandsEnd(u8 *selector,u8 *effect,RT_TransitionCommands *commands);
#endif
