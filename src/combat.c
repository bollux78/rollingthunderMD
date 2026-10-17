#include "rt_combat.h"
u8 RT_projectileOverlapsAgent(const RT_Projectile *shot,const RT_Agent *agent)
{
    if(!shot || !agent || (agent->contactPose&0x80))return 0;
    const RT_ContactBox *box=rtContactBoxes+agent->contactPose,*bullet=&rtBulletContactBox;
    /* Preserve source word subtraction at the boundary, not source records.
     * Negate the native Y separation before interpreting its signed side. */
    u16 y=(u16)(shot->y-agent->body.y-box->yOffset+bullet->yOffset);
    if((s16)y<0){if((u16)(0-y)>box->height)return 0;}
    else if(y>bullet->height)return 0;
    u16 x=(u16)(agent->body.x+box->xOffset[agent->direction==2]-shot->x-
        bullet->xOffset[shot->velocityX<0]);
    if((s16)x<0){if((u16)(0-x)>box->width)return 0;}
    else if(x>bullet->width)return 0;
    return 1;
}
u8 RT_enemyBulletHitPlayers(const RT_Projectile *shot,void *context)
{
    RT_PlayerTargets *targets=context;
    if(!shot || !(shot->flags&RT_PROJECTILE_ENEMY) || !targets || !targets->agents ||
        !targets->combat || !targets->players || targets->players>2)return 0;
    for(u8 i=0;i<targets->players;i++){
        RT_Agent *agent=targets->agents+i;RT_AgentCombat *combat=targets->combat+i;
        if(combat->hidden || combat->invincibleTicks || combat->counterattack ||
           agent->body.layer!=shot->layer || !RT_projectileOverlapsAgent(shot,agent))continue;
        combat->damagePending+=0x40;
        combat->reaction=shot->velocityX<0?RT_REACTION_SHOT_LEFT:RT_REACTION_SHOT_RIGHT;
        return 1;
    }
    return 0;
}
void RT_agentDamageTick(const RT_Agent *agent,const RT_WorldCamera *camera,
    RT_AgentCombat *combat,u8 damageProtected)
{
    if(!agent || !camera || !combat || !combat->health)return;
    s16 localY=(s16)(((s32)camera->y+256)*16-agent->body.y);
    if(localY>=0 && damageProtected)combat->priorityDamage=combat->damagePending=0;
    if(combat->priorityDamage){combat->priorityDamage--;combat->health--;}
    else if(combat->damagePending){combat->damagePending--;combat->health--;}
}
u8 RT_enemyCloseHitPlayers(const RT_Enemy *enemy,RT_PlayerTargets *targets)
{
    if(!enemy || !targets || !targets->agents || !targets->combat || targets->players>2)return 0;
    for(u8 i=0;i<targets->players;i++){
        RT_Agent *agent=targets->agents+i;RT_AgentCombat *combat=targets->combat+i;
        if(combat->hidden || combat->invincibleTicks || (agent->contactPose&0x80) ||
           agent->body.layer!=((enemy->state.attributes&0xc0)>>5))continue;
        const RT_ContactBox *box=rtContactBoxes+agent->contactPose;
        u16 y=(u16)(enemy->state.y-agent->body.y-box->yOffset-0x2f0);
        if((s16)y<0){if((u16)(0-y)>box->height)continue;}
        else if(y>0x80)continue;
        s16 offset=enemy->state.action&2?-0x1d0:0x100;
        u16 x=(u16)(agent->body.x+box->xOffset[agent->direction==2]-enemy->state.x-offset);
        if((s16)x<0){if((u16)(0-x)>box->width)continue;}
        else if(x>0x1d0)continue;
        if(combat->counterattack)return 0x78;
        combat->damagePending+=0x20;
        combat->reaction=(u8)(combat->damagePending+combat->priorityDamage)<combat->health?
            RT_REACTION_CLOSE_HURT:RT_REACTION_CLOSE_FATAL;
        combat->counterattack=enemy->state.decisions;
    }
    return 0;
}
