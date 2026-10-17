#ifndef RT_NATIVE_TYPE2C_H
#define RT_NATIVE_TYPE2C_H
#include "rt_enemies.h"
typedef struct {
    const RT_WorldCamera *camera;
    RT_EnemyProbe probe;
    void *probeContext;
    u8 phase; /* source VBlank phase, used only for voice selection */
    u8 *sound;
    u16 *award; /* packed BCD event */
} RT_Type2cEntryContext;
typedef enum {
    RT_TYPE2C_ENTRY_UNAVAILABLE,RT_TYPE2C_ENTRY_DECLINED,RT_TYPE2C_ENTRY_INSTALLED
} RT_Type2cEntryResult;
/* Original family44 ($2C) B88E and B8BF entry tables. No active-state update
 * or scene admission is implied. Missing context rejects without mutation. */
RT_Type2cEntryResult RT_type2cSelect(RT_Enemy *enemy,u8 selector,
    const RT_Type2cEntryContext *context);
RT_Type2cEntryResult RT_type2cStartup(RT_Enemy *enemy,
    const RT_Type2cEntryContext *context);
/* CC61 decision after horizontal, ground and object checks. SELECT means
 * an entry was installed; CONTINUE permits the walking timer to advance. */
RT_EnemyResult RT_type2cWalkDecision(RT_Enemy *enemy,s32 targetX,
    const RT_Type2cEntryContext *context);
/* D030/D08D turns38/70/74/48, D0C5 state54 and D105 aliases18/1C/78/7C/94/98, valid rows. */
RT_EnemyResult RT_type2cSimpleStep(RT_Enemy *enemy,const RT_Type2cEntryContext *context);
/* Original CCDC: first qualifying visible climb anchor in stable pool order.
 * selector is zero or AC; no source object records or addresses are retained. */
u8 RT_type2cObjectProbe(const RT_Enemy *enemy,const RT_StagePool *stage,
    const RT_WorldCamera *camera,u8 phase,u8 *selector);
typedef struct {
    RT_Type2cEntryContext entry;
    const RT_StagePool *stage;
    s32 targetX;
    u8 shadowXLow;
} RT_Type2cWalkContext;
RT_EnemyResult RT_type2cWalkStep(RT_Enemy *enemy,const RT_Type2cWalkContext *context);
typedef struct {
    RT_Type2cEntryContext entry;
    s16 accelerationY;
    u8 shadowXLow,shadowYLow;
} RT_Type2cMotionContext;
/* CE39/CEEC jumps28/34, CF6D falling/climbing2C and D208 death60/64/68/6C. */
RT_EnemyResult RT_type2cMotionStep(RT_Enemy *enemy,const RT_Type2cMotionContext *context);
/* All48 normal state groups. Scene must prepare visibility/perception first;
 * collision/ownership integration remains under validation. */
RT_EnemyResult RT_type2cStep(RT_Enemy *enemy,const RT_Type2cMotionContext *context,
    const RT_StagePool *stage,s32 targetX);
/* BAE2 response after horizontal sweep has aligned the native position. */
RT_EnemyResult RT_type2cWallResponse(RT_Enemy *enemy,u8 code,
    const RT_Type2cEntryContext *context);
/* B66E visibility and pending-entry service with native world coordinates. */
RT_EnemyResult RT_type2cPrepare(RT_Enemy *enemy,const RT_Type2cEntryContext *context,u8 *counted);
#endif
