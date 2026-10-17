#include "rt_survival_world.h"
#include "rt_survival_layout_data.h"

u8 RT_survivalWorldBuild(RT_SurvivalWorld *world,const RT_WorldStage *source,
    const RT_SurvivalWindow *window)
{
    if(!world || !source || !source->pages || !window || !window->initialized ||
       window->head>=RT_SURVIVAL_WINDOW_CHUNKS || source->story || source->area ||
       source->widthCells!=608 || source->heightCells!=64 || source->pageColumns!=38)return 0;
    RT_SurvivalWorld next={0};
    for(u8 chunk=0;chunk<RT_SURVIVAL_WINDOW_CHUNKS;chunk++){
        u8 selected=window->chunks[(window->head+chunk)&(RT_SURVIVAL_WINDOW_CHUNKS-1)];
        if(selected>=RT_SURVIVAL_SOURCE_CHUNKS)return 0;
        next.chunks[chunk]=selected;
        for(u8 row=0;row<4;row++){
            u16 from=(u16)row*38+(u16)selected*2;
            u16 to=(u16)row*(RT_SURVIVAL_WINDOW_CHUNKS*2)+(u16)chunk*2;
            next.pages[to]=source->pages[from];
            next.pages[to+1]=source->pages[from+1];
        }
    }
    next.stage=*source;
    next.stage.widthCells=RT_SURVIVAL_WINDOW_CHUNKS*32;
    next.stage.pageColumns=RT_SURVIVAL_WINDOW_CHUNKS*2;
    next.stage.entrances[0]=next.stage.entrances[1]=(RT_WorldCamera){0,-8};
    next.stage.cameraMinX=window->firstChunk==0 && window->stream.emitted==8?0:256;
    /* Keep the 320px display viewport in retained terrain. The wider VDP
     * prefetch ring may contain padding; retirement normally occurs earlier. */
    next.stage.cameraMaxX=next.stage.widthCells*8-320;
    next.firstChunk=window->firstChunk;
    next.densityTier=window->firstChunk>=64?4:(u8)(window->firstChunk/16);
    *world=next;
    world->stage.pages=world->pages;
    return 1;
}

u8 RT_survivalWorldSourceCell(const RT_SurvivalWorld *world,s16 localX,
    s16 localY,u16 *sourceX,u16 *sourceY)
{
    if(!world || !sourceX || !sourceY || world->stage.pages!=world->pages ||
       (u16)localX>=RT_SURVIVAL_WINDOW_CHUNKS*32 || (u16)localY>=64)return 0;
    u8 chunk=world->chunks[(u16)localX>>5];
    if(chunk>=RT_SURVIVAL_SOURCE_CHUNKS)return 0;
    *sourceX=(u16)chunk*32+((u16)localX&31);
    *sourceY=localY;
    return 1;
}
