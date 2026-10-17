#ifndef RT_SAVE_H
#define RT_SAVE_H
#include <genesis.h>
/* Two CRC-checked slots; scratch/initial-entry controls are never persisted.
 * Call only with ROM DMA drained and outside an active entry IRQ. */
bool RT_saveLoad(u8 records[35],u32 *best,u8 *credits,u8 *psgMuted);
bool RT_saveStore(const u8 records[35],u32 best,u8 credits,u8 psgMuted);
extern volatile u8 rtSaveAvailable;
#endif
