#include "rt_mcu_input.h"

bool RT_mapGenesisControlsToMcuScan(u16 pad1,u16 pad2,u8 scan[37])
{
    /* CUS60 board scan wiring from the source descriptor trace. A is source
       Button2 (jump), B is Button1 (fire/initials confirmation). Other scan
       pins belong to cabinet/credit service and remain caller-owned. */
    static const u8 pins[2][6]={{22,36,29,21,35,34},{30,33,28,20,32,19}};
    static const u16 buttons[6]={BUTTON_A,BUTTON_B,BUTTON_UP,BUTTON_DOWN,BUTTON_LEFT,BUTTON_RIGHT};
    u8 player,control;
    if(!scan)return FALSE;
    for(player=0;player<2;player++){
        u16 pad=player?pad2:pad1;
        for(control=0;control<6;control++)scan[pins[player][control]]=(pad&buttons[control])?0:1;
    }
    return TRUE;
}
