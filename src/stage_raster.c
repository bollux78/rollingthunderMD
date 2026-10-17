#include "rt_stage_raster.h"

static volatile bool rasterActive;
static volatile u16 rasterPhase;
static volatile u16 pendingScrollA,pendingScrollB;
static volatile s16 rasterWorldB;

static void writeRasterB(s16 scroll)
{
    *(vu32 *)VDP_CTRL_PORT=VDP_WRITE_VSRAM_ADDR(2);
    *(vu16 *)VDP_DATA_PORT=scroll;
}

static HINTERRUPT_CALLBACK stageRasterInterrupt(void)
{
    if (!rasterActive || rasterPhase>2) return;
    if (rasterPhase==0) {
        writeRasterB(rasterWorldB);
        VDP_setHIntCounter(167);
        rasterPhase=1;
    } else if (rasterPhase==1) {
        /* Counter reload precedes the first ISR register10 write. Consume
           the additional line48 IRQ without changing the world scroll. */
        VDP_setHIntCounter(255);
        rasterPhase=2;
    } else {
        writeRasterB((59-27)*8);
        VDP_setHIntCounter(255);
    }
}

void RT_setStageRasterScroll(u16 planeA,u16 planeB)
{
    SYS_disableInts();
    pendingScrollA=planeA&255;
    pendingScrollB=planeB&255;
    SYS_enableInts();
}

void RT_startStageRaster(void)
{
    SYS_disableInts();
    /* Keep HInt off until VInt establishes the top HUD and counter phase. */
    VDP_setHInterrupt(FALSE);
    rasterPhase=3; /* Ignore any old IRQ already pending before VInt. */
    rasterActive=TRUE;
    SYS_setHIntCallback(stageRasterInterrupt);
    SYS_enableInts();
}

void RT_stopStageRaster(void)
{
    SYS_disableInts();
    rasterActive=FALSE;
    VDP_setHInterrupt(FALSE);
    SYS_setHIntCallback(NULL);
    VDP_setHIntCounter(255);
    VDP_setVerticalScroll(BG_A,0);
    VDP_setVerticalScroll(BG_B,0);
    rasterPhase=0;
    pendingScrollA=pendingScrollB=0;
    rasterWorldB=0;
    SYS_enableInts();
}

void RT_serviceStageRasterVInt(void)
{
    if (!rasterActive) return;
    /* This representation keeps middle scanlines24..215 within world
       physical rows0..55, never reserved HUD rows56..59. */
    rasterWorldB=(s16)(((pendingScrollB+24)&255)-24);
    rasterPhase=0;
    writeRasterB(56*8);
    VDP_setVerticalScroll(BG_A,pendingScrollA);
    VDP_setHIntCounter(23);
    VDP_setHInterrupt(TRUE);
}
