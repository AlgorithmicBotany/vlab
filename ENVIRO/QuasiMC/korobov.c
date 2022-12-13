// FILE: korobov.c

/* Implementation of the Korobov Generator.
   For a description of the functions read the header file, korobov.h.

   Global Variables:
   1) a pointer to a block of memory of type double that is the point set
   2) the dimension of the current pointset
   3) a calculating variable used to generate the point set */

// Kris Luttmer and Mik Cieslak | February 25, 2001
// Updated on May 2, 2001

#include "qmc.h"
#include "korobov.h"

static double *point = NULL;
static UINT *x = NULL;
static int korobovdimen = 0;
static int count = 0;
static int n = 0;
static int a = 0;
static int type = 0;

// ----------------------------------------------------------------------
int InitKorobov(int s, int param_n, int param_a, int param_type)
/* Purpose: to initalize the korobov generator.
       Pre: dimension is the dimension of the new point.
            point and x must be NULL.
      Post: the dimension is saved in korobovdimen.
            memory has been allocated for point and x.
            x has been initialized (x[i] = a * x[i-1] mod n).
            a pointer to point has been returned. */
{
  int i; // a looping variable

  korobovdimen = s; // save the dimension
  n = param_n;
  a = param_a;
  type = param_type;
  count = 0;

  // allocate memory
  point = (double *)malloc(sizeof(double) * korobovdimen);
  x = (UINT *)malloc(sizeof(int) * korobovdimen);

  // check if memory was allocated
  if ((!point) || (!x)) {
    fprintf(stderr, "Unable to allocate enough memory to hold the point set\n");
    return (0);
  }

  // the first elements in pointset will be U0 which is defined as all 0's
  memset(point, 0, korobovdimen * sizeof(double));

  // init the arrary x
  x[0] = 1;
  x[1] = a;
  for (i = 2; i < korobovdimen; i++)
    x[i] = ((x[i - 1] * a) % n);

  return 1; // return the pointset pointer
}

// ----------------------------------------------------------------------

double *Korobov(void)
/* Purpose: to generate the next point. that is, another point.
       Pre: InitKorobov must have been called.
      Post: a pointer is returned to the newly generated point.
            actually, the old point has been over written. */
{
  int i; // looping variable

  if (count == 0) {
    count = 1;
    return (point);
  }

  // check if the point, and x exist
  if ((!point) || (!x)) {
    fprintf(stderr, "Korobov generator has not been initialized.\n");
    return (NULL);
  }

  if (count > n) {
    fprintf(stderr, "Korobov generator cannot generate anymore points.\n");
    return (NULL);
  }

  // MIK: changed korobov prime number generator because there was a mistake
  if (type == KOROBOV_PRIME_NUMBER) {
    // generate the point
    for (i = 0; i < korobovdimen; i++)
      *(point + i) = (double)((double)x[i] / (double)n);

    // update the x array for use with the next generation of the point
    // for (i = 0; i < korobovdimen-1; i++)
    // x[i] = x[i+1];
    // x[i] = (x[i-1] * a) % n;
    ++x[0];
    for (i = 1; i < korobovdimen; i++)
      x[i] = (x[i - 1] * a) % n;

    ++count;
    return (point); // return the point
  } else if (type == KOROBOV_POWER_OF_2) {
    for (i = 0; i < korobovdimen; i++) {
      *(point + i) = *(point + i) + (double)((double)x[i] / (double)n);

      while (*(point + i) > 1.0)
        *(point + i) -= 1.0;
    }

    ++count;
    return (point);
  } else {
    fprintf(stderr,
            "Korobov generator must be used with N prime or power of 2.\n");
    return (NULL);
  }

  return (NULL);
}

// ----------------------------------------------------------------------

void ResetKorobov(void)
/* Purpose: to reset the korobov generator to its inital state.
       Pre: the generator must already be initialized.
      Post: the generator is ready is start generating new points. */
{
  int i; // a looping variable

  count = 0;

  // the first point will be 0
  memset(point, 0, korobovdimen * sizeof(double));

  // re-init the arrary x
  x[0] = 1;
  x[1] = a;
  for (i = 2; i < korobovdimen; i++)
    x[i] = ((x[i - 1] * a) % n);

  return;
}

// ----------------------------------------------------------------------

double NextDimKorobov(int dim)
/* Purpose: to generate a point's next dimension as needed.
       Pre: the korobov generator must have been initialized.
            lastpoint must be the previous UNRANDOMIZED dimension of
            the point.
      Post: the next dimension of the point is returned. */
{
  static double lastpoint = 0;
  UINT iresult;
  double result;

  if (dim == korobovdimen)
    lastpoint = *(point + dim - 1);

  result = lastpoint * ((double)n);
  iresult = ((UINT)(result * a)) % n;
  result = ((double)iresult / n);

  return (result);
}

// ----------------------------------------------------------------------

void FreeKorobov(void)
/* Purpose: to free the korobov generator (the memory it uses).
       Pre: point and x should have been allocated otherwise this
            function is useless.
      Post: the memory used by the korobov generator is freed. */
{
  if (x != NULL) {
    free(x);
    x = NULL;
  }

  if (point != NULL) {
    free(point);
    point = NULL;
  }

  return;
}

// ----------------------------------------------------------------------
