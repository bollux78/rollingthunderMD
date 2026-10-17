#ifndef RT_SURVIVAL_STREAM_H
#define RT_SURVIVAL_STREAM_H
#include <genesis.h>
/* Source Level1 chunks are 256 pixels wide. Each selected chunk identifies
   its visual layers, collision, object cells and actor templates together.
   This sequencer alone does not establish traversal or enable Survival. */
typedef struct {
    u32 randomState, emitted;
    u8 previous, previousPrevious, prefixIndex;
    bool initialized;
} RT_SurvivalStream;
void RT_initSurvivalStream(RT_SurvivalStream *stream,u32 seed);
bool RT_nextSurvivalChunk(RT_SurvivalStream *stream,u8 *sourceChunk);

#define RT_SURVIVAL_WINDOW_CHUNKS 8
/* Local placement coordinates use the original signed-byte grid traversal.
   Keep that domain distinct from Level 1's nineteen source-directory columns.
   Endless play still needs to rebase before reaching this local limit. */
#define RT_SURVIVAL_PLACEMENT_COLUMNS 127
/* Shared, read-only lookup for all source consumers. Advancing is explicit:
   map, collision and spawn lookup order must never consume random choices. */
typedef struct {
    RT_SurvivalStream stream;
    u32 firstChunk;
    u8 chunks[RT_SURVIVAL_WINDOW_CHUNKS], head;
    bool initialized;
} RT_SurvivalWindow;
bool RT_initSurvivalWindow(RT_SurvivalWindow *window,u32 seed);
bool RT_advanceSurvivalWindow(RT_SurvivalWindow *window);
bool RT_resolveSurvivalPosition(const RT_SurvivalWindow *window,
    u32 logicalChunk,u8 pixelOffset,u16 *sourceX);
/* Explicit owner operation after collecting every still-needed chunk.
   Keeps the requested inclusive serial interval resident. Never rewinds;
   advances at most eight chunks per call and is atomic on failure. Caller
   must include pending map/spawn tasks and active collision consumers. */
bool RT_coverSurvivalWindow(RT_SurvivalWindow *window,
    u32 firstNeeded,u32 lastNeeded);
/* Keep one full chunk behind the camera for source map rows and westward
   spawn strips. Only the original unretired entry can scroll back to zero. */
bool RT_getSurvivalCameraMinimum(const RT_SurvivalWindow *window,u16 *minimum);
#endif
