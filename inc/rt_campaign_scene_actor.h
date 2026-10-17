#ifndef RT_NATIVE_CAMPAIGN_SCENE_ACTOR_H
#define RT_NATIVE_CAMPAIGN_SCENE_ACTOR_H
#include "rt_world.h"
/* Scene animation state, independent of gameplay AI and source RAM records.
 * Position/velocity use native12.4 world axes, Y down. Pose is an offline
 * asset index, never an arcade address. Directions retain the authored0..3
 * choices and the command rule that zero inherits the current direction. */
typedef enum {
 RT_SCENE_IDLE,RT_SCENE_WALK,RT_SCENE_RUN,RT_SCENE_COLLISION08,
 RT_SCENE_COLLISION0C,RT_SCENE_COLLISION24,RT_SCENE_COLLISION28,
 RT_SCENE_COLLISION3C,RT_SCENE_TURN,RT_SCENE_RANGED,RT_SCENE_RELEASE,
 RT_SCENE_POSE,RT_SCENE_ANIM94,RT_SCENE_ANIMB0,RT_SCENE_STEP,
 RT_SCENE_ANIMB8,RT_SCENE_HOLD,RT_SCENE_ANIMC0,RT_SCENE_ANIMC4,
 RT_SCENE_ANIMC8,RT_SCENE_ANIMCC,RT_SCENE_ANIMD0,RT_SCENE_ACTION_COUNT
} RT_SceneAction;
typedef struct {
 s32 x,y;
 s16 velocityX,velocityY;
 u16 pose;
 u8 kind,action,direction,requestedAction,requestedDirection;
 u8 request,scriptIndex,script,variant,row,timer,animationFlags;
 u8 delay,contact,visible,removed;
} RT_SceneActor;
typedef struct {u8 duration,flags;u16 pose;} RT_SceneAnimationRow;
typedef struct {const RT_SceneAnimationRow *rows;u8 count,timed;} RT_SceneAnimation;
typedef struct {u8 action,direction,delay,index;} RT_SceneCommand;
extern const RT_SceneAnimation rtSceneAnimations[2][RT_SCENE_ACTION_COUNT];
extern const RT_SceneCommand rtSceneCommands[32][16];
/* effect is optional; attractOpening selects CD75's scene effect8 branch. */
u8 RT_sceneActorEnter(RT_SceneActor *actor,u8 action,u8 direction,u8 attractOpening,u8 *effect);
/* Timed scene animation only. Walking/collision and visibility have distinct
 * owners and must run before the script continuation where applicable. */
u8 RT_sceneActorTimed(RT_SceneActor *actor);
u8 RT_sceneActorScript(RT_SceneActor *actor,u8 attractOpening,u8 *effect);
/* Returns0 invalid,1 active,2 newly removed. Pool owner adjusts population
 * and visible counts. Applies a pending entry after the source scene bounds. */
u8 RT_sceneActorVisibility(RT_SceneActor *actor,u8 attractOpening,u8 *effect);
extern const RT_SceneAnimationRow rtSceneWalkingRows[2][8];
typedef u8 (*RT_SceneWalkProbe)(const RT_SceneActor *actor,s16 cellX,
 u8 *code,u8 *flags,void *context);
typedef enum {RT_SCENE_WALK_INVALID,RT_SCENE_WALK_CONTINUE,
 RT_SCENE_WALK_SELECT,RT_SCENE_WALK_REMOVED} RT_SceneWalkResult;
/* Fixed transition origin X=0. Probe observes each ordered8px crossing.
 * SELECT returns a native action; caller enters it before script continuation.
 * Probe failure restores X and leaves animation/contact unchanged. */
RT_SceneWalkResult RT_sceneActorWalk(RT_SceneActor *actor,
 RT_SceneWalkProbe probe,void *context,u8 *action);
#define RT_SCENE_CAST_CAPACITY 5
typedef struct {s16 x,y;u8 kind,action,direction,script,variant;} RT_SceneSpawn;
typedef struct {RT_SceneSpawn actors[RT_SCENE_CAST_CAPACITY];u8 count;} RT_SceneCastDefinition;
typedef struct {RT_SceneActor actors[RT_SCENE_CAST_CAPACITY];u8 count;} RT_SceneCast;
extern const RT_SceneCastDefinition rtSceneCasts[11];
/* After the scene clears its CPU2 actors, allocate the complete authored cast
 * in column/record order. Five is the verified maximum, not an enemy cap.
 * Entries remain pending until the first visibility service, as in B1E0. */
u8 RT_sceneCastInitialize(RT_SceneCast *cast,u8 selector);
/* Ordered visibility, movement/timed animation, then script continuation.
 * Returns visible actors or255 on failure. Earlier successful actors may have
 * advanced on failure; caller must stop, not retry the partially serviced pass. */
u8 RT_sceneCastStep(RT_SceneCast *cast,RT_SceneWalkProbe probe,void *context);
#endif
