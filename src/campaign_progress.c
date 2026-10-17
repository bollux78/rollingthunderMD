#include "rt_campaign_progress.h"
RT_CampaignExitResult RT_campaignExitGate(u8 story,u8 area,u8 windowX,u8 windowY,
    s16 localX,u8 exiting,u8 bossComplete,u8 layer,const RT_TerrainMaterial *ground)
{
    if(story>1 || area>4)return RT_CAMPAIGN_EXIT_INVALID;
    if(exiting)return RT_CAMPAIGN_EXIT_NONE;
    if(story==1 && area==4)
        return bossComplete && localX>=0x900?RT_CAMPAIGN_EXIT_FINAL:RT_CAMPAIGN_EXIT_NONE;
    const RT_CampaignExitCell *exit=rtCampaignExitCells+story*5+area;
    u16 cell=(u16)(((u16)windowX<<8)|windowY)-0x101;
    if(cell!=(((u16)exit->column<<8)|exit->row) || localX<0xd00)return RT_CAMPAIGN_EXIT_NONE;
    if(localX<=0x1300){
        if(!ground || layer>6 || (layer&1))return RT_CAMPAIGN_EXIT_INVALID;
        u8 flags=ground->collision[layer>>1];
        if(!(flags&0x20) && (!(flags&2) || (ground->probeCode&0xee)==0x20))
            return RT_CAMPAIGN_EXIT_NONE;
    }
    return RT_CAMPAIGN_EXIT_WALK;
}
u8 RT_campaignNextArea(u8 story,u8 area,u8 *nextStory,u8 *nextArea)
{
    if(story>1 || area>4 || !nextStory || !nextArea || (story==1 && area==4))return 0;
    *nextStory=story+(area==4);*nextArea=area==4?0:area+1;return 1;
}

static const u8 exitSounds[10]={8,8,12,12,8,8,8,12,8,8};
static void exitFrame(RT_Agent *agent,u8 row)
{
    const RT_AgentFrame *frame=rtAgentSequences[RT_AGENT_WALK].frames+row;
    agent->phase=row;agent->timer=frame->duration;
    agent->pose=frame->pose;agent->contactPose=frame->contactPose;
}
u8 RT_campaignExitWalkBegin(RT_Agent *agent,u8 story,u8 area,u8 *sound)
{
    if(!agent || story>1 || area>4 || !sound)return 0;
    agent->action=RT_AGENT_WALK;agent->body.velocityX=agent->direction==2?-32:32;
    agent->body.layer=2;exitFrame(agent,0);*sound=exitSounds[story*5+area];return 1;
}
u8 RT_campaignExitWalkStep(RT_Agent *agent,const RT_WorldCamera *camera,
    u8 story,u8 area,u8 *sound)
{
    if(!agent || !camera || story>1 || area>4 || !sound ||
       agent->action!=RT_AGENT_WALK || agent->phase>5 || !agent->timer)return 0;
    *sound=0;
    agent->contactPose|=0x80;
    if(agent->body.x-(s32)camera->x*16>=0x1280)return 2;
    agent->body.x+=16;agent->body.layer=2;
    if(!--agent->timer){
        u8 row=agent->phase+1;
        if(row==6){row=0;*sound=exitSounds[story*5+area];}
        exitFrame(agent,row);
    }
    return agent->body.x-(s32)camera->x*16>=0x1280?2:1;
}
