#include "rt_banked_tiles.h"
#ifdef SGDK_GCC
__attribute__((noinline))
#endif
void RT_loadBankedTiles(const u8 *source,u16 firstTile,u16 count)
{
    u32 pixels[64];
    while(count){
        u16 tiles=count>8?8:count,remaining=tiles*32;
        u8 *destination=(u8 *)pixels;
        while(remaining){
            u32 available=0x80000UL-((u32)source&0x7FFFFUL);
            u16 bytes=available<remaining?available:remaining;
            memcpy(destination,SYS_getFarDataSafeEx((void *)source,bytes,TRUE),bytes);
            source+=bytes;destination+=bytes;remaining-=bytes;
        }
        VDP_loadTileData(pixels,firstTile,tiles,CPU);
        firstTile+=tiles;count-=tiles;
    }
}
