#include "rt_anchors.h"

typedef char RT_AnchorHasSixteenBytes[sizeof(RT_Anchor)==16?1:-1];

static u8 visible(const RT_Anchor *anchor,const RT_WorldCamera *camera)
{
    s32 left,bottom;
    if(!anchor || !camera || anchor->kind>3)return 0;
    left=(s32)camera->x*16;
    bottom=((s32)camera->y+256)*16;
    /* Comparing world bounds also avoids subtracting arbitrary distant
     * positions, and preserves the reversed Y boundary inclusivity. */
    return anchor->x>=left-0x200 && anchor->x<left+0x1400 &&
        anchor->y>bottom-0xE00 && anchor->y<=bottom-0x300;
}

u8 RT_anchorVisible(RT_Anchor *anchor,const RT_WorldCamera *camera)
{
    u8 result=visible(anchor,camera);
    if(anchor)anchor->visible=result;
    return result;
}

u8 RT_anchorFind(const RT_Anchor *anchors,u8 count,const RT_MotionBody *body,
    const RT_WorldCamera *camera,u8 lower)
{
    u16 slot;
    if(!anchors || !body || !camera)return 0;
    for(slot=0;slot<count;slot++)
    {
        const RT_Anchor *anchor=anchors+slot;
        s32 center,yOffset;
        if(!visible(anchor,camera))continue;
        if((anchor->layer==body->layer)==(lower!=0))continue;
        center=anchor->x+0xE0;
        if(body->x<=center-0x80 || body->x>=center+0x80)continue;
        yOffset=(!lower && !(anchor->flags&4))?0x470:0x3F0;
        if(body->y==anchor->y+yOffset)return (u8)(slot+1);
    }
    return 0;
}

RT_AnchorAction RT_anchorStep(RT_Anchor *anchor,u8 *sound)
{
    static const u8 sounds[3]={0x03,0x46,0x44};
    u8 phase,opening;
    if(!sound)return RT_ANCHOR_INVALID;
    *sound=0;
    if(!anchor || !anchor->visible || anchor->kind>2 || anchor->phase>4)
        return RT_ANCHOR_INVALID;
    phase=anchor->phase;
    opening=anchor->flags&1;
    if(opening)
    {
        anchor->flags&=0x3E;
        if(!phase)
        {
            anchor->phase=1;
            anchor->pose=1;
            *sound=sounds[anchor->kind];
            return RT_ANCHOR_REPLACE;
        }
        if(phase==4)return RT_ANCHOR_IDLE;
    }
    else if(!phase)
    {
        if(anchor->flags&0x3A)return RT_ANCHOR_IDLE;
        if(anchor->spawnDelay)
        {
            anchor->spawnDelay--;
            return RT_ANCHOR_IDLE;
        }
        return RT_ANCHOR_SPAWN;
    }
    anchor->timer=(anchor->timer+1)&3;
    if(anchor->timer)return RT_ANCHOR_IDLE;
    if(opening)
    {
        anchor->phase=phase+1;
        anchor->pose=phase==3?0:phase+1;
    }
    else
    {
        anchor->phase=phase-1;
        anchor->pose=phase==1?0:phase-1;
        if(phase==1)
        {
            *sound=sounds[anchor->kind];
            return RT_ANCHOR_RESTORE;
        }
    }
    return RT_ANCHOR_IDLE;
}

u8 RT_anchorPatchCell(const RT_Anchor *anchor,s16 *column,s16 *row)
{
    if(!anchor || !column || !row || anchor->kind>2)return 0;
    /* D125 uses the tile immediately above an exact downward Y edge.
     * Keep the fractional unit when reflecting the source's upward Y. */
    s32 x=anchor->x/128-(anchor->x%128<0);
    s32 y=anchor->y/128-(anchor->y%128<=0);
    if(x < -32768 || x>32767 || y < -32768 || y>32767)return 0;
    *column=(s16)x;*row=(s16)y;return 1;
}
