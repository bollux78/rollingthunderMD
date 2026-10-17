#include "rt_campaign_final.h"
u8 RT_campaignFinalBegin(RT_CampaignFinal *f,u16 timer,u8 player,u32 refresh)
{
 if(!f || player>1 || timer>9999)return 0;
 *f=(RT_CampaignFinal){.refresh=refresh,.phase=RT_FINAL_NATIVE_BONUS,.player=player};
 return RT_campaignBonusBegin(&f->bonus,timer,1,0);
}
RT_CampaignFinalResult RT_campaignFinalStep(RT_CampaignFinal *f,u32 refresh,
 RT_CampaignScore *score,u8 setting,u8 *reserves,u8 presented,
 u8 statusAvailable,u8 status,RT_CampaignFinalEvents *events)
{
 if(!f || !score || !reserves || !events || f->player>1 || setting>1 ||
    f->phase<RT_FINAL_NATIVE_BONUS || f->phase>RT_FINAL_NATIVE_ENTRY)
  return RT_FINAL_NATIVE_INVALID;
 *events=(RT_CampaignFinalEvents){0};
 /* The physical IRQ counter continues after bonus.done. Phase4 resets it;
  * phase5 tests the same counter's low7bits (AD99), not a new fixed delay. */
 f->bonus.clock+=(u8)(refresh-f->refresh);f->refresh=refresh;
 switch(f->phase){
 case RT_FINAL_NATIVE_BONUS:{
  RT_BonusResult r=RT_campaignBonusStep(&f->bonus,score,f->player,setting,reserves,&events->bonusEvents);
  if(r==RT_BONUS_INVALID)return RT_FINAL_NATIVE_INVALID;
  if(r!=RT_BONUS_COMPLETE)return RT_FINAL_NATIVE_RUNNING;
  f->phase=RT_FINAL_NATIVE_WAIT;return RT_FINAL_NATIVE_ADVANCED;
 }
 case RT_FINAL_NATIVE_WAIT:
  if(f->bonus.clock&127)return RT_FINAL_NATIVE_RUNNING;
  events->actions=RT_FINAL_NATIVE_SAVE_PLAYER;f->phase=RT_FINAL_NATIVE_LOAD;
  return RT_FINAL_NATIVE_ADVANCED;
 case RT_FINAL_NATIVE_LOAD:
  events->actions=RT_FINAL_NATIVE_LOAD_SCENE|RT_FINAL_NATIVE_CREDITS;f->phase=RT_FINAL_NATIVE_SCENE;
  return RT_FINAL_NATIVE_ADVANCED;
 case RT_FINAL_NATIVE_SCENE:
  if(!presented)return RT_FINAL_NATIVE_RUNNING;
  events->actions=RT_FINAL_NATIVE_CREDITS;
  if(!statusAvailable || status)return RT_FINAL_NATIVE_RUNNING;
  f->phase=RT_FINAL_NATIVE_CLEAR;return RT_FINAL_NATIVE_ADVANCED;
 case RT_FINAL_NATIVE_CLEAR:
  events->actions=RT_FINAL_NATIVE_CLEAR_SCENE|RT_FINAL_NATIVE_CREDITS;f->phase=RT_FINAL_NATIVE_ENTRY;
  return RT_FINAL_NATIVE_HIGH_SCORE;
 default:return RT_FINAL_NATIVE_HIGH_SCORE;
 }
}

static void clearInventory(RT_Agent *agent)
{
 agent->ammoPrimary=agent->ammoAlternate=0;agent->weaponMode=0;
}
u8 RT_campaignFinalClearInventory(RT_Agent *agent,RT_Agent *restartAgent,
 u8 *arcadeReserves)
{
 if(!agent || !restartAgent || (arcadeReserves && *arcadeReserves>99))return 0;
 clearInventory(agent);clearInventory(restartAgent);
 if(arcadeReserves)*arcadeReserves=0;
 return 1;
}
