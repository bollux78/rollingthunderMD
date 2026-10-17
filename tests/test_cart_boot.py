"""Fail closed when SDK startup changes; preserve vectors and TMSS ordering."""
import sys
from pathlib import Path
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from cart_boot import cartridge_boot

class CartridgeBootTests(unittest.TestCase):
    def test_entry_always_passes_tmss_before_wrapper(self):
        source = ('vectors stay here\n'
                  '        tst.l   0xa10008\n'
                  '        bne.s   SkipInit\n\n'
                  '        tst.w   0xa1000c\n'
                  '        bne.s   SkipInit\n'
                  'TMSS stays here\n'
                  'NoTMSS:\n        jmp     _start_entry\n\n'
                  'SkipInit:\n        jmp     _reset_entry\n'
                  'interrupt handlers stay here\n')
        result = cartridge_boot(source.replace('\n', '\r\n'))
        self.assertTrue(result.startswith('vectors stay here\n'))
        self.assertTrue(result.endswith('interrupt handlers stay here\n'))
        self.assertNotIn('SkipInit', result)
        self.assertNotIn('_start_entry', result)
        self.assertLess(result.index('TMSS stays here'), result.index('jmp     _reset_entry'))

    def test_unrecognized_sdk_is_rejected(self):
        with self.assertRaises(ValueError):
            cartridge_boot('unexpected startup')

if __name__ == '__main__':
    unittest.main()
