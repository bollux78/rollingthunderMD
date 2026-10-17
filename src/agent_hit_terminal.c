#include "rt_combat.h"
RT_AgentTerminalResult RT_agentHitFinish(RT_Agent *agent,RT_AgentCombat *combat,
    const RT_WorldCamera *camera,u8 mode,u8 damageProtected,u8 *input,
    RT_AgentCommand *recovery)
{
    if(!agent || !combat || !camera || !input || !recovery || combat->removed ||
       agent->action<RT_AGENT_HIT_60 || agent->action>RT_AGENT_HIT_6C || agent->phase!=7)
        return RT_AGENT_TERMINAL_INVALID;
    if(mode==3)return RT_AGENT_TERMINAL_SIGNAL;
    s16 localY=(s16)(((s32)camera->y+256)*16-agent->body.y);
    if(localY>=0 && damageProtected){
        combat->counterattack=0;combat->priorityDamage=combat->damagePending=0;
        agent->fireLatch=agent->jumpLatch=0;*input&=0xfc;
        *recovery=(RT_AgentCommand){RT_AGENT_RISE,1};return RT_AGENT_TERMINAL_RECOVER;
    }
    agent->weaponMode=0;agent->ammoAlternate=0;combat->removed=1;combat->hidden=1;
    return RT_AGENT_TERMINAL_REMOVE;
}
