#ifndef RT_NATIVE_TYPE20_H
#define RT_NATIVE_TYPE20_H
#include "rt_enemies.h"
/* Original-ROM stationary Type32 ($20) handlers: states38/48/54 and
 * death60/64/68/6C. Advance when the decremented byte timer reaches zero.
 * Wider family still quarantined pending full scene integration.
 * REMOVED marks the slot free; the scene owns aggregate pool bookkeeping. */
RT_EnemyResult RT_type20SimpleStep(RT_Enemy *enemy);
typedef struct {
    const RT_WorldCamera *camera;
    RT_EnemyProbe probe;
    void *probeContext;
    s16 accelerationY;
    u8 shadowXLow,shadowYLow;
    s32 targetX;
    u8 *sound;
    u16 *award; /* packed BCD event amount, consumed by the scene */
} RT_Type20Motion;
/* Moving18 aliases and28/2C/30/34, after source entry has initialized motion.
 * Missing collision data is surfaced as UNAVAILABLE after source partial
 * updates; the old source scene silently stopped this actor for that pass. */
RT_EnemyResult RT_type20MotionStep(RT_Enemy *enemy,const RT_Type20Motion *context);
/* Source startup80, idle00 and allBE9B walking aliases. */
RT_EnemyResult RT_type20WalkStep(RT_Enemy *enemy,const RT_Type20Motion *context);
/* Family dispatch. Original handler control flow is verified separately;
 * full scene ownership, perception and terrain integration remain. */
RT_EnemyResult RT_type20Step(RT_Enemy *enemy,const RT_Type20Motion *context);
/* Original ROM BE6A entry, including speed/delay absent from legacy C.
 * Accepts only selectors whose family entry table targets BE6A. Zero direction
 * inherits current direction as B88E does. Rejection is mutation-free. */
u8 RT_type20EnterWalk(RT_Enemy *enemy,u8 selector);
typedef enum {
    RT_TYPE20_ENTRY_UNAVAILABLE,RT_TYPE20_ENTRY_DECLINED,RT_TYPE20_ENTRY_INSTALLED
} RT_Type20EntryResult;
/* Original B88E entry selection except death60..6C and selector74. Missing
 * terrain/event output and unsupported requests return UNAVAILABLE unchanged.
 * Terrain refusal records the resolved requested selector and byte12 bit20,
 * preserving the active animation. Hop entries require a sound event output. */
RT_Type20EntryResult RT_type20Select(RT_Enemy *enemy,u8 selector,
    RT_EnemyProbe probe,void *context,u8 *sound);
/* BEC9 walking decision: SELECT installed an entry; CONTINUE permits the
 * walking animation timer to run; UNAVAILABLE means required context missing. */
RT_EnemyResult RT_type20WalkDecision(RT_Enemy *enemy,s32 targetX,
    RT_EnemyProbe probe,void *context,u8 *sound);
/* State80 B8BF entry dispatcher: no direction inheritance. */
RT_Type20EntryResult RT_type20Startup(RT_Enemy *enemy,
    RT_EnemyProbe probe,void *context,u8 *sound);
/* B66E camera-local visibility/startup gate over native world coordinates.
 * counted is source visible-count contribution, not state.hidden for startup.
 * Call before perception/state service. World positions are never camera-shifted. */
RT_EnemyResult RT_type20Prepare(RT_Enemy *enemy,const RT_Type20Motion *context,u8 *counted);
/* Full original entry table, including height-dependent74 and death awards.
 * Death entries require sound+award outputs;74 requires the current camera. */
RT_Type20EntryResult RT_type20SelectInContext(RT_Enemy *enemy,u8 selector,
    const RT_Type20Motion *context);
RT_Type20EntryResult RT_type20StartupInContext(RT_Enemy *enemy,
    const RT_Type20Motion *context);
#endif
