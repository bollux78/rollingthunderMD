#!/usr/bin/env python3
"""GX Team Player startup/reset with held buttons; no physical adapter claim."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
from test_soft_reset import Machine

class BeforeBoot(Machine):
    def run(self, frames):
        if getattr(self, 'ready', False):
            super().run(frames)

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--core', type=Path, required=True)
    p.add_argument('--rom', type=Path, default=Path('dist/RollingThunderMD.bin'))
    p.add_argument('--symbols', type=Path, default=Path('.build/game/out/release/symbol.txt'))
    p.add_argument('--device', type=int, choices=(1537, 1793))
    args = p.parse_args()
    if args.device:
        m = BeforeBoot(args)
        # GX subclasses 5/6: Team Player with 3/6-button pads, respectively.
        m.core.retro_set_controller_port_device(0, args.device)
        # With a normal pad on port two SGDK leaves Team Player disabled.
        m.core.retro_set_controller_port_device(1, 0)
        m.buttons = {(0, 0), (0, 4)}  # B + Up held during pad detection
        m.ready = True; m.run(300)
        assert m.number('portType') == 7 and m.number('portSupport') == 5
        assert m.number('joyState', 2) != 0, 'Held buttons did not reach the Team Player reader'
        m.buttons.clear(); m.run(300)
        assert m.number('rtAppState', 2) == 0
        assert m.number('joyEventCB', 4) == 0, 'Must exercise the nullable callback'
        for _ in range(3):
            m.buttons = {(0, 0), (0, 4)}
            m.core.retro_reset(); m.run(300)
            m.buttons.clear(); m.run(120)
            before = m.number('vtimer', 4); m.run(120)
            assert m.number('vtimer', 4) - before == 120
            assert m.number('rtAppState', 2) == 0
        m.core.retro_unload_game(); m.core.retro_deinit()
        print(json.dumps(dict(device=args.device, resets=3, passed=True)))
        return
    for device in (1537, 1793):
        cmd = [sys.executable, str(Path(__file__).resolve()), '--core', str(args.core),
               '--rom', str(args.rom), '--symbols', str(args.symbols), '--device', str(device)]
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        assert result.returncode == 0, result.stdout + result.stderr
        print(result.stdout.strip())

if __name__ == '__main__':
    main()
