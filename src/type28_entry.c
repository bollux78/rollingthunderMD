#include "rt_type28.h"
#include "rt_high_animation.h"
static RT_Type28EntryResult enter(RT_Enemy *e,u8 selector,const RT_Type28EntryContext *c,u8 inherit)
{
    if(!e || !c || e->state.kind!=40 || selector>=0xc0)return RT_TYPE28_ENTRY_UNAVAILABLE;
    u8 resolved=inherit && !(selector&3)?selector|(e->state.action&3):selector;
    if((resolved&0xfc)==0x48){
        resolved=e->state.action^3;
        if(!(resolved&3))resolved|=e->state.action&3;
        if(resolved>=0xc0 || (resolved&0xfc)==0x48)return RT_TYPE28_ENTRY_UNAVAILABLE;
    }
    switch(resolved&0xfc){
    case 0x18:case 0x1c:case 0x44:case 0x80:case 0x84:case 0x88:
    case 0x94:case 0xa0:case 0xa4:case 0xb0:case 0xb4:case 0xb8:case 0xbc:break;
    case 0x28:
        if(!c->accelerationY)return RT_TYPE28_ENTRY_UNAVAILABLE;
        *c->accelerationY=-4;e->velocityY=-4;e->velocityX=resolved&2?-16:16;break;
    case 0x30:
        if(!c->accelerationY)return RT_TYPE28_ENTRY_UNAVAILABLE;
        *c->accelerationY=e->variation?-4:4;e->velocityY=e->variation?48:-48;
        e->delay=e->variation?5:3;e->velocityX=resolved&2?-24:24;break;
    case 0x60:case 0x64:case 0x68:case 0x78:case 0x7c:case 0x98:
        if((e->state.action&0xfc)==0x28 || (e->state.action&0xfc)==0x6c){
            if(!c->award)return RT_TYPE28_ENTRY_UNAVAILABLE;
            *c->award=0x80;resolved=(resolved&3)|0x64;
        }else{
            if(!c->accelerationY || !c->command)return RT_TYPE28_ENTRY_UNAVAILABLE;
            *c->command=6;*c->accelerationY=-8;e->velocityY=-32;
            e->velocityX=resolved&2?-16:16;
        }break;
    case 0x6c:
        if(!c->accelerationY)return RT_TYPE28_ENTRY_UNAVAILABLE;
        *c->accelerationY=2;break;
    default:
        if(!c->accelerationY)return RT_TYPE28_ENTRY_UNAVAILABLE;
        *c->accelerationY=2;e->velocityY=-72;e->velocityX=resolved&2?-32:32;break;
    }
    e->state.action=e->state.spawnAction=resolved;e->state.row=0;
    const RT_HighAnimationRow *r=rtHighStateRows[2]+(resolved>>2);
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_TYPE28_ENTRY_INSTALLED;
}
RT_Type28EntryResult RT_type28Select(RT_Enemy *e,u8 selector,const RT_Type28EntryContext *c)
{return enter(e,selector,c,1);}
RT_Type28EntryResult RT_type28Startup(RT_Enemy *e,const RT_Type28EntryContext *c)
{return e?enter(e,e->state.spawnAction,c,0):RT_TYPE28_ENTRY_UNAVAILABLE;}
