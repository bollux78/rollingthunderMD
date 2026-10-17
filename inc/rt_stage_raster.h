#ifndef RT_STAGE_RASTER_H
#define RT_STAGE_RASTER_H
#include <genesis.h>
/* Fixed HUD in B rows56..59, world in logical rows0..31 and safe aliases.
   Caller must install that layout and protect foreground VDP accesses before
   enabling this driver. Starting does not change VSRAM until the next VInt. */
void RT_startStageRaster(void);
/* Disable HInt and detach its callback before resetting both vertical scrolls.
   Call before replacing the plane layout or entering menus. */
void RT_stopStageRaster(void);
/* Publish both world offsets atomically. They take effect together at VInt. */
void RT_setStageRasterScroll(u16 planeA,u16 planeB);
/* Chain from the application's existing VInt callback; does not own VInt. */
void RT_serviceStageRasterVInt(void);
#endif
