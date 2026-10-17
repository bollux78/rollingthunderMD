#include "rt_survival_loop.h"
#include "rt_survival_checkpoint.h"
static RT_CampaignRunState state(const RT_CampaignScene *scene)
{
    switch(RT_homePlayState(&scene->players.home,scene->players.combat)){
    case RT_HOME_PLAY_ACTIVE:return RT_CAMPAIGN_RUNNING;
    case RT_HOME_PLAY_CONTINUE:return RT_CAMPAIGN_CONTINUE;
    case RT_HOME_PLAY_GAME_OVER:return RT_CAMPAIGN_GAME_OVER;
    default:return RT_CAMPAIGN_FAULT;
    }
}
RT_CampaignRunState RT_survivalUpdate(RT_CampaignScene *scene,
    RT_SurvivalWorld *world,const u16 pads[2],u8 tick,u8 refresh,
    RT_CampaignUpdate *result)
{
    if(!scene || !scene->ready || !world || scene->stage!=&world->stage ||
       !pads || !result || scene->players.selection.mode!=RT_MODE_HOME ||
       scene->players.selection.game!=RT_HOME_SURVIVAL)return RT_CAMPAIGN_FAULT;
    result->respawnMask=result->updated=result->faultPhase=result->worldUpdated=0;
    result->exitSounds[0]=result->exitSounds[1]=0;
    result->battle.enemies.eventCount=result->battle.projectiles.soundCount=0;
    result->objects.count=0;
    result->battle.enemies.faultSlot=result->battle.enemies.faultAction=255;
    RT_CampaignRunState play=state(scene);
    if(play!=RT_CAMPAIGN_RUNNING)return play;
    RT_WorldCamera previous=scene->camera;
    if(!RT_campaignPlayersStep(scene,pads,&result->players)){
        result->faultPhase=1;return RT_CAMPAIGN_FAULT;
    }
    if(!RT_campaignBattleStep(scene,tick,refresh,&result->battle)){
        result->faultPhase=2;return RT_CAMPAIGN_FAULT;
    }
    result->respawnMask=RT_homeResolveDeaths(&scene->players.home,
        scene->players.agents,scene->players.combat,
        (pads[0]&BUTTON_START?1:0)|(pads[1]&BUTTON_START?2:0));
    for(u8 p=0;p<scene->players.home.players;p++)if(result->respawnMask&(1<<p))
        scene->players.previousButtons[p]=pads[p];
    if(!RT_campaignObjectsStep(scene,tick,&result->objects)){
        result->faultPhase=3;return RT_CAMPAIGN_FAULT;
    }
    /* ObjectsStep just refreshed visibility at this camera. Its services and
     * enemy culling/spawning change no object positions, kinds or visibility.
     * Reuse only within this 2P update; newly spawned objects stay hidden. */
    if(!RT_survivalSceneScrollPrepared(scene,world,&previous,scene->players.home.players==2)){
        result->faultPhase=4;return RT_CAMPAIGN_FAULT;
    }
    RT_survivalCheckpointCapture(scene,world);
    result->updated=result->worldUpdated=1;return state(scene);
}
