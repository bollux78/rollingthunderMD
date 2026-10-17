#include "rt_combat.h"
static void loadRow(RT_Agent *agent,u8 row)
{
    const RT_AgentFrame *frame=rtAgentSequences[RT_AGENT_CONTACT].frames+row;
    agent->phase=row;agent->timer=frame->duration;agent->contactPose=frame->contactPose;agent->pose=frame->pose;
}
u8 RT_agentContactEnter(RT_Agent *agent,RT_AgentCombat *combat)
{
    if(!agent || !combat)return 0;
    agent->body.velocityY=-64;agent->body.velocityX=combat->counterattack&4?32:-32;
    agent->action=RT_AGENT_CONTACT;loadRow(agent,0);return 1;
}
RT_AgentContactResult RT_agentContactStep(RT_Agent *agent,RT_AgentCombat *combat,
    u8 collision,u8 *input)
{
    if(!agent || !combat || !input || agent->action!=RT_AGENT_CONTACT || agent->phase>2)
        return RT_AGENT_CONTACT_INVALID;
    u8 velocity=(u8)-agent->body.velocityY;
    if(agent->phase==0){
        if(!collision && velocity>=0x40 && velocity<0x80)return RT_AGENT_CONTACT_WAIT;
        loadRow(agent,collision?2:1);return RT_AGENT_CONTACT_ADVANCED;
    }
    if(agent->phase==1){
        if(!collision && !(velocity>=0x80 && velocity<=0xc0))return RT_AGENT_CONTACT_WAIT;
        loadRow(agent,2);return RT_AGENT_CONTACT_ADVANCED;
    }
    if(!collision)return RT_AGENT_CONTACT_WAIT;
    combat->counterattack=0;agent->fireLatch=agent->jumpLatch=0;*input&=0xfc;
    return RT_AGENT_CONTACT_INPUT;
}
u8 RT_agentConsumeReaction(RT_Agent *agent,RT_AgentCombat *combat,u8 *sound)
{
    if(!agent || !combat || !sound)return 0;
    *sound=0;u8 result;
    switch(combat->reaction){
    case RT_REACTION_SHOT_RIGHT:result=RT_agentHitEnter(agent,combat,0,1,sound);break;
    case RT_REACTION_SHOT_LEFT:result=RT_agentHitEnter(agent,combat,0,2,sound);break;
    case RT_REACTION_CLOSE_FATAL:result=RT_agentHitEnter(agent,combat,0,0,sound);break;
    case RT_REACTION_THROW_HURT_RIGHT:
    case RT_REACTION_THROW_HURT_LEFT:
        agent->direction=combat->reaction==RT_REACTION_THROW_HURT_LEFT?2:1;
        result=RT_agentContactEnter(agent,combat);break;
    case RT_REACTION_CLOSE_HURT:result=RT_agentContactEnter(agent,combat);break;
    default:return 0;
    }
    if(result)combat->reaction=RT_REACTION_NONE;
    return result;
}
