#ifndef RT_NATIVE_CLIMB_H
#define RT_NATIVE_CLIMB_H
#include "rt_agent.h"
/* Internal action service; caller clears per-step output events first. */
u8 RT_agentClimbStep(RT_Agent *agent,u8 input,const RT_WorldStage *stage,
    const RT_WorldCamera *camera,RT_AnchorScene *scene);
#endif
