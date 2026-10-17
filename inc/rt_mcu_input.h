#ifndef RT_MCU_INPUT_H
#define RT_MCU_INPUT_H
#include <genesis.h>
/* Map Genesis A/B and directions to the twelve active-low board-control
   scan pins. Preserve every other scan byte; coins/start/DIPs are separate.
   No filtering, opposite-direction suppression or input publication. */
bool RT_mapGenesisControlsToMcuScan(u16 pad1,u16 pad2,u8 scan[37]);
/* Port cabinet bindings: each pad's C operates its coin slot and Start its
   source start input. Coin pins18/26 are active high; start pins17/25 are
   active low. Preserve the other33 pins. Filtering/accounting are separate. */
bool RT_mapGenesisCabinetToMcuScan(u16 pad1,u16 pad2,u8 scan[37]);
/* Source MCU8298..832E. scan is1400..1424; records is1425..1493
   (37 triples: rising edge, latest sample, filtered latch). combined is
   1494..14DD (37 edge/sample pairs). All buffers must be nonoverlapping.
   Applies the source polarity table and record0/17 current-bit merge.
   Does not sample a Genesis pad or copy the preceding scan to CUS30 RAM. */
bool RT_filterMcuInputs(const u8 scan[37],u8 records[111],u8 combined[74]);
/* Descriptor65/66/64/67/68 select coinA,coinB,service,start1,start2
   edge bytes. Source descriptor indices21/20/22/19/18, respectively. */
bool RT_getMcuCreditInputs(const u8 combined[74],u8 inputs[5]);
/* FD6D, configured descriptor55/57/69: copy exactly35 prior combined
   words to MCU123C..1281 (CPU1423C..4281), clearing/setting1188 around it.
   sharedInputs has70 bytes; the final two combined records are not copied.
   Volatile ready models the source publication writes. Buffers must not overlap. */
bool RT_publishMcuInputs(const u8 combined[74],u8 sharedInputs[70],
 volatile u8 *ready1188);
/* Enabled-IRQ input ordering: publish previous combined records, then filter
   the new scan. Caller supplies raw scan bytes and owns IRQ cadence/gating.
   Credit handling consumes the new combined records after this call. */
bool RT_serviceMcuInputScan(const u8 scan[37],u8 records[111],u8 combined[74],
 u8 sharedInputs[70],volatile u8 *ready1188);
#endif
