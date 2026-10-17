"""Build a raw-pixel cartridge below the Mega EverDrive Pro SRAM bank.

All input banks must first pass build.py's frozen-byte reproduction check.
Derived files stay under .build; the public assets remain the reference.
"""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import json
import re
import struct
from compact_assets import compact_stage, pattern_pool

PAGE = 0x80000
ROM_BYTES = 31 * PAGE
HIGH_ADDRESS = 0xd0000
COMPLETION_OFFSET = 0x4000


def symbols(elf, sdk, run):
    return [(int(v[0], 16), int(v[1], 16), v[3])
            for line in run([sdk/'bin/m68k-elf-nm', '-S', elf]).splitlines()
            if len(v := line.split()) == 4]


def link_stage(folder, objects, base, pool_base, offsets, sdk, run):
    assignments = '\n'.join(f'{name} = {0x80000000 + pool_base + off:#x};'
                            for name, off in offsets.items())
    script = assignments + f'''
OUTPUT_ARCH(m68k)
ENTRY(rtStageBankImage)
SECTIONS {{
 /DISCARD/ : {{ *(.rodata.rtSharedPose_*) *(.comment .note* .eh_frame*) }}
 .stage {base:#x} : {{
  KEEP(*(.stage.header)) *(.text .text.*) *(.rodata .rodata.*)
  . = ALIGN(4); rtStageBankEnd = .;
 }}
 .forbidden : {{ *(.data .data.* .bss .bss.* COMMON) }}
 ASSERT(SIZEOF(.forbidden)==0,"Stage banks cannot own mutable storage")
 ASSERT(rtStageBankImage=={base:#x},"Stage header moved")
 ASSERT(rtStageBankEnd<=0x380000,"Shared graphics require window 7")
}}
'''
    (folder/'compact.ld').write_text(script)
    run([sdk/'bin/m68k-elf-ld', '-T', folder/'compact.ld', '--gc-sections',
         *objects, '-o', folder/'bank.elf'])
    run([sdk/'bin/m68k-elf-objcopy', '-O', 'binary', folder/'bank.elf', folder/'bank.bin'])
    return (folder/'bank.bin').read_bytes()


def relocate_completion(root, sdk, reference, objects_dir, output, delta, run):
    """Relink pointer-bearing scene assets; preserve the frozen footer pixels."""
    output.mkdir(parents=True, exist_ok=True)
    script=(root/'assets/completion/bank.ld').read_text().replace('0x100000',hex(0x100000+delta))
    (output/'bank.ld').write_text(script)
    objects=[objects_dir/(name+'.o') for name in ('scene_background','scene_art','transition_art','completion_monitor_data')]
    run([sdk/'bin/m68k-elf-ld','-T',output/'bank.ld',*objects,'-o',output/'bank.elf'])
    run([sdk/'bin/m68k-elf-objcopy','-O','binary',output/'bank.elf',output/'bank.bin'])
    data=bytearray((output/'bank.bin').read_bytes())
    patch=json.loads((root/'assets/completion/ending_footer.json').read_text())
    data+=struct.pack('>'+str(len(patch['extra_words']))+'H',*patch['extra_words'])
    descriptor=[value+delta if i<3 else value for i,value in enumerate(patch['descriptor'])]
    struct.pack_into('>IIIHHH',data,patch['descriptor_offset'],*descriptor)
    struct.pack_into('>H',data,patch['actor_upper_offset'],patch['actor_upper'])
    if len(data)!=len(reference): raise ValueError('Completion relocation changed size')
    covered=set()
    for offset in range(0,len(data)-3,2):
        old=struct.unpack_from('>I',reference,offset)[0]
        new=struct.unpack_from('>I',data,offset)[0]
        if 0x100000<=old<0x100000+len(data) and new==old+delta:
            covered.update(range(offset,offset+4))
    if any(a!=b and i not in covered for i,(a,b) in enumerate(zip(reference,data))):
        raise ValueError('Completion relocation changed non-pointer data')
    return bytes(data)


