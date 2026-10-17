#ifndef RT_MUSIC_CONTROL_H
#define RT_MUSIC_CONTROL_H
#include "rt_music_clock.h"

typedef struct {
    RT_MusicClock clock;
    u16 cursor,returnCursor;
    u8 status,returnDepth;
    u8 loopCount[8],lastLoop;
    u16 loopKey[8];
} RT_MusicControl;
/* CPU1 AFC9..AFD5: publish E8 only when it differs from acknowledged E9.
   The MCU owns subsequent status changes (request -> FF -> 0). Unchanged
   requests must not overwrite either active or completed status. A changed
   zero request stops sequencing. This status is NOT the PCM drain flag. */
bool RT_publishMusicCommand(RT_MusicControl *state,u8 requestedE8,u8 *acknowledgedE9);
typedef enum {
    RT_MUSIC_CONTROL_INVALID,RT_MUSIC_CONTROL_IDLE,
    RT_MUSIC_CONTROL_WAIT,RT_MUSIC_CONTROL_EVENTS
} RT_MusicControlAction;
/* Maps a track selector to a native music-data cursor. No CPU instructions
   are interpreted. Failure is inert; callers must provide extracted tracks. */
typedef bool (*RT_MusicTrackLookup)(u8 selector,u16 *cursor,void *context);
/* MCU8481..84C4. status is shared1380 (CPU1 sees4380): 0=idle, FF=active,
   other bytes=requested track. IDLE requires native idle-silence writes;
   EVENTS dispatches the next native event group. A new track starts without
   advancing/resetting the tempo accumulator. WAIT has advanced its fraction. */
RT_MusicControlAction RT_beginMusicControlService(RT_MusicControl *state,
    RT_MusicTrackLookup lookup,void *context);
/* Data control handlers85E5/85FF: both use848F track installation, preserving
   tempo and resetting only the eight source166E+3*n control bytes. The call
   handler has ONE return cursor, not a stack, and an 8-bit nesting count. */
bool RT_transferMusicTrack(RT_MusicControl *state,u8 selector,bool call,
    u16 returnCursor,RT_MusicTrackLookup lookup,void *context);
/* 878D: nested end returns immediately to the saved cursor; terminal end
   clears the shared status. It never treats an empty command ring as done. */
RT_MusicControlAction RT_endMusicControlTrack(RT_MusicControl *state);
/* MCU850C/8565. Key identifies the counted-control record. Scan uses the
   last free slot, or the previous lastLoop slot when all eight are occupied.
   Count increments wrap as source bytes. Track installation clears counts
   but preserves keys/lastLoop. Targets are already-converted event cursors. */
bool RT_stepMusicRepeat(RT_MusicControl *state,u16 key,u8 count,
    u16 firstTarget,u16 repeatTarget,u16 next,bool conditional);
#endif
