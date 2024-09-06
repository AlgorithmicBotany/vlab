// FILE: randqmc.h

/* Header file for dealing with randomization of QMC point sets.
   There are four functions in this file:

   1) InitRandom (dimension, method, base)
     - initializes the randomization generator
     - must know the dimension to use
     - method is the randomization method to use (see 3 below)
     - base is the base to use when shifting

   2) GenRandom ()
     - returns a random vector in the previous dimension

   3) AppRandom (point, r)
     - applies the previously chosen randomization to point using r
     - returns the new point as pointer to a double

   4) Rpoint (currdim, lastpoint, rpoint)
     - for random dimensions this function will return either
       a value from rpoint or generate a value for currdim dimension
       and randomize it.

   5) FreeRandom ()
     - frees the randomization generator */

// Mik Cieslak and Kris Luttmer | February 26, 2001

#ifndef _RANDQMC_H
#define _RANDQMC_H

#define MAXARGS 10

// ----------------------------------------------------------------------

int InitRandom(double *parameters);
// initializes the randomization generator

double *GenRandom(void);
// generate a random number to be used by AppRandom

//extern double *(*AppRandom)(double *point, double *r);
// apply the randomization to point, and return the new point

double RPoint(int currdim);
// returns the a value for the (currdim) dimension from one point
// according to lastpoint and rpoint

void FreeRandom(void);
// free the randomization generator

void InitGen(void);

void InitGenNextBlock(void);

// ----------------------------------------------------------------------

#endif
