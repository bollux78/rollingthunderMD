#include "rt_stage_events.h"
u8 RT_eventWindowInitialize(RT_EventWindow *window,const RT_WorldStage *stage,u8 entrance)
{
    if(!window || !stage || entrance>1)return 0;
    const RT_WorldCamera *camera=stage->entrances+entrance;
    u16 upward=(u16)((s32)stage->heightCells*8-camera->y-256);
    window->gridX=(u8)(((u16)camera->x>>8)+1);
    window->gridY=(u8)((upward>>8)+1);
    return 1;
}
static void request(RT_EventWindow *window,RT_EventWindowRequest *result,u8 edge)
{
    result->edge=edge;result->gridX=window->gridX;result->gridY=window->gridY;
    if(edge==RT_EVENT_WINDOW_RIGHT)window->gridX++;
    else if(edge==RT_EVENT_WINDOW_LEFT)window->gridX--;
    else if(edge==RT_EVENT_WINDOW_UP)window->gridY++;
    else window->gridY--;
}
u8 RT_eventWindowMove(RT_EventWindow *window,const RT_WorldStage *stage,
    const RT_WorldCamera *previous,const RT_WorldCamera *current,
    RT_EventWindowRequest requests[2])
{
    if(!window || !stage || !previous || !current || !requests)return 255;
    s32 dx=(s32)current->x-previous->x,dy=(s32)current->y-previous->y;
    if(dx < -16 || dx>16 || dy < -16 || dy>16)return 255;
    u16 upward=(u16)((s32)stage->heightCells*8-current->y-256);
    u8 x=(u8)(((u16)current->x+144)>>8),y=(u8)((upward+128)>>8);
    u8 horizontal=0,vertical=0,count=0;
    if(dx>0 && x==window->gridX)horizontal=RT_EVENT_WINDOW_RIGHT;
    else if(dx<0 && x==(u8)(window->gridX-2))horizontal=RT_EVENT_WINDOW_LEFT;
    if(dy<0 && y==window->gridY)vertical=RT_EVENT_WINDOW_UP;
    else if(dy>0 && y==(u8)(window->gridY-2))vertical=RT_EVENT_WINDOW_DOWN;
    if(vertical)request(window,requests+count++,vertical);
    if(horizontal)request(window,requests+count++,horizontal);
    return count;
}
