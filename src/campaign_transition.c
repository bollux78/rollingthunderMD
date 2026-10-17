#include "rt_campaign_transition.h"
static u8 valid(const RT_TransitionClock *clock)
{
    return clock && clock->remaining<=9999 && clock->divider<64 &&
        clock->ending<=1 && clock->pending<=1 && clock->complete<=1;
}
u8 RT_transitionClockBegin(RT_TransitionClock *clock,u8 ending)
{
    if(!clock || ending>1)return 0;
    *clock=(RT_TransitionClock){8,0,ending,0,0};return 1;
}
u8 RT_transitionFrameRequest(RT_TransitionClock *clock)
{
    if(!valid(clock) || clock->pending || clock->complete)return 0;
    clock->pending=1;return 1;
}
RT_TransitionResult RT_transitionFramePresented(RT_TransitionClock *clock)
{
    if(!valid(clock) || !clock->pending || clock->complete)return RT_TRANSITION_INVALID;
    clock->pending=0;
    /* Ending8B9D only acknowledges rendering. It does not run the ordinary
     * countdown or invent a time limit for the final scene. */
    if(clock->ending)return RT_TRANSITION_WAIT;
    clock->divider=(clock->divider+1)&63;
    if(!clock->divider){
        clock->remaining=clock->remaining?clock->remaining-1:9999;
        if(!clock->remaining){clock->complete=1;return RT_TRANSITION_COMPLETE;}
    }
    return RT_TRANSITION_WAIT;
}
u8 RT_transitionCommandsBegin(u8 selector,u8 *effect,RT_TransitionCommands *commands)
{
    if(selector>10 || !effect || !commands)return 0;
    commands->count=2;
    if(selector==5){
        commands->events[0]=(RT_TransitionEvent){RT_TRANSITION_SCENE_COMMAND,0x4c};
        commands->events[1]=(RT_TransitionEvent){RT_TRANSITION_SCENE_COMMAND,0x67};
    }else{
        commands->events[0]=(RT_TransitionEvent){RT_TRANSITION_AUDIO,0x0c};
        commands->events[1]=(RT_TransitionEvent){RT_TRANSITION_SCENE_COMMAND,0x4c};
        *effect=5;
    }
    return 1;
}
u8 RT_transitionCommandsEnd(u8 *selector,u8 *effect,RT_TransitionCommands *commands)
{
    if(!selector || *selector>10 || !effect || !commands)return 0;
    commands->count=2;
    commands->events[0]=(RT_TransitionEvent){RT_TRANSITION_AUDIO,0x8c};
    commands->events[1]=(RT_TransitionEvent){RT_TRANSITION_SCENE_COMMAND,0x2b};
    *effect=0;(*selector)++;return 1;
}
