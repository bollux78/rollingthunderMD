#ifndef RT_HIGH_SCORE_INPUT_H
#define RT_HIGH_SCORE_INPUT_H
#include "rt_mcu_input.h"
/* VBlank owns scan/filter state. Main consumes pending source edges once per
   entry dispatch; rendering may span multiple refreshes. Level bytes always
   reflect the latest published source scan. Caller excludes the VBlank writer
   while taking input. This does not process credits or change source cadence. */
typedef struct {
    u8 scan[37],records[111],combined[74],shared[70],pending[22];
    volatile u8 ready;
    bool valid,fault;
} RT_HighScoreInput;
void RT_initHighScoreInput(RT_HighScoreInput *state);
bool RT_sampleHighScoreInput(RT_HighScoreInput *state,u16 pad1,u16 pad2);
bool RT_takeHighScoreInput(RT_HighScoreInput *state,u8 input[22]);
/* Exclusive NTSC/224-line custom-PCM owner only: call once from physical
   VInt, never the deferred VBlank callback. Polls three-button pads in the
   verified local-RAM interval and restores the interrupted bus policy.
   Late entry rejects. Stop calling before releasing the custom sound driver.
   Caller must verify the complete ISR + scene publication I/O budget. */
bool RT_pollHighScoreInputVInt(RT_HighScoreInput *state);
#endif
