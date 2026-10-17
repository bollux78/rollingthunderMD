#include "rt_campaign_exit.h"
u8 RT_campaignExitComplete(const RT_CampaignScene *scene)
{
    if(!scene || !scene->ready || !scene->exiting ||
       !scene->players.home.players || scene->players.home.players>2 ||
       (scene->exiting>>scene->players.home.players))return 0;
    for(u8 p=0;p<scene->players.home.players;p++)if(scene->exiting&(1<<p)){
        if(!scene->players.combat[p].removed &&
           scene->players.agents[p].body.x-(s32)scene->camera.x*16<0x1280)return 0;
    }
    return 1;
}
RT_CampaignExitResult RT_campaignExitCheck(RT_CampaignScene *scene,u8 bossComplete,u8 sounds[2])
{
    if(!scene || !scene->ready || !scene->stage || !sounds ||
       !scene->players.home.players || scene->players.home.players>2 ||
       scene->stage->story>1 || scene->stage->area>4)return RT_CAMPAIGN_EXIT_INVALID;
    sounds[0]=sounds[1]=0;
    if(scene->exiting)return RT_CAMPAIGN_EXIT_NONE;
    if(scene->stage->story!=1 || scene->stage->area!=4){
        const RT_CampaignExitCell *cell=rtCampaignExitCells+scene->stage->story*5+scene->stage->area;
        u16 grid=((u16)scene->objectWindow.gridX<<8)|scene->objectWindow.gridY;
        if((u16)(grid-0x101)!=(((u16)cell->column<<8)|cell->row))return RT_CAMPAIGN_EXIT_NONE;
    }else if(!bossComplete)return RT_CAMPAIGN_EXIT_NONE;
    u8 count=scene->players.home.players;
    for(u8 p=0;p<count;p++){
        RT_Agent *agent=scene->players.agents+p;
        const RT_AgentCombat *combat=scene->players.combat+p;
        if(combat->removed || (!combat->health && !(scene->players.selection.mode==RT_MODE_ARCADE &&
            RT_agentHasFatalAnimation(agent,combat))))continue;
        s16 x=(s16)(agent->body.x-(s32)scene->camera.x*16);
        RT_CampaignExitResult result=RT_campaignExitGate(scene->stage->story,scene->stage->area,
            scene->objectWindow.gridX,scene->objectWindow.gridY,x,0,bossComplete,agent->body.layer,NULL);
        if(result==RT_CAMPAIGN_EXIT_INVALID){
            if(!RT_terrainWindowMove(&scene->terrain,scene->stage,&scene->camera))return result;
            const RT_TerrainMaterial *ground=RT_terrainWindowProbe(&scene->terrain,agent->body.x,agent->body.y,0,1);
            result=RT_campaignExitGate(scene->stage->story,scene->stage->area,
                scene->objectWindow.gridX,scene->objectWindow.gridY,x,0,bossComplete,agent->body.layer,ground);
        }
        if(result==RT_CAMPAIGN_EXIT_NONE)continue;
        if(result==RT_CAMPAIGN_EXIT_FINAL){
            /* CPU2 990D: latch the gate independently of the audio/ending
             * owner. Co-op uses the first eligible player in slot order. */
            scene->bossProgress=0x80;scene->exitPlayer=p;
            return result;
        }
        if(result!=RT_CAMPAIGN_EXIT_WALK)return result;
        scene->exitPlayer=p;
        for(u8 q=0;q<count;q++){
            if(q!=p && (scene->players.combat[q].removed || !scene->players.combat[q].health))continue;
            if(!RT_campaignExitWalkBegin(scene->players.agents+q,scene->stage->story,scene->stage->area,sounds+q))return RT_CAMPAIGN_EXIT_INVALID;
            scene->players.combat[q].reaction=RT_REACTION_NONE;
            scene->players.agents[q].shotRequest=0;
            scene->exiting|=1<<q;
        }
        return RT_CAMPAIGN_EXIT_WALK;
    }
    return RT_CAMPAIGN_EXIT_NONE;
}
u8 RT_campaignExitAdvance(RT_CampaignScene *scene,u8 sounds[2])
{
    if(!scene || !scene->ready || !scene->stage || !sounds || !scene->exiting ||
       !scene->players.home.players || scene->players.home.players>2 ||
       (scene->exiting>>scene->players.home.players))return 0;
    sounds[0]=sounds[1]=0;u8 complete=1;
    for(u8 p=0;p<scene->players.home.players;p++)if(scene->exiting&(1<<p)){
        if(scene->players.combat[p].removed)continue;
        u8 result=RT_campaignExitWalkStep(scene->players.agents+p,&scene->camera,
            scene->stage->story,scene->stage->area,sounds+p);
        if(!result)return 0;
        if(result==1)complete=0;
    }
    return complete?2:1;
}
