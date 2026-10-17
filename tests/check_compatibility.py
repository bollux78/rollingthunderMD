#!/usr/bin/env python3
"""Report cartridge metadata and isolated libretro boot probes, including failures.

Requires a locally built ROM and matching symbols. Region probes change only
the header in a temporary copy. GX dirty-launch probes model leftover work RAM
and controller I/O, not an EverDrive FPGA or a physical console. GX mapper hooks
forward every write unchanged and observe bank-zero mapping before/after it.
"""
import argparse
import ctypes as C
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

from test_soft_reset import Machine


class Probe(Machine):
    def run(self, frames):
        # Machine's constructor normally boots immediately. Install probes first.
        if getattr(self, 'ready', False):
            for _ in range(frames):
                self.core.retro_run()
                self.frame += 1

    def snapshot(self):
        return dict(state=self.number('rtAppState', 2),
                    fault=self.number('rtAppFault', 2),
                    refreshes=self.number('rtAppRefreshes', 4),
                    vblanks=self.number('vtimer', 4),
                    updates=self.number('rtAppUpdates', 4),
                    save_available=self.number('rtSaveAvailable'),
                    int_callback=self.read('intCB', 4).hex())


def probe(args):
    with tempfile.TemporaryDirectory() as folder:
        data = bytearray(args.rom.read_bytes())
        data[0x1f0:0x200] = args.region.encode().ljust(16, b' ')
        args.rom = Path(folder) / 'region-probe.bin'
        args.rom.write_bytes(data)
        m = Probe(args)
        m.ready, m.frame = True, 0
        m.core.retro_get_region.restype = C.c_uint
        region = m.core.retro_get_region()
        assert region == int(args.region == 'E'), 'Core selected wrong video region'
        events = []
        if args.engine == 'gx':
            # Genesis Plus GX's exported 256-entry CPU map, as in Side Arms QA.
            class Map(C.Structure):
                _fields_ = [(n, C.c_void_p) for n in
                            ('base', 'read8', 'read16', 'write8', 'write16')]

            maps = (Map * 256).in_dll(m.core, 'm68k')
            origin = maps[0].base
            write = C.CFUNCTYPE(None, C.c_uint, C.c_uint)
            io8, io16 = write(maps[0xa1].write8), write(maps[0xa1].write16)

            def observe(original, address, value):
                before = maps[0].base
                original(address, value)
                if address & 0xffff00 == 0xa13000 and len(events) < 256:
                    events.append(dict(frame=m.frame, address=hex(address), value=value,
                                       bank0_before=before-origin,
                                       bank0_after=maps[0].base-origin,
                                       int_callback=m.read('intCB', 4).hex(),
                                       save_available=m.number('rtSaveAvailable')))

            cb8 = write(lambda a, d: observe(io8, a, d))
            cb16 = write(lambda a, d: observe(io16, a, d))
            maps[0xa1].write8 = C.cast(cb8, C.c_void_p)
            maps[0xa1].write16 = C.cast(cb16, C.c_void_p)
            if args.dirty:
                C.memset(m.ram, 0xa5, 65536)
                io = (C.c_uint8 * 16).in_dll(m.core, 'io_reg')
                io[4] = io[6] = 0x40
        m.run(600)
        before = m.snapshot()
        m.run(120)
        after = m.snapshot()
        progress = (after['vblanks'] - before['vblanks']) & 0xffffffff
        boot = after['state'] == 0 and after['fault'] == 0 and 100 <= progress <= 125
        result = dict(engine=args.engine, region_header=args.region,
                      video_region='PAL' if region else 'NTSC', dirty_launch=args.dirty,
                      boot_pass=boot, menu_vblank_progress=progress,
                      after_boot=after, mapper_writes=events)
        if boot:
            try:
                m.start('campaign_2p')
                assert m.number('rtAppFault', 2) == 0
                m.press(3, 1)
                assert m.number('rtAppPaused') == 1
                updates = m.number('rtAppUpdates', 4)
                m.run(120)
                assert m.number('rtAppUpdates', 4) == updates
                m.press(3, 1)
                assert m.number('rtAppPaused') == 0
                assert m.number('rtAppUpdates', 4) > updates
                m.core.retro_reset()
                m.run(300)
                assert m.number('rtAppFault', 2) == 0
                assert m.number('rtAppState', 2) == 0
                m.start('campaign_2p')
                result['coop_pause_reset_restart_pass'] = True
            except AssertionError as error:
                result['coop_pause_reset_restart_pass'] = False
                result['error'] = str(error) or 'Gameplay assertion failed'
        result['passed'] = boot and result.get('coop_pause_reset_restart_pass', False)
        m.core.retro_unload_game()
        m.core.retro_deinit()
        print(json.dumps(result))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--rom', type=Path, default=Path('dist/RollingThunderMD.bin'))
    p.add_argument('--symbols', type=Path, default=Path('.build/game/out/release/symbol.txt'))
    p.add_argument('--picodrive', type=Path)
    p.add_argument('--gx', type=Path)
    p.add_argument('--output', type=Path, default=Path('.build/compatibility/results.json'))
    p.add_argument('--core', type=Path, help=argparse.SUPPRESS)
    p.add_argument('--engine', choices=('gx', 'picodrive'), help=argparse.SUPPRESS)
    p.add_argument('--region', choices=('U', 'E', 'J'), help=argparse.SUPPRESS)
    p.add_argument('--dirty', action='store_true', help=argparse.SUPPRESS)
    args = p.parse_args()
    if args.engine:
        probe(args)
        return
    raw = args.rom.read_bytes()
    checksum = sum(v[0] for v in struct.iter_unpack('>H', raw[512:])) & 65535
    report = dict(rom_sha256=hashlib.sha256(raw).hexdigest(), rom_bytes=len(raw),
                  header_system=raw[0x100:0x110].decode('ascii'),
                  rom_range=list(struct.unpack_from('>II', raw, 0x1a0)),
                  ram_range=list(struct.unpack_from('>II', raw, 0x1a8)),
                  sram_header=raw[0x1b0:0x1bc].hex(),
                  checksum_valid=checksum == int.from_bytes(raw[0x18e:0x190], 'big'),
                  exceeds_16_mib=len(raw) > 16*1024*1024,
                  last_nonzero_end=len(raw.rstrip(b'\0')), cores={}, cases=[],
                  scope='Emulator probes and metadata only; no physical hardware certification. '
                        'U/E/J use temporary header variants; GX dirty launch seeds RAM/I/O. '
                        'Gameplay coverage is opening Campaign 2P, P2 pause/resume, reset/restart.')
    for engine in ('picodrive', 'gx'):
        core = getattr(args, engine)
        if not core:
            continue
        report['cores'][engine] = dict(sha256=hashlib.sha256(core.read_bytes()).hexdigest(),
                                      file=core.name)
        for region in ('U', 'E', 'J'):
            for dirty in ((False, True) if engine == 'gx' else (False,)):
                cmd = [sys.executable, str(Path(__file__).resolve()), '--rom', str(args.rom),
                       '--symbols', str(args.symbols), '--engine', engine, '--core', str(core),
                       '--region', region] + (['--dirty'] if dirty else [])
                try:
                    run = subprocess.run(cmd, capture_output=True, text=True, timeout=45)
                    if run.returncode:
                        raise RuntimeError(f'exit {run.returncode}: {run.stderr[-2000:]}')
                    row = next(json.loads(line) for line in run.stdout.splitlines()
                               if line.startswith('{'))
                except (RuntimeError, subprocess.TimeoutExpired, StopIteration) as error:
                    row = dict(engine=engine, region_header=region, dirty_launch=dirty,
                               passed=False, error=str(error))
                report['cases'].append(row)
                print(engine, region, 'dirty' if dirty else 'cold',
                      'PASS' if row['passed'] else 'FAIL', flush=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(args.output)
    # Current oversized ROM must fail the compatibility gate even without cores.
    return int(report['exceeds_16_mib'] or not report['checksum_valid'] or
               any(not row['passed'] for row in report['cases']))


if __name__ == '__main__':
    sys.exit(main())
