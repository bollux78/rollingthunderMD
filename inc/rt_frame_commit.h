#ifndef RT_NATIVE_FRAME_COMMIT_H
#define RT_NATIVE_FRAME_COMMIT_H
#include "rt_display_runtime.h"
/* NTSC H40/224-only late admission, bounded by remaining VBlank transfer
 * budget including per-entry overhead. Otherwise wait for VBlank start.
 * Returns1 for current-blank commit. Main-loop caller must audit refresh and RT_frameCommitVideoEnd(). Added audio, palette fades or callbacks require revalidation. */
u8 RT_frameCommit(RT_DisplayRuntime *display,u32 previousRefresh);
/* Final native graphics-write boundary, before controller polling. Caller
 * audits this instead of the return scanline: controller IO may cross VBlank.
 * Requires frame-load display disabled (the native scenes' normal setting). */
u16 RT_frameCommitVideoEnd(void);
#endif
