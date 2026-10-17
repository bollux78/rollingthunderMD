#include <genesis.h>
extern u32 rtSpriteBirthEpoch;
#include "rt_stage_pool.h"
u8 RT_stagePoolInitialize(RT_StagePool *pool,const RT_StageEventLevel *level)
{
    if(!pool || !level || !level->events || !level->cells || !level->columns || !level->rows)return 0;
    pool->level=level;pool->count=0;
    for(u16 i=0;i<RT_NATIVE_STAGE_CAPACITY;i++){
        pool->objects[i]=(RT_Anchor){0};pool->objects[i].kind=255;
        pool->definitions[i]=65535;
    }
    return 1;
}
u8 RT_stagePoolEventVisitor(const RT_StageEvent *event,u16 index,void *context)
{
    RT_StagePool *pool=context;
    if(!pool || !pool->level || !event || index>=pool->level->eventCount || event->kind>31)return 0;
    for(u16 i=0;i<RT_NATIVE_STAGE_CAPACITY;i++)if(pool->objects[i].kind==255){
        RT_Anchor *object=pool->objects+i;
    rtSpriteBirthEpoch++;
        object->x=(s32)event->x*16;object->y=(s32)event->y*16;
        object->kind=event->kind;object->flags=event->initialState;
        object->layer=event->parameter>>5;
        object->phase=object->timer=object->spawnDelay=object->pose=0;
        /* Source constructors mark new records hidden; visibility belongs
         * to the subsequent service pass, not allocation. */
        object->visible=0;pool->definitions[i]=index;return 1;
    }
    return 0;
}
u8 RT_stagePoolSpawn(RT_StagePool *pool,const RT_EventWindowRequest *request)
{
    if(!pool || !pool->level || !request)return 0;
    return RT_stageVisitEvents(pool->level,request->gridX,request->gridY,
        (RT_EventWindowEdge)request->edge,&pool->count,RT_stagePoolEventVisitor,pool);
}
u8 RT_stagePoolRelease(RT_StagePool *pool,u8 slot)
{
    if(!pool || slot>=RT_NATIVE_STAGE_CAPACITY || !pool->count ||
       pool->objects[slot].kind==255)return 0;
    pool->objects[slot].kind=255;pool->count--;return 1;
}

u8 RT_stagePoolCull(RT_StagePool *pool,const RT_WorldCamera *camera,u8 boundaries)
{
    if(!pool || !camera || boundaries>15)return 255;
    if(!pool->count || !boundaries || boundaries==3 || boundaries==7 || boundaries>=11)return 0;
    u8 remaining=pool->count,removed=0;
    s32 left=(s32)camera->x*16-0x700,right=(s32)camera->x*16+0x1900;
    s32 top=((s32)camera->y+256)*16-0x1800;
    s32 bottom=((s32)camera->y+256)*16+0x800;
    for(u16 i=0;i<RT_NATIVE_STAGE_CAPACITY;i++){
        RT_Anchor *object=pool->objects+i;
        if(object->kind==255)continue;
        if(!object->visible &&
           (((boundaries&RT_STAGE_BOUNDARY_RIGHT) && object->x<left) ||
            ((boundaries&RT_STAGE_BOUNDARY_LEFT) && object->x>=right) ||
            ((boundaries&RT_STAGE_BOUNDARY_UP) && object->y>bottom) ||
            ((boundaries&RT_STAGE_BOUNDARY_DOWN) && object->y<=top))){
            object->kind=255;pool->count--;removed++;
        }
        if(!--remaining)return removed;
    }
    return 255;
}

u8 RT_stagePoolRefreshVisibility(RT_StagePool *pool,const RT_WorldCamera *camera)
{
    if(!pool || !camera)return 255;
    u8 count=0,remaining=pool->count;
    s32 left=(s32)camera->x*16-0x200,right=(s32)camera->x*16+0x1400;
    s32 origin=((s32)camera->y+256)*16;
    /* Kinds below 8 and the others use two fixed vertical bands. */
    s32 topLow=origin-0xe00,bottomLow=origin-0x300,topHigh=origin-0xf00,bottomHigh=origin+0x100;
    RT_Anchor *object=pool->objects,*end=pool->objects+RT_NATIVE_STAGE_CAPACITY;
    for(;remaining && object<end;object++){
        if(object->kind==255)continue;
        remaining--;
        s32 x=object->x,y=object->y;
        u8 visible=0;
        if(x>=left && x<right){
            if(object->kind<8)visible=y>topLow && y<=bottomLow;
            else visible=y>topHigh && y<=bottomHigh;
        }
        object->visible=visible;count+=visible;
    }
    return remaining?255:count;
}
u8 RT_stageFanStep(RT_Anchor *object)
{
    if(!object || object->kind!=8 || !object->visible)return 0;
    if(object->spawnDelay)object->spawnDelay--;
    object->timer++;
    if(!(object->timer&7)){
        object->phase=(object->phase+1)&3;
        object->pose=object->phase;
    }
    return 1;
}
u8 RT_stagePoolAdvanceWindow(RT_StagePool *pool,RT_EventWindow *window,
    const RT_WorldStage *stage,const RT_WorldCamera *previous,
    const RT_WorldCamera *current)
{
    if(!pool || !pool->level)return 0;
    RT_EventWindowRequest requests[2];
    u8 count=RT_eventWindowMove(window,stage,previous,current,requests),flags=0;
    if(count==255)return 0;
    for(u16 i=0;i<count;i++)flags|=1u<<(requests[i].edge-1);
    if(RT_stagePoolCull(pool,current,flags)==255)return 0;
    for(u16 i=0;i<count;i++)if(!RT_stagePoolSpawn(pool,requests+i))return 0;
    return 1;
}
