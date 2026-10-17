#include "rt_campaign_transition_scene.h"
static u8 population(const RT_StagePool *pool)
{
    u8 count=0;
    for(u8 i=0;i<RT_NATIVE_STAGE_CAPACITY;i++){
        u8 kind=pool->objects[i].kind;
        if(kind==255)continue;
        if(kind>=8 && (kind<17 || kind>31))return 255;
        count++;
    }
    return count;
}
static void construct(RT_StagePool *pool,u8 slot,u8 kind,u8 x)
{
    pool->objects[slot]=(RT_Anchor){.x=(s32)x*16,.y=193*16,
        .kind=kind,.layer=6};
    pool->definitions[slot]=65535;pool->count++;
}
u8 RT_transitionSceneConstruct(RT_StagePool *pool,u8 selector,u8 *spawned)
{
    static const u8 groups[11]={0,1,2,3,4,5,3,4,3,4,6};
    static const u8 counts[7]={0,2,3,2,2,2,3};
    static const u8 kinds[7][3]={{0},{17,18},{19,20,21},{22,23},
        {24,25},{30,31},{27,28,29}};
    static const u8 xs[7][3]={{0},{48,112},{96,136,32},{80,208},
        {64,192},{96,96},{32,144,176}};
    if(!pool || !spawned || selector>10 || pool->count>RT_NATIVE_STAGE_CAPACITY || population(pool)!=pool->count)return 0;
    u8 group=groups[selector],needed=counts[group],slots[3],found=0;
    if(needed){
        /* D76B checks population only after constructing the first actor. */
        u8 available=pool->count>=30?1:31-pool->count;
        if(needed>available)needed=available;
    }
    for(u8 i=0;i<RT_NATIVE_STAGE_CAPACITY && found<needed;i++)
        if(pool->objects[i].kind==255)slots[found++]=i;
    if(found!=needed)return 0;
    for(u8 i=0;i<needed;i++)construct(pool,slots[i],kinds[group][i],xs[group][i]);
    *spawned=needed;return 1;
}
static u8 visible(const RT_Anchor *actor)
{
    /* Source X[-32,320), Y-up[-16,240); converted to Y-down(16,272]. */
    return actor->x>=-512 && actor->x<5120 && actor->y>256 && actor->y<=4352;
}
u8 RT_transitionSceneStep(RT_StagePool *pool,u16 remaining,u8 ordinaryCompletion)
{
    if(!pool || remaining>9999 || ordinaryCompletion>1 || pool->count>RT_NATIVE_STAGE_CAPACITY ||
       population(pool)!=pool->count)return 255;
    u8 reserved[6]={0};
    /* Forward-only allocation must succeed before changing any live actor. */
    for(u8 slot=0;slot<RT_NATIVE_STAGE_CAPACITY;slot++){
        const RT_Anchor *actor=pool->objects+slot;
        if(actor->kind!=24 || !visible(actor) || ((u8)(actor->timer+1)&7))continue;
        u8 child;
        for(child=slot+1;child<RT_NATIVE_STAGE_CAPACITY;child++)
            if(pool->objects[child].kind==255 && !(reserved[child>>3]&(1<<(child&7))))break;
        if(child==RT_NATIVE_STAGE_CAPACITY)return 255;
        reserved[child>>3]|=1<<(child&7);
    }
    u8 count=0;
    for(u8 slot=0;slot<RT_NATIVE_STAGE_CAPACITY;slot++){
        RT_Anchor *actor=pool->objects+slot;
        if(actor->kind==255 || actor->kind<8)continue;
        actor->visible=visible(actor);
        if(!actor->visible)continue;
        count++;
        if(actor->spawnDelay)actor->spawnDelay--;
        switch(actor->kind){
        case 24:
            actor->timer++;
            if(!(actor->timer&7)){
                u8 child=slot+1;
                while(pool->objects[child].kind!=255)child++;
                construct(pool,child,26,64);
            }
            break;
        case 26:
            actor->timer++;
            if(!(actor->timer&1)){actor->kind=255;pool->count--;count--;}
            break;
        case 30:case 31:
            if(actor->kind==30 && remaining<=(ordinaryCompletion?6:1))break;
            actor->timer++;
            if(!(actor->timer&7)){actor->kind=actor->kind==30?31:30;actor->timer=0;}
            break;
        }
    }
    return count;
}
