// FILE: sobol.c

/* Implementation of the SOBOL quasi-random number generator.
   Read the header file for a description of the functions.
   There is only one define: ATMOST = 2^30 - 1. Which is the maximum
   upper bound on the number of points the user can generate.

   Global Variables:
   1) poly and minit are the primitive polynomials
   2) double *point is one point from the point set
   3) the rest are used to generate *point */

// Mik Cieslak and Kris Luttmer
// February 4, 2001

#include "qmc.h"
#include "sobol.h"
#include "soboldata.h"

#define MAXCOLS 32

static double *point = NULL;
static UINT *lastpoint = NULL;
static UINT v[MAXDIM][MAXCOLS];
static double recipd = 0.0;
static int soboldimen = 0;
static int count = -1;
static int numcols = 0;
static int scrambled = 0;

// prototypes
static void InitV(void);

// --------------------------------------------------------------------

int InitSobol(int dimension, UINT max)
/* Purpose: initalize the sobol generator.
       Pre: dimension is the dimension and maxpoint is the upper bound.
      Post: memory for lastpoint and point has been allocated.
            The sobol generator is ready to generate points. */
{
  soboldimen = dimension; // save the dimension

  // check that the dimension is between 1 and MAXDIM
  if (soboldimen < 1 || soboldimen > MAXDIM) {
    fprintf(stderr, "Dimension must be between 1 and %d.\n", MAXDIM);
    return (0);
  }

  // check that point and lastpoint are free
  if ((point != NULL) || (lastpoint != NULL)) {
    fprintf(stderr, "Free the sobol generator before re-initalizing.\n");
    return (0);
  }

  // find the number of columns needed
  numcols = 31;
  while (!((max >> numcols) & 0x1))
    numcols--;
  numcols++;

  if (numcols >= MAXCOLS) {
    fprintf(stderr, "Sobol generator cannot generate 2^%d points.\n", numcols);
    return (0);
  }

  // allocate memory for lastpoint and point
  lastpoint = (UINT *)malloc(sizeof(int) * soboldimen);
  point = (double *)malloc(sizeof(double) * soboldimen);

  if ((!lastpoint) || (!point)) {
    fprintf(stderr, "Cannot allocate memory for Sobol Generator.\n");
    return (0);
  }

  InitV(); // initialize the v array

  // compute the recipd
  recipd = ((double)((UINT)(1 << numcols)));
  recipd = ((double)(1.0 / recipd));

  // clear the blocks of memory
  memset(lastpoint, 0, sizeof(int) * soboldimen);
  memset(point, 0, sizeof(double) * soboldimen);
  count = -1;
  scrambled = 0;

  return (1);
}

// --------------------------------------------------------------------

double *Sobol(void)
/* Purpose: to generate the next quasi-random point.
       Pre: the sobol generator has been initalized.
      Post: a pointer to the next point is returned, or NULL. */
{
  int i;          // a looping variable
  int column = 0; // the column to use

  // check if the generator has been initalized
  if ((point == NULL) || (lastpoint == NULL))
    return (NULL);

  // check if this is the first point
  if (count == -1) {
    count = 0;
    return (point);
  }

  // find the posistion of the right-hand-zero of count
  while ((count >> column) & 0x1)
    column++;

  // if there is no next point return NULL
  if (column >= numcols) {
    fprintf(stderr, "Sobol generator cannot generate anymore points.\n");
    return (NULL);
  }

  // calculate the next point using Gray Code,
  for (i = 0; i < soboldimen; i++) {
    *(lastpoint + i) = *(lastpoint + i) ^ v[i][column];
    *(point + i) = ((double)*(lastpoint + i)) * recipd;
  }

  count++;

  return (point);
}

// --------------------------------------------------------------------

void ResetSobol(void)
/* Purpose: reset the sobol generator to its initial state.
       Pre: the generator must have already been initialized.
      Post: the generator is ready to generate a new point set. */
{
  // clear the blocks of memory
  if (lastpoint != NULL)
    memset(lastpoint, 0, sizeof(int) * soboldimen);
  if (point != NULL)
    memset(point, 0, sizeof(double) * soboldimen);

  count = -1; // reset the number of times Sobol has been called

  if (scrambled == 1) {
    scrambled = 0;
    InitV();
  }

  return;
}

// --------------------------------------------------------------------

void FreeSobol(void)
/* Purpose: free the memory used by point and lastpoint.
       Pre: the sobol generator should have been initialized.
      Post: the memory used by point and lastpoint is free. */
{
  if (lastpoint != NULL) {
    free(lastpoint);
    lastpoint = NULL;
  }
  if (point != NULL) {
    free(point);
    point = NULL;
  }

  return;
}

// --------------------------------------------------------------------

void ScrambleSobol(int *rmatrix, int *rvector, int dim)
/* Purpose:
       Pre:
      Post: */
{
  int i, j, k;      // looping variabes
  int temp1, temp2; // calculating variables

  // check if the generator has been initalized
  if ((point == NULL) || (lastpoint == NULL)) {
    fprintf(stderr, "Cannot scramble Sobol because it was not initialized.\n");
    return;
  }

  // scramble the sobol data
  for (i = 0; i < numcols; i++) {
    // next scramble the v[i] for this dimension
    temp2 = 0;
    for (j = 0; j < numcols; j++) {
      temp1 = 0;

      for (k = 0; k < numcols; k++)
        temp1 +=
            *(rmatrix + j + (k * numcols)) * (v[dim][i] >> (numcols - 1 - k));

      temp2 = temp2 + ((temp1 & 0x1) << (numcols - 1 - j));
    }
    v[dim][i] = temp2;
  }

  // first the first point
  *(lastpoint + dim) = 0;
  for (i = 0; i < numcols; i++)
    *(lastpoint + dim) += (*(rvector + i) << (numcols - 1 - i));
  *(point + dim) = ((double)*(lastpoint + dim)) * recipd;

  scrambled = 1;

  return;
}

// --------------------------------------------------------------------

void InitV(void) {
  int i, j, k;
  int newv, temp, degree;

  // first the first dimension
  for (i = 0; i < numcols; i++)
    v[0][i] = 1;

  // now the rest of the dimensions
  for (i = 1; i < soboldimen; i++) {
    // find the degree of polynomial i
    degree = MAXDEGREE;
    while (!((poly[i] >> degree) & 0x1))
      degree--;

    // copy known values from minit
    memcpy(v[i], minit[i - 1], degree * sizeof(int));

    // find the rest of the values
    for (j = degree; j < numcols; j++) {
      newv = v[i][j - degree];
      temp = 1;
      for (k = degree - 1; k >= 0; k--) {
        if ((poly[i] >> k) & 0x1)
          newv = newv ^ (v[i][j - (degree - k)] << temp);
        temp++;
      }
      v[i][j] = newv;
    }
  }

  // calculate corresponding v<i,j>, v<i,j> = m<i,j>/2^j
  temp = 1;
  for (j = numcols - 2; j >= 0; j--) {
    for (i = 0; i < soboldimen; i++)
      v[i][j] = v[i][j] << temp;
    temp++;
  }

  return;
}

// --------------------------------------------------------------------
