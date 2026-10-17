#include "rt_tile_cache.h"
u8 RT_tileCacheInitialize(RT_TileCache *cache,u16 patterns,u16 first,u16 slots)
{
    if(!cache || !patterns || patterns>RT_TILE_CACHE_PATTERNS || !slots ||
       slots>RT_TILE_CACHE_SLOTS || first>=1536 || slots>1536-first)return 0;
    cache->ready=0;
    for(u16 i=0;i<patterns;i++)cache->mapping[i]=0;
    for(u16 i=0;i<slots;i++){
        cache->references[i]=0;cache->pattern[i]=65535;
        cache->previousFree[i]=i?i-1:65535;
        cache->nextFree[i]=i+1<slots?i+1:65535;
    }
    cache->patternCount=patterns;cache->slots=slots;cache->firstTile=first;
    cache->freeHead=0;cache->freeTail=slots-1;cache->ready=1;return 1;
}
static void removeFree(RT_TileCache *cache,u16 slot)
{
    u16 previous=cache->previousFree[slot],next=cache->nextFree[slot];
    if(previous!=65535)cache->nextFree[previous]=next;else cache->freeHead=next;
    if(next!=65535)cache->previousFree[next]=previous;else cache->freeTail=previous;
}
u8 RT_tileCacheAcquire(RT_TileCache *cache,u16 attribute,u16 *physical,
    RT_TileCacheUpload upload,void *context)
{
    if(!cache || !cache->ready || !physical)return 0;
    u16 pattern=attribute&2047;
    if(pattern>=cache->patternCount)return 0;
    u16 mapped=cache->mapping[pattern],slot;
    if(mapped){
        slot=mapped-1;
        if(cache->references[slot]==65535)return 0;
    }else{
        if(!upload)return 0;
        slot=cache->freeHead;if(slot==65535)return 0;
        if(!upload(pattern,cache->firstTile+slot,context))return 0;
        u16 old=cache->pattern[slot];
        if(old!=65535)cache->mapping[old]=0;
        cache->pattern[slot]=pattern;cache->mapping[pattern]=slot+1;

    }
    if(!cache->references[slot])removeFree(cache,slot);
    cache->references[slot]++;
    *physical=(attribute&0xf800)|(cache->firstTile+slot);return 1;
}
u8 RT_tileCacheRelease(RT_TileCache *cache,u16 attribute)
{
    if(!cache || !cache->ready)return 0;
    u16 tile=attribute&2047;
    if(tile<cache->firstTile || tile-cache->firstTile>=cache->slots)return 0;
    u16 slot=tile-cache->firstTile;
    if(!cache->references[slot])return 0;
    if(!--cache->references[slot]){
        cache->previousFree[slot]=cache->freeTail;cache->nextFree[slot]=65535;
        if(cache->freeTail!=65535)cache->nextFree[cache->freeTail]=slot;
        else cache->freeHead=slot;
        cache->freeTail=slot;
    }
    return 1;
}
