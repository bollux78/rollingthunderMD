#include "rt_type2c.h"
#include "rt_high_animation.h"
static RT_Type2cEntryResult enter(RT_Enemy *e,u8 selector,
    const RT_Type2cEntryContext *c,u8 inherit)
{
    if(!e || !c || e->state.kind!=44 || selector>=0xc0)return RT_TYPE2C_ENTRY_UNAVAILABLE;
    u8 state=selector&0xfc;
    u8 resolved=inherit && !(selector&3)?selector|(e->state.action&3):selector;
    if(state==0x74){
        if(!c->camera)return RT_TYPE2C_ENTRY_UNAVAILABLE;
        s16 y=(s16)(((s32)c->camera->y+256)*16-e->state.y);
        state=y>=0x700?0x2c:0x28;
        resolved=state|(e->state.action&3);
    }
    switch(state){
    case 0:case 4:case 8:case 0x0c:case 0x10:case 0x14:
    case 0x3c:case 0x40:case 0x4c:case 0x50:case 0x58:case 0x5c:
    case 0x8c:case 0x90:case 0x9c:case 0xa8:{
        static const u8 delays[4]={96,64,48,32};
        static const s16 speeds[4]={20,24,28,32};
        u8 variant=(e->variation>>4)&3;
        e->velocityY=0;e->velocityX=resolved&2?-speeds[variant]:speeds[variant];
        e->delay=delays[variant];break;
    }
    case 0x18:case 0x1c:case 0x78:case 0x7c:case 0x94:case 0x98:case 0xa0:case 0xa4:
        if(!c->sound)return RT_TYPE2C_ENTRY_UNAVAILABLE;
        *c->sound=c->phase&4?0x61:0x69;break;
    case 0x20:case 0x24:case 0x30:case 0x44:case 0x84:case 0x88:
        e->velocityY=-96;e->velocityX=resolved&2?-48:48;break;
    case 0x28:case 0x2c:{
        u8 code,flags;
        if(!c->probe || !c->probe(0,state==0x28?1:-3,&code,&flags,c->probeContext))
            return RT_TYPE2C_ENTRY_UNAVAILABLE;
        e->state.spawnAction=resolved;
        if(state==0x28?(code==0x23 || code==0x2f):!(flags&0x10)){
            e->byte12|=0x20;return RT_TYPE2C_ENTRY_DECLINED;
        }
        e->velocityY=state==0x28?-144:-64;
        e->byte12&=state==0x28?0xed:0xfd;break;
    }
    case 0x60:case 0x64:case 0x68:case 0x6c:
        if(!c->sound || !c->award)return RT_TYPE2C_ENTRY_UNAVAILABLE;
        *c->sound=0x69;*c->award=0x0050;resolved^=3;break;
    case 0xac:e->velocityY=-48;break;
    default:break;
    }
    e->state.action=e->state.spawnAction=resolved;e->state.row=0;
    const RT_HighAnimationRow *r=rtHighStateRows[3]+(resolved>>2);
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_TYPE2C_ENTRY_INSTALLED;
}
RT_Type2cEntryResult RT_type2cSelect(RT_Enemy *e,u8 selector,const RT_Type2cEntryContext *c)
{return enter(e,selector,c,1);}
RT_Type2cEntryResult RT_type2cStartup(RT_Enemy *e,const RT_Type2cEntryContext *c)
{return e?enter(e,e->state.spawnAction,c,0):RT_TYPE2C_ENTRY_UNAVAILABLE;}


RT_EnemyResult RT_type2cWalkDecision(RT_Enemy *e,s32 targetX,
    const RT_Type2cEntryContext *c)
{
    if(!e || !c || e->state.kind!=44)return RT_ENEMY_UNAVAILABLE;
    if(e->byte12&0x20)return RT_ENEMY_CONTINUE;
    u8 flags=e->state.decisions,selector;
    if(flags&1){
        if(!(flags&0x80)){
            u16 distance=(u16)((u32)e->state.x-(u32)targetX);
            if((s16)distance<0)distance=(u16)(0-distance);
            if((distance>>8)==5){selector=0x44;goto select;}
        }
        if(--e->delay)return RT_ENEMY_CONTINUE;
        selector=0x18;
    }else if(flags&0x20){
        if(--e->delay)return RT_ENEMY_CONTINUE;
        if(!!(flags&4)==!!(e->state.action&2))selector=0x38;
        else goto reset_delay;
    }else{
        if(!(e->variation&0x40) && --e->delay)return RT_ENEMY_CONTINUE;
        selector=flags&8?0x28:0x2c;
    }
select:;
    RT_Type2cEntryResult result=RT_type2cSelect(e,selector,c);
    if(result==RT_TYPE2C_ENTRY_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_TYPE2C_ENTRY_INSTALLED)return RT_ENEMY_SELECT;
reset_delay:
    e->delay=e->variation&0x20?64:96;
    return RT_ENEMY_CONTINUE;
}
