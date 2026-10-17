#include "rt_opn_music.h"

/* YM2612 F-numbers at block 4 for OPM semitones C#..C (key-code order after
 * removing the gaps), plus the next C# for fraction interpolation. Source OPM
 * at 3579545 Hz puts octave 4 note A at 440 Hz; Genesis YM2612 at 7670453 Hz. */
static const u16 fnTable[13]={682,723,766,811,859,910,965,1022,1083,1147,1215,1288,1364};
/* PSG periods (3579545 Hz / 32) for the same semitones at OPM octave 0. */
static const u16 psgTable[13]={6457,6095,5753,5430,5125,4837,4566,4310,4068,3839,3624,3420,3229};
/* Per-service envelope advance in 1/256 PSG steps (2 dB) for OPM decay rates
 * (5-bit) and release rates (4-bit), at the 2000/33 Hz source service. */
static const u16 decayTable[32]={0,1,1,2,3,4,5,7,10,15,21,30,42,59,84,118,167,236,334,
    473,669,946,1337,1891,2675,3783,5349,7565,10699,15131,21398,30261};
static const u16 releaseTable[16]={1,2,4,7,15,30,59,118,236,473,946,1891,3783,7565,15131,30261};
/* OPM LFO increments ((16+LFRQ low)<<LFRQ high) at the YM2612 LFO midpoints. */
static const u32 lfoMid[7]={91575,111156,118932,127187,158480,554822,1154760};
/* Phase depth at PMD 127 (0.1 cent) and YM2612 FMS midpoints. */
static const u16 pmSens[8]={0,62,124,248,496,992,3970,7940};
static const u16 fmsMid[7]={17,50,83,120,170,300,600};
/* Amplitude depth at AMD 127 (0.1 dB) and YM2612 AMS midpoints. */
static const u16 amSens[4]={0,239,478,956};
static const u8 amsMid[3]={7,36,88};
/* YM2612 MUL nearest to OPM MUL x DT2 (x1, x1.41, x1.57, x1.73), MUL 0 = x0.5. */
static const u8 dt2Mul[64]={0,1,1,1,1,2,2,2,2,3,3,3,3,4,5,5,4,6,6,7,5,7,8,9,6,9,9,10,7,10,11,12,
    8,11,13,14,9,13,14,15,10,14,15,15,11,15,15,15,12,15,15,15,13,15,15,15,14,15,15,15,15,15,15,15};

#define ENV_MAX 12288

static u8 nearest(u16 value,const void *mid,u8 count,u8 wide)
{
    u8 i=0;
    while(i<count && value>=(wide?((const u16*)mid)[i]:((const u8*)mid)[i]))i++;
    return i;
}

static void fmWrite(RT_OpnMusic *m,u8 ch,u8 reg,u8 value)
{
    m->fm(ch>=3,reg+(ch>=3?ch-3:ch),value);
}

static void fmPan(RT_OpnMusic *m,u8 source,u8 ch)
{
    const RT_OpnSource *s=m->src+source;
    u8 pms=(s->pmsams>>4)&7,ams=s->pmsams&3;
    u8 fms=nearest((u16)(((u32)pmSens[pms]*m->pmd)>>7),fmsMid,7,1);
    u8 amsOut=nearest((u16)(((u32)amSens[ams]*m->amd)>>7),amsMid,3,0);
    /* OPM bit6 = left, bit7 = right; YM2612 bit7 = left, bit6 = right. */
    u8 lr=((s->rl&0x40)<<1)|((s->rl&0x80)>>1);
    fmWrite(m,ch,0xB4,lr|(amsOut<<4)|fms);
}

static void lfo(RT_OpnMusic *m)
{
    u8 value=0;
    if(m->pmd || m->amd){
        u32 step=(u32)(16+(m->lfrq&15))<<(m->lfrq>>4);
        u8 i=0;while(i<7 && step>=lfoMid[i])i++;
        value=8|i;
    }
    m->fm(0,0x22,value);
}

/* Writes block/F-number for a pitch in 1/64 semitones above OPM octave 0
 * note C#, at the given high/low register pair. */
