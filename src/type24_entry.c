#include "rt_type24.h"
#include "rt_high_animation.h"
static RT_Type24EntryResult enter(RT_Enemy *e,u8 selector,const RT_Type24EntryContext *c,u8 inherit)
{
    if(!e || !c || e->state.kind!=36 || selector>=0xc0)return RT_TYPE24_ENTRY_UNAVAILABLE;
    u8 state=selector&0xfc,resolved=inherit && !(selector&3)?selector|(e->state.action&3):selector;
    switch(state){
    case 0:e->delay=3;break;
    case 4:case 8:case 0x0c:case 0x10:case 0x14:case 0x28:case 0x2c:
    case 0x30:case 0x34:case 0x38:case 0x3c:case 0x40:case 0x4c:case 0x50:
    case 0x58:case 0x5c:case 0x70:case 0x74:case 0x8c:case 0x90:case 0x98:
    case 0x9c:case 0xa8:case 0xac:{
        if(!c->accelerationY || !c->command)return RT_TYPE24_ENTRY_UNAVAILABLE;
        static const u8 delays[4]={35,23,13,7};
        u8 variant=(e->variation>>4)&3;s16 speed=40+variant*8;
        if(resolved&2)speed=-speed;
        e->velocityX=speed;e->velocityY=-speed;
        *c->accelerationY=resolved&2?-8:8;e->delay=delays[variant];*c->command=8;
        break;
    }
    case 0x18:case 0x1c:case 0x44:case 0x84:case 0x88:case 0x94:case 0xa0:case 0xa4:{
        if(!c->sound)return RT_TYPE24_ENTRY_UNAVAILABLE;
        u16 source=(u16)(0-(u16)e->velocityY);
        source=(source&255)|((u16)(((e->variation>>4)&3)+3)<<8);
        e->velocityY=(s16)(0-source);e->delay=1;*c->sound=0x62;break;
    }
    case 0x20:case 0x24:case 0x78:case 0x7c:
        if(!c->accelerationY)return RT_TYPE24_ENTRY_UNAVAILABLE;
        e->delay=5;e->velocityY=-4;*c->accelerationY=-4;break;
    case 0x60:case 0x64:case 0x68:case 0x6c:
        if(!c->sound || !c->award)return RT_TYPE24_ENTRY_UNAVAILABLE;
        *c->sound=0x62;*c->award=0x80;resolved^=3;break;
    default:break;
    }
    e->state.action=e->state.spawnAction=resolved;e->state.row=0;
    const RT_HighAnimationRow *r=rtHighStateRows[1]+(resolved>>2);
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_TYPE24_ENTRY_INSTALLED;
}
RT_Type24EntryResult RT_type24Select(RT_Enemy *e,u8 selector,const RT_Type24EntryContext *c)
{return enter(e,selector,c,1);}
RT_Type24EntryResult RT_type24Startup(RT_Enemy *e,const RT_Type24EntryContext *c)
{return e?enter(e,e->state.spawnAction,c,0):RT_TYPE24_ENTRY_UNAVAILABLE;}
