#include "rt_campaign_ending.h"
u8 RT_campaignEndingBegin(RT_CampaignEnding *e,RT_CompletionScene *scene,
 RT_CampaignScore *score,RT_Agent *agent,RT_Agent *restart,u8 *reserves,
 u8 player,u16 timer)
{
 if(!e || e->active || !scene || !score || !agent || !restart || player>1 ||
    timer>9999 || (reserves && *reserves>99))return 0;
 *e=(RT_CampaignEnding){.scene=scene,.score=score,.agent=agent,.restart=restart,
     .arcadeReserves=reserves};
 if(!RT_completionMonitorShowFinal(player) ||
    !RT_campaignFinalBegin(&e->flow,timer,player,vtimer)){e->fault=1;return 0;}
 RT_completionMonitorUpdate(score->high,score->players[0],score->players[1],timer);
 e->flow.refresh=vtimer;e->active=1;return 1;
}
RT_EndingResult RT_campaignEndingStep(RT_CampaignEnding *e,u32 refresh,
 u8 available,u8 status,void (*beginFrame)(u8),void (*beginLoad)(void),void (*endIO)(void))
{
 if(!e || !e->active || e->fault || !beginFrame || !beginLoad || !endIO)return RT_ENDING_FAULT;
 e->events=(RT_CampaignFinalEvents){0};e->earnedReserves=0;
 if(e->entryReady)return RT_ENDING_ENTRY_READY;
 u8 *reserves=e->arcadeReserves?e->arcadeReserves:&e->earnedReserves;
 RT_CampaignFinalResult result=RT_campaignFinalStep(&e->flow,refresh,e->score,0,
     reserves,e->sceneLoaded && e->scene->frames!=0,available,status,&e->events);
 if(result==RT_FINAL_NATIVE_INVALID){e->fault=2;return RT_ENDING_FAULT;}
 if(e->events.actions&RT_FINAL_NATIVE_SAVE_PLAYER){
  if(!RT_campaignFinalClearInventory(e->agent,e->restart,e->arcadeReserves)){
   e->fault=3;return RT_ENDING_FAULT;
  }
  return RT_ENDING_RUNNING;
 }
 if(e->events.actions&RT_FINAL_NATIVE_LOAD_SCENE){
  /* Mark ownership before Begin so failure cleanup still releases callbacks. */
  e->sceneLoaded=1;
  if(!RT_completionEndingBeginIO(e->scene,10,beginLoad,endIO)){
   e->fault=4;return RT_ENDING_FAULT;
  }
  return RT_ENDING_RUNNING;
 }
 if(e->events.actions&RT_FINAL_NATIVE_CLEAR_SCENE){
  RT_completionSceneEnd(e->scene);e->sceneLoaded=0;e->entryReady=1;
  return RT_ENDING_ENTRY_READY;
 }
 if(e->flow.phase==RT_FINAL_NATIVE_CLEAR)return RT_ENDING_RUNNING;
 if(e->sceneLoaded){
  if(RT_completionSceneStepIO(e->scene,beginFrame,endIO)!=1){
   e->fault=5;return RT_ENDING_FAULT;
  }
 }else{
  RT_completionMonitorPrepare(e->score->high,e->score->players[0],e->score->players[1],e->flow.bonus.remaining);
  beginFrame(0);SYS_doVBlankProcessEx(IMMEDIATELY);
  RT_completionMonitorCommit();endIO();
 }
 return RT_ENDING_RUNNING;
}
void RT_campaignEndingEnd(RT_CampaignEnding *e)
{
 if(!e)return;
 if(e->sceneLoaded)RT_completionSceneEnd(e->scene);
 e->sceneLoaded=0;e->active=0;
}
