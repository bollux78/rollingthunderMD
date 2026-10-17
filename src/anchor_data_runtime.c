#include "rt_anchor_data.h"
u8 RT_anchorInitialize(RT_Anchor *anchor,const RT_AnchorDefinition *definition,
    const RT_WorldCamera *camera)
{
    if(!anchor || !definition || !camera || definition->kind>3 || definition->layer>7)return 0;
    anchor->x=(s32)definition->x*16;anchor->y=(s32)definition->y*16;
    anchor->kind=definition->kind;anchor->flags=definition->flags;
    anchor->layer=definition->layer;
    anchor->phase=anchor->timer=anchor->spawnDelay=anchor->pose=0;
    RT_anchorVisible(anchor,camera);
    return 1;
}
