#include "rt_home_session.h"
RT_HomeAgentStep RT_sceneStepAgent(RT_Agent *agent,RT_AgentCombat *combat,
    u8 input,const RT_WorldStage *stage,RT_WorldCamera *camera,
    RT_AnchorScene *scene,u8 lockedCamera,u8 *sound)
{
    if(!agent || !combat || !stage || !camera || !sound || input>=20 || lockedCamera>1)
        return RT_HOME_AGENT_INVALID;
    *sound=0;
    if(combat->removed || (!combat->health && !RT_agentHasFatalAnimation(agent,combat))){
        agent->shotRequest=0;combat->hidden=1;return RT_HOME_AGENT_INACTIVE;
    }
    if(combat->reaction && !RT_agentConsumeReaction(agent,combat,sound))
        return RT_HOME_AGENT_INVALID;
    if(agent->action==RT_AGENT_CONTACT){
        RT_AgentContactResult result=RT_agentContactUpdate(agent,combat,input,
            stage,camera,lockedCamera,scene);
        if(result==RT_AGENT_CONTACT_INVALID)return RT_HOME_AGENT_INVALID;
        if(result==RT_AGENT_CONTACT_FELL_OUT){
            combat->removed=combat->hidden=1;agent->shotRequest=0;
            return RT_HOME_AGENT_REMOVED;
        }
        return RT_HOME_AGENT_CONTACT;
    }
    if(agent->action>=RT_AGENT_HIT_60 && agent->action<=RT_AGENT_HIT_6C){
        agent->shotRequest=0;
        RT_AgentHitResult result=RT_agentHitStep(agent,combat);
        if(result==RT_AGENT_HIT_INVALID)return RT_HOME_AGENT_INVALID;
        if(result==RT_AGENT_HIT_TERMINAL){
            RT_AgentCommand recovery;
            if(RT_agentHitFinish(agent,combat,camera,0,0,&input,&recovery)!=RT_AGENT_TERMINAL_REMOVE)
                return RT_HOME_AGENT_INVALID;
            return RT_HOME_AGENT_REMOVED;
        }
        return RT_HOME_AGENT_HIT;
    }
    if(RT_agentStepInScene(agent,input,stage,camera,lockedCamera,scene))return RT_HOME_AGENT_NORMAL;
    if(agent->unsupported==255){
        combat->removed=combat->hidden=1;agent->shotRequest=0;
        return RT_HOME_AGENT_REMOVED;
    }
    return RT_HOME_AGENT_INVALID;
}
RT_HomeAgentStep RT_homeStepAgent(RT_Agent *agent,RT_AgentCombat *combat,
    u8 input,const RT_WorldStage *stage,RT_WorldCamera *camera,
    RT_AnchorScene *scene,u8 *sound)
{return RT_sceneStepAgent(agent,combat,input,stage,camera,scene,1,sound);}
