#include "rt_campaign_scene_actor.h"
u8 RT_sceneCastInitialize(RT_SceneCast *cast,u8 selector)
{
 if(!cast || selector>10)return 0;
 const RT_SceneCastDefinition *definition=rtSceneCasts+selector;
 if(definition->count>RT_SCENE_CAST_CAPACITY)return 0;
 cast->count=definition->count;
 for(u8 slot=0;slot<RT_SCENE_CAST_CAPACITY;slot++){
  RT_SceneActor *actor=cast->actors+slot;*actor=(RT_SceneActor){0};
  if(slot>=definition->count){actor->removed=1;continue;}
  const RT_SceneSpawn *spawn=definition->actors+slot;
  actor->x=(s32)spawn->x*16;actor->y=(s32)spawn->y*16;
  actor->kind=spawn->kind;actor->action=RT_SCENE_ACTION_COUNT;actor->pose=65535;
  actor->requestedAction=spawn->action;actor->requestedDirection=spawn->direction;
  actor->script=spawn->script;actor->variant=spawn->variant;
 }
 return 1;
}
