// FILE: korobov.h

/* Header file for the korbov generator.
 */
// Kris Luttmer and Mik Cieslak | February 25, 2001

#ifndef _KOROBOV_H
#define _KOROBOV_H

// ---------------------------------------------------------------------------

int InitKorobov(int s, int n, int a, int type);
// initializes the korobov generator

double *Korobov(void);
// generates the next point

double NextDimKorobov(int dim);
// returns a value for the next dimension of one point for random dimensions

void ResetKorobov(void);
// reset the korobov generator

void FreeKorobov(void);
// frees the korobov generator

// ---------------------------------------------------------------------------

#endif
