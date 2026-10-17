#ifndef RT_OPN_MUSIC_H
#define RT_OPN_MUSIC_H
#include <genesis.h>
/* Live translation of the source YM2151 register contract (rt_music_registers.h)
 * to the Genesis YM2612 + PSG. Each of the eight source channels is routed by a
 * per-track map: 0..5 YM2612 channel, 0x10..0x12 PSG tone channel, 0xFF muted.
 * DT2 and the LFO waveform have no YM2612 equivalent and are dropped; LFO rate
 * and per-channel depth map to the nearest YM2612 setting. PSG voices follow
 * pitch and an approximation of the final carrier's envelope. */
#define RT_OPN_PSG 0x10
#define RT_OPN_MUTE 0xFF
/* YM2612 channel 3 runs in special mode: per-operator pitch, exact DT2. */
#define RT_OPN_SPECIAL 2
typedef void (*RT_OpnFmWrite)(u8 port,u8 reg,u8 value);
typedef void (*RT_OpnPsgWrite)(u8 value);
/* Waits at least one YM2612 sample (~19 us) so a key-off is latched. */
typedef void (*RT_OpnSettle)(void);
typedef struct {
    u8 rl,kc,kf,pmsams,keyed,stage;
    u8 tl,d1r,d2r,d1lrr;  /* final carrier (C2), PSG voices */
    u8 dt1mul[4],dt2[4];  /* FM voices: source DT1/MUL and DT2 per operator */
    u8 psgAtt;            /* last PSG attenuation written, 0xFF unknown */
    u16 env;              /* PSG voices: attenuation in 1/256 PSG steps */
} RT_OpnSource;
typedef struct {
    RT_OpnSource src[8];
    u8 map[8];
    u8 lfrq,pmd,amd;
    s8 psgOffset;         /* PSG attenuation steps added to every PSG voice */
    u8 pendingOn[6];      /* FM key-on slot masks, flushed by RT_opnMusicTick */
    u8 offMask;           /* FM channels keyed off during this service */
    RT_OpnFmWrite fm;
    RT_OpnPsgWrite psg;
    RT_OpnSettle settle;
} RT_OpnMusic;
/* Silences both chips and adopts a channel map (eight entries). */
void RT_opnMusicReset(RT_OpnMusic *m,const u8 map[8],RT_OpnFmWrite fm,RT_OpnPsgWrite psg,RT_OpnSettle settle);
/* RT_MusicRegisterWrite-compatible: context is the RT_OpnMusic. */
void RT_opnMusicWrite(u8 reg,u8 value,void *context);
/* Once after each source service: issues queued FM key-ons (after a settle
 * when the same channel was keyed off) and advances PSG envelopes. */
void RT_opnMusicTick(RT_OpnMusic *m);
/* Key-off and mute everything (stop / pause). */
void RT_opnMusicSilence(RT_OpnMusic *m);
#endif
