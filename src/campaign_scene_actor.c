#include "rt_campaign_scene_actor.h"
static u8 family(const RT_SceneActor *actor){return actor->kind==0x13?1:0;}
static u8 valid(const RT_SceneActor *actor){return actor &&
 (actor->kind==7 || actor->kind==11 || actor->kind==0x13);}
static void row(RT_SceneActor *actor,const RT_SceneAnimationRow *value)
{
 actor->timer=value->duration;actor->animationFlags=value->flags;actor->pose=value->pose;
}
u8 RT_sceneActorEnter(RT_SceneActor *actor,u8 action,u8 direction,u8 attractOpening,u8 *effect)
{
 if(!valid(actor) || action>=RT_SCENE_ACTION_COUNT || direction>3 || attractOpening>1)return 0;
 const RT_SceneAnimation *animation=&rtSceneAnimations[family(actor)][action];
 if(!animation->count)return 0;
 if(!direction)direction=actor->direction;
 actor->requestedAction=action;actor->requestedDirection=direction;
 if(family(actor)){
  if(action==RT_SCENE_IDLE && attractOpening && effect)*effect=8;
  if(action==RT_SCENE_WALK || action==RT_SCENE_RUN){
   s16 speed=action==RT_SCENE_WALK?16:48;
   actor->velocityX=direction&2?-speed:speed;
  }
 }else if(action==RT_SCENE_WALK || action==RT_SCENE_RUN){
  s16 speed=20+((actor->variant&0x30)>>2);
  actor->velocityY=0;actor->delay=direction&2?30:20;
  actor->velocityX=direction&2?-speed:speed;
 }else if(action==RT_SCENE_COLLISION08 || action==RT_SCENE_COLLISION3C){
  if(actor->variant&3)actor->delay=0;
  else{actor->delay=2;actor->contact|=4;}
 }else if(action==RT_SCENE_COLLISION24 || action==RT_SCENE_COLLISION28){
  actor->velocityY=action==RT_SCENE_COLLISION24?-112:-160;
  if(action==RT_SCENE_COLLISION24)actor->contact&=0x7e;
 }
 actor->action=action;actor->direction=direction;actor->row=0;row(actor,animation->rows);return 1;
}
u8 RT_sceneActorTimed(RT_SceneActor *actor)
{
 if(!valid(actor) || actor->action>=RT_SCENE_ACTION_COUNT)return 0;
 const RT_SceneAnimation *animation=&rtSceneAnimations[family(actor)][actor->action];
 if(!animation->timed)return 0;
 u8 ignoreRow=!family(actor) && (actor->action==RT_SCENE_IDLE || actor->action==RT_SCENE_HOLD);
 u8 anyTerminal=!family(actor) && (actor->action==RT_SCENE_ANIMCC || actor->action==RT_SCENE_ANIMD0);
 if(!ignoreRow && !anyTerminal && actor->row>=animation->count)return 0;
 if(--actor->timer)return 1;
 if(ignoreRow || actor->row>=animation->count-1){
  actor->request++;
  if(!family(actor) && actor->action==RT_SCENE_STEP){
   actor->x+=actor->direction&2?-80:80;actor->request++;
  }
  return 1;
 }
 if(actor->action==RT_SCENE_TURN && actor->row==2){
  actor->direction^=3;actor->requestedDirection=actor->direction;
  actor->requestedAction=actor->action;
 }
 row(actor,animation->rows+(++actor->row));return 1;
}
u8 RT_sceneActorScript(RT_SceneActor *actor,u8 attractOpening,u8 *effect)
{
 if(!valid(actor) || actor->script>=32 || attractOpening>1)return 0;
 if(actor->removed || !actor->visible || !actor->request)return 1;
 const RT_SceneCommand *command=&rtSceneCommands[actor->script][(actor->scriptIndex+1)&15];
 if(command->delay)actor->timer=command->delay;
 else if(!RT_sceneActorEnter(actor,command->action,command->direction,attractOpening,effect))return 0;
 actor->scriptIndex=command->index;actor->request=0;return 1;
}

u8 RT_sceneActorVisibility(RT_SceneActor *actor,u8 attractOpening,u8 *effect)
{
 if(!valid(actor) || actor->removed || attractOpening>1)return 0;
 if(actor->x< -768 || actor->x>=5376 || actor->y<=0 || actor->y>5120){
  actor->removed=1;actor->visible=0;return 2;
 }
 if((actor->action!=actor->requestedAction || actor->direction!=actor->requestedDirection) &&
    !RT_sceneActorEnter(actor,actor->requestedAction,actor->requestedDirection,attractOpening,effect))return 0;
 actor->visible=1;return 1;
}
