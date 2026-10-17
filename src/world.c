#include "rt_world.h"
#ifndef __m68k__
#include <stddef.h>
#endif

_Static_assert(sizeof(RT_TerrainMaterial)==8,"native material stride");

const u16 (*rtWorldPages)[256];
static RT_WorldStage boundStage;
static u8 boundReady;
u8 RT_worldBind(const u16 (*cells)[256],const u16 *directory,u16 count,u8 story,u8 area)
{
    boundReady=0;rtWorldPages=0;
    if(!cells || !directory || !count)return 0;
    for(u16 i=0;i<rtWorldStageCount;i++){
        const RT_WorldStage *stage=rtWorldStages+i;
        if(stage->story!=story || stage->area!=area)continue;
        u16 entries=stage->pageColumns*((stage->heightCells+15)>>4);
        if((u32)directory<0x100000UL || (u32)directory+entries*2>0x400000UL)return 0;
        for(u16 j=0;j<entries;j++)if(directory[j]>=count)return 0;
        boundStage=*stage;boundStage.pages=directory;rtWorldPages=cells;boundReady=1;return 1;
    }
    return 0;
}
const RT_WorldStage *RT_worldStage(u8 story,u8 area)
{return boundReady && boundStage.story==story && boundStage.area==area?&boundStage:0;}

const RT_TerrainMaterial *RT_worldCell(const RT_WorldStage *stage,s16 x,s16 y)
{
    if(!stage)return NULL;
    if((u16)x>=stage->widthCells || (u16)y>=stage->heightCells)
        return rtWorldMaterials+rtWorldPaddingMaterial;
    u16 page=stage->pages[((u16)y>>4)*stage->pageColumns+((u16)x>>4)];
    u16 cell=(((u16)y&15)<<4)|((u16)x&15);
    return rtWorldMaterials+rtWorldPages[page][cell];
}

const RT_TerrainMaterial *RT_worldPoint(const RT_WorldStage *stage,s16 x,s16 y)
{
    /* Explicit negative bounds avoid implementation-defined signed shifts. */
    if(x<0 || y<0)return stage?rtWorldMaterials+rtWorldPaddingMaterial:NULL;
    return RT_worldCell(stage,(u16)x>>3,(u16)y>>3);
}

const RT_TerrainMaterial *RT_worldFixed(const RT_WorldStage *stage,s32 x,s32 y)
{
    if(!stage)return NULL;
    if(x<0 || y<0 || x>=(s32)stage->widthCells*128 ||
       y>=(s32)stage->heightCells*128)
        return rtWorldMaterials+rtWorldPaddingMaterial;
    return RT_worldCell(stage,(u16)(x>>7),(u16)(y>>7));
}

const RT_TerrainMaterial *RT_worldLocal(const RT_WorldStage *stage,
    const RT_WorldCamera *camera,s16 x,s16 y)
{
    if(!camera)return NULL;
    return RT_worldFixed(stage,(s32)camera->x*16+x,(s32)camera->y*16+y);
}

u8 RT_worldCameraBlocked(const RT_WorldStage *stage,
    const RT_WorldCamera *camera,RT_WorldDirection direction)
{
    if(!stage || !camera)return 1;
    if(stage->cameraMaxX &&
       ((direction==RT_WORLD_LEFT && camera->x<=stage->cameraMinX) ||
        (direction==RT_WORLD_RIGHT && camera->x>=stage->cameraMaxX)))return 1;
    u16 fineX=(u16)camera->x&7,fineY=(u16)camera->y&7;
    /* The source's upward Y origin becomes a ceiling-aligned native top.
     * C division truncates toward zero; adjust explicitly for signed bounds. */
    s16 left=camera->x/8-(camera->x<0 && fineX);
    s16 top=camera->y/8+(camera->y>0 && fineY);
    if(direction==RT_WORLD_LEFT || direction==RT_WORLD_RIGHT){
        if(fineX)return 0;
        u8 right=direction==RT_WORLD_RIGHT;
        s16 x=left+(right?36:-1);
        if(RT_worldCell(stage,x,top+31)->cameraBarriers&(right?2:1))return 1;
        return !!(RT_worldCell(stage,x,top+(fineY?3:4))->cameraBarriers&
                  ((right && !fineY)?2:1));
    }
    if(direction==RT_WORLD_UP || direction==RT_WORLD_DOWN){
        if(fineY)return 0;
        s16 y=top+(direction==RT_WORLD_UP?3:32);
        if(RT_worldCell(stage,left,y)->cameraBarriers&1)return 1;
        return !!(RT_worldCell(stage,left+(fineX?36:35),y)->cameraBarriers&1);
    }
    return 1;
}
