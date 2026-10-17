#ifndef RT_MUSIC_PLAYBACK_H
#define RT_MUSIC_PLAYBACK_H
#include "rt_music_sequence.h"
#include "rt_pcm_rom_driver.h"
/* Exclusive finite-clip playback. Caller supplies a matching pre-rendered
   clip and the persistent source sequence (never reset it between tracks).
   This does not provide mixing, looping, clip selection or a safe bus window;
   the frame owner must satisfy the PCM driver's scheduling contract. */
typedef struct {
 RT_MusicSequence *sequence;
 const RT_MusicEventBank *bank;
 RT_MusicRefreshState clock;
 RT_PcmRomStatus pcm;
 bool active,fault,commandStopped;
} RT_MusicPlayback;
/* Zero-initialize playback before first use. A repeated acknowledged request
   is inert. Invalid or failed admission must not be exposed as completion. */
bool RT_beginMusicPlayback(RT_MusicPlayback *playback,RT_MusicSequence *sequence,
 const RT_MusicEventBank *bank,u8 request,u8 *ack,u32 refresh,
 u32 address,u32 samples,u32 romBytes,u16 eventBudget,
 RT_MusicRegisterWrite write,void *context);
/* Explicit long-clip admission. windows=2 reserves CPU mapper regions5/6;
   the owner must use compatible resources throughout playback. Descriptor
   validation precedes source request/ack mutation. No loop/mixing support or
   automatic reservation of resources belonging to another scene owner. */
bool RT_beginMusicPlaybackWindows(RT_MusicPlayback *playback,RT_MusicSequence *sequence,
 const RT_MusicEventBank *bank,u8 request,u8 *ack,u32 refresh,
 u32 address,u32 samples,u32 romBytes,u16 windows,u16 eventBudget,
 RT_MusicRegisterWrite write,void *context);
/* Replace a session at an already serviced source-command boundary. Preserves
   the SAME sequencer (including tempo/register/loop state); never injects E8=0
   or reports the interrupted clip as drained. Rejects invalid descriptors,
   absent track selectors and repeated requests before touching the device.
   Caller must first catch the clock up to refresh and stop raster/blank display:
   resident device replacement interrupts the old clip and is not gapless.
   Window count must match the current session; no null-driver/chip reload.
   Driver/sequence failure latches fault; do not retry or expose completion. */
bool RT_replaceMusicPlaybackWindows(RT_MusicPlayback *playback,
 const RT_MusicEventBank *bank,u8 request,u8 *ack,u32 refresh,
 u32 address,u32 samples,u32 romBytes,u16 windows,u16 eventBudget,
 RT_MusicRegisterWrite write,void *context);
RT_MusicRefreshResult RT_serviceMusicPlayback(RT_MusicPlayback *playback,
 u32 refresh,u16 maxRefreshes,u16 eventBudget,
 RT_MusicRegisterWrite write,void *context);
/* Source4380 status and PCM drain are separate. A fault leaves outputs intact.
   A valid zero source status may precede the final PCM sample. Confirmed
   completion remains readable after device release, until the next admission.
   Generic cancellation before confirmed completion makes status unavailable.
   An accepted source stop command instead exposes status0; drained stays FALSE
   when the clip was interrupted rather than naturally drained. */
bool RT_readMusicPlaybackStatus(const RT_MusicPlayback *playback,
 u8 *sourceStatus,bool *drained);
/* Releases only a session this controller admitted; driver ownership loss
   fails without unloading another owner. Does not reset source bookkeeping. */
bool RT_stopMusicPlayback(RT_MusicPlayback *playback);
/* Apply source E8=0 at the caller's command boundary, after servicing preceding
   refreshes. Release only this controller's device, then publish/acknowledge
   command0. Failure never acknowledges a successful source stop. Preserves
   tempo, gain and loop bookkeeping; does not fabricate a terminal endRefresh.
   Successful repeat calls are inert. This is explicit cancellation, not a
   natural track-end/drain event or a replacement for overlapping audio mixing. */
bool RT_stopMusicPlaybackCommand(RT_MusicPlayback *playback,u8 *ack);
#endif
