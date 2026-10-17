#!/usr/bin/env python3
"""Extract reachable musical records and control edges from original MCU data.
No CPU code is emitted. Does not claim playback, register or audio parity.
"""
from pathlib import Path
from zipfile import ZipFile
from collections import Counter
import hashlib,json,argparse
root=Path(__file__).resolve().parents[1]
def extract():
 with ZipFile(root/'rthunder.zip') as z:rom=z.read('rt3_4.6b')
 assert hashlib.sha256(rom).hexdigest()=='4ab04d23bc2fd1bd788122e39ad94451781e5b370387c64a1abf60f02ca940c3'
 def data(at,n):
  if not 0x4000<=at<=0xc000-n:raise ValueError(f'data range {at:04x}+{n}')
  return rom[at-0x4000:at-0x4000+n]
 def byte(at):return data(at,1)[0]
 def word(at):return int.from_bytes(data(at,2),'big')
 handlers=[0x860b,0x8734,0x8769,0x877e,0x85ff,0x85ff,0x85e5,0x878d,0x860c,0x87a3,0x8689,0x85ff,0x86d8,0x85e4,0x85f7,0x85ff,0x85ba,0x850c,0x8565,0x885a,0x8706,0x87ea,0x8805,0x883f,0x8822]
 assert [word(0x84da+i*2) for i in range(25)]==handlers
 names=['yield','master_gain','tempo_base','tempo_multiplier','track','track','call','end','key_notes','instruments','channel_gains','track','pitch_fraction','yield','jump','track','key_off','repeat','conditional','pitch_only','channel_modulation','lfo_frequency','phase_modulation_depth','lfo_waveform','amplitude_modulation_depth']
 tracks={i:word(0x9ad9+i*2) for i in range(1,14)};pending=list(tracks.values());nodes={};occupied={}
 while pending:
  at=pending.pop()
  if at in nodes:continue
  op=byte(at)
  if op>=len(names):raise ValueError(f'unknown music command {op:02x} at{at:04x}')
  node=dict(address=at,event=names[op]);p=at+1;edges=[]
  if op in (8,9,10,12,16,19,20):
   mask=byte(p);p+=1;channels=[]
   for ch in range(8):
    if mask&(128>>ch):
     c=dict(channel=ch)
     if op!=16:c['value']=byte(p);p+=1
     if op==9:c['operator_data']=data(0x8899+c['value']*32,32).hex()
     channels.append(c)
   node['channels']=channels;node['next']=p;edges.append(p)
  elif op in (1,2,3,21,22,23,24):node['value']=byte(p);p+=1;node['next']=p;edges.append(p)
  elif op in (4,5,6,11,15):
   selector=byte(p);p+=1;tracks[selector]=word(0x9ad9+selector*2)
   node['track']=selector;edges.append(tracks[selector])
   if op==6:node['return']=p;edges.append(p)
  elif op==14:node['target']=word(p);p+=2;edges.append(node['target'])
  elif op in (17,18):
   node['count']=byte(p);node['target']=word(p+1);p+=3
   node['next']=p;edges.extend([node['target'],p])
   if op==17:
    # 8553 reads indexed+1 AFTER854C increments X. Keep this distinct
    # from853B's first-pass target; never silently repair the source code.
    node['subsequent_target']=word(at+3)
    if node['count']!=2:raise ValueError('repeat requires further reachability analysis')
  elif op in (0,13):node['next']=p;edges.append(p)
  node['size']=p-at;node['source_bytes']=data(at,p-at).hex()
  for address in range(at,p):
   if address in occupied:raise ValueError(f'overlapping music record at{address:04x}')
   occupied[address]=at
  nodes[at]=node;pending.extend(edges)
 result=dict(format='native-music-graph-v1',source_sha256=hashlib.sha256(rom).hexdigest(),selectable_tracks=list(range(1,14)),tracks=[dict(selector=i,address=a) for i,a in sorted(tracks.items())],events=[nodes[a] for a in sorted(nodes)])
 prior=json.loads((root/'assets/extracted/logic/game_over_music_events.json').read_text())
 at=tracks[12]
 for event in prior['events']:
  node=nodes[at]
  assert {k:v for k,v in node.items() if k in ('event','value','channels')}=={k:v for k,v in event.items() if k!='tick'}
  if node['event']!='end':at=node['next']
 assert nodes[at]['event']=='end'
 report=dict(status='extracted',game_over_matched_events=len(prior['events']),records=len(nodes),selectable_tracks=13,reachable_tracks=len(tracks),event_counts=dict(Counter(n['event'] for n in nodes.values())),repeats=[n for n in nodes.values() if n['event'] in ('repeat','conditional')],scope='Reachable source musical data and handler-table verification. Repeat count2 makes the source subsequent-target branch unreachable in a fresh correctly serviced loop. No runtime sequencer/audio integration or playback parity claimed.')
 return result,report
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');a=p.parse_args()
 graph,report=extract();dest=root/'assets/extracted/logic/music_graph.json';payload=json.dumps(graph,indent=2)+'\n'
 if a.check:assert dest.read_text()==payload
 else:dest.write_text(payload)
 (root/'out/research/music_graph_extraction.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
