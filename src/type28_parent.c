#include "rt_type28.h"
#include "rt_high_enemy_motion.h"
#include "rt_high_animation.h"
static const RT_Type28Child children[2][3]={
    {{0,0x6e,0x50,-0x150,-16},{1,0x6e,-0x80,-0xd0,-32},{0,0x6d,0xd0,0,16}},
    {{1,0x6d,-0x50,-0x150,16},{0,0x6d,0x80,-0xd0,32},{1,0x6e,-0xd0,0,-16}}
};
RT_EnemyResult RT_type28ParentStep(RT_Enemy *e,const RT_Type28MotionContext *c,
    u8 population,RT_Type28Spawn spawn,void *spawnContext)
{
    if(!e || !c || !c->camera || !c->entry.accelerationY || e->state.kind!=40 || e->state.row>4)
        return RT_ENEMY_UNAVAILABLE;
    switch(e->state.action&0xfc){case 0x60:case 0x68:case 0x78:case 0x7c:case 0x98:break;
    default:return RT_ENEMY_UNAVAILABLE;}
    if(e->state.row==4 && e->state.timer==1 && population<40 && !spawn)return RT_ENEMY_UNAVAILABLE;
    const RT_HighAnimationRow *r;
    if(e->state.row<4){
        u8 code;
        RT_EnemySweep result=RT_highEnemyHorizontal(e,c->camera,c->shadowXLow,c->probe,c->probeContext,&code);
        if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        if(result==RT_ENEMY_SWEEP_HIT){
            e->state.action^=3;e->state.spawnAction=e->state.action;
            e->velocityX=(s16)(0-(u16)e->velocityX);
        }
        u16 scratch=0;
        result=RT_highEnemyVertical(e,c->camera,*c->entry.accelerationY,c->shadowYLow,c->probe,c->probeContext,&scratch);
        if(result==RT_ENEMY_SWEEP_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
        if(result==RT_ENEMY_SWEEP_REMOVED){e->state.kind=RT_ENEMY_FREE;return RT_ENEMY_REMOVED;}
        if(!e->velocityY)e->state.row=4;
        else{
            s16 y=(s16)(((s32)c->camera->y+256)*16-e->state.y);
            if(y>=0x600)*c->entry.accelerationY=8;
            if(--e->state.timer)return RT_ENEMY_CONTINUE;
            e->state.row=e->state.row==3?1:e->state.row+1;
        }
        r=rtHighCycleRows+rtHighCycleOffsets[9]+e->state.row;
    }else{
        if(--e->state.timer)return RT_ENEMY_CONTINUE;
        u8 left=!!(e->state.action&2);
        if(population<40)for(u8 i=0;i<3;i++)if(!spawn(e,children[left]+i,spawnContext))break;
        /* Preserve source signed16 camera-local wrap while retaining native
         * world coordinates. Parent always transforms, even if no slot fits. */
        s32 ox=(s32)c->camera->x*16,oy=((s32)c->camera->y+256)*16;
        e->state.x=ox+(s16)((u16)(e->state.x-ox)+(left?-0x1f0:0x1f0));
        e->state.y=oy-(s16)((u16)(oy-e->state.y)+0x70);
        e->velocityX=left?-32:32;e->velocityY=32;e->traits=0;e->variation=left?0:1;
        e->state.action=e->state.spawnAction=left?0x6e:0x6d;e->state.row=0;
        r=rtHighStateRows[2]+(0x6c>>2);
    }
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_ENEMY_CONTINUE;
}
