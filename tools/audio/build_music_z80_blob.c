/* Host build tool: converts rtArcadeMusicBank plus the YM2151->YM2612/PSG
 * translation tables into one blob the Z80 music driver reads through its
 * 32 KB bank window. Output: a C file defining rtMusicZ80Blob (32 KB aligned,
 * linked after boot data at 0x80000+, ROM window 1).
 * Build: clang -I<host genesis.h> -I<inc> build_music_z80_blob.c arcade_music_bank.c
 * Usage: build_music_z80_blob OUT_C MAPS_H
 *
 * Layout (little-endian words; offsets from blob start = Z80 $8000):
 *   header  16 words: offsets of EVENTS DATA PATCHES TRACKS FN PSG DT2MUL DECAY
 *           RELEASE LFO FMS AMS TL38 MAPS DT2PITCH, then eventCount
 *   EVENTS  4 bytes: kind|idxHi<<5, mask (value hi for 16-bit jumps), value lo, idx lo
 *           idx = 8-byte DATA block, 0x7FF none
 *   DATA    8-byte blocks; INSTRUMENTS blocks hold 8 patch numbers (0xFF none);
 *           REPEAT blocks hold firstTarget, repeatTarget (LE words)
 *   PATCHES 28-byte YM2151 channel patches, addressed via a word table
 *   TRACKS  trackCount words (0xFFFF none)
 *   FN      768 words: YM2612 F-number at block 4 for semitone*64+fraction
 *   PSG     768 words: PSG period at OPM octave 0 for semitone*64+fraction
 *   DT2MUL  64 bytes, DECAY 32 words, RELEASE 16 words, LFO 256 bytes (0x22 value)
 *   FMS     8x128 bytes, AMS 4x128 bytes, TL38 128 bytes, MAPS 14x8, DT2PITCH 4 words */
#include "rt_music_sequence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static unsigned char blob[65536];static unsigned size;
static unsigned put8(unsigned v){blob[size]=v;return size++;}
static void put16(unsigned v){put8(v&255);put8(v>>8);}
static void set16(unsigned at,unsigned v){blob[at]=v;blob[at+1]=v>>8;}

