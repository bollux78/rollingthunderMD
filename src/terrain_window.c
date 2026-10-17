#include "rt_terrain_window.h"
static s16 floor8(s16 x){return x/8-(x<0 && ((u16)x&7));}
static s16 ceil8(s16 y){return y/8+(y>0 && ((u16)y&7));}
static void publishStrip(RT_TerrainWindow *window,const RT_WorldStage *stage,
    s16 x,s16 y,u16 count,u8 vertical)
{
    while(count){
        u16 sx=(u16)x,sy=(u16)y;
        if(sx>=stage->widthCells || sy>=stage->heightCells){
            window->materials[(sy&31)*64+(sx&63)]=rtWorldPaddingMaterial;
            if(vertical)y++;else x++;
            count--;continue;
        }
        u16 run=16-((vertical?sy:sx)&15);
        u16 edge=vertical?stage->heightCells-sy:stage->widthCells-sx;
        if(run>count)run=count;
        if(run>edge)run=edge;
        const u16 *cells=rtWorldPages[stage->pages[(sy>>4)*stage->pageColumns+(sx>>4)]]+
            ((sy&15)*16)+(sx&15);
        u16 index=(sy&31)*64+(sx&63);
        count-=run;
        if(vertical){
            y+=run;
            do{window->materials[index]=*cells;if(run>1)cells+=16;index=(index+64)&2047;}while(--run);
        }else{
            x+=run;
            do{window->materials[index++]=*cells++;}while(--run);
        }
    }
}
u8 RT_terrainWindowInitialize(RT_TerrainWindow *window,const RT_WorldStage *stage,
    const RT_WorldCamera *camera)
{
    if(!window || !stage || !camera)return 0;
    window->ready=0;window->camera=*camera;
    for(u16 i=0;i<2048;i++)window->materials[i]=rtWorldPaddingMaterial;
    s16 left=floor8(camera->x)-4,top=ceil8(camera->y)+2;
    for(u16 y=0;y<32;y++)publishStrip(window,stage,left,top+y,44,0);
    window->ready=1;return 1;
}
u8 RT_terrainWindowMove(RT_TerrainWindow *window,const RT_WorldStage *stage,
    const RT_WorldCamera *camera)
{
    if(!window || !window->ready || !stage || !camera)return 0;
    s32 dx=(s32)camera->x-window->camera.x,dy=(s32)camera->y-window->camera.y;
    if(dx < -16 || dx>16 || dy < -16 || dy>16)return 0;
    if(!dx && !dy)return 1;
    /* Publication changes only at tile boundaries. Preserve horizontal then
     * vertical order and every crossed strip, including corner overwrites. */
    s16 cell=floor8(window->camera.x),end=floor8(camera->x);
    s16 top=ceil8(window->camera.y)+2;
    while(cell!=end){
        cell+=dx>0?1:-1;
        s16 x=cell+(dx>0?39:-4);
        publishStrip(window,stage,x,top,32,1);
    }
    s16 left=end-4;
    cell=ceil8(window->camera.y);end=ceil8(camera->y);
    while(cell!=end){
        cell+=dy>0?1:-1;
        s16 y=cell+(dy>0?33:2);
        publishStrip(window,stage,left,y,44,0);
    }
    window->camera=*camera;
    return 1;
}
const RT_TerrainMaterial *RT_terrainWindowProbe(const RT_TerrainWindow *window,
    s32 x,s32 y,s16 cellX,s16 cellY)
{
    if(!window || !window->ready)return 0;
    /* Source feet sample one fixed-point unit above the native anchor. */
    /* Only six X and five Y cell bits address the published ring. Taking
     * the low word before unsigned shifts preserves floor(x/128) modulo64
     * and floor((y-1)/128) modulo32, including negative anchors. */
    u16 cx=(u16)x>>7;
    u16 cy=(u16)((u16)y-1)>>7;
    u16 index=(((u16)(cy+cellY)&31)*64)+((u16)(cx+cellX)&63);
    return rtWorldMaterials+window->materials[index];
}