static void fmFrequency(RT_OpnMusic *m,u8 port,u8 high,u8 low,u16 pitch64)
{
    u8 oct=pitch64/768;u16 rest=pitch64%768;
    u8 semi=rest>>6,kf=rest&63;
    u16 fnum=fnTable[semi]+(((fnTable[semi+1]-fnTable[semi])*kf)>>6);
    if(oct>7){fnum<<=oct-7;oct=7;if(fnum>2047)fnum=2047;}
    m->fm(port,high,(oct<<3)|(fnum>>8));
    m->fm(port,low,(u8)fnum);
}

/* OPM DT2 offsets in 1/64 semitones (600, 781 and 950 cents). */
static const u16 dt2Pitch[4]={0,384,500,608};
/* Channel-3 special-mode registers for OPM operator order M1,M2,C1,C2. */
static const u8 specialLow[4]={0xA9,0xA8,0xAA,0xA2};

static void pitch(RT_OpnMusic *m,u8 source)
{
    const RT_OpnSource *s=m->src+source;
    u8 target=m->map[source];
    u8 code=s->kc&15;
    u16 pitch64=((s->kc>>4)&7)*768+(code-(code>>2))*64+(s->kf>>2);
    if(target==RT_OPN_SPECIAL){
        /* Per-operator frequencies reproduce DT2 exactly. */
        for(u8 op=0;op<4;op++)
            fmFrequency(m,0,specialLow[op]+4,specialLow[op],pitch64+dt2Pitch[s->dt2[op]]);
    }else if(target<6)fmFrequency(m,target>=3,0xA4+target%3,0xA0+target%3,pitch64);
    else if(target!=RT_OPN_MUTE){
        u8 oct=pitch64/768;u16 rest=pitch64%768;
        u8 semi=rest>>6,kf=rest&63;
        u16 period=psgTable[semi]-(((psgTable[semi]-psgTable[semi+1])*kf)>>6);
        period=(period+(1<<oct>>1))>>oct;
        while(period>1023)period>>=1;
        u8 voice=(target-RT_OPN_PSG)<<5;
        m->psg(0x80|voice|(period&15));
        m->psg((u8)(period>>4));
    }
}

static void psgLevel(RT_OpnMusic *m,u8 source)
{
    RT_OpnSource *s=m->src+source;
    s16 att=15;
    if(s->stage && (s->rl&0xC0)){
        att=(s16)(((u16)s->tl*3)>>3)+(s16)(s->env>>8)+m->psgOffset;
        if(att<0)att=0;
        if(att>15)att=15;
    }
    if(s->psgAtt==att)return;
    s->psgAtt=att;
    m->psg(0x90|((m->map[source]-RT_OPN_PSG)<<5)|att);
}

void RT_opnMusicReset(RT_OpnMusic *m,const u8 map[8],RT_OpnFmWrite fm,RT_OpnPsgWrite psg,RT_OpnSettle settle)
{
    memset(m,0,sizeof *m);
    m->fm=fm;m->psg=psg;m->settle=settle;m->psgOffset=0;
    for(u8 i=0;i<8;i++){m->map[i]=map[i];m->src[i].psgAtt=0xFF;m->src[i].tl=0x7F;}
    fm(0,0x22,0);fm(0,0x27,0x40);fm(0,0x2B,0);
    for(u8 ch=0;ch<6;ch++){
        m->fm(0,0x28,ch<3?ch:ch+1);
        for(u8 op=0;op<16;op+=4){
            fmWrite(m,ch,0x40+op,0x7F);fmWrite(m,ch,0x80+op,0xFF);fmWrite(m,ch,0x90+op,0);
        }
        fmWrite(m,ch,0xB4,0);
    }
    for(u8 v=0;v<4;v++)psg(0x9F|(v<<5));
}

void RT_opnMusicSilence(RT_OpnMusic *m)
{
    memset(m->pendingOn,0,sizeof m->pendingOn);m->offMask=0;
    for(u8 i=0;i<8;i++){
        u8 target=m->map[i];
        if(target<6){
            m->fm(0,0x28,target<3?target:target+1);
            for(u8 op=0;op<16;op+=4){fmWrite(m,target,0x80+op,0xFF);fmWrite(m,target,0x40+op,0x7F);}
        }else if(target!=RT_OPN_MUTE){m->src[i].stage=0;psgLevel(m,i);}
    }
}

