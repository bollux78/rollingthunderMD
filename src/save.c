#include "rt_save.h"
#define SLOT_SIZE 64
#define PAYLOAD 10
#define CHECKSUM 60
static u8 previous[41],havePrevious,activeSlot;
static u32 generation;
static u8 cached[2][SLOT_SIZE],warmReset;
volatile u8 rtSaveAvailable;
static u32 get32(const u8 *p){return ((u32)p[0]<<24)|((u32)p[1]<<16)|((u32)p[2]<<8)|p[3];}
static void put32(u8 *p,u32 n){p[0]=n>>24;p[1]=n>>16;p[2]=n>>8;p[3]=n;}
static u32 crc(const u8 *p)
{
    u32 c=0xffffffffUL;
    for(u8 i=0;i<CHECKSUM;i++){
        c^=p[i];for(u8 j=0;j<8;j++)c=(c>>1)^((c&1)?0xedb88320UL:0);
    }
    return ~c;
}
static bool valid(const u8 *p)
{
    if(p[0]!='R'||p[1]!='T'||p[2]!='M'||p[3]!='D'||p[4]!=1||p[5]!=41||
       get32(p+CHECKSUM)!=crc(p)||p[10]<1||p[10]>9||p[11]>1||get32(p+12)>999999)return FALSE;
    u32 last=999999;
    for(u8 r=0;r<5;r++){
        const u8 *row=p+16+r*7;u32 score=0;
        for(u8 j=0;j<3;j++){
            if((row[j]>>4)>9||(row[j]&15)>9)return FALSE;
            score=score*100+(row[j]>>4)*10+(row[j]&15);
        }
        if(score>last||score>get32(p+12)||!row[3]||(row[3]>9&&row[3]!=0x10))return FALSE;
        for(u8 j=4;j<7;j++)if(row[j]<10||row[j]>41)return FALSE;
        last=score;
    }
    return TRUE;
}
/* SRAM replaces ROM at $200000. Hold both interrupt and Z80 consumers until
 * the mapper is back in ROM mode; all save code/data live in the fixed bank. */
static bool enter(void){SYS_disableInts();bool bus=Z80_getAndRequestBus(TRUE);SRAM_enable();return bus;}
static void leave(bool bus){SRAM_disable();if(!bus)Z80_releaseBus();SYS_enableInts();}
static void readSlot(u8 slot,u8 *p){for(u8 i=0;i<SLOT_SIZE;i++)p[i]=SRAM_readByte(slot*SLOT_SIZE+i);}
bool RT_saveLoad(u8 records[35],u32 *best,u8 *credits,u8 *psgMuted)
{
    u8 *a=cached[0],*b=cached[1];havePrevious=0;generation=0;activeSlot=1;
    if(warmReset){
        /* The wrapper restored the mirror after clearing application state.
         * Reads through PicoDrive's remapped SSF window would return ROM. */
        warmReset=0;
    }else{
        rtSaveAvailable=0;
        bool bus=enter();readSlot(0,a);readSlot(1,b);
        u8 probe=SRAM_readByte(128);
        SRAM_writeByte(128,probe^0xff);
        rtSaveAvailable=SRAM_readByte(128)==(u8)(probe^0xff);
        SRAM_writeByte(128,probe);leave(bus);
    }
    bool va=valid(a),vb=valid(b);if(!va&&!vb)return FALSE;
    activeSlot=vb&&(!va||(s32)(get32(b+6)-get32(a+6))>0)?1:0;
    const u8 *p=activeSlot?b:a;generation=get32(p+6);
    memcpy(previous,p+PAYLOAD,sizeof previous);havePrevious=1;
    *credits=p[10];*psgMuted=p[11];*best=get32(p+12);memcpy(records,p+16,35);return TRUE;
}
bool RT_saveStore(const u8 records[35],u32 best,u8 credits,u8 psgMuted)
{
    if(!rtSaveAvailable)return FALSE;
    u8 data[SLOT_SIZE]={0};
    data[0]='R';data[1]='T';data[2]='M';data[3]='D';data[4]=1;data[5]=41;
    put32(data+6,generation+1);data[10]=credits;data[11]=psgMuted;put32(data+12,best);memcpy(data+16,records,35);
    put32(data+CHECKSUM,crc(data));if(!valid(data))return FALSE;
    if(havePrevious&&!memcmp(previous,data+PAYLOAD,sizeof previous))return TRUE;
    u8 target=activeSlot^1;u16 base=target*SLOT_SIZE;
    bool bus=enter();
    SRAM_writeByte(base,0); /* invalidate first; commit magic last */
    for(u8 i=1;i<SLOT_SIZE;i++)SRAM_writeByte(base+i,data[i]);
    SRAM_writeByte(base,data[0]);leave(bus);
    /* CRC and commit marker are checked on the next cold boot. Do not read
     * back through a window that an SSF emulator may now map directly to ROM. */
    /* Publish the inactive mirror with the same commit-last discipline.
     * A reset during this copy still has the other CRC-valid slot. */
    cached[target][0]=0;
    __asm__ volatile("" ::: "memory");
    memcpy(cached[target]+1,data+1,SLOT_SIZE-1);
    __asm__ volatile("" ::: "memory");
    cached[target][0]=data[0];
    memcpy(previous,data+PAYLOAD,sizeof previous);havePrevious=1;activeSlot=target;generation++;return TRUE;
}

void RT_saveResetExport(RT_SaveReset *state)
{
    memcpy(state->slots,cached,sizeof cached);state->available=rtSaveAvailable;
}
void RT_saveResetImport(const RT_SaveReset *state)
{
    memcpy(cached,state->slots,sizeof cached);rtSaveAvailable=state->available;
    warmReset=1;
}
