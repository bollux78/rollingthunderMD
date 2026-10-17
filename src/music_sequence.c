#include "rt_music_sequence.h"
RT_MusicRefreshResult RT_serviceMusicSequenceRefreshes(RT_MusicRefreshState *clock,
 RT_MusicSequence *state,const RT_MusicEventBank *bank,u32 targetRefresh,
 u16 maxRefreshes,u16 eventBudget,RT_MusicRegisterWrite write,void *context)
{
 if(!clock)return RT_MUSIC_REFRESH_FAULT;
 if(clock->fault)return RT_MUSIC_REFRESH_FAULT;
 if(!state || !bank || !write || !maxRefreshes || !eventBudget || clock->remainder>=7874999 ||
    (u32)(targetRefresh-clock->refresh)>0x7FFFFFFFUL){
  clock->fault=TRUE;return RT_MUSIC_REFRESH_FAULT;
 }
 while(clock->refresh!=targetRefresh && maxRefreshes--){
  u8 services=RT_musicServicesForNtscRefresh(&clock->remainder);
  clock->refresh++;
  while(services--){
   u8 previous=state->control.status;
   if(RT_serviceMusicSequence(state,bank,eventBudget,write,context)==RT_MUSIC_CONTROL_INVALID){
    clock->fault=TRUE;return RT_MUSIC_REFRESH_FAULT;
   }
   clock->services++;
   if(previous && !state->control.status)clock->endRefresh=clock->refresh;
  }
 }
 return clock->refresh==targetRefresh?RT_MUSIC_REFRESH_CURRENT:RT_MUSIC_REFRESH_PENDING;
}

static bool lookupTrack(u8 selector,u16 *cursor,void *opaque)
{
 const RT_MusicEventBank *bank=opaque;
 if(selector>=bank->trackCount || bank->tracks[selector]>=bank->eventCount)return FALSE;
 *cursor=bank->tracks[selector];return TRUE;
}
RT_MusicControlAction RT_serviceMusicSequence(RT_MusicSequence *state,
 const RT_MusicEventBank *bank,u16 budget,RT_MusicRegisterWrite write,void *context)
{
 RT_MusicControlAction action;
 u8 channel;
 if(!state || !bank || !bank->events || !bank->tracks || !write || !budget)
  return RT_MUSIC_CONTROL_INVALID;
 action=RT_beginMusicControlService(&state->control,lookupTrack,(void*)bank);
 if(action==RT_MUSIC_CONTROL_IDLE){RT_musicIdleSilence(write,context);return action;}
 if(action!=RT_MUSIC_CONTROL_EVENTS)return action;
 while(budget--){
  const RT_MusicEvent *event;
  if(state->control.cursor>=bank->eventCount)return RT_MUSIC_CONTROL_INVALID;
  event=bank->events+state->control.cursor++;
  switch(event->kind){
  case RT_MUSIC_EVENT_GAIN:RT_musicMasterGain(&state->registers,event->value,write,context);break;
  case RT_MUSIC_EVENT_BASE:RT_setMusicClockBase(&state->control.clock,event->value);break;
  case RT_MUSIC_EVENT_MULTIPLIER:RT_setMusicClockMultiplier(&state->control.clock,event->value);break;
  case RT_MUSIC_EVENT_NOTES:case RT_MUSIC_EVENT_INSTRUMENTS:case RT_MUSIC_EVENT_CHANNEL_GAINS:
  case RT_MUSIC_EVENT_FRACTION:case RT_MUSIC_EVENT_PITCH:case RT_MUSIC_EVENT_MODULATION:
   if(!event->data)return RT_MUSIC_CONTROL_INVALID;
   if(event->kind==RT_MUSIC_EVENT_CHANNEL_GAINS)
    RT_musicChannelGains(&state->registers,event->mask,event->data,write,context);
   else for(channel=0;channel<8;channel++)if(event->mask&(1<<channel)){
    if(event->kind==RT_MUSIC_EVENT_INSTRUMENTS)
     RT_musicInstrument(channel,event->data+channel*28,write,context);
    else RT_musicChannelEvent(event->kind==RT_MUSIC_EVENT_NOTES?RT_MUSIC_KEY_NOTE:
     event->kind==RT_MUSIC_EVENT_FRACTION?RT_MUSIC_PITCH_FRACTION:
     event->kind==RT_MUSIC_EVENT_PITCH?RT_MUSIC_PITCH_ONLY:RT_MUSIC_CHANNEL_MODULATION,
     channel,event->data[channel],write,context);
   }
   if(event->kind==RT_MUSIC_EVENT_NOTES)return RT_MUSIC_CONTROL_EVENTS;
   break;
  case RT_MUSIC_EVENT_LFO:case RT_MUSIC_EVENT_PHASE_DEPTH:
  case RT_MUSIC_EVENT_AMPLITUDE_DEPTH:case RT_MUSIC_EVENT_WAVEFORM:
   RT_musicGlobalEvent(event->kind==RT_MUSIC_EVENT_LFO?RT_MUSIC_LFO_FREQUENCY:
    event->kind==RT_MUSIC_EVENT_PHASE_DEPTH?RT_MUSIC_PHASE_DEPTH:
    event->kind==RT_MUSIC_EVENT_AMPLITUDE_DEPTH?RT_MUSIC_AMPLITUDE_DEPTH:RT_MUSIC_LFO_WAVEFORM,
    event->value,write,context);break;
  case RT_MUSIC_EVENT_END:
   action=RT_endMusicControlTrack(&state->control);
   if(action!=RT_MUSIC_CONTROL_EVENTS)return action;
   break;
  case RT_MUSIC_EVENT_CALL:case RT_MUSIC_EVENT_TRACK:
   if(event->value>255 || !RT_transferMusicTrack(&state->control,event->value,
      event->kind==RT_MUSIC_EVENT_CALL,state->control.cursor,lookupTrack,(void*)bank))
    return RT_MUSIC_CONTROL_INVALID;
   break;
  case RT_MUSIC_EVENT_REPEAT:case RT_MUSIC_EVENT_CONDITIONAL:
   if(!event->data || !RT_stepMusicRepeat(&state->control,state->control.cursor-1,
       event->value,((u16)event->data[0]<<8)|event->data[1],
       ((u16)event->data[2]<<8)|event->data[3],state->control.cursor,
       event->kind==RT_MUSIC_EVENT_CONDITIONAL))return RT_MUSIC_CONTROL_INVALID;
   break;
  case RT_MUSIC_EVENT_JUMP:state->control.cursor=event->value;break;
  case RT_MUSIC_EVENT_YIELD:return RT_MUSIC_CONTROL_EVENTS;
  default:return RT_MUSIC_CONTROL_INVALID;
  }
 }
 return RT_MUSIC_CONTROL_INVALID;
}
