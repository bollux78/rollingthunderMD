#include "rt_music_playback.h"

bool RT_beginMusicPlaybackWindows(RT_MusicPlayback *p,RT_MusicSequence *sequence,
 const RT_MusicEventBank *bank,u8 request,u8 *ack,u32 refresh,
 u32 address,u32 samples,u32 romBytes,u16 windows,u16 budget,
 RT_MusicRegisterWrite write,void *context)
{
 RT_PcmRomPlan plan;
 if(!RT_planPcmRomClipWindows(address,samples,romBytes,windows,&plan))return FALSE;
 if(!p || p->active || p->fault || !sequence || !bank || !ack ||
    !request || request==0xFF || request==*ack || !budget || !write)return FALSE;
 if(RT_getPcmRomStatus()!=RT_PCM_ROM_INACTIVE)return FALSE;
 p->sequence=sequence;p->bank=bank;
 p->clock=(RT_MusicRefreshState){0};p->clock.refresh=refresh;
 RT_publishMusicCommand(&sequence->control,request,ack);
 if(RT_serviceMusicSequence(sequence,bank,budget,write,context)==RT_MUSIC_CONTROL_INVALID ||
    !sequence->control.status){p->fault=TRUE;return FALSE;}
 if(!RT_startPcmRomDriverWindows(address,samples,romBytes,windows)){p->fault=TRUE;return FALSE;}
 p->active=TRUE;p->pcm=RT_PCM_ROM_STARTING;p->commandStopped=FALSE;
 return TRUE;
}

bool RT_beginMusicPlayback(RT_MusicPlayback *p,RT_MusicSequence *sequence,
 const RT_MusicEventBank *bank,u8 request,u8 *ack,u32 refresh,
 u32 address,u32 samples,u32 romBytes,u16 budget,
 RT_MusicRegisterWrite write,void *context)
{
 return RT_beginMusicPlaybackWindows(p,sequence,bank,request,ack,refresh,
     address,samples,romBytes,1,budget,write,context);
}

bool RT_replaceMusicPlaybackWindows(RT_MusicPlayback *p,
 const RT_MusicEventBank *bank,u8 request,u8 *ack,u32 refresh,
 u32 address,u32 samples,u32 romBytes,u16 windows,u16 budget,
 RT_MusicRegisterWrite write,void *context)
{
 RT_PcmRomPlan plan;
 if(!p || !p->active || p->fault || !p->sequence || !bank || !ack ||
    !request || request==0xFF || request==*ack || !budget || !write ||
    p->clock.refresh!=refresh || !bank->tracks || !bank->events ||
    request>=bank->trackCount || bank->tracks[request]>=bank->eventCount ||
    !RT_planPcmRomClipWindows(address,samples,romBytes,windows,&plan))return FALSE;
 RT_PcmRomStatus owner=RT_getPcmRomStatus();
 if(owner==RT_PCM_ROM_INACTIVE || owner==RT_PCM_ROM_OWNERSHIP_LOST){
  p->fault=TRUE;return FALSE;
 }
 /* The resident transport changes clip without a null-driver/chip reload.
    Failure does not publish or acknowledge a source request. */
 if(!RT_replacePcmRomClipWindows(address,samples,romBytes,windows)){
  p->fault=TRUE;return FALSE;
 }
 p->bank=bank;p->clock=(RT_MusicRefreshState){0};p->clock.refresh=refresh;
 p->pcm=RT_PCM_ROM_STARTING;p->commandStopped=FALSE;
 RT_publishMusicCommand(&p->sequence->control,request,ack);
 if(RT_serviceMusicSequence(p->sequence,bank,budget,write,context)==RT_MUSIC_CONTROL_INVALID ||
    !p->sequence->control.status){p->fault=TRUE;return FALSE;}
 return TRUE;
}

RT_MusicRefreshResult RT_serviceMusicPlayback(RT_MusicPlayback *p,
 u32 refresh,u16 maxRefreshes,u16 budget,RT_MusicRegisterWrite write,void *context)
{
 RT_MusicRefreshResult result;
 if(!p || !p->active || p->fault)return RT_MUSIC_REFRESH_FAULT;
 result=RT_serviceMusicSequenceRefreshes(&p->clock,p->sequence,p->bank,
     refresh,maxRefreshes,budget,write,context);
 p->pcm=RT_getPcmRomStatus();
 if(result==RT_MUSIC_REFRESH_FAULT || p->pcm==RT_PCM_ROM_INACTIVE ||
    p->pcm==RT_PCM_ROM_OWNERSHIP_LOST ||
    (result==RT_MUSIC_REFRESH_CURRENT && p->pcm==RT_PCM_ROM_COMPLETE &&
     p->sequence->control.status)){
  p->fault=TRUE;return RT_MUSIC_REFRESH_FAULT;
 }
 return result;
}

bool RT_readMusicPlaybackStatus(const RT_MusicPlayback *p,u8 *status,bool *drained)
{
 if(!p || (!p->active && p->pcm!=RT_PCM_ROM_COMPLETE && !p->commandStopped) || p->fault ||
    !p->sequence || !status || !drained)return FALSE;
 *status=p->sequence->control.status;*drained=p->pcm==RT_PCM_ROM_COMPLETE;
 return TRUE;
}

bool RT_stopMusicPlayback(RT_MusicPlayback *p)
{
 bool completed;
 if(!p)return FALSE;
 if(!p->active)return !p->fault;
 completed=!p->fault && p->pcm==RT_PCM_ROM_COMPLETE && p->sequence &&
     !p->sequence->control.status;
 if(!RT_stopPcmRomDriver()){p->fault=TRUE;return FALSE;}
 /* Releasing the device cannot erase the source's latched terminal status.
    Interrupted or failed sessions do not acquire that completed state. */
 p->active=FALSE;p->pcm=completed?RT_PCM_ROM_COMPLETE:RT_PCM_ROM_INACTIVE;
 return TRUE;
}

bool RT_stopMusicPlaybackCommand(RT_MusicPlayback *p,u8 *ack)
{
 if(!p || p->fault || !p->sequence || !ack ||
    (!*ack && p->sequence->control.status))return FALSE;
 if(!RT_stopMusicPlayback(p))return FALSE;
 RT_publishMusicCommand(&p->sequence->control,0,ack);
 p->commandStopped=TRUE;
 return TRUE;
}
