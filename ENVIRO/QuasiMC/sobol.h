// FILE: sobol.h

/* Header file for the Sobol Quasi-Random Number Generator.
   There are four functions declared in this file:

   1) int InitSobol(int dimension, int maxpoint)
     - initializes the sobol generator
     - dimension is the dimension of the point set
     - max point is the upper bound on the number of points to generate
     - returns a 1 on success else 0

   2) double *Sobol (void)
     - returns a pointer to one point in the point set else NULL

   3)  void ResetSobol (void)
     - resets the sobol generator to its initial state

   4) void FreeSobol (void)
     - frees the sobol generator from memory

   5) void ScrambleSobol (int *rmatrix, int dimension, int res)
     - scrambles the minit 2d-array used by the generator */

// Mik Cieslak and Kris Luttmer | February 17, 2001

#ifndef _SOBOL_H
#define _SOBOL_H

// ----------------------------------------------------------------------

int InitSobol(int dimension, unsigned int max);
// initializes the sobol generator
// returns 1 upon success else 0

double *Sobol(void);
// returns a pointer to the next quasirandom number in the sequence,
// else NULL

void ResetSobol(void);
// reset the sobol generator to its initial state

void FreeSobol(void);
// frees the memory used by the Sobol generator

void ScrambleSobol(int *rmatrix, int *rvector, int dim);
// scrambles the initial Sobol data, Owen-Scramble

// ----------------------------------------------------------------------

#endif
