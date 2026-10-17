#include "rt_campaign_scene.h"
#include "rt_campaign_progress.h"
RT_CampaignInit RT_campaignSceneAdvance(RT_CampaignScene *scene,u8 enemySet)
{
    if(!scene || !scene->ready || !scene->stage || enemySet>1 ||
       !scene->players.home.players || scene->players.home.players>2)
        return RT_CAMPAIGN_INIT_INVALID;
    u8 story,area;
    if(!RT_campaignNextArea(scene->stage->story,scene->stage->area,&story,&area))
        return RT_CAMPAIGN_INIT_INVALID;
    RT_SessionConfig selection=scene->players.selection;
    u16 primary[2],alternate[2],credits=scene->players.home.credits;
    u8 modes[2],dead=0,players=scene->players.home.players;
    for(u8 p=0;p<players;p++){
        primary[p]=scene->players.agents[p].ammoPrimary;
        alternate[p]=scene->players.agents[p].ammoAlternate;
        modes[p]=scene->players.agents[p].weaponMode;
        if(scene->players.combat[p].removed ||
           (!scene->players.combat[p].health && !(scene->exiting&(1<<p))))dead|=1<<p;
    }
    /* No dead-party bypass of Continue/Arcade life ownership. */
    if(dead==((1<<players)-1))return RT_CAMPAIGN_INIT_INVALID;
    RT_CampaignInit result=RT_campaignSceneInitialize(scene,&selection,story,area,0,enemySet,credits);
    if(result)return result;
    for(u8 p=0;p<players;p++){
        RT_Agent *a=scene->players.agents+p;
        u16 minimum=scene->players.home.players==2?75:50;
        a->ammoPrimary=primary[p]<minimum?minimum:primary[p];
        a->ammoAlternate=alternate[p];
        a->weaponMode=modes[p];
        scene->players.home.spawn[p]=*a;
        if(dead&(1<<p)){
            scene->players.combat[p].health=0;
            scene->players.combat[p].removed=1;
        }
    }
    return RT_CAMPAIGN_INIT_OK;
}
