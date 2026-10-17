#include <genesis.h>
extern u32 rtSpriteBirthEpoch;
#include "rt_type28.h"
#include "rt_high_animation.h"
static const RT_EnemyTemplate definitions[2][2]={
    {{2,40,0x6d,0,0,0x20,0x80,0,0},{2,40,0x6e,0,0,0x20,0x80,0,0}},
    {{2,40,0x6d,0,1,0x20,0x80,0,0},{2,40,0x6e,0,1,0x20,0x80,0,0}}
};
u8 RT_type28SpawnChild(RT_EnemyPool *pool,u8 parent,const RT_WorldCamera *camera,
    const RT_Type28Child *child,u8 *slot)
{
    if(!pool || !camera || !child || !slot || parent>=RT_ENEMY_CAPACITY ||
       pool->count>=RT_ENEMY_CAPACITY || pool->slots[parent].state.kind!=40 ||
       child->variation>1 || (child->selector!=0x6d && child->selector!=0x6e))return 0;
    u8 i=parent;
    while(i<RT_ENEMY_CAPACITY && pool->slots[i].state.kind!=RT_ENEMY_FREE)i++;
    if(i==RT_ENEMY_CAPACITY)return 0;
    const RT_Enemy *p=pool->slots+parent;RT_Enemy *e=pool->slots+i;
    s32 ox=(s32)camera->x*16,oy=((s32)camera->y+256)*16;
    e->state.x=ox+(s16)((u16)(p->state.x-ox)+child->deltaX);
    e->state.y=oy-(s16)((u16)(oy-p->state.y)-child->deltaY);
    rtSpriteBirthEpoch++;
    e->state.kind=40;e->state.hidden=0;e->state.action=e->state.spawnAction=child->selector;
    e->state.attributes=p->state.attributes;e->state.row=e->state.decisions=0;
    e->traits=0;e->variation=child->variation;e->byte4=0x20;e->byte12=e->byte14=0;
    e->velocityX=child->velocityX;e->velocityY=32;pool->accelerationY[i]=2;
    e->definition=&definitions[child->variation][child->selector-0x6d];e->parent=255;
    pool->scoreOwner[i]=0;
    /* B580 leaves byte6/delay intact on reuse. B5AE supplies animation fields. */
    const RT_HighAnimationRow *r=rtHighStateRows[2]+(child->selector>>2);
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    pool->count++;*slot=i;return 1;
}
