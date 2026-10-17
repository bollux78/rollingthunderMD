#include "rt_agent.h"
#include "rt_climb.h"

static void frame(RT_Agent *agent,u8 index)
{
    const RT_AgentFrame *row=rtAgentSequences[agent->action].frames+index;
    agent->phase=index;agent->timer=row->duration;
    agent->pose=row->pose;agent->contactPose=row->contactPose;
}
static RT_AgentCommand command(u8 action,u8 direction)
{
    RT_AgentCommand c={action,direction};return c;
}
static u8 unsupported(RT_Agent *agent,u8 action)
{
    agent->unsupported=action+1;return 0;
}
static void requestShot(RT_Agent *agent,u8 kneel,u8 alternate,u8 direction)
{
    agent->shotRequest=RT_SHOT_PENDING|(alternate?RT_SHOT_ALTERNATE:0)|
        (kneel?RT_SHOT_KNEEL:0)|(direction==2?RT_SHOT_LEFT:0)|
        (agent->direction==2?RT_SHOT_PRIOR_LEFT:0);
}
u8 RT_agentInput(RT_Agent *agent,u8 direction,u8 jumpPressed,u8 firePressed,u8 fireHeld)
{
    u8 fire=firePressed;
    if(jumpPressed)agent->jumpLatch++;
    if(agent->weaponMode==1){
        if(firePressed)agent->fireRepeat=0;
        else if(fireHeld && ++agent->fireRepeat==5){agent->fireRepeat=0;fire=1;}
    }
    if(fire)agent->fireLatch++;
    return direction*4+((jumpPressed || agent->jumpLatch)?2:0)+((fire || agent->fireLatch)?1:0);
}
u8 RT_agentSelectInScene(RT_Agent *agent,RT_AgentCommand next,const RT_WorldStage *stage,
    const RT_WorldCamera *camera,RT_AnchorScene *scene)
{
    u8 action=next.action,direction=next.direction?next.direction:agent->direction;
    u8 same=agent->action==action,phase=0;
    agent->unsupported=0;
    if(scene && (action==RT_AGENT_LOOK || action==RT_AGENT_LOWER)){
        u8 index=RT_anchorFind(scene->anchors,scene->count,&agent->body,camera,action==RT_AGENT_LOWER);
        if(index){
            agent->anchorIndex=index;
            action=(scene->anchors[index-1].kind==3?RT_AGENT_CLIMB_UP:RT_AGENT_DOOR_ENTER)+(action==RT_AGENT_LOWER);
            direction=agent->direction; /* Entry lookup preserves active facing. */
        }
    }
    if(action>=RT_AGENT_DOOR_ENTER && action<=RT_AGENT_CLIMB_DOWN &&
       (!scene || !agent->anchorIndex || agent->anchorIndex>scene->count))
        return unsupported(agent,action);
    if(action>=RT_AGENT_ACTION_COUNT)
        return unsupported(agent,action);
    if(action==RT_AGENT_FIRE || action==RT_AGENT_KNEEL_FIRE ||
       action==RT_AGENT_AUTO_FIRE || action==RT_AGENT_AUTO_KNEEL_FIRE){
        u8 kneel=action==RT_AGENT_KNEEL_FIRE || action==RT_AGENT_AUTO_KNEEL_FIRE;
        requestShot(agent,kneel,agent->weaponMode==1,direction);
        if(agent->weaponMode==1){
            action=kneel?RT_AGENT_AUTO_KNEEL_FIRE:RT_AGENT_AUTO_FIRE;
            direction=agent->direction;
        }
    }else if(action==RT_AGENT_WALK){
        if(same && direction==agent->direction)return 1;
        static const u8 sounds[10]={8,8,12,12,8,8,8,12,8,8};
        agent->sounds[agent->soundCount++]=sounds[stage->story*5+stage->area];
        agent->body.velocityX=direction==2?-32:32;
    }else if(action==RT_AGENT_JUMP || action==RT_AGENT_RUNNING_JUMP)agent->body.velocityY=-112;
    else if(action==RT_AGENT_FALL || action==RT_AGENT_VAULT_FALL){
        if(same){
            if(agent->phase>=2)return unsupported(agent,action);
            if(direction!=agent->direction && action==RT_AGENT_VAULT_FALL)
                agent->body.velocityX=-agent->body.velocityX;
            phase=agent->phase+1;
        }
    }else if(action==RT_AGENT_VAULT){
        if(same && direction!=agent->direction)agent->body.velocityX=-agent->body.velocityX;
        if(same && agent->phase<9)phase=agent->phase+1;
        else agent->body.velocityY=-160;
    }else if(action==RT_AGENT_DROP){
        if(same){
            if(direction!=agent->direction)agent->body.velocityX=-agent->body.velocityX;
            if(agent->phase<6)phase=agent->phase+1;
        }
        if(!phase && !(RT_motionProbe(stage,&agent->body,camera,0,-3)->collision[agent->body.layer>>1]&16))
            return RT_agentSelectInScene(agent,command(RT_AGENT_JUMP,0),stage,camera,scene);
    }else if(action==RT_AGENT_STAIRS_UP || action==RT_AGENT_STAIRS_DOWN){
        if(same && direction==agent->direction){
            if(agent->phase>=5 && agent->phase!=255)return unsupported(agent,action);
            phase=(u8)(agent->phase+1);
        }else{
            agent->sounds[agent->soundCount++]=0x48;
            /* The original descending-left initializer preserves X speed. */
            if(action==RT_AGENT_STAIRS_UP || direction!=2)
                agent->body.velocityX=direction==2?-16:16;
            agent->body.velocityY=action==RT_AGENT_STAIRS_UP?-16:16;
        }
    }
    agent->action=action;agent->direction=direction;frame(agent,phase);return 1;
}
static void landed(RT_Agent *agent,u8 *input)
{
    agent->fireLatch=agent->jumpLatch=0;*input&=0xfc;
}
static void layerSwap(RT_Agent *agent)
{
    agent->body.layer=agent->body.layer==2?4:2;
}
static u8 fallOut(RT_Agent *agent)
{
    /* Health, credits and death transitions must handle this event. */
    agent->unsupported=255;return 0;
}
u8 RT_agentStepInScene(RT_Agent *agent,u8 input,const RT_WorldStage *stage,
    RT_WorldCamera *camera,u8 lockedCamera,RT_AnchorScene *scene)
{
    if(input>=20 || !agent || !stage || !camera)return 0;
    agent->soundCount=0;agent->unsupported=0;agent->shotRequest=0;agent->alternateReset=0;agent->interactionEvent=0;
    u8 action=agent->action,phase=agent->phase;
    if(action>=RT_AGENT_DOOR_ENTER && action<=RT_AGENT_CLIMB_DOWN)
        return RT_agentClimbStep(agent,input,stage,camera,scene);
    if(action==RT_AGENT_FIRE || action==RT_AGENT_KNEEL_FIRE ||
       action==RT_AGENT_AUTO_FIRE || action==RT_AGENT_AUTO_KNEEL_FIRE){
        u8 alternate=action>=RT_AGENT_AUTO_FIRE;
        u8 kneel=action==RT_AGENT_KNEEL_FIRE || action==RT_AGENT_AUTO_KNEEL_FIRE;
        if(alternate && !agent->ammoAlternate){
            alternate=0;agent->weaponMode=0;agent->alternateReset=1;
        }
        u8 direction=input&28;
        if(direction==4 || direction==8)agent->direction=direction==4?1:2;
        else if(kneel && direction==12)kneel=0;
        else if(!kneel && direction==16)kneel=1;
        agent->action=alternate?(kneel?RT_AGENT_AUTO_KNEEL_FIRE:RT_AGENT_AUTO_FIRE):
            (kneel?RT_AGENT_KNEEL_FIRE:RT_AGENT_FIRE);
        if((phase==1 && agent->fireLatch!=1) || (phase>=2 && agent->fireLatch)){
            if(phase==1)agent->fireLatch--;
            requestShot(agent,kneel,alternate,agent->direction);frame(agent,0);return 1;
        }
        if(--agent->timer)return 1;
        if(phase<3){
            if(phase==1)agent->fireLatch--;
            frame(agent,phase+1);return 1;
        }
        return RT_agentSelectInScene(agent,rtAgentInputMaps[11+kneel][input],stage,camera,scene);
    }
    if(action==RT_AGENT_IDLE || action==RT_AGENT_WALK){
        if(action==RT_AGENT_WALK){
            RT_motionHorizontal(&agent->body,stage,camera,lockedCamera);
            u8 left=agent->direction==2;
            const RT_TerrainMaterial *m=RT_motionProbe(stage,&agent->body,camera,-1,left?0:1);
            u8 flags=m->collision[agent->body.layer>>1];
            if((flags&32) && !(m->probeCode&1))return RT_agentSelectInScene(agent,command(left?RT_AGENT_STAIRS_UP:RT_AGENT_STAIRS_DOWN,0),stage,camera,scene);
            m=RT_motionProbe(stage,&agent->body,camera,1,left?1:0);flags=m->collision[agent->body.layer>>1];
            if((flags&32) && (m->probeCode&1))return RT_agentSelectInScene(agent,command(left?RT_AGENT_STAIRS_DOWN:RT_AGENT_STAIRS_UP,0),stage,camera,scene);
            s32 local=agent->body.x-(s32)camera->x*16;
            if(local>=-256 && local<=4864){
                m=RT_motionProbe(stage,&agent->body,camera,0,1);flags=m->collision[agent->body.layer>>1];
                if((flags&32) || ((flags&2) && (m->probeCode&0xee)!=0x20))agent->body.layer=(flags>>5)&6;
                else return RT_agentSelectInScene(agent,command(RT_AGENT_FALL,0),stage,camera,scene);
            }
            if(!--agent->timer){
                frame(agent,phase==5?0:phase+1);
                if(phase==5){static const u8 sounds[10]={8,8,12,12,8,8,8,12,8,8};agent->sounds[agent->soundCount++]=sounds[stage->story*5+stage->area];}
            }
        }else if(--agent->timer)return 1;
        return RT_agentSelectInScene(agent,rtAgentInputMaps[0][input],stage,camera,scene);
    }
    if(action>=RT_AGENT_LOOK && action<=RT_AGENT_RISE){
        if(--agent->timer)return 1;
        return RT_agentSelectInScene(agent,rtAgentInputMaps[action-RT_AGENT_LOOK][input],stage,camera,scene);
    }
    if(action==RT_AGENT_JUMP || action==RT_AGENT_RUNNING_JUMP){
        static const s16 drift[4]={0,8,-8,0};agent->body.velocityX=drift[(input&12)>>2];
        if(action==RT_AGENT_RUNNING_JUMP)agent->body.velocityX+=agent->direction==2?-32:32;
        RT_motionHorizontal(&agent->body,stage,camera,lockedCamera);
        RT_MotionResult result=RT_motionVertical(&agent->body,stage,camera,RT_MOTION_ORDINARY);
        if(result==RT_MOTION_FELL_OUT)return fallOut(agent);
        u8 byte=(u8)(0-agent->body.velocityY);s16 speed=byte<128?byte:(s16)byte-256;
        if(!phase){if(result==RT_MOTION_CONTACT)frame(agent,2);else if(speed<64)frame(agent,1);}
        else if(phase==1){if(result==RT_MOTION_CONTACT || speed<=-64)frame(agent,2);}
        else if(result==RT_MOTION_CONTACT){landed(agent,&input);return RT_agentSelectInScene(agent,rtAgentInputMaps[0][input],stage,camera,scene);}
        return 1;
    }
    if(action==RT_AGENT_FALL || action==RT_AGENT_VAULT_FALL){
        if(action==RT_AGENT_FALL){static const s16 drift[4]={0,8,-8,0};agent->body.velocityX=drift[(input&12)>>2];}
        if(!phase){
            if(action==RT_AGENT_FALL)RT_motionHorizontal(&agent->body,stage,camera,lockedCamera);
            RT_MotionResult result=RT_motionVertical(&agent->body,stage,camera,RT_MOTION_ORDINARY);
            if(result==RT_MOTION_FELL_OUT)return fallOut(agent);
            if(result==RT_MOTION_CONTACT){landed(agent,&input);agent->body.velocityY=0;frame(agent,1);}
        }else if(!--agent->timer)
            return RT_agentSelectInScene(agent,rtAgentInputMaps[phase==1?(action==RT_AGENT_FALL?4:6):0][input],stage,camera,scene);
        return 1;
    }
    if(action==RT_AGENT_VAULT){
        if(phase==1 || phase==2){
            RT_MotionResult result=RT_motionVertical(&agent->body,stage,camera,RT_MOTION_VAULT);
            if(result==RT_MOTION_FELL_OUT)return fallOut(agent);
            if(result==RT_MOTION_CONTACT)frame(agent,3);
            else if(!agent->body.velocityY)return RT_agentSelectInScene(agent,command(RT_AGENT_VAULT_FALL,0),stage,camera,scene);
            else if(phase==1 && !--agent->timer)frame(agent,2);
        }else if(!--agent->timer){
            if(phase<8){
                if(phase==5){landed(agent,&input);agent->body.velocityY=0;layerSwap(agent);}
                frame(agent,phase+1);
            }else return RT_agentSelectInScene(agent,rtAgentInputMaps[phase==8?5:0][input],stage,camera,scene);
        }
        return 1;
    }
    if(action==RT_AGENT_DROP){
        if(phase==4){
            RT_MotionResult result=RT_motionVertical(&agent->body,stage,camera,RT_MOTION_DROP);
            if(result==RT_MOTION_FELL_OUT)return fallOut(agent);
            if(result==RT_MOTION_CONTACT){landed(agent,&input);frame(agent,5);}
        }else if(!--agent->timer){
            if(phase<4){if(phase==1)layerSwap(agent);frame(agent,phase+1);}
            else{
                RT_AgentCommand next=rtAgentInputMaps[0][input];
                if(phase==5){
                    if(!(input&3) && input<16)next=command(RT_AGENT_DROP,input==4?1:input==8?2:0);
                    if(input==16)next=command(RT_AGENT_KNEEL,0);
                }
                return RT_agentSelectInScene(agent,next,stage,camera,scene);
            }
        }
        return 1;
    }
    if(action==RT_AGENT_STAIRS_UP || action==RT_AGENT_STAIRS_DOWN){
        RT_motionStairCamera(&agent->body,stage,camera,&agent->stairCameraFlags);
        if(--agent->timer)return 1;
        u8 up=action==RT_AGENT_STAIRS_UP,left=agent->direction==2;
        u8 map=(up?7:9)+left;
        if((up && (phase==2 || phase==5)) || (!up && (phase==1 || phase==4))){
            agent->body.x+=left?-192:192;
            agent->body.y+=up?-128:128;
            const RT_TerrainMaterial *m=RT_motionProbe(stage,&agent->body,camera,
                up?(left?-1:1):(left?1:-1),up?0:1);
            if(!(m->collision[agent->body.layer>>1]&32))map=0;
        }
        if(phase==5){agent->sounds[agent->soundCount++]=0x48;agent->phase=255;}
        return RT_agentSelectInScene(agent,rtAgentInputMaps[map][input],stage,camera,scene);
    }
    return unsupported(agent,action);
}

u8 RT_agentSelect(RT_Agent *agent,RT_AgentCommand next,const RT_WorldStage *stage,
    const RT_WorldCamera *camera)
{
    return RT_agentSelectInScene(agent,next,stage,camera,0);
}
u8 RT_agentStep(RT_Agent *agent,u8 input,const RT_WorldStage *stage,
    RT_WorldCamera *camera,u8 lockedCamera)
{
    return RT_agentStepInScene(agent,input,stage,camera,lockedCamera,0);
}
