#include "rt_type20.h"
#include "rt_high_animation.h"
/* CPU1 BE6A..BE8A: clear vertical velocity, select the BE8B delay/speed
 * pair using variation bits30 and selector bit1, sign-extend speed, then
 * perform B5AE's state/row/timer/list installation. */
static u8 enterWalk(RT_Enemy *e,u8 selector,u8 inherit)
{
    static const u8 enabled[48]={
        0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,
        1,0,0,1,1,0,1,1,0,0,0,0,1,0,1,1,
        0,1,1,1,1,0,1,1,0,0,1,1,0,0,0,0
    };
    static const u8 delays[4]={96,64,48,32};
    static const s16 speeds[4]={20,24,28,32};
    if(!e || e->state.kind!=32 || selector>=0xc0 || !enabled[selector>>2])return 0;
    if(inherit && !(selector&3))selector|=e->state.action&3;
    u8 variant=(e->variation>>4)&3;
    e->velocityY=0;e->velocityX=selector&2?-speeds[variant]:speeds[variant];
    e->delay=delays[variant];e->state.spawnAction=e->state.action=selector;e->state.row=0;
    const RT_HighAnimationRow *r=rtHighStateRows[0]+(selector>>2);
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return 1;
}

u8 RT_type20EnterWalk(RT_Enemy *e,u8 selector)
{return enterWalk(e,selector,1);}

/* B88E entries required by walking and movement. Audio is returned as a
 * native event; no source queue or source address is retained at runtime. */
static RT_Type20EntryResult selectEntry(RT_Enemy *e,u8 selector,
    RT_EnemyProbe probe,void *context,u8 *sound,u8 inherit,
    const RT_WorldCamera *camera,u16 *award)
{
    if(!e || e->state.kind!=32 || selector>=0xc0)return RT_TYPE20_ENTRY_UNAVAILABLE;
    u8 state=selector&0xfc;
    u8 resolved=inherit && !(selector&3)?selector|(e->state.action&3):selector;
    if(state==0x74){
        if(!camera)return RT_TYPE20_ENTRY_UNAVAILABLE;
        s16 y=(s16)(((s32)camera->y+256)*16-e->state.y);
        state=y>=0x700?0x2c:0x28;
        /* BA7B re-enters B88E: inherit the active direction, not request74. */
        resolved=state|(e->state.action&3);
    }
    if(state>=0x60 && state<=0x6c){
        if(!sound || !award)return RT_TYPE20_ENTRY_UNAVAILABLE;
        *sound=0x63;*award=0x0030;resolved^=3;
    }else if(state==0x28 || state==0x2c){
        u8 code,flags;
        /* Source BB6E uses upward cell offsets; native terrain is downward. */
        if(!probe || !probe(0,state==0x28?1:-3,&code,&flags,context))
            return RT_TYPE20_ENTRY_UNAVAILABLE;
        e->state.spawnAction=resolved;
        if(state==0x28?(code==0x23 || code==0x2f):!(flags&0x10)){
            e->byte12|=0x20;return RT_TYPE20_ENTRY_DECLINED;
        }
        e->velocityY=state==0x28?-160:-64;
        e->byte12&=state==0x28?0xed:0xfd;
    }else if(state==0x18 || state==0x1c || state==0x20 || state==0x24 ||
        state==0x44 || state==0x94 || state==0xa0 || state==0xa4){
        if(!sound)return RT_TYPE20_ENTRY_UNAVAILABLE;
        e->velocityY=-96;e->velocityX=resolved&2?-48:48;*sound=0x63;
    }else if(state==0x30){
        /* C142 loads D with horizontal constants but never stores them. */
        e->velocityY=-40;
    }else if(enterWalk(e,resolved,0))return RT_TYPE20_ENTRY_INSTALLED;
    e->state.action=e->state.spawnAction=resolved;e->state.row=0;
    const RT_HighAnimationRow *r=rtHighStateRows[0]+(resolved>>2);
    e->state.timer=r->timer;e->state.flags=r->flags;e->state.pose=r->pose;
    return RT_TYPE20_ENTRY_INSTALLED;
}

RT_Type20EntryResult RT_type20Select(RT_Enemy *e,u8 selector,
    RT_EnemyProbe probe,void *context,u8 *sound)
{return selectEntry(e,selector,probe,context,sound,1,0,0);}

RT_Type20EntryResult RT_type20Startup(RT_Enemy *e,
    RT_EnemyProbe probe,void *context,u8 *sound)
{
    if(!e)return RT_TYPE20_ENTRY_UNAVAILABLE;
    /* B8BF enters below B88E's direction inheritance; preserve selector zero. */
    return selectEntry(e,e->state.spawnAction,probe,context,sound,0,0,0);
}

RT_Type20EntryResult RT_type20SelectInContext(RT_Enemy *e,u8 selector,const RT_Type20Motion *c)
{
    if(!c)return RT_TYPE20_ENTRY_UNAVAILABLE;
    return selectEntry(e,selector,c->probe,c->probeContext,c->sound,1,c->camera,c->award);
}
RT_Type20EntryResult RT_type20StartupInContext(RT_Enemy *e,const RT_Type20Motion *c)
{
    if(!e || !c)return RT_TYPE20_ENTRY_UNAVAILABLE;
    return selectEntry(e,e->state.spawnAction,c->probe,c->probeContext,c->sound,0,c->camera,c->award);
}

/* Original BEC9. Return CONTINUE only when BE9B should service its walking
 * animation timer; SELECT means an entry installed a different active state. */
RT_EnemyResult RT_type20WalkDecision(RT_Enemy *e,s32 targetX,
    RT_EnemyProbe probe,void *context,u8 *sound)
{
    if(!e || e->state.kind!=32)return RT_ENEMY_UNAVAILABLE;
    if(e->byte12&0x20)return RT_ENEMY_CONTINUE;
    u8 flags=e->state.decisions,selector;
    if(flags&1){
        u16 distance=(u16)((u32)e->state.x-(u32)targetX);
        if((s16)distance<0)distance=(u16)(0-distance);
        if((distance>>8)!=4)return RT_ENEMY_CONTINUE;
        selector=0x44;
    }else if(flags&0x20){
        if(--e->delay)return RT_ENEMY_CONTINUE;
        if(!!(flags&4)==!!(e->state.action&2))selector=0x38;
        else goto reset_delay;
    }else{
        if(!(e->variation&0x40) && --e->delay)return RT_ENEMY_CONTINUE;
        selector=flags&8?0x28:0x2c;
    }
    RT_Type20EntryResult result=RT_type20Select(e,selector,probe,context,sound);
    if(result==RT_TYPE20_ENTRY_UNAVAILABLE)return RT_ENEMY_UNAVAILABLE;
    if(result==RT_TYPE20_ENTRY_INSTALLED)return RT_ENEMY_SELECT;
reset_delay:
    /* BF0A indexes BE8B with THREE shifts: not BE6A's entry-delay index. */
    e->delay=(e->variation&0x20)?64:96;
    return RT_ENEMY_CONTINUE;
}
