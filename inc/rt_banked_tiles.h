#ifndef RT_BANKED_TILES_H
#define RT_BANKED_TILES_H
#include <genesis.h>
/* Synchronous menu/scene upload. Uses region7 only, including bank-crossing
   resources; no borrowed mapper pointer survives a copy or CPU VDP transfer. */
void RT_loadBankedTiles(const u8 *source,u16 firstTile,u16 count);
#endif
