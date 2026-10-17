#include "rt_music_control.h"

bool RT_publishMusicCommand(RT_MusicControl *state,u8 requestedE8,u8 *acknowledgedE9)
{
    if(!state || !acknowledgedE9 || requestedE8==*acknowledgedE9)return FALSE;
    state->status=requestedE8;
    *acknowledgedE9=requestedE8;
    return TRUE;
}

static void installTrack(RT_MusicControl *state,u16 cursor)
{
    u8 channel;
    state->cursor=cursor;state->status=0xFF;
    for(channel=0;channel<8;channel++)state->loopCount[channel]=0;
}

RT_MusicControlAction RT_beginMusicControlService(RT_MusicControl *state,
    RT_MusicTrackLookup lookup,void *context)
{
    u16 cursor;
    if(!state)return RT_MUSIC_CONTROL_INVALID;
    if(!state->status)return RT_MUSIC_CONTROL_IDLE;
    if(state->status==0xFF)
        return RT_advanceMusicClock(&state->clock)?RT_MUSIC_CONTROL_EVENTS:RT_MUSIC_CONTROL_WAIT;
    if(!lookup || !lookup(state->status,&cursor,context))return RT_MUSIC_CONTROL_INVALID;
    state->returnDepth=0;
    installTrack(state,cursor);
    return RT_MUSIC_CONTROL_EVENTS;
}

bool RT_transferMusicTrack(RT_MusicControl *state,u8 selector,bool call,
    u16 returnCursor,RT_MusicTrackLookup lookup,void *context)
{
    u16 cursor;
    if(!state || !lookup || !lookup(selector,&cursor,context))return FALSE;
    if(call){state->returnCursor=returnCursor;state->returnDepth++;}
    installTrack(state,cursor);
    return TRUE;
}

RT_MusicControlAction RT_endMusicControlTrack(RT_MusicControl *state)
{
    if(!state)return RT_MUSIC_CONTROL_INVALID;
    if(!state->returnDepth){state->status=0;return RT_MUSIC_CONTROL_IDLE;}
    state->returnDepth--;state->cursor=state->returnCursor;
    return RT_MUSIC_CONTROL_EVENTS;
}

bool RT_stepMusicRepeat(RT_MusicControl *state,u16 key,u8 count,
    u16 firstTarget,u16 repeatTarget,u16 next,bool conditional)
{
    u8 slot,last;bool found=FALSE;
    if(!state)return FALSE;
    last=state->lastLoop;
    for(slot=0;slot<8;slot++){
        if(!state->loopCount[slot])last=slot;
        else if(state->loopKey[slot]==key){last=slot;found=TRUE;break;}
    }
    if(last>=8)return FALSE;
    state->lastLoop=last;
    if(!found){
        state->loopKey[last]=key;state->loopCount[last]=1;
        state->cursor=conditional?next:firstTarget;
    }else{
        state->loopCount[last]++;
        if(state->loopCount[last]==count){
            state->loopCount[last]=0;state->cursor=conditional?firstTarget:next;
        }else state->cursor=conditional?next:repeatTarget;
    }
    return TRUE;
}
