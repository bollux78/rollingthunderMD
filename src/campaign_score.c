#include "rt_campaign_score.h"
u8 RT_campaignTimerBegin(RT_CampaignTimer *t,u8 setting)
{
    if(!t || setting>1)return 0;
    *t=(RT_CampaignTimer){setting?150:120,0};return 1;
}
RT_TimerResult RT_campaignTimerStep(RT_CampaignTimer *t,u8 exiting)
{
    if(!t || t->remaining>9999 || t->divider>63)return RT_TIMER_INVALID;
    if(!t->remaining || exiting)return RT_TIMER_IDLE;
    t->divider=(t->divider+1)&63;
    if(t->divider)return RT_TIMER_IDLE;
    return --t->remaining?RT_TIMER_CHANGED:RT_TIMER_EXPIRED;
}
RT_ScoreResult RT_campaignAward(RT_CampaignScore *s,u8 player,u16 award,
    u16 secondary,u8 setting,u8 *reserves)
{
    static const u16 thresholds[2][2]={{7000,20000},{10000,30000}};
    if(!s || !reserves || player>1 || setting>1 || award>9999 || secondary>9999 ||
       *reserves>99 || s->players[player]>999999 || s->high>999999)return RT_SCORE_INVALID;
    u16 amount=award+secondary;
    if(amount>=10000)amount-=10000;
    u32 total=s->players[player]+amount;
    if(total>=1000000)total-=1000000;
    s->players[player]=total;
    if(total>s->high)s->high=total;
    if(s->bonusIndex[player]>=2 || total<thresholds[setting][s->bonusIndex[player]])return RT_SCORE_ADDED;
    *reserves=*reserves==99?0:*reserves+1;s->bonusIndex[player]++;
    return RT_SCORE_EXTRA_LIFE;
}
u8 RT_campaignBonusBegin(RT_CampaignBonus *b,u16 timer,u8 ending,u8 clock)
{
    if(!b || timer>9999 || ending>1)return 0;
    *b=(RT_CampaignBonus){timer,clock,ending,0};return 1;
}
void RT_campaignBonusRefresh(RT_CampaignBonus *b,u32 elapsed)
{
    if(b && !b->done)b->clock+=(u8)elapsed;
}
RT_BonusResult RT_campaignBonusStep(RT_CampaignBonus *b,RT_CampaignScore *s,
    u8 player,u8 setting,u8 *reserves,u8 *events)
{
    if(!b || !s || !reserves || !events || b->remaining>9999 || b->ending>1 ||
       player>1 || setting>1 || *reserves>99 || s->players[player]>999999 || s->high>999999)
        return RT_BONUS_INVALID;
    *events=0;if(b->done)return RT_BONUS_COMPLETE;
    u8 awarded=0;
    if(b->remaining){
        RT_ScoreResult result=RT_campaignAward(s,player,12,0,setting,reserves);
        *events=RT_BONUS_SOUND|RT_BONUS_SCORE_HUD|RT_BONUS_TIMER_HUD;
        if(result==RT_SCORE_EXTRA_LIFE)*events|=RT_BONUS_EXTRA_LIFE_SOUND;
        b->remaining--;awarded=1;
        if(b->remaining)return RT_BONUS_AWARDED;
        if(!b->ending)b->clock=0;
    }
    if(b->ending?b->clock<192:(b->clock&63)!=0)return awarded?RT_BONUS_AWARDED:RT_BONUS_WAIT;
    if(b->ending)b->clock=0;
    b->done=1;return RT_BONUS_COMPLETE;
}
