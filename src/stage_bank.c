#include "rt_stage_bank.h"
#ifdef __m68k__
u8 RT_stageBankSelect(u8 firstPage)
{
    if(firstPage<2 || firstPage>58 || (VDP_getReg(1)&0x40) ||
       DMA_getQueueSize() || SPR_isInitialized())return 0;
    /* Packer records the physical image's inclusive end in the ROM header.
     * Reject absent pages rather than relying on emulator address wrapping. */
    u32 end=*(const volatile u32*)0x1a4;
    if(end>=0x2000000UL || (end&0x7ffffUL)!=0x7ffffUL ||
       ((u32)firstPage+6)*0x80000UL>end+1)return 0;
    /* No VInt may observe partially installed windows. All six mappings are
     * stable throughout subsequent gameplay, including queued ROM DMA. */
    SYS_disableInts();
    for(u16 region=2;region<8;region++)SYS_setBank(region,firstPage+region-2);
    SYS_enableInts();
    return 1;
}
#endif
