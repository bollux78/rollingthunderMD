#!/usr/bin/env python3
"""Deterministic opening-game pacing sample; rates use emulated refreshes.

This is one workload, not a campaign-wide benchmark. Inputs follow simulation
updates so two builds are not given different input timing when one drops draws.
"""
import argparse
import hashlib
import json
from pathlib import Path
from test_soft_reset import Machine

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--core',type=Path,required=True)
p.add_argument('--rom',type=Path,required=True)
p.add_argument('--symbols',type=Path,required=True)
p.add_argument('--case',choices=['campaign_1p','campaign_2p','survival_2p'],default='campaign_2p')
p.add_argument('--frames',type=int,default=1800)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args();m=Machine(a)
assert m.core.retro_get_region()==0, 'This benchmark expects an NTSC ROM/region'
m.start(a.case)
names=['rtAppUpdates','rtAppPresentations','rtAppFullPresentations','rtAppReusedPresentations','rtAppSkippedPresentations']
def snapshot():return {name:m.number(name,4) for name in names}
before=snapshot();last=before['rtAppUpdates'];stalls=compact=0;longest=run=0
for frame in range(a.frames):
    update=m.number('rtAppUpdates',4)
    m.buttons={(0,7),(0,0),(1,7),(1,0)} if (update//180)%3<2 else {(0,0),(1,0)}
    m.run(1)
    current=m.number('rtAppUpdates',4)
    if current==last:stalls+=1;run+=1;longest=max(longest,run)
    else:run=0
    last=current;compact+=int(m.number('rtCoopCompact')!=0)
after=snapshot();delta={name:after[name]-before[name] for name in names}
# The installed NTSC cores use approximately 59.9227 refreshes per second.
seconds=a.frames/59.922743
report=dict(rom_sha256=hashlib.sha256(a.rom.read_bytes()).hexdigest(),case=a.case,
            refreshes=a.frames,seconds=seconds,counters=delta,
            simulation_updates_per_second=delta['rtAppUpdates']/seconds,
            presentations_per_second=delta['rtAppPresentations']/seconds,
            stalled_refreshes=stalls,longest_stall_refreshes=longest,compact_refreshes=compact,
            final_state=m.number('rtAppState',2),fault=m.number('rtAppFault',2),
            scope='One NTSC opening workload with movement/shooting; emulated timing, not host execution speed or full-game performance.')
a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report))
m.core.retro_unload_game();m.core.retro_deinit()
