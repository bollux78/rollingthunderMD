#!/usr/bin/env python3
"""Keep SGDK vectors, but initialize TMSS on every cartridge entry.

The project reset wrapper validates its own retained-state marker. Controller
registers can belong to a flash-cart menu and must not select the warm path.
Only the build-local assembly is adapted; the shared SDK is never modified.
"""
from pathlib import Path
import re
import sys


def cartridge_boot(source):
    source = source.replace('\r\n', '\n')
    pattern = (r'        tst\.l   0xa10008\n'
               r'        bne\.s   SkipInit\n\n'
               r'        tst\.w   0xa1000c\n'
               r'        bne\.s   SkipInit\n')
    source, count = re.subn(pattern, '* Cartridge entry: always perform TMSS.\n', source)
    if count != 1:
        raise ValueError('Unrecognized SGDK controller-port boot heuristic')
    source, count = re.subn(
        r'NoTMSS:\n        jmp     _start_entry\n\nSkipInit:\n        jmp     _reset_entry',
        'NoTMSS:\n* Project wrapper validates retained state and initializes globals.\n'
        '        jmp     _reset_entry', source)
    if count != 1:
        raise ValueError('Unrecognized SGDK startup branch')
    return source


if __name__ == '__main__':
    Path(sys.argv[2]).write_text(cartridge_boot(Path(sys.argv[1]).read_text()))
