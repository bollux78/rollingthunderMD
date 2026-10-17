#ifndef RT_VBLANK_POLICY_H
#define RT_VBLANK_POLICY_H
#include <genesis.h>
/* Optional SGDK frame-task ordering. DMA, scroll and palette tasks still run
   in order; TRUE samples controllers after DMA/scroll and before palette
   fade calculation and the user VBlank callback.
   FALSE preserves SGDK's default callback-then-controller order. Reset sets
   FALSE. This does not establish a safe Z80 bus window or change bus policy. */
void RT_setJoyBeforeVBlankCallback(bool enabled);
#endif
