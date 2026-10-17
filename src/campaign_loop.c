#include "rt_campaign_loop.h"
#include "rt_campaign_exit.h"
static void emptyWorld(RT_CampaignUpdate *result)
{
    RT_campaignBattlePassClear(&result->battle);
    result->objects.count=result->objects.spawned=0;result->objects.unservicedKinds=0;
}
void RT_campaignProtectionAdvance(RT_CampaignScene *scene,u32 refreshes)
{
    if(!scene || !scene->ready)return;
    for(u8 p=0;p<scene->players.home.players && p<2;p++){
        u16 remaining=scene->players.combat[p].invincibleTicks;
        scene->players.combat[p].invincibleTicks=refreshes>=remaining?0:remaining-(u16)refreshes;
    }
}
static RT_CampaignRunState state(const RT_CampaignScene *scene)
{
    if(!scene->players.home.players || scene->players.home.players>2)return RT_CAMPAIGN_FAULT;
    /* Source completion tests actor removal, not the independently drained
     * health counter. A latched exit keeps ownership of a live actor. */
    if(scene->exiting)for(u8 p=0;p<scene->players.home.players;p++)
        if((scene->exiting&(1<<p)) && !scene->players.combat[p].removed)return RT_CAMPAIGN_RUNNING;
    if(scene->players.selection.mode==RT_MODE_ARCADE &&
       RT_agentHasFatalAnimation(scene->players.agents,scene->players.combat))return RT_CAMPAIGN_RUNNING;
    RT_HomePlayState play=RT_homePlayState(&scene->players.home,scene->players.combat);
    if(play==RT_HOME_PLAY_INVALID)return RT_CAMPAIGN_FAULT;
    if(play==RT_HOME_PLAY_ACTIVE)return RT_CAMPAIGN_RUNNING;
    if(scene->players.selection.mode==RT_MODE_ARCADE)return RT_CAMPAIGN_ARCADE_DEATH;
    return play==RT_HOME_PLAY_CONTINUE?RT_CAMPAIGN_CONTINUE:RT_CAMPAIGN_GAME_OVER;
}
RT_CampaignRunState RT_campaignUpdate(RT_CampaignScene *scene,const u16 pads[2],
    u8 tick,u8 refresh,RT_CampaignUpdate *result)
{
    if(!scene || !scene->ready || !pads || !result)return RT_CAMPAIGN_FAULT;
    /* Event arrays are bounded by their published counts. Clearing their
     * entire capacities here, then again in each phase, wastes the 68000's
     * frame budget and is unnecessary for consumers of the live ranges. */
    result->respawnMask=result->updated=result->faultPhase=result->worldUpdated=0;
    result->exitSounds[0]=result->exitSounds[1]=0;
    result->battle.enemies.eventCount=result->battle.projectiles.soundCount=0;
    result->objects.count=0;
    result->battle.enemies.faultSlot=result->battle.enemies.faultAction=255;
    RT_CampaignRunState play=state(scene);
    if(play!=RT_CAMPAIGN_RUNNING)return play;
    if(scene->bossProgress&0x80){
        emptyWorld(result);result->players=(RT_CampaignPlayerPass){0};
        return RT_CAMPAIGN_FINAL_EXIT;
    }
    if(scene->exiting){
        emptyWorld(result);result->players=(RT_CampaignPlayerPass){0};
        for(u8 p=0;p<2;p++)result->players.steps[p]=
            (scene->exiting&(1<<p)) && !scene->players.combat[p].removed?
            RT_HOME_AGENT_NORMAL:RT_HOME_AGENT_INACTIVE;
        u8 exit=RT_campaignExitAdvance(scene,result->exitSounds);
        if(!exit){result->faultPhase=6;return RT_CAMPAIGN_FAULT;}
        result->updated=1;
        return exit==2?RT_CAMPAIGN_STAGE_CLEAR:RT_CAMPAIGN_RUNNING;
    }
    RT_WorldCamera previous=scene->camera;
    if(!RT_campaignPlayersStep(scene,pads,&result->players)){result->faultPhase=1;return RT_CAMPAIGN_FAULT;}
    RT_CampaignExitResult exit=RT_campaignExitCheck(scene,scene->bossProgress,result->exitSounds);
    if(exit==RT_CAMPAIGN_EXIT_FINAL){
        emptyWorld(result);result->updated=1;return RT_CAMPAIGN_FINAL_EXIT;
    }
    if(exit==RT_CAMPAIGN_EXIT_INVALID){result->faultPhase=6;return RT_CAMPAIGN_FAULT;}
    if(exit==RT_CAMPAIGN_EXIT_WALK){
        emptyWorld(result);
        result->updated=1;return RT_campaignExitComplete(scene)?RT_CAMPAIGN_STAGE_CLEAR:RT_CAMPAIGN_RUNNING;
    }
    if(!RT_campaignBattleStep(scene,tick,refresh,&result->battle)){result->faultPhase=2;return RT_CAMPAIGN_FAULT;}
    /* CPU1's timer follows the actor/projectile/damage phase. Queue selector60
     * for the next player phase, preserving the native fatal animation. */
    RT_TimerResult timer=RT_campaignTimerStep(&scene->timer,scene->exiting);
    if(timer==RT_TIMER_INVALID){result->faultPhase=5;return RT_CAMPAIGN_FAULT;}
    if(timer==RT_TIMER_EXPIRED){
        for(u8 p=0;p<scene->players.home.players;p++){
            RT_AgentCombat *combat=scene->players.combat+p;
            if(!combat->removed && combat->health)combat->reaction=RT_REACTION_CLOSE_FATAL;
        }
    }
    /* Home shares one stage clock. Once expired, finish the party's deaths
     * without survivor respawns bypassing timeout; Continue resets the clock. */
    if(scene->players.selection.mode==RT_MODE_HOME && scene->timer.remaining){
        result->respawnMask=RT_homeResolveDeaths(&scene->players.home,
            scene->players.agents,scene->players.combat,
            (pads[0]&BUTTON_START?1:0)|(pads[1]&BUTTON_START?2:0));
        for(u8 p=0;p<scene->players.home.players;p++)if(result->respawnMask&(1<<p))
            scene->players.previousButtons[p]=pads[p];
    }
    if(!RT_campaignObjectsStep(scene,tick,&result->objects)){result->faultPhase=3;return RT_CAMPAIGN_FAULT;}
    /* ObjectsStep just refreshed visibility at this camera. Its services and
     * enemy culling/spawning change no object positions, kinds or visibility.
     * Reuse only within this 2P update; newly spawned objects stay hidden. */
    if(!RT_campaignScrollPrepared(scene,&previous,scene->players.home.players==2)){result->faultPhase=4;return RT_CAMPAIGN_FAULT;}
    result->updated=result->worldUpdated=1;return state(scene);
}
