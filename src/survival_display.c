#include <genesis.h>
#include "rt_survival_display.h"
static s16 floor8(s16 x){return x/8-(x<0 && ((u16)x&7));}
static u8 strip(const RT_SurvivalDisplay *s,s16 at,u8 vertical,TransferMethod method)
{
    const RT_DisplayRuntime *d=s->display;const RT_DisplayStage *source=d->stage;
    u16 cells[64],count=vertical?32:64;
    s16 start=vertical?d->row:d->column;
    for(u16 plane=0;plane<2;plane++){
        for(u16 i=0;i<count;i++){
            s16 coordinate=start+i,x=vertical?at:coordinate,y=vertical?coordinate:at;
            u16 attribute=source->padding[plane];
            if((u16)x<256 && (u16)y<64){
                u16 sx=(u16)s->world->chunks[(u16)x>>5]*32+((u16)x&31);
                attribute=RT_displayMapCell(source,plane,(u32)(u16)y*source->widthCells+sx);
            }
            cells[(u16)coordinate&(vertical?31:63)]=attribute;
        }
        u16 address=(plane?0xe000:0xc000)+(vertical?((u16)at&63)*2:((u16)at&31)*128);
        if(!DMA_transfer(method,DMA_VRAM,cells,address,count,vertical?128:2))return 0;
    }
    return 1;
}
u8 RT_survivalDisplayInitialize(RT_SurvivalDisplay *s,RT_DisplayRuntime *d,
    const RT_DisplayStage *source,const RT_SurvivalWorld *world,const RT_WorldCamera *camera)
{
    if(!s || !d || !source || !world || world->stage.pages!=world->pages ||
       source->story || source->area || source->widthCells!=608 || source->heightCells!=64 ||
       source->patternCount>640)return 0;
    for(u8 i=0;i<8;i++)if(world->chunks[i]>=19)return 0;
    s->display=0;
    if(!RT_displayConfigure(d,source,camera))return 0;
    s->display=d;s->world=world;s->firstChunk=world->firstChunk;
    VDP_loadTileData(source->patterns,0,source->patternCount,CPU);
    for(u16 y=0;y<32;y++)if(!strip(s,d->row+y,0,CPU)){s->display=0;return 0;}
    d->ready=1;RT_displayCommit(d);VDP_setEnable(TRUE);return 1;
}
u8 RT_survivalDisplayPrepare(RT_SurvivalDisplay *s,const RT_WorldCamera *camera)
{
    if(!s || !s->display || !s->display->ready || !camera || !s->world ||
       s->firstChunk!=s->world->firstChunk)return 0;
    RT_DisplayRuntime *d=s->display;
    s16 sx=camera->x-16,sy=camera->y+32,x=floor8(sx),y=floor8(sy);
    s16 dx=x-d->column,dy=y-d->row;
    if(dx< -2 || dx>2 || dy< -2 || dy>2)return 0;
    d->queuedBytes=0;d->column=x;d->row=y;
    for(s16 i=0;i<(dx<0?-dx:dx);i++){
        s16 entering=dx>0?x+64-dx+i:x-dx-1-i;
        if(!strip(s,entering,1,DMA_QUEUE_COPY)){d->ready=0;return 0;}
        d->queuedBytes+=128;
    }
    for(s16 i=0;i<(dy<0?-dy:dy);i++){
        s16 entering=dy>0?y+32-dy+i:y-dy-1-i;
        if(!strip(s,entering,0,DMA_QUEUE_COPY)){d->ready=0;return 0;}
        d->queuedBytes+=256;
    }
    d->scrollX=sx;d->scrollY=sy;return 1;
}
u8 RT_survivalDisplayRebase(RT_SurvivalDisplay *s)
{
    if(!s || !s->display || !s->display->ready || !s->world ||
       s->world->firstChunk-s->firstChunk!=2 || DMA_getQueueSize() ||
       s->display->column<64 || s->display->scrollX<512)return 0;
    s->display->column-=64;s->display->scrollX-=512;
    s->display->queuedBytes=0;s->firstChunk=s->world->firstChunk;return 1;
}
