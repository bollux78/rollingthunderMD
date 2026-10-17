#ifndef RT_STAGE_TRANSITION_BACKGROUND_H
#define RT_STAGE_TRANSITION_BACKGROUND_H
#include <genesis.h>

typedef struct
{
    const u8 *data;
    u32 length;
    u16 patterns;
    u16 palettes[3][64];
    u8 lowerIndices[64],footerIndices[64];
    u8 lowerCount,footerCount;
} RT_StageTransitionBackground;

/* Validate/load RTPGv1 with exclusive scene VDP ownership. Leaves the display
   disabled for subsequent sprite loading. Invalid input changes neither state
   nor VDP. Caches all three palettes and changed-index lists in RAM, so a
   raster handler never reads or remaps ROM. Footer changes occupy two or three
   consecutive CRAM entries. No IRQ schedule is installed; start
   rt_stage_transition_raster separately after loading the sprites.
   The two gap tile rows use only invariant colors; lower background colors
   also survive the footer change. Tile0 is blank, patterns start512, maps
   C000/E000, SAT F000, HScroll F400, centered288-pixel viewport. */
bool RT_loadStageTransitionBackground(RT_StageTransitionBackground *state,
    const u8 *data,u32 length);
/* Pure validation/palette preparation; no VDP writes or mapper ownership. */
bool RT_prepareStageTransitionBackground(RT_StageTransitionBackground *state,
    const u8 *data,u32 length);
typedef struct {
    RT_StageTransitionBackground *background;
    u16 cursor;
    u8 phase;
    bool valid,ready;
} RT_StageTransitionBackgroundUpload;
/* Zero-initialize upload. Prepare outside the PCM I/O window; state/resource
   must remain alive. start requires an empty DMA queue and inactive SGDK
   sprite allocator; it configures/blank-disables VDP inside a safe window.
   Queue outside the window with a total DMA byte budget (minimum256); flush
   inside subsequent safe windows. finish requires the final queue drained.
   Uploads exactly the synchronous loader's blank planes, patterns and maps;
   caller separately starts sprites/raster. Does not enforce time budgets or
   claim that a queued transfer has reached VRAM. No PCM bus policy changes. */
bool RT_beginStageTransitionBackgroundUpload(RT_StageTransitionBackgroundUpload *upload,
    RT_StageTransitionBackground *state,const u8 *data,u32 length);
bool RT_startStageTransitionBackgroundUpload(RT_StageTransitionBackgroundUpload *upload);
bool RT_queueStageTransitionBackgroundUpload(RT_StageTransitionBackgroundUpload *upload,u16 budget);
bool RT_finishStageTransitionBackgroundUpload(RT_StageTransitionBackgroundUpload *upload);
#endif
