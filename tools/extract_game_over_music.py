#!/usr/bin/env python3
"""Translate game-over music data into named, timed native-port events.

Offline data conversion only. No MCU instructions are executed, and no CPU
program or source music bytecode is emitted into a cartridge resource.
The resulting JSON is not yet connected to a Genesis audio backend.
"""
from pathlib import Path
from zipfile import ZipFile
import hashlib,json
ROOT=Path(__file__).resolve().parents[1]

def extract():
    with ZipFile(ROOT/'rthunder.zip') as z:rom=z.read('rt3_4.6b')
    def word(addr):return int.from_bytes(rom[addr-0x4000:addr-0x4000+2],'big')
    # Dispatch and tempo/end handlers establish the data contract below.
    signatures={0x8481:'f6138027d0c1ff272b',0x84b5:'b61385f615723df31576fd1576250139',
        0x8769:'cc0000fd1576fe156ee60008ff156ef713857e84c5',
        0x877e:'fe156ee60008ff156ef715727e84c5',0x878d:'b6158a26054fb7138039'}
    for addr,h in signatures.items():
        expected=bytes.fromhex(h)
        if rom[addr-0x4000:addr-0x4000+len(expected)]!=expected:raise ValueError(hex(addr))
    names={1:'master_gain',2:'tempo_base',3:'tempo_multiplier',7:'end',8:'key_notes',
           9:'instruments',10:'channel_gains',12:'pitch_fraction',19:'pitch_only',
           20:'channel_modulation',21:'lfo_frequency',22:'phase_modulation_depth',
           23:'lfo_waveform',24:'amplitude_modulation_depth'}
    expected_handlers={1:0x8734,2:0x8769,3:0x877e,7:0x878d,8:0x860c,9:0x87a3,
        10:0x8689,12:0x86d8,19:0x885a,20:0x8706,21:0x87ea,22:0x8805,23:0x883f,24:0x8822}
    for opcode,handler in expected_handlers.items():assert word(0x84da+opcode*2)==handler
    pc=word(0x9ad9+0x0c*2);start=pc;events=[];trace=[]
    tick=0;accumulator=0;tempo_base=0;tempo_multiplier=0
    def byte():
        nonlocal pc
        if not 0x4000<=pc<0xc000:raise ValueError(hex(pc))
        value=rom[pc-0x4000];pc+=1;return value
    while len(events)<4096:
        address=pc;opcode=byte()
        if opcode not in names:raise ValueError(f'Unsupported music data command {opcode:02x} at {address:04x}')
        event=dict(tick=tick,event=names[opcode])
        if opcode in (8,9,10,12,19,20):
            mask=byte();channels=[]
            for channel in range(8):
                if mask&(128>>channel):channels.append(dict(channel=channel,value=byte()))
            event['channels']=channels
            if opcode==9:
                for channel in channels:
                    pos=0x8899+channel['value']*32
                    channel['operator_data']=rom[pos-0x4000:pos-0x4000+32].hex()
                    assert len(channel['operator_data'])==64
        elif opcode!=7:event['value']=byte()
        if opcode==2:tempo_base=event['value'];accumulator=0
        if opcode==3:tempo_multiplier=event['value']
        events.append(event);trace.append(dict(address=address,opcode=opcode,tick=tick))
        if opcode==7:break
        if opcode==8:
            if not tempo_base or not tempo_multiplier:raise ValueError('Stopped tempo')
            while True:
                tick+=1;total=accumulator+tempo_base*tempo_multiplier
                accumulator=total&65535
                if total>65535:break
    else:raise ValueError('No end event')
    output=dict(format='native-music-events-v1',track='game_over',source_sha256=hashlib.sha256(rom).hexdigest(),
        clock='one source sequencer service per tick; 16-bit carry accumulator',events=events,
        scope='Data conversion and source timing contract; no Genesis audio backend or game-over handoff yet.')
    dest=ROOT/'assets/extracted/logic/game_over_music_events.json';dest.write_text(json.dumps(output,indent=2)+'\n')
    audit=dict(start_address=start,end_address=pc,source_handler_signatures=len(signatures),event_count=len(events),end_tick=tick,commands=trace)
    (ROOT/'out/research/game_over_music_extraction.json').write_text(json.dumps(audit,indent=2)+'\n')
    print({k:v for k,v in audit.items() if k!='commands'})
if __name__=='__main__':extract()
