#include "rt_combat.h"
u8 RT_agentHasFatalAnimation(const RT_Agent *agent,const RT_AgentCombat *combat)
{
    return agent && combat && !combat->removed &&
        ((agent->action>=RT_AGENT_HIT_60 && agent->action<=RT_AGENT_HIT_6C) ||
         combat->reaction==RT_REACTION_SHOT_RIGHT || combat->reaction==RT_REACTION_SHOT_LEFT ||
         combat->reaction==RT_REACTION_CLOSE_FATAL);
}
static void loadFrame(RT_Agent *agent,const RT_AgentFrame *frame)
{
    agent->timer=frame->duration;agent->contactPose=frame->contactPose;agent->pose=frame->pose;
}
u8 RT_agentHitEnter(RT_Agent *agent,RT_AgentCombat *combat,u8 family,u8 direction,u8 *sound)
{
    if(!agent || !combat || !sound || family>3 || direction>3)return 0;
    agent->action=RT_AGENT_HIT_60+family;if(direction)agent->direction=direction;
    agent->phase=0;combat->counterattack++;*sound=0x0b;
    loadFrame(agent,rtAgentSequences[agent->action].frames);return 1;
}
RT_AgentHitResult RT_agentHitStep(RT_Agent *agent,RT_AgentCombat *combat)
{
    if(!agent || !combat || agent->action<RT_AGENT_HIT_60 || agent->action>RT_AGENT_HIT_6C || agent->phase>7)
        return RT_AGENT_HIT_INVALID;
    u8 row=agent->phase;
    if(row==7)return RT_AGENT_HIT_TERMINAL;
    if(--agent->timer)return RT_AGENT_HIT_WAIT;
    if(!row){agent->action=RT_AGENT_HIT_6C;combat->hitFlags=2;}
    else if(row==1)combat->hitFlags=0;
    agent->phase++;
    if(row<6)loadFrame(agent,rtAgentSequences[RT_AGENT_HIT_6C].frames+row+1);
    return RT_AGENT_HIT_WAIT;
}
