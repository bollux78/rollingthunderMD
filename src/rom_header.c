#include <genesis.h>

__attribute__((externally_visible))
const ROMHeader rom_header = {
#if (MODULE_MEGAWIFI  && (MEGAWIFI_IMPLEMENTATION == MEGAWIFI_IMPLEMENTATION_MW_CART))
    "SEGA MEGAWIFI   ",
#else
    /* Standard SSF2 bank registers + SRAM, not Krikzz's extended SSF. */
    "SEGA MEGA DRIVE ",
#endif
    "(C)SGDK 2024    ",
    "ROLLING THUNDER MD                               ",
    "ROLLING THUNDER MD                               ",
    "GM 00000000-00",
    0x000,
    "JD              ",
    0x00000000,
#if (ENABLE_BANK_SWITCH != 0)
    0x003FFFFF,
#else
    0x000FFFFF,
#endif
    0x00FF0000,
    0x00FFFFFF,
    "RA",
    0xF820,
    0x00200001,
    0x0020FFFF,
    "            ",
    "DEMONSTRATION PROGRAM                   ",
    "JUE             "
};
