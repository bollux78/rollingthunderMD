#ifndef RT_NATIVE_TYPE24_H
#define RT_NATIVE_TYPE24_H
#include "rt_enemies.h"
typedef struct {
    s16 *accelerationY; /* owned mutable per-actor acceleration, downward-positive */
    u8 *command,*sound;
    u16 *award;
} RT_Type24EntryContext;
typedef enum {RT_TYPE24_ENTRY_UNAVAILABLE,RT_TYPE24_ENTRY_INSTALLED} RT_Type24EntryResult;
/* Original kind36 B88E/B8BF entries. Mutable acceleration is not written into
 * shared immutable constructor templates. Caller consumes command/audio/award. */
RT_Type24EntryResult RT_type24Select(RT_Enemy *enemy,u8 selector,const RT_Type24EntryContext *context);
RT_Type24EntryResult RT_type24Startup(RT_Enemy *enemy,const RT_Type24EntryContext *context);
/* C7E1 idle00/98 and C871 death60/64/68/6C, normal rows0..3. */
RT_EnemyResult RT_type24SimpleStep(RT_Enemy *enemy,const RT_Type24EntryContext *context);
typedef struct {
    RT_Type24EntryContext entry;
    const RT_WorldCamera *camera;
    RT_EnemyProbe probe;
    void *probeContext;
    u8 shadowXLow,shadowYLow;
    s32 targetX,targetY;
} RT_Type24MotionContext;
/* CA3A turns48/54, including CA51 acceleration reversal. */
RT_EnemyResult RT_type24TurnStep(RT_Enemy *enemy,const RT_Type24MotionContext *context);
/* Shared CA51 vertical sweep and acceleration reversal; context validated by caller. */
RT_EnemyResult RT_type24Oscillate(RT_Enemy *enemy,const RT_Type24MotionContext *context);
RT_EnemyResult RT_type24WalkStep(RT_Enemy *enemy,const RT_Type24MotionContext *context);
/* C8A0 six-row launch sequence20/24/78/7C. */
RT_EnemyResult RT_type24LaunchStep(RT_Enemy *enemy,const RT_Type24MotionContext *context);
/* CAE7, including CB5A dominant-axis pursuit and CBE9 ground response. */
RT_EnemyResult RT_type24ChaseStep(RT_Enemy *enemy,const RT_Type24MotionContext *context);
RT_EnemyResult RT_type24Prepare(RT_Enemy *enemy,const RT_Type24MotionContext *context,u8 *counted);
/* All48 normal groups. Scene owns preparation, acceleration storage and events. */
RT_EnemyResult RT_type24Step(RT_Enemy *enemy,const RT_Type24MotionContext *context);
#endif
