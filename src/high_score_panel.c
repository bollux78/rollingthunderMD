#include "rt_high_score.h"

bool RT_writeHighScorePanelByte(u16 address, u8 value, void *context)
{
    u16 offset, row, column;
    if (!context || address < 0x3410) return false;
    offset = address - 0x3410;
    row = offset / 128;
    column = offset % 128;
    if (row >= 12 || column >= 56) return false;
    ((u8 *)context)[row * 56 + column] = value;
    return true;
}

static bool cell(RT_HighScoreTextWrite write, void *context, u16 address,
    u8 glyph, u8 attribute)
{
    return write(address, glyph, context) &&
        write(address + 1, attribute, context);
}

static bool number(const u8 *bytes, u8 count, u16 address,
    RT_HighScoreTextWrite write, void *context)
{
    u8 index, seen = 0;
    for (index = 0; index < count * 2; index++)
    {
        u8 digit = (index & 1) ? bytes[index / 2] & 15 : bytes[index / 2] >> 4;
        if (digit) seen = 1;
        if (!cell(write, context, address + index * 2,
            seen ? digit : 0xFF, 0xFC)) return false;
    }
    return true;
}

bool RT_drawHighScorePanel(const u8 records[35], RT_HighScoreTextWrite write,
    void *context)
{
    /* Named source display data from 95B3..95DF, never executable bytes. */
    static const u8 title[5] = {0x1D,0x18,0x19,0xFF,5};
    static const u8 headings[18] = {
        0x1C,0x0C,0x18,0x1B,0x0E,0xFF,0xFF,0x0A,0x1B,
        0x0E,0x0A,0xFF,0xFF,0x12,0x17,0x12,0x2B,0x1D
    };
    static const u8 ranks[5][3] = {
        {1,0x1C,0x1D},{2,0x17,0x0D},{3,0x1B,0x0D},
        {4,0x1D,0x11},{5,0x1D,0x11}
    };
    u16 row, col, address;
    if (!records || !write) return false;
    for (row = 0; row < 12; row++)
        for (col = 0; col < 28; col++)
            if (!cell(write, context, 0x3410 + row * 128 + col * 2,
                0xFF, 0)) return false;
    for (col = 0; col < 5; col++)
        if (!cell(write, context, 0x3426 + col * 2, title[col], 0xFC)) return false;
    for (col = 0; col < 18; col++)
        if (!cell(write, context, 0x34A0 + col * 2, headings[col], 0xFC)) return false;
    for (row = 0; row < 5; row++)
        for (col = 0; col < 3; col++)
            if (!cell(write, context, 0x3594 + row * 256 + col * 2,
                ranks[row][col], 0xFC)) return false;
    for (row = 0; row < 5; row++)
    {
        address = 0x359C + row * 256;
        if (!number(records + row * 7, 3, address, write, context) ||
            !cell(write, context, address + 12, 0, 0xFC) ||
            !number(records + row * 7 + 3, 1, address + 20, write, context))
            return false;
        for (col = 0; col < 3; col++)
            if (!cell(write, context, address + 32 + col * 2,
                records[row * 7 + 4 + col], 0xFC)) return false;
    }
    return true;
}

bool RT_armHighScoreEntry(u8 state[50], u8 row[46], u8 *eventE8)
{
    u8 index;
    if (!state || !row || !eventE8 || state[0x2D] != 0 ||
        state[0x2A] > 4 || state[0x31] != 1) return false;
    *eventE8 = 0x0B;
    for (index = 1; index < 42; index += 2) row[index] = 0xE4;
    state[0x2D] = 1;
    state[0x2E] = 7;
    state[0x2F] = 0;
    return true;
}
