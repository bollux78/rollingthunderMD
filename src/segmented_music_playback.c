#include "rt_segmented_music_playback.h"
#include "rt_segmented_pcm.h"
bool RT_beginSegmentedMusicPlayback(RT_MusicPlayback *p,
    RT_MusicSequence *sequence,const RT_MusicEventBank *bank,u8 request,u8 *ack,
    u32 refresh,u32 address,u32 samples,u32 romBytes,u16 budget,
    RT_MusicRegisterWrite write,void *context)
{
    RT_SegmentedPcmPlan plan;
    if(!RT_planSegmentedPcm(address,samples,romBytes,&plan))return FALSE;
    if(!p || p->active || p->fault || !sequence || !bank || !ack ||
       !request || request==0xff || request==*ack || !budget || !write ||
       !bank->tracks || !bank->events || request>=bank->trackCount ||
       bank->tracks[request]>=bank->eventCount)return FALSE;
    if(RT_getPcmRomStatus()!=RT_PCM_ROM_INACTIVE)return FALSE;
    p->sequence=sequence;p->bank=bank;
    p->clock=(RT_MusicRefreshState){0};p->clock.refresh=refresh;
    RT_publishMusicCommand(&sequence->control,request,ack);
    if(RT_serviceMusicSequence(sequence,bank,budget,write,context)==RT_MUSIC_CONTROL_INVALID ||
       !sequence->control.status){p->fault=TRUE;return FALSE;}
    if(!RT_startSegmentedPcm(address,samples,romBytes)){p->fault=TRUE;return FALSE;}
    p->active=TRUE;p->pcm=RT_PCM_ROM_STARTING;p->commandStopped=FALSE;
    return TRUE;
}
