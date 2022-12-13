
/* Rand.h for ANSI C */

#ifndef MRG_Rand_H
#define MRG_Rand_H

typedef unsigned long ulongint;

typedef enum { StartStream, StartBlock, NextBlock } SeedType;

struct InfoStream;
typedef struct InfoStream *RngStream;

#ifdef __cplusplus
extern "C" {
#endif

void Rand_CreateStream(RngStream *g, char name[32]);

void Rand_DeleteStream(RngStream *g);

void Rand_ResetStream(RngStream g, SeedType where);

void Rand_DoubleGenerator(RngStream g, int d);

void Rand_SetAntithetic(RngStream g, int a);

void Rand_SetPackageSeed(ulongint seed[6]);

void Rand_SetSeed(RngStream g, ulongint seed[6]);

void Rand_AdvanceState(RngStream g, long e, long c);

void Rand_GetState(RngStream g, ulongint seed[6]);

void Rand_WriteState(RngStream g);

double Rand_RandU01(RngStream g);

long Rand_RandInt(RngStream g, long i, long j);

#ifdef __cplusplus
}
#endif

#endif
