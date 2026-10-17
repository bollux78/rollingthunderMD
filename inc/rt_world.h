#ifndef RT_NATIVE_WORLD_H
#define RT_NATIVE_WORLD_H
#ifdef __m68k__
#include <genesis.h>
#else
#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef int16_t s16;
typedef int32_t s32;
#endif

/* The gameplay world is read-only ROM geometry plus explicit mutable objects.
 * Coordinates are world pixels, x right/y down. No emulated tile RAM, bank
 * addresses, task queue, camera mirror or arcade direct page is required. */
typedef struct {
    u8 probeCode;
    u8 collision[4];
    u8 cameraBarriers; /* bit0 negative edge; bit1 positive edge */
    u16 reserved;
} RT_TerrainMaterial;

/* Top-left of the 256-pixel logical terrain window, in world pixels.
 * The hardware viewport's crop/offset is a presentation concern. */
typedef struct { s16 x,y; } RT_WorldCamera;

typedef struct {
    const u16 *pages;
    u16 widthCells, heightCells, pageColumns;
    u8 story, area;
    RT_WorldCamera entrances[2]; /* initial entry, checkpoint entry */
    /* Generated-world retention bounds; max0 disables both for authored maps.
     * Checked before whole-pixel camera movement, never actor collision. */
    s16 cameraMinX,cameraMaxX;
} RT_WorldStage;

extern const RT_TerrainMaterial rtWorldMaterials[];
extern const u16 (*rtWorldPages)[256];
u8 RT_prepareWorldBank(u8 story,u8 area);
u8 RT_worldBind(const u16 (*cells)[256],const u16 *directory,u16 count,u8 story,u8 area);
extern const RT_WorldStage rtWorldStages[];
extern const u16 rtWorldStageCount, rtWorldPaddingMaterial;

/* Select once at level entry. Invalid stage selections do not alias level1. */
const RT_WorldStage *RT_worldStage(u8 story,u8 area);
/* A cell query is useful for collision sweeps; a pixel query floors to 8px.
 * Bounds use the original map padding material. Door artwork does not change
 * collision terrain; mutable object contacts belong to entity simulation. */
const RT_TerrainMaterial *RT_worldCell(const RT_WorldStage *stage,s16 x,s16 y);
const RT_TerrainMaterial *RT_worldPoint(const RT_WorldStage *stage,s16 x,s16 y);
/* Native entities use world coordinates with four fractional bits. A 32-bit
 * position covers the longest stage without retaining camera-relative state. */
const RT_TerrainMaterial *RT_worldFixed(const RT_WorldStage *stage,s32 x,s32 y);
/* Migration/display-window adapter. Gameplay entities should query their
 * world positions directly, without adding the camera on every contact. */
const RT_TerrainMaterial *RT_worldLocal(const RT_WorldStage *stage,
    const RT_WorldCamera *camera,s16 x,s16 y);
typedef enum {
    RT_WORLD_LEFT,RT_WORLD_RIGHT,RT_WORLD_UP,RT_WORLD_DOWN
} RT_WorldDirection;
/* Terrain sentinels choose actor motion instead of camera motion. They are
 * not actor collision walls. Check before each accepted whole-pixel scroll;
 * input/dead-zone/co-op policy is owned by the camera controller. Generated
 * directories can additionally restrict scrolling to their retained window. */
u8 RT_worldCameraBlocked(const RT_WorldStage *stage,
    const RT_WorldCamera *camera,RT_WorldDirection direction);
#endif
