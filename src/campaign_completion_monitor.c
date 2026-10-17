#include "rt_stage_bank.h"
#include "rt_rom_layout.h"
#include "rt_campaign_completion_monitor.h"
void RT_completionMonitorFormat(u32 high,u32 player0,u32 player1,u16 timer,u8 digits[24])
{
 const u32 scores[3]={player0,high,player1};
 for(u8 p=0;p<3;p++){
  static const u32 places[6]={100000,10000,1000,100,10,1};
  u32 value=scores[p];u8 *d=digits+p*7;
  /* Validated campaign scores are six decimal digits. At most nine subtracts
   * per place; no 32-bit divide helper (toolchain libgcc needs a later CPU). */
  for(u8 i=0;i<6;i++){d[i]=0;while(value>=places[i]){value-=places[i];d[i]++;}}
  for(u8 i=0;i<5 && !d[i];i++)d[i]=10;
  d[6]=0;
 }
 timer%=1000;digits[21]=timer/100;digits[22]=(timer/10)%10;digits[23]=timer%10;
 if(!digits[21]){digits[21]=10;if(!digits[22])digits[22]=10;}
}
#ifdef __m68k__
static u8 previous[24],previousGroup,pending[24],pendingGroup,pendingReady;
void RT_completionMonitorPrepare(u32 high,u32 player0,u32 player1,u16 timer)
{
 RT_completionMonitorFormat(high,player0,player1,timer,pending);
 pendingGroup=timer>=60?1:timer>=30?2:0;pendingReady=1;
}
void RT_completionMonitorCommit(void)
{
 if(!pendingReady)return;
 const u8 *digits=pending;u8 group=pendingGroup;
 for(u8 i=0;i<24;i++){
  if(digits[i]==previous[i] && (i<21 || group==previousGroup))continue;
  u16 cell=i<7?38+i:i<14?50+i-7:i<21?63+i-14:27*36+32+i-21;
  const u16 *pair=rtCompletionMonitorDigits[i<21?i:21+group*3+i-21][digits[i]];
  VDP_setTileMapXY(BG_A,pair[0],cell%36+2,cell/36);
  VDP_setTileMapXY(BG_B,pair[1],cell%36+2,cell/36);
  previous[i]=digits[i];
 }
 previousGroup=group;pendingReady=0;
}
void RT_completionMonitorUpdate(u32 high,u32 player0,u32 player1,u16 timer)
{RT_completionMonitorPrepare(high,player0,player1,timer);RT_completionMonitorCommit();}
#include "rt_stage_raster.h"
#include "rt_stage_transition_raster.h"
static u8 show(u8 labelIndex)
{
 RT_stopStageRaster();RT_stopStageTransitionRaster();VDP_setEnable(FALSE);
 DMA_flushQueue();
 if(!RT_stageBankSelect(RT_COMPLETION_PAGE))return 0;
 if(!RT_sceneBackgroundUpload(&rtCompletionMonitorBackground))return 0;
 VDP_loadTileData(rtCompletionMonitorHud.patterns,1,rtCompletionMonitorHud.count,DMA);
 for(u8 i=0;i<24;i++)previous[i]=255;
 previousGroup=255;pendingReady=0;
 const RT_CompletionMonitorLabel *label=rtCompletionMonitorLabels+labelIndex;
 for(u16 i=0;i<label->count;i++){
  const RT_CompletionMonitorPatch *p=label->patches+i;
  VDP_setTileMapXY(BG_A,p->planeA,p->cell%36+2,p->cell/36);
  VDP_setTileMapXY(BG_B,p->planeB,p->cell%36+2,p->cell/36);
 }
 /* Display-disable forces the VDP blank status bit. Wait on real VInt. */
 u32 refresh=vtimer;while(vtimer==refresh);
 VDP_setEnable(TRUE);return 1;
}
u8 RT_completionMonitorShow(u8 story,u8 area)
{
 if(story>1 || area>4 || (story==1 && area==4))return 0;
 return show(story*5+area);
}
u8 RT_completionMonitorShowFinal(u8 player)
{if(player>1)return 0;return show(9+player);}
#endif
