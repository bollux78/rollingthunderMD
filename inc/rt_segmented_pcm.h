#ifndef RT_SEGMENTED_PCM_H
#define RT_SEGMENTED_PCM_H
#include <genesis.h>
typedef struct {u16 page,bank,preload,remainingPages;u8 tail,preloadEnd,lastPage;} RT_SegmentedPcmPlan;
enum {RT_SEGPCM_OFF,RT_SEGPCM_STARTING,RT_SEGPCM_PLAYING,RT_SEGPCM_COMPLETE,RT_SEGPCM_LOST};
bool RT_planSegmentedPcm(u32 address,u32 count,u32 romBytes,RT_SegmentedPcmPlan *out);
/* Experimental exclusive window1 owner. Fixed code/data MUST fit below512KiB.
 * Other mapper-window1 writers are forbidden, including direct writes. SDK
 * cache checks detect some violations, not arbitrary hardware remaps. */
bool RT_startSegmentedPcm(u32 address,u32 count,u32 romBytes);
bool RT_rearmSegmentedPcm(u32 address,u32 count,u32 romBytes);
/* Intentional interruption only: resets the same owned Z80 driver, discarding
 * the old clip. Retains original window mapping for stop. Not gapless/mixing. */
bool RT_replaceSegmentedPcm(u32 address,u32 count,u32 romBytes);
u8 RT_segmentedPcmStatus(void);
/* Blocking teardown: stop raster owners before calling. Restores window1,
 * not the prior sound driver. No gapless rearm or mixing is claimed. */
bool RT_stopSegmentedPcm(void);
#endif
