#include <genesis.h>
#include "rt_frame_commit.h"
/* This SDK exports its callback pointer; retain the installed callback when
 * inserting the native display commit before SYS's controller polling. */
extern VoidCallback *vblankCB;
static VoidCallback *previousCallback;
static RT_DisplayRuntime *pendingDisplay;
static u16 videoEndLine;
static void commitDisplay(void)
{
    if(previousCallback)previousCallback();
    RT_displayCommit(pendingDisplay);
    videoEndLine=GET_VCOUNTER;
}
u16 RT_frameCommitVideoEnd(void){return videoEndLine;}
#ifdef RT_FRAME_COMMIT_TRACE
volatile u16 rtFrameCommitTrace[8];
volatile u32 rtFrameCommitTransfers[48];
#endif
#ifdef RT_FRAME_SYSTEM_TRACE
extern volatile u16 rtCommitSystemArmed;
#endif
u8 RT_frameCommit(RT_DisplayRuntime *display,u32 previousRefresh)
{
    previousCallback=vblankCB;pendingDisplay=display;
    SYS_setVBlankCallback(commitDisplay);
    u16 line=GET_VCOUNTER;
    u8 early=0;
    if(vtimer==previousRefresh+1 && line>=224 && line<=238 &&
        !IS_PAL_SYSTEM && VDP_getScreenWidth()==320 && VDP_getScreenHeight()==224){
        u16 bytes=DMA_getQueueTransferSize(),entries=DMA_getQueueSize();
        /* NTSC VCounter229..234 can occur twice; use the later occurrence.
         * Stop budgeting at counter244, with additional headroom for platform work.
         * 160 bytes/line is below H40's204; reserve64 bytes per queue entry
         * for setup/dispatch, and bound entry count independently. */
        u16 budget=(244-line)*160;
        if(display->queuedBytes>bytes)bytes=display->queuedBytes;
        early=entries<=16 && bytes<=3072 && (u32)bytes+(u32)entries*64<=budget;
    }
#ifdef RT_FRAME_COMMIT_TRACE
    rtFrameCommitTrace[0]=line;
    rtFrameCommitTrace[1]=DMA_getQueueTransferSize();
    rtFrameCommitTrace[2]=DMA_getQueueSize();
    rtFrameCommitTrace[3]=early;
    for(u16 i=0;i<16;i++){
        if(i<rtFrameCommitTrace[2]){
            DMAOpInfo *op=dmaQueues+i;
            rtFrameCommitTransfers[i*3]=((op->regLenL&255)|((op->regLenH&255)<<8))*2;
            rtFrameCommitTransfers[i*3+1]=((op->regAddrMStep&0xff0000)>>7)|((op->regAddrHAddrL&0x7f00ff)<<1);
            rtFrameCommitTransfers[i*3+2]=op->regCtrlWrite;
        }else rtFrameCommitTransfers[i*3]=0;
    }
    rtFrameCommitTrace[4]=GET_VCOUNTER;
#endif
#ifdef RT_FRAME_SYSTEM_TRACE
    rtCommitSystemArmed=1;
#endif
    SYS_doVBlankProcessEx(early?ON_VBLANK:ON_VBLANK_START);
#ifdef RT_FRAME_SYSTEM_TRACE
    rtCommitSystemArmed=0;
#endif
#ifdef RT_FRAME_COMMIT_TRACE
    rtFrameCommitTrace[5]=GET_VCOUNTER;
    rtFrameCommitTrace[6]=DMA_getQueueSize();
#endif
    SYS_setVBlankCallback(previousCallback);
    pendingDisplay=0;
#ifdef RT_FRAME_COMMIT_TRACE
    rtFrameCommitTrace[7]=videoEndLine;
#endif
    return early;
}
