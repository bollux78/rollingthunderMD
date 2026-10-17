#ifndef RT_NATIVE_TYPE10_H
#define RT_NATIVE_TYPE10_H
#include "rt_combat.h"
/* Script request and shared scene-command side effects are explicit, native
 * state. Caller owns script consumption, audio and pool removal bookkeeping. */
typedef struct {u8 scriptRequest,sharedCommand,sound;} RT_Type10Effects;
extern const RT_EnemyEntry rtType10Entries[48]; /* poses335..382 */
u8 RT_type10Enter(RT_Enemy *enemy,u8 selector);
u8 RT_type10Hit(RT_Enemy *enemy,u8 selector,RT_Type10Effects *effects);
RT_EnemyResult RT_type10Step(RT_Enemy *enemy,const RT_WorldCamera *camera,
    s32 playerX,RT_Type10Effects *effects,u8 *selector);
typedef struct {s8 visibleDelta;u8 sound,command,deaths;} RT_Type10PassEvent;
/* Source visibility -> pending entry -> perception -> one actor handler.
 * Effects retain script request/shared-command state; event reports this call's
 * visible-count delta, emitted sound/command and death completion. */
RT_EnemyResult RT_type10Update(RT_Enemy *enemy,const RT_WorldCamera *camera,
    const RT_Agent *target,u8 scenePhase,u8 sceneStep,RT_Type10Effects *effects,
    RT_Type10PassEvent *event);
/* Optional Home reinforcement delay: consume only ordinary visible updates,
 * before pending entry/perception/service. Source state80 activation proceeds
 * unchanged; offscreen updates do not consume the delay. NULL disables it. */
RT_EnemyResult RT_type10UpdateWithDelay(RT_Enemy *enemy,const RT_WorldCamera *camera,
    const RT_Agent *target,u8 scenePhase,u8 sceneStep,RT_Type10Effects *effects,
    RT_Type10PassEvent *event,u8 *admissionDelay);
#endif
