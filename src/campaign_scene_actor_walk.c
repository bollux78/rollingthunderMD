#include "rt_campaign_scene_actor.h"
RT_SceneWalkResult RT_sceneActorWalk(RT_SceneActor *actor,
 RT_SceneWalkProbe probe,void *context,u8 *action)
{
 static const u8 codes[8]={0x19,0x22,0x24,0x25,0x26,0x27,0x2c,0x2d};
 static const u8 responses[2][8]={
  {RT_SCENE_TURN,RT_SCENE_TURN,RT_SCENE_COLLISION24,RT_SCENE_COLLISION08,
   RT_SCENE_COLLISION24,RT_SCENE_COLLISION28,RT_SCENE_COLLISION24,RT_SCENE_COLLISION3C},
  {RT_SCENE_TURN,RT_SCENE_TURN,RT_SCENE_COLLISION0C,RT_SCENE_COLLISION08,
   RT_SCENE_COLLISION0C,RT_SCENE_COLLISION28,RT_SCENE_COLLISION0C,RT_SCENE_COLLISION3C}};
 if(!actor || !probe || !action || actor->removed || !actor->visible ||
    (actor->kind!=7 && actor->kind!=11 && actor->kind!=0x13) ||
    (actor->action!=RT_SCENE_WALK && actor->action!=RT_SCENE_RUN))return RT_SCENE_WALK_INVALID;
 u8 family=actor->kind==0x13,last=family?7:5;
 if(actor->row>last)return RT_SCENE_WALK_INVALID;
 *action=255;
 s32 initial=actor->x,target=initial+actor->velocityX;
 u8 left=actor->velocityX<0;
 if((!left && target>=5120) || (left && target< -512)){
  actor->removed=1;actor->visible=0;return RT_SCENE_WALK_REMOVED;
 }
 if(!family){
  u16 phase=(u16)initial&0x70;
  s32 position=initial+(left?-(s16)phase:(s16)((0-phase)&0x70));
  while(left?position>=target:position<=target){
   actor->x=position;u8 code,flags;
   if(!probe(actor,left?-2:1,&code,&flags,context)){
    actor->x=initial;return RT_SCENE_WALK_INVALID;
   }
   if(flags&1){
    actor->x=position-((u16)position&15);actor->contact|=1;
    *action=RT_SCENE_COLLISION24;
    for(u8 i=0;i<8;i++)if(codes[i]==code){*action=responses[actor->variant>>7][i];break;}
    return RT_SCENE_WALK_SELECT;
   }
   position+=left?-128:128;
  }
 }
 actor->x=target;
 if(--actor->timer)return RT_SCENE_WALK_CONTINUE;
 if(actor->row==last){actor->request++;return RT_SCENE_WALK_CONTINUE;}
 const RT_SceneAnimationRow *row=rtSceneWalkingRows[family]+(++actor->row);
 actor->timer=row->duration;actor->animationFlags=row->flags;actor->pose=row->pose;
 return RT_SCENE_WALK_CONTINUE;
}