def prepare(root, sdk, build, game, rows, images, completion, high, audio, jobs, run):
    output = build/'compact'
    output.mkdir(parents=True, exist_ok=True)
    tables = [symbols(build/'stages'/str(row['stage'])/'bank.elf', sdk, run) for row in rows]
    pool, offsets = pattern_pool(images, tables)
    (output/'patterns.bin').write_bytes(pool)
    (output/'pattern_offsets.json').write_text(json.dumps(offsets))
    victory_address = (HIGH_ADDRESS + len(high) + 32767) & ~32767
    completion = relocate_completion(root,sdk,completion,build/'completion',output/'completion',COMPLETION_OFFSET,run)
    exports=(root/'src/completion_exports.s').read_text()
    exports=re.sub(r'(\.set \w+, )(\d+)',lambda m:m[1]+str(int(m[2])+COMPLETION_OFFSET),exports)
    (game/'src/completion_exports.s').write_text(exports)
    chunks = [(HIGH_ADDRESS, high), (victory_address, audio[51]),
              (0x100000+COMPLETION_OFFSET, completion)]
    if victory_address + len(audio[51]) > 0x100000+COMPLETION_OFFSET:
        raise ValueError('UI and victory audio exceed fixed-ROM spare space')
    cursor = 0x100000 + COMPLETION_OFFSET + len(completion)
    stages, objects_by_stage, stats = [], [], []
    for row in rows:
        folder = output/'stages'/str(row['stage'])
        stats.append(compact_stage(root/row['folder'], row['sources'], folder))
        def compile_one(name):
            target = folder/(Path(name).stem + '.o')
            run([sdk/'bin/m68k-elf-gcc', '-m68000', '-Os', '-fno-builtin',
                 '-ffunction-sections', '-fdata-sections', '-c', folder/name,
                 '-I'+str(folder), '-I'+str(game/'inc'), '-isystem'+str(sdk/'inc'), '-o', target])
            return target
        with ThreadPoolExecutor(max_workers=jobs) as executor:
            objects = list(executor.map(compile_one, row['sources'] + ['geometry_pool.c']))
        base = 0x100000 + (cursor & (PAGE-1))
        data = link_stage(folder, objects, base, 0, offsets, sdk, run)
        stages.append((cursor, len(data)))
        objects_by_stage.append(objects)
        cursor += len(data)
        print(f'  Compact stage {row["stage"]+1}: {len(data)} metadata bytes', flush=True)
    clips = {}
    for page in (52, 55, 58):
        cursor = (cursor + 32767) & ~32767
        clips[page] = cursor
        chunks.append((cursor, audio[page]))
        cursor += len(audio[page])
    pool_base = (cursor + 31) & ~31
    used_bytes = pool_base + len(pool)
    if used_bytes > ROM_BYTES:
        raise ValueError('Compact image overlaps reserved SRAM bank 31')
    chunks.append((pool_base, pool))
    for row, (address, size), objects in zip(rows, stages, objects_by_stage):
        folder = output/'stages'/str(row['stage'])
        data = link_stage(folder, objects, 0x100000+(address & (PAGE-1)), pool_base, offsets, sdk, run)
        if len(data) != size:
            raise ValueError('Physical pattern relocation changed stage size')
        chunks.append((address, data))
    (game/'inc/rt_campaign_bank_pages.h').write_text(
        'static const u8 rtCampaignBankPages[10]={' + ','.join(str(a//PAGE) for a, _ in stages) + '};\n')
    (game/'inc/rt_rom_layout.h').write_text(f'''#ifndef RT_ROM_LAYOUT_H
#define RT_ROM_LAYOUT_H
#define RT_ROM_BYTES {ROM_BYTES:#x}UL
#define RT_FINAL_VICTORY_WINDOWS 2
#define RT_COMPLETION_PAGE 2
#define RT_HIGH_SCORE_PAGE 1
#define RT_HIGH_SCORE_BASE {0x100000+HIGH_ADDRESS-PAGE:#x}UL
#define RT_HIGH_COLD_ADDRESS {clips[55]:#x}UL
#define RT_HIGH_WARM_ADDRESS {clips[58]:#x}UL
static const u32 rtCampaignBankOffsets[10]={{{','.join(hex(a & (PAGE-1))+'UL' for a, _ in stages)}}};
#endif
''')
    for header in (game/'inc').glob('*.h'):
        text = header.read_text()
        if '#define RT_FINAL_VICTORY_ADDRESS ' in text:
            text = re.sub(r'#define RT_FINAL_VICTORY_ADDRESS .*',
                          f'#define RT_FINAL_VICTORY_ADDRESS {victory_address:#x}UL', text)
            text = re.sub(r'#define RT_FINAL_ENDING_AFTER_VICTORY_ADDRESS .*',
                          f'#define RT_FINAL_ENDING_AFTER_VICTORY_ADDRESS {clips[52]:#x}UL', text)
            header.write_text(text)
    report = dict(stages=stages, pattern_pool=pool_base, pattern_bytes=len(pool),
                  used_bytes=used_bytes, completion_offset=COMPLETION_OFFSET,
                  audio_addresses=dict(victory=victory_address,ending=clips[52],high_cold=clips[55],high_warm=clips[58]), geometry_and_maps=stats, reserved_sram_page=31)
    (output/'layout.json').write_text(json.dumps(report, indent=2)+'\n')
    return chunks, report
