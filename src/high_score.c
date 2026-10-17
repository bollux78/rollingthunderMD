#include "rt_high_score.h"

bool RT_initializeHighScores(u8 state[50],u8 highScore[3])
{
    static const u8 records[35]={
        0x00,0x30,0x00,5,0x0A,0x15,0x0B,
        0x00,0x20,0x00,4,0x15,0x12,0x1B,
        0x00,0x10,0x00,3,0x10,0x15,0x1B,
        0x00,0x08,0x00,2,0x16,0x0A,0x0B,
        0x00,0x04,0x00,1,0x16,0x19,0x12
    };
    u8 i;
    if(!state || !highScore)return false;
    for(i=0;i<35;i++)state[i]=records[i];
    state[0x2D]=0;
    highScore[0]=0;highScore[1]=0x30;highScore[2]=0;
    return true;
}


RT_HighScoreAction RT_beginHighScoreEntry(u8 state[50], const u8 scores[8],
    u8 player, u8 areaC5)
{
    u8 rank, byte;
    const u8 *score;
    if (!state || !scores || player > 1 || state[0x2D] != 0)
        return RT_HIGH_SCORE_INVALID;
    score = scores + player * 4;
    state[0x2A] = 0;
    state[0x31] = 1;
    for (rank = 0; rank < 5; rank++)
    {
        const u8 *record = state + rank * 7;
        if (score[0] > record[0] ||
            (score[0] == record[0] && (score[1] > record[1] ||
            (score[1] == record[1] && score[2] >= record[2]))))
            break;
        state[0x2A]++;
    }
    if (rank == 5)
    {
        state[0x2D] = 0;
        state[0x31] = 0;
        return RT_HIGH_SCORE_NOT_QUALIFIED;
    }
    /* Scratch is observable source state. Swapping forward leaves the old
       fifth record there, unlike a simple memmove-based insertion. */
    for (byte = 0; byte < 3; byte++) state[0x23 + byte] = score[byte];
    state[0x26] = areaC5;
    state[0x27] = state[0x28] = state[0x29] = 0x0A;
    for (; rank < 5; rank++)
        for (byte = 0; byte < 7; byte++)
        {
            u8 previous = state[rank * 7 + byte];
            state[rank * 7 + byte] = state[0x23 + byte];
            state[0x23 + byte] = previous;
        }
    state[0x2B] = 0;
    state[0x2C] = 0;
    return RT_HIGH_SCORE_DRAW_ENTRY;
}

bool RT_stepHighScoreEntry(u8 state[50], u8 row[46], const u8 input[22],
    u8 player, u8 cabinet4259, u8 frameCounter0E)
{
    unsigned timer, base, direction, index;
    if (!state || !row || !input || player > 1 || state[0x2A] > 4 ||
        state[0x2B] > 2 || (state[0x2D] != 1 && state[0x2D] != 2))
        return false;
    if (state[0x2D] == 2)
    {
        state[0x2E]--;
        if (!state[0x2E])
        {
            state[0x2D] = state[0x31] = 0;
            return true;
        }
        if (!(frameCounter0E & 3))
            for (index = 1; index < 46; index += 2)
                row[index] = (frameCounter0E & 4) ? 0xFC : 0xE4;
        return true;
    }
    timer = (((unsigned)state[0x2E] << 8) | state[0x2F]);
    timer = (timer - 1) & 0xFFFF;
    /* Source branches before storing a zero result. On expiry the low
       timer byte therefore retains 01, including after the B4 high store. */
    if (!timer) goto finish;
    state[0x2E] = timer >> 8;
    state[0x2F] = timer;
    base = (player && cabinet4259) ? 0 : 12;
    if (input[base])
    {
        if (state[0x2B] == 2) goto finish;
        state[0x2B]++;
        goto draw;
    }
    /* Decrement has priority over increment. Low-byte held input advances
       only when the shared repeat accumulator overflows; release retains it. */
    direction = base + 8;
    if (!input[direction] && !input[direction + 1])
    {
        direction = base + 6;
        if (!input[direction] && !input[direction + 1]) return true;
    }
    if (!input[direction])
    {
        unsigned repeat = (unsigned)state[0x30] + 0x20;
        state[0x30] = repeat;
        if (repeat < 256) return true;
    }
    if (direction == base + 8) state[0x2C]--;
    else state[0x2C]++;
draw:
    index = 40 + state[0x2B] * 2;
    row[index] = (state[0x2C] & 31) + 10;
    row[index + 1] = 0xE4;
    state[0x30] = 0;
    return true;
finish:
    for (index = 0; index < 3; index++)
        state[state[0x2A] * 7 + 4 + index] = row[40 + index * 2];
    state[0x2D]++;
    state[0x2E] = 0xB4;
    return true;
}
