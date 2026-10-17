#ifndef RT_SEGMENTED_MUSIC_PLAYBACK_H
#define RT_SEGMENTED_MUSIC_PLAYBACK_H
#include "rt_music_playback.h"
/* Explicit segmented-window1 admission; the linked PCM status/stop adapter
 * must own the same segmented transport. Does not relax fixed-window planning.
 * Caller supplies PCM matching the sequencer/chip history, reserves window1,
 * and provides the verified PCM-safe scene I/O schedule. Continuing scores
 * must be cancelled before their finite recorded clip drains.
 * Invalid descriptors/selectors and occupied ownership leave state untouched.
 * A source or transport failure after admission begins latches playback fault. */
bool RT_beginSegmentedMusicPlayback(RT_MusicPlayback *playback,
    RT_MusicSequence *sequence,const RT_MusicEventBank *bank,u8 request,u8 *ack,
    u32 refresh,u32 address,u32 samples,u32 romBytes,u16 eventBudget,
    RT_MusicRegisterWrite write,void *context);
#endif
