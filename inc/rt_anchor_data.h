#ifndef RT_NATIVE_ANCHOR_DATA_H
#define RT_NATIVE_ANCHOR_DATA_H
#include "rt_anchors.h"
/* Immutable authored locations. Lifecycle/window allocation is separate;
 * compiling a whole stage does not instantiate or keep its doors alive. */
typedef struct {s16 x,y;u8 kind,flags,layer,reserved;} RT_AnchorDefinition;
typedef struct {const RT_AnchorDefinition *definitions;u16 count;} RT_AnchorLevel;
extern const RT_AnchorLevel rtAnchorLevels[10];
u8 RT_anchorInitialize(RT_Anchor *anchor,const RT_AnchorDefinition *definition,
    const RT_WorldCamera *camera);
#endif
