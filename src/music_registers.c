#include "rt_music_registers.h"

void RT_musicIdleSilence(RT_MusicRegisterWrite write, void *context)
{
    u16 channel;
    if (!write) return;
    for (channel = 0; channel < 8; channel++) write(0x20 + channel, 0, context);
    for (channel = 0; channel < 8; channel++) write(0x78 + channel, 0x7F, context);
}

static u8 attenuation(const RT_MusicRegisters *state, u8 channel)
{
    u16 product = (u16)state->masterGain * state->channelGain[channel];
    return (255 - (product >> 8)) >> 1;
}

void RT_musicMasterGain(RT_MusicRegisters *state, u8 gain, RT_MusicRegisterWrite write, void *context)
{
    u16 channel;
    if (!state || !write) return;
    state->masterGain = gain;
    for (channel = 0; channel < 8; channel++)
        write(0x78 + channel, attenuation(state, channel), context);
}

void RT_musicChannelGains(RT_MusicRegisters *state, u8 mask, const u8 values[8], RT_MusicRegisterWrite write, void *context)
{
    s16 channel;
    if (!state || !values || !write) return;
    /* Source stack unwinding applies selected gains in descending order. */
    for (channel = 7; channel >= 0; channel--)
        if (mask & (1 << channel))
        {
            state->channelGain[channel] = values[channel];
            write(0x78 + channel, attenuation(state, channel), context);
        }
}

void RT_musicInstrument(u8 channel, const u8 parameters[28], RT_MusicRegisterWrite write, void *context)
{
    u16 i;
    if (channel >= 8 || !parameters || !write) return;
    /* Stored source patches have 32 bytes; only the first 28 are written. */
    for (i = 0; i < 28; i++) write(0x20 + channel + i * 8, parameters[i], context);
}

void RT_musicChannelEvent(RT_MusicChannelEvent event, u8 channel, u8 value, RT_MusicRegisterWrite write, void *context)
{
    if (channel >= 8 || !write) return;
    switch (event)
    {
        case RT_MUSIC_KEY_NOTE:
            write(0x01, 2, context);
            write(0x01, 0, context);
            write(0x08, channel, context);
            if (value != 0xFF)
            {
                write(0x28 + channel, value, context);
                write(0x08, 0x78 | channel, context);
            }
            break;
        case RT_MUSIC_PITCH_ONLY:
            if (value != 0xFF) write(0x28 + channel, value, context);
            break;
        case RT_MUSIC_PITCH_FRACTION:
            write(0x30 + channel, value & 0xFC, context);
            break;
        case RT_MUSIC_CHANNEL_MODULATION:
            write(0x38 + channel, value & 0x73, context);
            break;
        default: break;
    }
}

void RT_musicGlobalEvent(RT_MusicGlobalEvent event, u8 value, RT_MusicRegisterWrite write, void *context)
{
    if (!write) return;
    switch (event)
    {
        case RT_MUSIC_LFO_FREQUENCY: write(0x18, value, context); break;
        case RT_MUSIC_PHASE_DEPTH: write(0x19, value | 0x80, context); break;
        case RT_MUSIC_AMPLITUDE_DEPTH: write(0x19, value & 0x7F, context); break;
        case RT_MUSIC_LFO_WAVEFORM: write(0x1B, value, context); break;
        default: break;
    }
}
