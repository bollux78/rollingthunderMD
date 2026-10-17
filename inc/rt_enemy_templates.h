#ifndef RT_NATIVE_ENEMY_TEMPLATES_H
#define RT_NATIVE_ENEMY_TEMPLATES_H
#include <genesis.h>
/* Resolved immutable constructor inputs. Byte4/6 names deliberately retain
 * their source meanings until their owning native behaviors are implemented.
 * accelerationY is the constructor acceleration, native downward-positive. */
typedef struct {
    s16 accelerationY;
    u8 kind,spawnAction,traits,variation,initialByte4,attributes,initialByte6,reserved;
} RT_EnemyTemplate;
extern const RT_EnemyTemplate rtDoorEnemyTemplates[];
extern const u8 rtDoorEnemyTemplateIds[10][8];
/* Original D091 choice, including byte addition wrap. Stage=root*5+segment.
 * Returned data is immutable ROM storage; no allocation or door mutation. */
const RT_EnemyTemplate *RT_doorEnemyTemplate(u8 stage,u8 worldXLow,u8 counter);
#endif
