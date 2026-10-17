#include "rt_type24.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
RT_EnemyResult RT_type24ChaseStep(RT_Enemy *e,const RT_Type24MotionContext *c)
{
    if(!e || !c || !c->camera || !c->entry.accelerationY || !c->entry.sound ||
       e->state.kind!=36 || e->state.row>3)return RT_ENEMY_UNAVAILABLE;
    switch(e->state.action&0xfc){
    case 0x18:case 0x1c:case 0x44:case 0x84:case 0x88:case 0x94:case 0xa0:case 0xa4:break;
    default:return RT_ENEMY_UNAVAILABLE;
    }
    if((e->state.decisions&0x30)==0x30){
        u8 selector;
        if(!RT_highEnemyGround(e,c->camera,c->probe,c->probeContext,&selector))return RT_ENEMY_UNAVAILABLE;
        if(selector)e->state.y+=32;
        else return RT_type24Select(e,0x20,&c->entry)==RT_TYPE24_ENTRY_INSTALLED?
            RT_ENEMY_CONTINUE:RT_ENEMY_UNAVAILABLE;
        return RT_ENEMY_CONTINUE;
    }
    s32 originX=(s32)c->camera->x*16,originY=((s32)c->camera->y+256)*16;
    u16 x=(u16)(e->state.x-originX),y=(u16)(originY-e->state.y);
    u16 targetX=(u16)(c->targetX-originX),targetY=(u16)(originY-c->targetY);
    u8 steps=e->delay;
    /* Original loop decrements after movement: count zero means256 steps.
     * Masking before sign/magnitude retains the arcade subpixel tie behavior. */
    do{
        s16 dx=(s16)((u16)(x-targetX)&0xfff0),dy=(s16)((u16)(y-targetY)&0xfff0);
        u16 ax=dx<0?(u16)(0-(u16)dx):(u16)dx,ay=dy<0?(u16)(0-(u16)dy):(u16)dy;
        if(ax>=ay)x=(u16)(x+(dx<0?16:-16));
        if(ay>=ax)y=(u16)(y+(dy<0?16:-16));
    }while(--steps);
    e->state.x=originX+(s16)x;e->state.y=originY-(s16)y;
    if(--e->state.timer)return RT_ENEMY_CONTINUE;
    u8 limit=(u16)(0-(u16)e->velocityY)>>8;
    if(e->delay<limit)e->delay++;
    if(e->state.row==3){*c->entry.sound=0x62;e->state.row=0;}else e->state.row++;
    const RT_HighAnimationRow *r=rtHighCycleRows+rtHighCycleOffsets[17]+e->state.row;
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
