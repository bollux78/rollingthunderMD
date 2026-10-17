#ifndef RT_MUSIC_REGISTERS_H
#define RT_MUSIC_REGISTERS_H
#include <genesis.h>

/* Source FM register contract, consumed by a future Genesis audio backend.
   These are NOT YM2612 port addresses. No chip or CPU is emulated here. */
typedef void (*RT_MusicRegisterWrite)(u8 reg, u8 value, void *context);
typedef struct
{
    u8 masterGain;
    u8 channelGain[8];
} RT_MusicRegisters;

typedef enum
{
    RT_MUSIC_KEY_NOTE,
    RT_MUSIC_PITCH_ONLY,
    RT_MUSIC_PITCH_FRACTION,
    RT_MUSIC_CHANNEL_MODULATION
} RT_MusicChannelEvent;
typedef enum
{
    RT_MUSIC_LFO_FREQUENCY,
    RT_MUSIC_PHASE_DEPTH,
    RT_MUSIC_AMPLITUDE_DEPTH,
    RT_MUSIC_LFO_WAVEFORM
} RT_MusicGlobalEvent;

void RT_musicMasterGain(RT_MusicRegisters *state, u8 gain, RT_MusicRegisterWrite write, void *context);
/* Bit N selects channel N. Values are indexed by channel, not packed.
   Caller preserves state across tracks; instrument writes do not change gains. */
void RT_musicChannelGains(RT_MusicRegisters *state, u8 mask, const u8 values[8], RT_MusicRegisterWrite write, void *context);
void RT_musicInstrument(u8 channel, const u8 parameters[28], RT_MusicRegisterWrite write, void *context);
void RT_musicChannelEvent(RT_MusicChannelEvent event, u8 channel, u8 value, RT_MusicRegisterWrite write, void *context);
void RT_musicGlobalEvent(RT_MusicGlobalEvent event, u8 value, RT_MusicRegisterWrite write, void *context);
/* Source idle service mutes channel outputs and final-operator levels.
   It preserves gain bookkeeping and does not send key-off events. */
void RT_musicIdleSilence(RT_MusicRegisterWrite write, void *context);
#endif
