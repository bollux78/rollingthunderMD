#include "rt_campaign_transition_art.h"
u8 RT_transitionSpritesPrepare(const RT_TransitionArt *art,const RT_StagePool *pool,
    RT_SpriteBatch *batch,u32 budgetDelta[225])
{
    if(!batch)return 0;
    batch->failed=1;batch->count=batch->queuedBytes=0;
    if(!art || !art->parts || !pool || !budgetDelta || pool->count>48)return 0;
    for(u16 row=0;row<225;row++)budgetDelta[row]=0;
    u8 population=0;
    for(u8 slot=48;slot--;){
        const RT_Anchor *actor=pool->objects+slot;
        if(actor->kind==255)continue;
        population++;
        if(actor->kind<8 || !actor->visible)continue;
        if(actor->kind<17 || actor->kind>31 || actor->layer!=6)return 0;
        const RT_TransitionPose *pose=art->poses+(actor->kind-17);
        if(!pose->count || pose->first>art->partCount || pose->count>art->partCount-pose->first)return 0;
        s32 actorX=actor->x>>4,actorY=(4096-actor->y)>>4;
        for(u16 j=pose->count;j--;){
            const RT_TransitionPart *part=art->parts+pose->first+j;
            s32 cx=actorX+part->dx,cy=actorY+part->dy;
            if(cx< -31 || cx>287 || cy< -31 || cy>223)continue;
            s16 x=(s16)cx+16+part->left;
            s16 y=-(u8)cy-part->sourceHeight-16;
            y=(((u16)(y+16)&255)-16)-16+part->top;
            u16 width=((part->size>>2)+1)*8,height=((part->size&3)+1)*8;
            if(x+width<=16 || x>=304 || y+(s16)height<=0 || y>=224)continue;
            if(x<16 || x+width>304 || batch->count==80)return 0;
            RT_SpriteCommand *command=batch->commands+batch->count;
            command->x=x+128;command->y=y+128;command->size=part->size;
            command->attribute=part->attribute;command->link=++batch->count;
            /* Already in hardware overlap order; do not run generic sorting. */
            batch->orderKeys[batch->count-1]=0;
            u16 first=y<0?0:y,last=y+(s16)height>224?224:y+height;
            budgetDelta[first]+=0x10000UL+width;
            budgetDelta[last]-=0x10000UL+width;
        }
    }
    if(population!=pool->count)return 0;
    u32 budget=0;
    for(u16 row=0;row<224;row++){
        budget+=budgetDelta[row];
        if(budget>0x14ffffUL || (u16)budget>320)return 0;
    }
    if(batch->count)batch->commands[batch->count-1].link=0;
    else batch->commands[0]=(RT_SpriteCommand){0};
    batch->failed=0;return 1;
}

#ifdef __m68k__
u8 RT_transitionArtUploadIO(const RT_TransitionArt *art,void (*beginIO)(void),void (*endIO)(void))
{
    if((!beginIO != !endIO) || !art || !art->patterns || !art->palette || !art->tileCount ||
       art->tileCount>768 || art->upperTile<512 || art->upperTile>1280 ||
       (art->tileCount>256 && art->tileCount-256>1536-art->upperTile))return 0;
    for(u16 tile=0;tile<art->tileCount;){
        u16 count=art->tileCount-tile;if(count>32)count=32;
        if(beginIO)beginIO();
        VDP_loadTileData(art->patterns+(u32)tile*8,
            tile<256?256+tile:art->upperTile+tile-256,count,DMA);
        if(endIO)endIO();tile+=count;
    }
    if(beginIO)beginIO();
    PAL_setColors(0,art->palette,64,CPU);
    if(endIO)endIO();
    return 1;
}
u8 RT_transitionArtUpload(const RT_TransitionArt *art)
{return RT_transitionArtUploadIO(art,0,0);}
u8 RT_transitionSpritesPublish(RT_SpriteBatch *batch)
{
    typedef char SatSizeCheck[sizeof(RT_SpriteCommand)==8?1:-1];
    (void)sizeof(SatSizeCheck);
    if(!batch || batch->failed || batch->count>80)return 0;
    /* A complete high/low scene can approach the80-entry SAT limit after a
     * changed low-pattern upload. CPU port writes overrun NTSC blanking in
     * those frames; synchronous DMA keeps both transfers in that interval. */
    if(!DMA_transfer(DMA,DMA_VRAM,batch->commands,0xf000,batch->count?batch->count*4:4,2))return 0;
    batch->failed=1;return 1;
}
#endif
