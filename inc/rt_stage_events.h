#ifndef RT_NATIVE_STAGE_EVENTS_H
#define RT_NATIVE_STAGE_EVENTS_H
#include "rt_world.h"
/* Build-time world positions and semantic constructor fields. kind0..31 maps
 * the stage-object family; these are definitions, not live pooled instances.
 * Preserve all event kinds and per-cell order when applying the shared31 cap. */
typedef struct {
    s16 x,y;
    u8 kind,initialState,parameter,color;
} RT_StageEvent;
typedef struct {u16 first,count;} RT_StageEventCell;
typedef struct {
    const RT_StageEvent *events;
    const RT_StageEventCell *cells;
    u16 eventCount;
    u8 columns,rows;
} RT_StageEventLevel;
/* Cell Y follows source event-grid order (upward256px bands); event positions
 * are ordinary downward world pixels. Cell and display coordinates differ. */
extern const RT_StageEventLevel rtStageEventLevels[10];
typedef enum {
    RT_EVENT_WINDOW_INITIAL,RT_EVENT_WINDOW_RIGHT,RT_EVENT_WINDOW_LEFT,
    RT_EVENT_WINDOW_UP,RT_EVENT_WINDOW_DOWN
} RT_EventWindowEdge;
typedef u8 (*RT_StageEventVisitor)(const RT_StageEvent *event,u16 eventIndex,void *context);
/* gridX/Y are the retained event-window center, not tile coordinates. The
 * LEFT strip deliberately spans four source rows; other strips span three.
 * population is the shared live stage-object count, incremented after each
 * successful visitor. Stop at31. A failed visitor must leave its own pool
 * unchanged; earlier successful visits remain committed. No allocation here. */
u8 RT_stageVisitEvents(const RT_StageEventLevel *level,u8 gridX,u8 gridY,
    RT_EventWindowEdge edge,u8 *population,RT_StageEventVisitor visit,void *context);
typedef struct {u8 gridX,gridY;} RT_EventWindow;
typedef struct {u8 edge,gridX,gridY;} RT_EventWindowRequest;
u8 RT_eventWindowInitialize(RT_EventWindow *window,const RT_WorldStage *stage,u8 entrance);
/* Consume accepted camera movement once per update, after any previous requests
 * have been serviced. Returns0..2 requests in original vertical-before-horizontal
 * order, or255 for invalid input/scene jumps. Requests snapshot the grid before
 * advancing it. Large scene changes require a new authored entry initialization. */
u8 RT_eventWindowMove(RT_EventWindow *window,const RT_WorldStage *stage,
    const RT_WorldCamera *previous,const RT_WorldCamera *current,
    RT_EventWindowRequest requests[2]);
#endif
