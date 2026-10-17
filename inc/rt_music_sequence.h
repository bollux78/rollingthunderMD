#ifndef RT_MUSIC_SEQUENCE_H
#define RT_MUSIC_SEQUENCE_H
#include "rt_music_control.h"
#include "rt_music_registers.h"
typedef enum {
 RT_MUSIC_EVENT_GAIN,RT_MUSIC_EVENT_BASE,RT_MUSIC_EVENT_MULTIPLIER,
 RT_MUSIC_EVENT_NOTES,RT_MUSIC_EVENT_INSTRUMENTS,RT_MUSIC_EVENT_CHANNEL_GAINS,
 RT_MUSIC_EVENT_FRACTION,RT_MUSIC_EVENT_PITCH,RT_MUSIC_EVENT_MODULATION,
 RT_MUSIC_EVENT_LFO,RT_MUSIC_EVENT_PHASE_DEPTH,RT_MUSIC_EVENT_AMPLITUDE_DEPTH,
 RT_MUSIC_EVENT_WAVEFORM,RT_MUSIC_EVENT_END,RT_MUSIC_EVENT_CALL,
 RT_MUSIC_EVENT_TRACK,RT_MUSIC_EVENT_JUMP,RT_MUSIC_EVENT_YIELD,RT_MUSIC_EVENT_REPEAT,RT_MUSIC_EVENT_CONDITIONAL
} RT_MusicEventKind;
typedef struct {
 u16 value;
 u8 kind,mask; /* mask bit N selects channel N */
 const u8 *data; /* eight indexed values or eight 28-byte instruments */
} RT_MusicEvent;
typedef struct {
 const RT_MusicEvent *events;
 const u16 *tracks;
 u16 eventCount,trackCount;
} RT_MusicEventBank;
/* Extracted game-over track0C; other selectors are unavailable in this bank. */
extern const RT_MusicEventBank rtGameOverMusicBank;
/* Selectable tracks1..13 plus all reachable subtracks. */
extern const RT_MusicEventBank rtArcadeMusicBank;
/* Tracks0B/0C and every reachable subtrack; compact native event indices. */
extern const RT_MusicEventBank rtGameOverChainMusicBank;
/* Victory07, ending09 and initials0B, with every reachable musical subtrack. */
extern const RT_MusicEventBank rtFinalMusicBank;
typedef struct {RT_MusicControl control;RT_MusicRegisters registers;} RT_MusicSequence;
/* One source sequencer service, operating on build-time-converted musical
   data, not original CPU code. Emits the source FM register contract, NOT
   YM2612 addresses. Notes yield; control/register events continue immediately.
   budget bounds malformed zero-time cycles; INVALID must abort playback and
   must never be reported to gameplay as successful sound completion. */
RT_MusicControlAction RT_serviceMusicSequence(RT_MusicSequence *state,
 const RT_MusicEventBank *bank,u16 budget,RT_MusicRegisterWrite write,void *context);
typedef struct {
 u32 refresh,remainder,services,endRefresh;
 bool fault;
} RT_MusicRefreshState;
typedef enum {RT_MUSIC_REFRESH_FAULT,RT_MUSIC_REFRESH_CURRENT,
 RT_MUSIC_REFRESH_PENDING} RT_MusicRefreshResult;
/* Initialize refresh to the hardware counter at admission, other fields zero.
   Catch up to a snapshot of the actual NTSC VInt counter, not gameplay frames.
   maxRefreshes bounds this call; PENDING preserves the backlog for a later call.
   Forward counter deltas wrap naturally; stale snapshots or gaps over 2^31-1
   refreshes fault. Source service callbacks remain ordered. endRefresh records
   the last nonzero-to-zero sequence status transition, not PCM completion.
   Commands use RT_publishMusicCommand separately. The admission service may
   run immediately before starting this clock; services counts only later work.
   A fault is latched: caller must abort the session and must not expose its
   sequence status as successful completion. No PCM or mapper access occurs. */
RT_MusicRefreshResult RT_serviceMusicSequenceRefreshes(RT_MusicRefreshState *clock,
 RT_MusicSequence *state,const RT_MusicEventBank *bank,u32 targetRefresh,
 u16 maxRefreshes,u16 eventBudget,RT_MusicRegisterWrite write,void *context);
#endif
