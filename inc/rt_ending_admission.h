#ifndef RT_NATIVE_ENDING_ADMISSION_H
#define RT_NATIVE_ENDING_ADMISSION_H
#include "rt_world.h"
/* Native final route + mode12 cleanup phases0..2. Readiness is ownership,
 * not elapsed time. EVENTS includes source command0; caller stops music and
 * clears event producers, then acknowledges. MAP resets graphics writers;
 * SCENE clears layers/panel and releases movement/actor resources. */
typedef struct {u8 phase,entered,pending;} RT_EndingAdmission;
typedef enum {RT_ENDING_ADMISSION_INVALID,RT_ENDING_ADMISSION_WAIT,
 RT_ENDING_ADMISSION_EVENTS,RT_ENDING_ADMISSION_MAP,
 RT_ENDING_ADMISSION_SCENE,RT_ENDING_ADMISSION_READY} RT_EndingAdmissionAction;
/* Zero-initialize once at the latched final-boss exit. Caller must not reset
 * while a returned action is pending. False status availability never equals
 * silence. Phase3 READY permits monitor/track09 admission, not game completion. */
RT_EndingAdmissionAction RT_endingAdmissionStep(RT_EndingAdmission *state,
 u8 statusAvailable,u8 soundStatus,u8 graphicsIdle,u8 movementIdle);
u8 RT_endingAdmissionAcknowledge(RT_EndingAdmission *state,
 RT_EndingAdmissionAction completed);
#endif
