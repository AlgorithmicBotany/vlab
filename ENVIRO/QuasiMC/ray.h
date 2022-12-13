/* ray.h - definition of a ray type */

#ifndef _RAY_H
#define _RAY_H

typedef struct tagRAY {
  float pt[3];
  float dir[3];
  float intensity[MAX_SPECTRUM_SAMPLES];
  unsigned int signature;
  unsigned int depth;
} RAY;

typedef struct tagRUSSIANROULETTE {
  float threshold;
  float prob;
} RUSSIANROULETTE;

#endif
