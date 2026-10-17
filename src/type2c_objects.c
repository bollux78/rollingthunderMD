#include "rt_type2c.h"
u8 RT_type2cObjectProbe(const RT_Enemy *e,const RT_StagePool *stage,
    const RT_WorldCamera *camera,u8 phase,u8 *selector)
{
    if(!e || !stage || !camera || !selector || stage->count>RT_NATIVE_STAGE_CAPACITY)return 0;
    *selector=0;
    if(!(e->traits&0x20) || ((u16)(e->state.x-(s32)camera->x*16)&0xf0) || !stage->count)return 1;
    for(u8 i=0;i<RT_NATIVE_STAGE_CAPACITY;i++){
        const RT_Anchor *o=stage->objects+i;
        if(o->kind!=3 || !o->visible)continue;
        u16 distance=(u16)((u32)o->x+0xe0-(u32)e->state.x);
        if((s16)distance<0)distance=(u16)(0-distance);
        if(distance>=0x80)continue;
        u16 height=(u16)((u32)e->state.y-(u32)o->y);
        if((u8)(o->layer<<5)!=e->state.attributes){
            if(height!=0x3f0)continue;
            *selector=phase&1?0xac:0;
            return 1;
        }
        if(height==(o->flags&4?0x3f0:0x470))return 1;
    }
    return 1;
}
