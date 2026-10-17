#include <genesis.h>
#include "rt_save.h"

extern u8 _start, _bend;
extern VoidCallback *intCB;
extern void __real__reset_entry(void);

/* SGDK warm reset deliberately retains application globals. This game needs
 * cold application state (DMA/pipeline owners, pause, menus and audio), while
 * retaining only the committed save mirror. Called by boot/sega.s with IRQs
 * masked, Z80 halted and a fresh supervisor stack above .data/.bss. */
__attribute__((externally_visible, noinline, noreturn))
void __wrap__reset_entry(void)
{
    RT_SaveReset saved;
    VoidCallback *defaultInt=intCB;
    RT_saveResetExport(&saved);
    while(GET_VDP_STATUS(VDP_DMABUSY_FLAG));
    /* The prior PCM owner may have replaced window 1. Restore fixed ROM data
     * before copying the .data image; this function executes in window 0. */
    *(vu8 *)0xa130f3=1;
    volatile u8 *dst=&_start;
    while(dst<&_bend)*dst++=0;
    const u8 *src=(const u8 *)&_stext;
    dst=(vu8 *)RAM;
    for(u32 n=(u32)&_sdata;n;n--)*dst++=*src++;
    /* Unlike _start_entry, we keep the active stack intact. The SDK reset
     * below rebuilds heap, DMA, VDP, sound and callbacks after global reset. */
    intCB=defaultInt;
    RT_saveResetImport(&saved);
    __real__reset_entry();
    while(TRUE);
}
