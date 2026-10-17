#ifndef RT_NATIVE_TYPE28_H
#define RT_NATIVE_TYPE28_H
#include "rt_enemies.h"
typedef struct {
    s16 *accelerationY;
    u8 *command;
    u16 *award;
} RT_Type28EntryContext;
typedef enum {RT_TYPE28_ENTRY_UNAVAILABLE,RT_TYPE28_ENTRY_INSTALLED} RT_Type28EntryResult;
/* Original kind40 B88E/B8BF entry effects, with native downward-positive Y.
 * Entry48 requires a non48 current action: the original otherwise recurses. */
RT_Type28EntryResult RT_type28Select(RT_Enemy *enemy,u8 selector,const RT_Type28EntryContext *context);
RT_Type28EntryResult RT_type28Startup(RT_Enemy *enemy,const RT_Type28EntryContext *context);
/* C738 death64 and C3A6 four-row cycle and native selector30 transition. */
RT_EnemyResult RT_type28SimpleStep(RT_Enemy *enemy,const RT_Type28EntryContext *context);
typedef struct {
    RT_Type28EntryContext entry;
    const RT_WorldCamera *camera;
    RT_EnemyProbe probe;
    void *probeContext;
    u8 shadowXLow,shadowYLow;
} RT_Type28MotionContext;
/* C3F6 three-row state28 motion/retirement. */
RT_EnemyResult RT_type28ArcStep(RT_Enemy *enemy,const RT_Type28MotionContext *context);
/* C501 state30 sweep, acceleration reversal and contact entry28. */
RT_EnemyResult RT_type28SweepStep(RT_Enemy *enemy,const RT_Type28MotionContext *context);
/* C2E6 thirty common movement aliases, velocity-selected animation. */
RT_EnemyResult RT_type28WalkStep(RT_Enemy *enemy,const RT_Type28MotionContext *context);
/* C768 twelve-row child motion, landing, timed entry28. */
RT_EnemyResult RT_type28ChildStep(RT_Enemy *enemy,const RT_Type28MotionContext *context);
typedef struct {u8 variation,selector;s16 deltaX,deltaY,velocityX;} RT_Type28Child;
/* Callback must allocate in forward slot order and return0 when exhausted.
 * Coordinates are native right/down fixed-point deltas. */
typedef u8 (*RT_Type28Spawn)(const RT_Enemy *parent,const RT_Type28Child *child,void *context);
RT_EnemyResult RT_type28ParentStep(RT_Enemy *enemy,const RT_Type28MotionContext *context,
    u8 population,RT_Type28Spawn spawn,void *spawnContext);
/* B580/C663 native shared-slot allocation. Never wraps before parent.
 * Scene owns per-pass visibility/work counters; pool count increments here. */
u8 RT_type28SpawnChild(RT_EnemyPool *pool,u8 parent,const RT_WorldCamera *camera,
    const RT_Type28Child *child,u8 *slot);
/* Original B7BA active family table, all48 normal groups. */
RT_EnemyResult RT_type28Step(RT_Enemy *enemy,const RT_Type28MotionContext *context,
    u8 population,RT_Type28Spawn spawn,void *spawnContext);
RT_EnemyResult RT_type28Prepare(RT_Enemy *enemy,const RT_Type28MotionContext *context,u8 *counted);
#endif
