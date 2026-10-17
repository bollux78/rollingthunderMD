#ifndef RT_NATIVE_DISPLAY_H
#define RT_NATIVE_DISPLAY_H
#include "rt_world.h"
#ifndef __m68k__
typedef uint32_t u32;
#endif
/* Build-time resolved Genesis patterns and name-table attributes. Neither
 * source graphics words nor palette fitting are used by the display runtime. */
typedef struct {
    const u32 *patterns;
    const u16 *map[2];
    const u16 *palettes;
    u16 widthCells,heightCells,patternCount;
    u16 padding[2];
    u8 story,area;
    /* Optional shared16-cell blocks. map holds block indices when non-null;
     * otherwise it holds direct attributes. Dimensions must be16-cell aligned. */
    const u16 *blocks;
} RT_DisplayStage;
static inline u16 RT_displayMapCell(const RT_DisplayStage *s,u16 plane,u32 offset)
{
    return s->blocks?s->blocks[(u32)s->map[plane][offset>>4]*16+(offset&15)]:s->map[plane][offset];
}
typedef struct {
    const u16 *cells;
    u8 width,height;
} RT_DisplayPatch;
/* Indexed by native door kind 0..2 and restore/replacement variant 0..1.
 * Attributes reference the associated stage atlas; graphics plane 0 only. */
extern const RT_DisplayPatch rtDisplayDoorPatches[3][2];
extern const RT_DisplayStage rtDisplayOpening;
#endif