void RT_opnMusicTick(RT_OpnMusic *m)
{
    u8 settled=0;
    for(u8 ch=0;ch<6;ch++){
        u8 mask=m->pendingOn[ch];
        if(!mask)continue;
        if(!settled && (m->offMask&(1<<ch))){m->settle();settled=1;}
        m->pendingOn[ch]=0;
        m->fm(0,0x28,(mask<<4)|(ch<3?ch:ch+1));
    }
    m->offMask=0;
    for(u8 i=0;i<8;i++){
        RT_OpnSource *s=m->src+i;
        if(m->map[i]<RT_OPN_PSG || m->map[i]==RT_OPN_MUTE || !s->stage)continue;
        u16 step=s->stage==1?decayTable[s->d1r]:s->stage==2?decayTable[s->d2r]:releaseTable[s->d1lrr&15];
        u32 env=(u32)s->env+step;
        if(s->stage==1){
            u16 sustain=(s->d1lrr>>4)==15?ENV_MAX:(u16)(s->d1lrr>>4)*384;
            if(env>=sustain){env=sustain;s->stage=2;}
        }
        if(env>=ENV_MAX){env=ENV_MAX;if(s->stage==3)s->stage=0;}
        s->env=(u16)env;
        psgLevel(m,i);
    }
}

void RT_opnMusicWrite(u8 reg,u8 value,void *context)
{
    RT_OpnMusic *m=context;
    if(reg==0x08){
        u8 source=value&7,target=m->map[source],mask=(value>>3)&15;
        RT_OpnSource *s=m->src+source;
        if(target<6){
            /* The source re-keys with off-then-on writes; the YM2612 must
             * latch the off, so key-ons wait for the end of the service. */
            m->pendingOn[target]=mask;
            if(!mask){m->fm(0,0x28,target<3?target:target+1);m->offMask|=1<<target;}
        }
        else if(target!=RT_OPN_MUTE){
            if(mask){s->stage=1;s->env=0;}
            else if(s->stage)s->stage=3;
            psgLevel(m,source);
        }
        s->keyed=mask!=0;
        return;
    }
    if(reg==0x18){m->lfrq=value;lfo(m);return;}
    if(reg==0x19){
        if(value&0x80)m->pmd=value&0x7F;else m->amd=value&0x7F;
        lfo(m);
        for(u8 i=0;i<8;i++)if(m->map[i]<6)fmPan(m,i,m->map[i]);
        return;
    }
    if(reg<0x20)return;
    u8 source=reg&7,target=m->map[source];
    RT_OpnSource *s=m->src+source;
    if(target==RT_OPN_MUTE)return;
    if(reg<0x40){
        switch(reg&0xF8){
            case 0x20:
                s->rl=value;
                if(target<6){fmWrite(m,target,0xB0,value&0x3F);fmPan(m,source,target);}
                else psgLevel(m,source);
                break;
            case 0x28:s->kc=value;pitch(m,source);break;
            case 0x30:s->kf=value;pitch(m,source);break;
            default:s->pmsams=value;if(target<6)fmPan(m,source,target);break;
        }
        return;
    }
    /* Operator registers: OPM M1,M2,C1,C2 at +0/+8/+16/+24 are YM2612
     * S1,S3,S2,S4 at +0/+4/+8/+12, so the index carries over directly. */
    u8 op=(reg>>3)&3,kind=reg>>5;
    if(target<6){
        /* DT2 has no YM2612 register; fold its ratio into MUL. */
        if(kind==2)s->dt1mul[op]=value;
        else if(kind==6){
            s->dt2[op]=value>>6;fmWrite(m,target,0x70+op*4,value&0x1F);
            if(target==RT_OPN_SPECIAL){pitch(m,source);return;}
        }else {fmWrite(m,target,((kind+1)<<4)+op*4,value);return;}
        if(target==RT_OPN_SPECIAL){fmWrite(m,target,0x30+op*4,s->dt1mul[op]);return;}
        u8 v=s->dt1mul[op];
        fmWrite(m,target,0x30+op*4,(v&0x70)|dt2Mul[(v&15)*4+s->dt2[op]]);
        return;
    }
    if(op!=3)return;
    switch(kind){
        case 3:s->tl=value&0x7F;psgLevel(m,source);break;
        case 5:s->d1r=value&0x1F;break;
        case 6:s->d2r=value&0x1F;break;
        case 7:s->d1lrr=value;break;
        default:break;
    }
}
