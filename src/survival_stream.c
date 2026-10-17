#include "rt_survival_stream.h"
#include "rt_survival_layout_data.h"

void RT_initSurvivalStream(RT_SurvivalStream *stream,u32 seed)
{
    if(!stream)return;
    stream->randomState=seed?seed:0x9E3779B9UL;
    stream->emitted=0;
    stream->previous=stream->previousPrevious=255;
    stream->prefixIndex=0;
    stream->initialized=TRUE;
}

bool RT_nextSurvivalChunk(RT_SurvivalStream *stream,u8 *sourceChunk)
{
    u8 selected;
    if(!stream || !sourceChunk || !stream->initialized)return FALSE;
    if(stream->prefixIndex<sizeof(rtSurvivalPrefix))
        selected=rtSurvivalPrefix[stream->prefixIndex++];
    else {
        u32 choices,varied,random;
        u8 count=0,index,rank;
        if(stream->previous>=RT_SURVIVAL_SOURCE_CHUNKS)return FALSE;
        choices=rtSurvivalSuccessors[stream->previous];
        if(!choices)return FALSE;
        varied=choices&~(1UL<<stream->previous);
        if(stream->previousPrevious<RT_SURVIVAL_SOURCE_CHUNKS)
            varied&=~(1UL<<stream->previousPrevious);
        if(varied)choices=varied;
        for(index=0;index<RT_SURVIVAL_SOURCE_CHUNKS;index++)
            if(choices&(1UL<<index))count++;
        random=stream->randomState;
        random^=random<<13;random^=random>>17;random^=random<<5;
        stream->randomState=random;
        /* Keep full 32-bit seed ordering without libgcc's __umodsi3:
           this toolchain's helper uses a 68020-only long branch. The
           running remainder is always below count (at most 19). */
        rank=0;
        for(index=0;index<32;index++){
            rank=(rank<<1)|(random>>31);
            random<<=1;
            if(rank>=count)rank-=count;
        }
        selected=0;
        for(index=0;index<RT_SURVIVAL_SOURCE_CHUNKS;index++)
            if(choices&(1UL<<index)){
                if(!rank){selected=index;break;}
                rank--;
            }
    }
    stream->previousPrevious=stream->previous;
    stream->previous=selected;
    /* Saturate the reporting counter; the deterministic stream continues. */
    if(stream->emitted!=0xFFFFFFFFUL)stream->emitted++;
    *sourceChunk=selected;
    return TRUE;
}

bool RT_initSurvivalWindow(RT_SurvivalWindow *window,u32 seed)
{
    RT_SurvivalWindow next;
    u8 index;
    if(!window)return FALSE;
    RT_initSurvivalStream(&next.stream,seed);
    next.firstChunk=0;next.head=0;next.initialized=TRUE;
    for(index=0;index<RT_SURVIVAL_WINDOW_CHUNKS;index++)
        if(!RT_nextSurvivalChunk(&next.stream,&next.chunks[index]))return FALSE;
    *window=next;
    return TRUE;
}

bool RT_advanceSurvivalWindow(RT_SurvivalWindow *window)
{
    RT_SurvivalStream next;
    u8 chunk;
    if(!window || !window->initialized || window->head>=RT_SURVIVAL_WINDOW_CHUNKS)
        return FALSE;
    next=window->stream;
    if(!RT_nextSurvivalChunk(&next,&chunk))return FALSE;
    window->stream=next;
    window->chunks[window->head]=chunk;
    window->head=(window->head+1)&(RT_SURVIVAL_WINDOW_CHUNKS-1);
    window->firstChunk++;
    return TRUE;
}

bool RT_resolveSurvivalPosition(const RT_SurvivalWindow *window,
    u32 logicalChunk,u8 pixelOffset,u16 *sourceX)
{
    u32 offset;
    u8 chunk;
    if(!window || !window->initialized || !sourceX ||
       window->head>=RT_SURVIVAL_WINDOW_CHUNKS)return FALSE;
    /* Unsigned distance also covers the logical serial counter's wrap. */
    offset=logicalChunk-window->firstChunk;
    if(offset>=RT_SURVIVAL_WINDOW_CHUNKS)return FALSE;
    chunk=window->chunks[(window->head+(u8)offset)&(RT_SURVIVAL_WINDOW_CHUNKS-1)];
    if(chunk>=RT_SURVIVAL_SOURCE_CHUNKS)return FALSE;
    *sourceX=((u16)chunk<<8)|pixelOffset;
    return TRUE;
}

bool RT_coverSurvivalWindow(RT_SurvivalWindow *window,
    u32 firstNeeded,u32 lastNeeded)
{
    RT_SurvivalWindow next;
    u32 firstOffset,lastOffset,advance;
    u16 source;
    u8 index;
    if(!window || !window->initialized || window->head>=RT_SURVIVAL_WINDOW_CHUNKS)
        return FALSE;
    /* Serial distances cover counter wrap. A request may retain any span of
       at most eight chunks, but cannot recover an already retired chunk. */
    if(lastNeeded-firstNeeded>=RT_SURVIVAL_WINDOW_CHUNKS)return FALSE;
    firstOffset=firstNeeded-window->firstChunk;
    lastOffset=lastNeeded-window->firstChunk;
    if(firstOffset>=16 || lastOffset>=16)return FALSE;
    advance=lastOffset<RT_SURVIVAL_WINDOW_CHUNKS?0:
        lastOffset-(RT_SURVIVAL_WINDOW_CHUNKS-1);
    if(advance>firstOffset)return FALSE;
    next=*window;
    for(index=0;index<advance;index++)
        if(!RT_advanceSurvivalWindow(&next))return FALSE;
    for(index=0;index<=(u8)(lastNeeded-firstNeeded);index++)
        if(!RT_resolveSurvivalPosition(&next,firstNeeded+index,0,&source))return FALSE;
    *window=next;
    return TRUE;
}
