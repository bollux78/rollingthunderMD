#!/usr/bin/env python3
"""Fresh-process SRAM load and corrupt-slot fallback in a real libretro core."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import zlib
from test_soft_reset import Machine


def worker(args):
    if getattr(args, 'everdrive_model', False):
        from test_everdrive_model import ProMachine
        m = ProMachine(args)
    else:
        m = Machine(args)
    assert m.number('rtSaveAvailable') == 1
    if args.phase == 'write':
        for button in (5, 3, 5, 5, 3, 7, 5, 5, 3, 8, 3):
            m.press(button)
        assert m.number('rtHomeStartingCredits') == 7 and m.number('psgMuted') == 1
        m.core.retro_reset(); m.run(300)
        m.start('campaign_2p')
        records = m.read('highRecords', 35)
        records = bytes.fromhex('654321') + records[3:4] + bytes([35, 40, 41]) + records[7:]
        m.patch('score', (654321).to_bytes(4, 'big')); m.patch('highRecords', records)
        m.press(3, 1)
        assert m.number('rtAppPaused') == 1
        args.sram_output.write_bytes(m.sram())
    else:
        assert m.number('score', 4) == (3000 if args.phase == 'recover' else 654321)
        assert m.number('rtHomeStartingCredits') == 7 and m.number('psgMuted') == 1
        if args.phase == 'load':
            assert m.read('highRecords', 3) == bytes.fromhex('654321')
            assert m.read('highRecords', 3, 4) == bytes([35, 40, 41])
        assert m.sram() == args.sram_load.read_bytes(), 'Cold load changed committed SRAM'
    m.core.retro_unload_game(); m.core.retro_deinit()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--everdrive-model', action='store_true', help='Use the documented extended-SSF software bus model; not physical hardware')
    p.add_argument('--core', type=Path, required=True)
    p.add_argument('--rom', type=Path, default=Path('dist/RollingThunderMD.bin'))
    p.add_argument('--symbols', type=Path, default=Path('.build/game/out/release/symbol.txt'))
    p.add_argument('--output', type=Path, default=Path('.build/cold-save-tests.json'))
    p.add_argument('--phase', choices=('write', 'load', 'recover'), help=argparse.SUPPRESS)
    p.add_argument('--sram-load', type=Path, help=argparse.SUPPRESS)
    p.add_argument('--sram-output', type=Path, help=argparse.SUPPRESS)
    args = p.parse_args()
    if args.phase:
        worker(args); return
    with tempfile.TemporaryDirectory() as folder:
        save = Path(folder) / 'save.srm'
        broken = Path(folder) / 'broken.srm'
        command = [sys.executable, str(Path(__file__).resolve()), '--core', str(args.core),
                   '--rom', str(args.rom), '--symbols', str(args.symbols)]
        if args.everdrive_model: command.append('--everdrive-model')
        def run(phase, *extra):
            result = subprocess.run(command + ['--phase', phase, *map(str, extra)],
                                    capture_output=True, text=True, timeout=45)
            assert result.returncode == 0, result.stdout + result.stderr
        run('write', '--sram-output', save)
        raw = bytearray(save.read_bytes())
        # Cores export either logical SRAM bytes or odd-byte bus storage.
        # Select the representation by checking both independent slot CRCs.
        layouts = []
        for stride, offset in ((1, 0), (2, 0), (2, 1)):
            logical = raw[offset::stride]
            slots = [logical[start:start+64] for start in (0, 64)]
            if all(len(s) == 64 and s[:4] == b'RTMD' and
                   zlib.crc32(s[:60]) == int.from_bytes(s[60:], 'big') for s in slots):
                layouts.append((stride, offset, slots))
        assert len(layouts) == 1, 'Could not identify both CRC-valid SRAM slots'
        stride, offset, slots = layouts[0]
        newest = max(range(2), key=lambda i: int.from_bytes(slots[i][6:10], 'big'))
        assert int.from_bytes(slots[newest][12:16], 'big') == 654321
        assert int.from_bytes(slots[1-newest][12:16], 'big') == 3000
        run('load', '--sram-load', save)
        raw[offset + (newest*64 + 22)*stride] ^= 1
        broken.write_bytes(raw)
        run('recover', '--sram-load', broken)
    report = dict(rom_sha256=hashlib.sha256(args.rom.read_bytes()).hexdigest(),
                  extended_ssf_software_model=args.everdrive_model, physical_hardware_tested=False,
                  core_sha256=hashlib.sha256(args.core.read_bytes()).hexdigest(),
                  cold_restore_pass=True, corrupt_slot_fallback_pass=True,
                  scope='Three fresh core processes; controller settings, RAM-seeded score '
                        'and initials saved through P2 pause; emulator/model battery RAM, no savestates.')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report))


if __name__ == '__main__':
    main()
