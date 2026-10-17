#include "rt_enemy_throw.h"
u8 RT_throwHitPlayers(const RT_Projectile *object,const RT_ThrowMotion *motion,
    u8 explosion,void *context)
{
    RT_PlayerTargets *targets=context;
    if(!object || !motion || !targets || !targets->agents || !targets->combat ||
       !targets->players || targets->players>2 || explosion>1)return 0;
    u8 hit=0,left=object->velocityX<0;
    for(u8 i=0;i<targets->players;i++){
        RT_Agent *a=targets->agents+i;RT_AgentCombat *c=targets->combat+i;
        if(c->hidden || c->removed || !c->health || c->invincibleTicks || c->counterattack ||
           (a->contactPose&128) || (u8)(a->body.layer<<5)!=motion->attributes)continue;
        const RT_ContactBox *box=rtContactBoxes+a->contactPose;
        u16 y=(u16)(object->y-a->body.y-box->yOffset);
        if((s16)y<0){if((u16)(0-y)>box->height)continue;}
        else if(y>(explosion?0x140:0xc0))continue;
        s16 origin=explosion?(left?0x60:-0xa0):0x20;
        u16 x=(u16)(a->body.x+box->xOffset[!!(a->direction&2)]-object->x-origin);
        if((s16)x<0){if((u16)(0-x)>box->width)continue;}
        else if(x>(explosion?0x140:0xc0))continue;
        c->damagePending+=0x20;
        u8 alive=(u8)(c->damagePending+c->priorityDamage)<c->health;
        c->reaction=alive?(left?RT_REACTION_THROW_HURT_LEFT:RT_REACTION_THROW_HURT_RIGHT):
            (left?RT_REACTION_SHOT_LEFT:RT_REACTION_SHOT_RIGHT);
        hit=1;
    }
    return hit;
}
