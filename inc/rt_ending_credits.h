#ifndef RT_ENDING_CREDITS_H
#define RT_ENDING_CREDITS_H
#include <genesis.h>
typedef struct {u16 value;u8 enabled,labelPublished,pendingReady,pending[5],published[5];} RT_EndingCredits;
typedef void (*RT_EndingCreditWrite)(u8 plane,u8 column,u16 entry);
extern const u16 rtEndingCreditTiles[5][11][2],rtEndingCreditLabel[7][2];
void RT_endingCreditsReset(RT_EndingCredits*);
void RT_endingCreditsPrepare(RT_EndingCredits*,u16);
/* Caller owns VDP/audio-safe window. Returns number of changed map writes. */
u8 RT_endingCreditsCommit(RT_EndingCredits*,RT_EndingCreditWrite);
#endif
