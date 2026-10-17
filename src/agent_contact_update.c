#include "rt_combat.h"
RT_AgentContactResult RT_agentContactUpdate(RT_Agent *agent,RT_AgentCombat *combat,
    u8 input,const RT_WorldStage *stage,RT_WorldCamera *camera,u8 lockedCamera,
    RT_AnchorScene *scene)
{
    if(!agent || !combat || !stage || !camera || input>=20 ||
       agent->action!=RT_AGENT_CONTACT || agent->phase>2)return RT_AGENT_CONTACT_INVALID;
    agent->soundCount=0;agent->unsupported=0;agent->shotRequest=0;agent->alternateReset=0;
    RT_motionHorizontal(&agent->body,stage,camera,lockedCamera);
    RT_MotionResult movement=RT_motionVertical(&agent->body,stage,camera,RT_MOTION_ORDINARY);
    if(movement==RT_MOTION_FELL_OUT)return RT_AGENT_CONTACT_FELL_OUT;
    RT_AgentContactResult result=RT_agentContactStep(agent,combat,movement==RT_MOTION_CONTACT,&input);
    if(result==RT_AGENT_CONTACT_INPUT &&
        !RT_agentSelectInScene(agent,rtAgentInputMaps[0][input],stage,camera,scene))
        return RT_AGENT_CONTACT_INVALID;
    return result;
}
