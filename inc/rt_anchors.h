#ifndef RT_NATIVE_ANCHORS_H
#define RT_NATIVE_ANCHORS_H
#include "rt_motion.h"

/* World position uses the motion body's 12.4, x-right/y-down convention.
 * Kinds 0..2 are doors; 3 is a climb anchor; 255 is an unused slot. Slots
 * remain stable while an agent holds an index. No source record is retained. */
typedef struct {
    s32 x,y;
    u8 kind,flags,phase,timer,layer,spawnDelay,visible,pose;
} RT_Anchor;
/* players==2 enables the co-op supply bonus; 0/1 retain solo supplies. */
typedef struct { RT_Anchor *anchors; u8 count; u8 players; } RT_AnchorScene;
typedef enum {
    RT_ANCHOR_IDLE,RT_ANCHOR_RESTORE,RT_ANCHOR_REPLACE,
    RT_ANCHOR_SPAWN,RT_ANCHOR_INVALID
} RT_AnchorAction;

/* flags: request-open 1, enemy ownership 2, flat 4, supply level 0x18,
 * alternate/consumed 0x20. phase is 0 closed through 4 open. timer is the
 * four-update door animation divider; pose 0 hides the door sprite, 1..3
 * selects its authored component list. layer is the source class >> 5. */

/* Refresh visibility from the current camera without rebasing world state.
 * Source CEEA local bounds are X [-0x200,0x1400), Y [0x300,0xE00), with
 * source Y pointing up from the bottom of the 256px logical window. */
u8 RT_anchorVisible(RT_Anchor *anchor,const RT_WorldCamera *camera);

/* Return slot+1, or zero when no anchor matches. count is the number of
 * native slots, not a cached visible count. Evaluate visibility at this
 * camera, then choose the first eligible slot, never the nearest. lower
 * selects a different layer and the lower entry; zero selects the same
 * layer and upper entry. Canonical layers are 0..7. World coordinates do
 * not alias distant anchors through a truncated source 16-bit position. */
u8 RT_anchorFind(const RT_Anchor *anchors,u8 count,const RT_MotionBody *body,
    const RT_WorldCamera *camera,u8 lower);

/* World-cell origin for generator-0 door rectangles. The source upward-Y
 * edge convention selects the cell above an exact downward tile boundary.
 * Reject non-door anchors and coordinates outside the display cell range. */
u8 RT_anchorPatchCell(const RT_Anchor *anchor,s16 *column,s16 *row);

/* Step each visible door once, after all players have renewed open requests.
 * The caller owns tile replacement/restoration and enemy allocation. A
 * successful allocation must establish ownership and the source cooldown.
 * sound is required and cleared on every call, including invalid inputs.
 * Hidden anchors, non-door kinds and phases above 4 return INVALID without
 * changing the anchor. Spawn requests do not consume or invent an enemy. */
RT_AnchorAction RT_anchorStep(RT_Anchor *anchor,u8 *sound);
#endif
