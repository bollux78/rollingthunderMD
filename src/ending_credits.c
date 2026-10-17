#include "rt_ending_credits.h"
void RT_endingCreditsReset(RT_EndingCredits *s)
{*s=(RT_EndingCredits){0};}
void RT_endingCreditsPrepare(RT_EndingCredits *s,u16 value)
{
 if(s->enabled && s->value==value)return;
 s->enabled=1;s->value=value;s->pendingReady=1;
 for(u8 i=5;i;i--){s->pending[i-1]=value%10;value/=10;}
 for(u8 i=0;i<4 && !s->pending[i];i++)s->pending[i]=10;
}
u8 RT_endingCreditsCommit(RT_EndingCredits *s,RT_EndingCreditWrite write)
{
 if(!s->pendingReady || !write)return 0;
 u8 writes=0;
 if(!s->labelPublished)for(u8 i=0;i<7;i++)for(u8 p=0;p<2;p++){
  write(p,i,rtEndingCreditLabel[i][p]);writes++;
 }
 for(u8 i=0;i<5;i++)if(!s->labelPublished || s->pending[i]!=s->published[i]){
  for(u8 p=0;p<2;p++){write(p,i+7,rtEndingCreditTiles[i][s->pending[i]][p]);writes++;}
  s->published[i]=s->pending[i];
 }
 s->labelPublished=1;s->pendingReady=0;return writes;
}
