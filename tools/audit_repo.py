#!/usr/bin/env python3
"""Check tracked source for accidentally included ROMs, build outputs and intros."""
from pathlib import Path
import subprocess,sys
root=Path(__file__).resolve().parents[1]
files=subprocess.check_output(['git','ls-files','-z'],cwd=root).decode().split('\0')
forbidden={'.bin','.zip','.7z','.rar','.smd','.gen','.mdrom','.pcm','.wav','.o','.elf','.raw','.pyc'}
errors=[]
for name in filter(None,files):
 p=Path(name)
 if p.suffix.lower() in forbidden or any(x in p.parts for x in ('.build','dist','roms','__pycache__')):errors.append(name)
 if any(x in p.name.lower() for x in ('sega_logo','sega_chant','namcot_logo','boot_intros')):errors.append(name)
 if (root/p).is_symlink():errors.append(name+' (symlink)')
 if (root/p).stat().st_size>50*1024*1024:errors.append(name+' (>50 MiB)')
 if p.suffix in ('.c','.h','.s','.s80','.res','.py'):
  text=(root/p).read_text()
  if ('/'+'Users/') in text or ('/'+'home/') in text:errors.append(name+' (machine-specific path)')
  if p.suffix in ('.c','.h','.s','.s80') and any(x in text for x in ('sega_chant_pcm','rtNamcotTiles','RT_bootIntros')):errors.append(name+' (intro asset/reference)')
if errors:raise SystemExit('\n'.join(errors))
print(f'Source audit passed: {len(list(filter(None,files)))} tracked files; no ROM/output archives or intro assets.')
