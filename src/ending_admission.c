#include "rt_ending_admission.h"
RT_EndingAdmissionAction RT_endingAdmissionStep(RT_EndingAdmission *s,
 u8 available,u8 status,u8 graphicsIdle,u8 movementIdle)
{
 if(!s || s->phase>3 || s->entered>1 || s->pending>1 ||
    (!s->entered && (s->phase || s->pending)))return RT_ENDING_ADMISSION_INVALID;
 if(s->pending)return RT_ENDING_ADMISSION_WAIT;
 if(!s->entered){
  if(!available || status)return RT_ENDING_ADMISSION_WAIT;
  s->entered=1;
 }
 if(s->phase==3)return RT_ENDING_ADMISSION_READY;
 if((s->phase==1 && !graphicsIdle) || (s->phase==2 && !movementIdle))return RT_ENDING_ADMISSION_WAIT;
 s->phase++;s->pending=1;
 return (RT_EndingAdmissionAction)(RT_ENDING_ADMISSION_EVENTS+s->phase-1);
}
u8 RT_endingAdmissionAcknowledge(RT_EndingAdmission *s,RT_EndingAdmissionAction action)
{
 if(!s || !s->entered || !s->pending || s->phase<1 || s->phase>3 ||
    action!=RT_ENDING_ADMISSION_EVENTS+s->phase-1)return 0;
 s->pending=0;return 1;
}
