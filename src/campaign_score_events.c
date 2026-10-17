#include "rt_campaign_score.h"
u8 RT_campaignScoreEnemies(RT_CampaignScore *score,const RT_EnemyPassResult *pass,
    u8 players,u8 setting,u8 *reserves)
{
    if(!score || !pass || !reserves || !players || players>2 || setting>1 ||
       *reserves>99 || score->high>999999 || score->players[0]>999999 ||
       score->players[1]>999999 || pass->eventCount>RT_ENEMY_PASS_EVENT_CAPACITY)return 0;
    u16 amounts[2]={0,0};
    for(u8 i=0;i<pass->eventCount;i++){
        const RT_EnemyPassEvent *event=pass->events+i;
        u16 value=event->scoreBCD;
        if(!value)continue;
        if(event->player>=players)return 0;
        u16 check=value;
        for(u8 digit=0;digit<4;digit++,check>>=4)if((check&15)>9)return 0;
        u16 award=(value>>12)*1000+((value>>8)&15)*100+((value>>4)&15)*10+(value&15);
        u16 total=amounts[event->player]+award;
        amounts[event->player]=total>=10000?total-10000:total;
    }
    /* Source CPU2 accumulates a four-digit award before CPU1's single score
     * service. Check thresholds once per player per tick, including zero. */
    for(u8 p=0;p<players;p++)RT_campaignAward(score,p,amounts[p],0,setting,reserves);
    return 1;
}
