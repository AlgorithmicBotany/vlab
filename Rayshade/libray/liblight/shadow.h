#ifndef SHADOW_H
#define SHADOW_H

#include "libobj/geom.h"
#include "libsurf/surface.h"
#include "light.h"

int Shadowed(Color *result, Color *color, /* resultant intensity, light color */
             ShadowCache *cache,          /* shadow cache for light */
             Ray *ray,   /* ray, origin on surface, dir towards light */
             Float dist, /* distance from pos to light source */
             int noshadow);
void ShadowStats(unsigned long *shadowrays, unsigned long *shadowhit,
                 unsigned long *cachehit, unsigned long *cachemiss);

#endif
