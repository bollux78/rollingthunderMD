#include "rt_campaign_scene_actor.h"
u8 RT_sceneCastStep(RT_SceneCast *cast,RT_SceneWalkProbe probe,void *context)
{
 if(!cast || !probe || cast->count>RT_SCENE_CAST_CAPACITY)return 255;
 u8 population=0;
 for(u8 i=0;i<RT_SCENE_CAST_CAPACITY;i++)if(!cast->actors[i].removed){
  u8 kind=cast->actors[i].kind;
  if(kind!=7 && kind!=11 && kind!=0x13)return 255;
  population++;
 }
 if(population!=cast->count)return 255;
 u8 visible=0;
 for(u8 i=0;i<RT_SCENE_CAST_CAPACITY;i++){
  RT_SceneActor *actor=cast->actors+i;if(actor->removed)continue;
  u8 result=RT_sceneActorVisibility(actor,0,0);
  if(!result)return 255;
  if(result==2){cast->count--;continue;}
  if(actor->action==RT_SCENE_WALK || actor->action==RT_SCENE_RUN){
   u8 action;
   RT_SceneWalkResult movement=RT_sceneActorWalk(actor,probe,context,&action);
   if(movement==RT_SCENE_WALK_INVALID)return 255;
   if(movement==RT_SCENE_WALK_REMOVED){cast->count--;continue;}
   if(movement==RT_SCENE_WALK_SELECT && !RT_sceneActorEnter(actor,action,0,0,0))return 255;
  }else if(actor->kind!=0x13 && (actor->action==RT_SCENE_COLLISION08 ||
    actor->action==RT_SCENE_COLLISION0C || actor->action==RT_SCENE_COLLISION24 ||
    actor->action==RT_SCENE_COLLISION28 || actor->action==RT_SCENE_COLLISION3C)){
   /* Scene dispatch selects RTS for these collision outcomes. */
  }else if(!RT_sceneActorTimed(actor))return 255;
  if(!RT_sceneActorScript(actor,0,0))return 255;
  visible++;
 }
 return visible;
}
