#ifndef RT_NATIVE_CAMPAIGN_COMPLETION_MONITOR_H
#define RT_NATIVE_CAMPAIGN_COMPLETION_MONITOR_H
#include "rt_campaign_scene_background.h"
typedef struct {u16 cell,planeA,planeB;} RT_CompletionMonitorPatch;
typedef struct {const RT_CompletionMonitorPatch *patches;u16 count;} RT_CompletionMonitorLabel;
extern const RT_SceneBackground rtCompletionMonitorBackground;
extern const RT_CompletionMonitorLabel rtCompletionMonitorLabels[11];
typedef struct {const u32 *patterns;u16 count;} RT_CompletionMonitorHud;
extern const RT_CompletionMonitorHud rtCompletionMonitorHud;
extern const u16 rtCompletionMonitorDigits[30][11][2];
/* Valid campaign scores (0..999999 units) and decimal timer to source glyph
 * indexes;10 means blank. Timer displays its low three decimal digits. */
void RT_completionMonitorFormat(u32 high,u32 player0,u32 player1,u16 timer,u8 digits[24]);
#ifdef __m68k__
/* Call after Show, during blank. Publishes only changed score/time cells. */
void RT_completionMonitorUpdate(u32 high,u32 player0,u32 player1,u16 timer);
/* Prepare numeric glyphs outside the narrow audio/VDP window; Commit writes
 * only changed cells inside it. Repeated Prepare replaces the pending values.
 * Show discards pending work. The legacy Update performs both consecutively. */
void RT_completionMonitorPrepare(u32 high,u32 player0,u32 player1,u16 timer);
void RT_completionMonitorCommit(void);
/* Ordinary completed-stage monitor and source label. Final mission uses its
 * distinct ending route. Caller owns HUD, bonus score/timer events and audio.
 * Must not be called while a completion animation owns the VDP callbacks. */
u8 RT_completionMonitorShow(u8 story,u8 area);
/* Source mode12 phase3 labels for the completing player (0/1).
 * Caller starts track09 and the final bonus clock at this phase. */
u8 RT_completionMonitorShowFinal(u8 player);
#endif
#endif
