#ifndef RT_HIGH_SCORE_H
#define RT_HIGH_SCORE_H
#include <genesis.h>

/* CPU1 95E3..960C: install five records from960D and high score003000.
   Preserves scratch/control bytes except542D. Call only at cold startup;
   new-game score clearing must preserve the table and high score. */
bool RT_initializeHighScores(u8 state[50],u8 highScore[3]);

typedef enum
{
    RT_HIGH_SCORE_INVALID,
    RT_HIGH_SCORE_NOT_QUALIFIED,
    RT_HIGH_SCORE_DRAW_ENTRY
} RT_HighScoreAction;

/* CPU1 9642..96C0 and nonqualifying 97CC..97D3. state is source
   5400..5431: five seven-byte records, a seven-byte scratch record,
   then entry controls. scores is 5454..545B (two four-byte records).
   Requires entry phase 542D == 0. Equal scores qualify ahead of old ties.
   DRAW_ENTRY stops before B4B8/97D4 rendering: caller must draw the panel,
   issue source event E8=0B, highlight the row, then advance 542D and set
   542E/F=0700. Do not call again until that continuation is complete.
   No rendering, sound completion, or gameplay integration is supplied here. */
RT_HighScoreAction RT_beginHighScoreEntry(u8 state[50], const u8 scores[8],
    u8 player, u8 areaC5);

/* CPU1 96E8..97D3. Operates only on phases 1 (initials) and 2 (blink).
   row holds the 46 text bytes at 3594 + rank*256, including attributes;
   input holds source 426C..4281, already processed by the input dispatcher.
   Player two uses the alternate controls only when cabinet4259 != 0.
   Caller must publish changed row bytes before the next service tick.
   TRUE means a valid service, including idle and completion; phase/busy in
   state report completion. Invalid pointers/state are inert. No hardware
   input polling or rendering is performed here. */
bool RT_stepHighScoreEntry(u8 state[50], u8 row[46], const u8 input[22],
    u8 player, u8 cabinet4259, u8 frameCounter0E);

typedef bool (*RT_HighScoreTextWrite)(u16 address, u8 value, void *context);
/* Callback adapter: context is a writable 672-byte packed panel. Rejects
   source addresses outside the B4B8 rectangle without modifying storage. */
bool RT_writeHighScorePanelByte(u16 address, u8 value, void *context);
/* B4B8 followed by 97D4: clears the source panel and emits all five rows.
   Addresses are source text RAM coordinates, not Genesis VRAM addresses.
   On writer failure, retry the whole panel; records are never modified. */
bool RT_drawHighScorePanel(const u8 records[35], RT_HighScoreTextWrite write,
    void *context);
/* 96C9..96E7 continuation after successful panel construction. row is the
   selected 46-byte row. Writes event E8=0B, highlights the first 21 cells
   (the source blink later covers 23), and arms phase 1 with timer 0700. */
bool RT_armHighScoreEntry(u8 state[50], u8 row[46], u8 *eventE8);
#endif