int main(int argc,char**argv){
 const RT_MusicEventBank*b=&rtArcadeMusicBank;
 /* Channel maps come from rt_game_music_maps.h, included by the caller's -D. */
 static const u8 maps[14][8]={
#include "rt_game_music_maps_rows.h"
 };
 if(b->trackCount!=38){fprintf(stderr,"driver assumes 38 tracks\n");return 2;}
 unsigned header=size;for(int i=0;i<16;i++)put16(0);
 /* Collect unique data blocks and patches. */
 const u8 *blockPtr[2048];int blockKind[2048],nb=0;
 const u8 *patchPtr[1024];int np=0;
 int idx[8192];
 for(int i=0;i<b->eventCount;i++){
  const RT_MusicEvent*e=b->events+i;idx[i]=0x7FF;
  if(!e->data)continue;
  int kind=e->kind==RT_MUSIC_EVENT_INSTRUMENTS?1:(e->kind==RT_MUSIC_EVENT_REPEAT||e->kind==RT_MUSIC_EVENT_CONDITIONAL)?2:0;
  int f=-1;for(int j=0;j<nb;j++)if(blockPtr[j]==e->data&&blockKind[j]==kind){f=j;break;}
  if(f<0){f=nb;blockPtr[nb]=e->data;blockKind[nb++]=kind;}
  if(f>=0x7FF){fprintf(stderr,"too many blocks\n");return 2;}
  idx[i]=f;
 }
 /* EVENTS */
 unsigned evOff=size;
 for(int i=0;i<b->eventCount;i++){
  const RT_MusicEvent*e=b->events+i;unsigned mask=e->mask;
  if(e->kind==RT_MUSIC_EVENT_JUMP){
   if(mask){fprintf(stderr,"jump with mask\n");return 2;}
   mask=e->value>>8;            /* the driver reads a jump's high byte here */
  }else if(e->value>255){fprintf(stderr,"16-bit value on kind %d\n",e->kind);return 2;}
  put8(e->kind|((idx[i]>>8)<<5));put8(mask);put8(e->value&255);put8(idx[i]&255);
 }
 /* DATA */
 unsigned dataOff=size;
 for(int j=0;j<nb;j++){
  const u8*d=blockPtr[j];
  if(blockKind[j]==1){
   for(int c=0;c<8;c++){
    const u8*p=d+c*28;int f=-1;
    for(int k=0;k<np;k++)if(!memcmp(patchPtr[k],p,28)){f=k;break;}
    if(f<0){f=np;patchPtr[np++]=p;}
    if(f>=255){fprintf(stderr,"too many patches\n");return 2;}
    put8(f);
   }
  }else if(blockKind[j]==2){
   put16((d[0]<<8)|d[1]);put16((d[2]<<8)|d[3]);put16(0);put16(0);
  }else for(int c=0;c<8;c++)put8(d[c]);
 }
 /* PATCHES: word table then bodies */
 unsigned patchOff=size;for(int k=0;k<np;k++)put16(0);
 for(int k=0;k<np;k++){set16(patchOff+2*k,0x8000+size);for(int i=0;i<28;i++)put8(patchPtr[k][i]);}
 /* TRACKS */
 unsigned trackOff=size;for(int t=0;t<b->trackCount;t++)put16(b->tracks[t]);
 /* FN / PSG for semitone*64+fraction (linear interpolation like rt_opn_music.c) */
 static const u16 fn[13]={682,723,766,811,859,910,965,1022,1083,1147,1215,1288,1364};
 static const u16 ps[13]={6457,6095,5753,5430,5125,4837,4566,4310,4068,3839,3624,3420,3229};
 unsigned fnOff=size;for(int i=0;i<768;i++){int s=i>>6,k=i&63;put16(fn[s]+(((fn[s+1]-fn[s])*k)>>6));}
 unsigned psgOff=size;for(int i=0;i<768;i++){int s=i>>6,k=i&63;put16(ps[s]-(((ps[s]-ps[s+1])*k)>>6));}
 static const u8 dt2Mul[64]={0,1,1,1,1,2,2,2,2,3,3,3,3,4,5,5,4,6,6,7,5,7,8,9,6,9,9,10,7,10,11,12,
  8,11,13,14,9,13,14,15,10,14,15,15,11,15,15,15,12,15,15,15,13,15,15,15,14,15,15,15,15,15,15,15};
 unsigned dt2Off=size;for(int i=0;i<64;i++)put8(dt2Mul[i]);
 static const u16 decay[32]={0,1,1,2,3,4,5,7,10,15,21,30,42,59,84,118,167,236,334,
  473,669,946,1337,1891,2675,3783,5349,7565,10699,15131,21398,30261};
 static const u16 rel[16]={1,2,4,7,15,30,59,118,236,473,946,1891,3783,7565,15131,30261};
 unsigned decOff=size;for(int i=0;i<32;i++)put16(decay[i]);
 unsigned relOff=size;for(int i=0;i<16;i++)put16(rel[i]);
 static const unsigned lfoMid[7]={91575,111156,118932,127187,158480,554822,1154760};
 unsigned lfoOff=size;
 for(int l=0;l<256;l++){unsigned step=(16u+(l&15))<<(l>>4);int i=0;while(i<7&&step>=lfoMid[i])i++;put8(8|i);}
 static const u16 pmSens[8]={0,62,124,248,496,992,3970,7940},fmsMid[7]={17,50,83,120,170,300,600};
 static const u16 amSens[4]={0,239,478,956};static const u8 amsMid[3]={7,36,88};
 unsigned fmsOff=size;
 for(int p=0;p<8;p++)for(int d=0;d<128;d++){unsigned v=(pmSens[p]*d)>>7;int i=0;while(i<7&&v>=fmsMid[i])i++;put8(i);}
 unsigned amsOff=size;
 for(int a=0;a<4;a++)for(int d=0;d<128;d++){unsigned v=(amSens[a]*d)>>7;int i=0;while(i<3&&v>=amsMid[i])i++;put8(i<<4);}
 unsigned tlOff=size;for(int t=0;t<128;t++)put8((t*3)>>3);
 unsigned mapOff=size;for(int t=0;t<14;t++)for(int c=0;c<8;c++)put8(maps[t][c]);
 unsigned dt2pOff=size;put16(0);put16(384);put16(500);put16(608);
 unsigned offs[15]={evOff,dataOff,patchOff,trackOff,fnOff,psgOff,dt2Off,decOff,relOff,lfoOff,fmsOff,amsOff,tlOff,mapOff,dt2pOff};
 for(int i=0;i<15;i++)set16(header+2*i,0x8000+offs[i]);
 set16(header+30,b->eventCount);
 fprintf(stderr,"blob %u bytes: events %u data %u (%d blocks) patches %d tracks %d tables %u\n",
  size,dataOff-evOff,patchOff-dataOff,nb,np,b->trackCount,size-fnOff);
 if(size>32768){fprintf(stderr,"blob exceeds the 32 KB Z80 window\n");return 3;}
 FILE*f=fopen(argv[1],"w");
 fprintf(f,"/* Generated by tools/audio/build_music_z80_blob.c -- do not edit. */\n#include <genesis.h>\n");
 fprintf(f,"/* Z80 music driver data: whole 32 KB bank window, in ROM window 1 (0x80000+),\n * which only the ending/high-score PCM driver remaps (never with this driver loaded). */\n");
 fprintf(f,"const u8 rtMusicZ80Blob[%u] __attribute__((aligned(32768),section(\".rodata_binf\")))={\n",size);
 for(unsigned i=0;i<size;i++)fprintf(f,"%u%s",blob[i],i+1<size?(i%32==31?",\n":","):"\n};\n");
 fclose(f);return 0;
}
