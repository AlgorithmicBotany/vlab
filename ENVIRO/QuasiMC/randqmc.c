// FILE: randqmc.c

/* Implementation of the randomization of QMC point sets.
   Refer to the header file, randqmc.h, for a description of the
   functions.  Outlined here are the static members,
    - generator is the random number generator from rand.h
    - randdimen is the dimension used for rpoint and rvector.
    - rpoint is the memory used to hold the newly shifted point.
    - rvector is the memory used to hold a random vector generated
      with the random number generator mentioned earlier.

   The functions that AppRandom can point to are the following:
   1) AppAddRandom (point, r)
    - applies an add shift to point (point + r).

   2) AppXorRandom (point, r)
    - applies an XOR shift in base rbase to point (point ^ r).

   3) AppNoRandom (point, r)
   - applies no randomization to point

   4) AppScrambleRandom (point, r)
   - applies the Scramble Randomization */

// Mik Cieslak and Kris Luttmer | February 26, 2001

#include "qmc.h"
#include "randqmc.h"

#ifdef MRG
#include "MRGrand.h"
#else
#include "rand.h"
#endif

// include the methods that can be scrambled
#include "sobol.h"

// include the methods that can return random dimensions
#include "mc.h"
#include "korobov.h"
#include "combinedpk.h"


#ifdef MRG
static RngStream generator = NULL; // the random number generator
static unsigned long state[6];     // the state of the generator
#else
static unsigned short rqmc_xsubi[3] = {0x330E, 0, 0};
static unsigned short saved_xsubi[3] = {0x330E, 0, 0};
#endif
static int randdimen = 0;      // the dimension to use
static int rmethod = 0;        // the method to use
static int rbase = 0;          // the base to shift in
static int rmaxcoeff = 0;      // the maximum number of coefficients
static double rmaxint = 0;     // the value of rbase^rmaxcoeff
static double *rpoint = NULL;  // the shifted point in randdimen
static double *rvector = NULL; // the random vector in randdimen

// prototypes
static double *AppAddRandom(double *point, double *r);
static double *AppXorRandom(double *point, double *r);
static double *AppNoRandom(double *point, double *r);

static double (*NextDimQMC)(int dim) = 0;
// returns a value for the next dimension of one point

// -----------------------------------------------------------------------

int InitRandom(double *parameters) {
  /* Purpose: Initialize the randomization generator */

  // create the generator
#ifdef MRG
  Rand_CreateStream(&generator, "generator");
#endif

  // check if InitRandom has not been called already
  if ((rpoint != NULL) || (rvector != NULL)) {
    fprintf(stderr,
            "Free the randomization generator before re-initializing.\n");
    return (0);
  }

  // save the dimension, method, base, and allocate memory
  randdimen = (int)parameters[QMC_DIMEN];
  rmethod = (int)parameters[QMC_RMETHOD];
  rbase = (int)parameters[QMC_RBASE];
  rpoint = (double *)malloc(sizeof(double) * randdimen);
  rvector = (double *)malloc(sizeof(double) * randdimen);

  // check if memory was allocated
  if ((rpoint == NULL) || (rvector == NULL)) {
    fprintf(stderr, "Cannot allocate memory for randomization.\n");
    return (0);
  }

  // clear the memory
  memset(rpoint, 0, sizeof(double) * randdimen);
  memset(rvector, 0, sizeof(double) * randdimen);

  // assign the random dimension method
  if (parameters[QMC_METHOD] == MONTECARLO_METHOD)
    NextDimQMC = NextDimMC;
  else if (parameters[QMC_METHOD] == KOROBOV_METHOD)
    NextDimQMC = NextDimKorobov;
  else if (parameters[QMC_METHOD] == PKOROBOV_METHOD)
    NextDimQMC = NextDimCpKorobov;

  // choose the method
  switch (rmethod) {
  case NOSHIFT:
    AppRandom = AppNoRandom;
    break;
  case ADDSHIFT:
    AppRandom = AppAddRandom;
    break;
  case XORSHIFT:
    if (rbase < 2) {
      fprintf(stderr, "The base, %d, cannot be used.\n", rbase);
      free(rpoint);
      free(rvector);
      return (0);
    }
    AppRandom = AppXorRandom;
    rmaxcoeff = (int)((sizeof(int) * 8 * log(2)) / log(rbase));
    rmaxint = pow(rbase, rmaxcoeff);
    break;
  default:
    AppRandom = 0;
    free(rpoint);
    free(rvector);
    fprintf(stderr, "The method, %d, does not exist.\n", rmethod);
    return (0);
  }

  return (1);
}

// -----------------------------------------------------------------------

double *GenRandom(void)
/* Purpose: Generate a random vector in randdimen using "rand.h".
   Pre: the randomization generator must have been initialized.
   Post: the random vector in dimension randdimen is returned as
         a pointer to a double. */
{
  int i;

  // if the generator from "rand.h" is not initialized, return NULL
#ifdef MRG
  if (generator == NULL) {
    fprintf(stderr,
            "Cannot generate random number because no generator exists.\n");
    return (NULL);
  }
#endif

  // loop through the dimension generating the random vector
  for (i = 0; i < randdimen; i++)
#ifdef MRG
    *(rvector + i) = Rand_RandU01(generator);
#else
    *(rvector + i) = erand48(rqmc_xsubi);
#endif

    // save the state of the generator
#ifdef MRG
  Rand_GetState(generator, state);
#else
  saved_xsubi[0] = rqmc_xsubi[0];
  saved_xsubi[1] = rqmc_xsubi[1];
  saved_xsubi[2] = rqmc_xsubi[2];
#endif

  return (rvector);
}

// -----------------------------------------------------------------------

double *AppAddRandom(double *point, double *r)
/* Purpose: to apply a regular add shift to the point using r.
   Pre: point must be a valid point in dimension randdimen.
        r must be a random vector in randdimen. Notice, this means
        that r does not have to come from GenRandom().
   Post: a pointer to the shifted point in randdimen is returned. */
{
  int i; // looping variable

  // check if the generator exists
#ifdef MRG
  if ((rpoint == NULL) || (generator == NULL))
#else
  if (rpoint == NULL)
#endif
  {
    fprintf(stderr, "Randomization generator was not initialized.\n");
    return (NULL);
  }

  // check if the point and r exist
  if ((point == NULL) || (r == NULL)) {
    fprintf(stderr, "Cannot apply randomization to non-existent point.\n");
    return (NULL);
  }

  // apply the randomization
  for (i = 0; i < randdimen; i++) {
    // add the two points
    if ((point + i) && (r + i))
      *(rpoint + i) = *(point + i) + *(r + i);
    else {
      fprintf(stderr, "The dimensions for randomization do not match.\n");
      return (NULL);
    }

    // modulo 1 the new point
    if (*(rpoint + i) > 1.0)
      *(rpoint + i) -= 1.0;
  }

  return (rpoint); // return rpoint
}

// -----------------------------------------------------------------------

double *AppXorRandom(double *point, double *r)
/* Purpose: to apply an XOR shift to point using r in base, rbase.
   Pre: the randomization generator must be initialized.
        point must be a valid point in randdimen dimension.
        r must be a valid random vector in dimension, randdimen.
        Notice, that the random vector does not have to come from
        GenRandom().
   Post: the XOR shift in base rbase has been applied and a pointer
         to the new point has been returned. */
{
  int i, j, k;        // looping variables
  double pointdouble; // temp storage for a single element of r
  double randdouble;  // temp storage for a single element of point
  UINT pointint;      // the int representation of pointdouble
  UINT randint;       // the int representation of randdouble
  UINT result;        // the result of pointint XORed with randint

  // check if the generator exists
#ifdef MRG
  if ((rpoint == NULL) || (generator == NULL))
#else
  if (rpoint == NULL)
#endif
  {
    fprintf(stderr, "Randomization generator was not initialized.\n");
    return (NULL);
  }

  // check if the point and r exist
  if ((point == NULL) || (r == NULL)) {
    fprintf(stderr, "Cannot apply randomization to non-existent point.\n");
    return (NULL);
  }

  // apply the randomization
  for (i = 0; i < randdimen; i++) {
    // get one element from the point, and represent it as an int
    if (!(point + i)) {
      fprintf(stderr, "The dimension of point is < %d.\n", randdimen);
      return (NULL);
    }
    pointdouble = *(point + i);
    pointdouble *= rmaxint;
    pointint = (UINT)pointdouble;

    // get one element from the random vector, and represent as an int
    if (!(r + i)) {
      fprintf(stderr, "The dimension of random vector is < %d.\n", randdimen);
      return (NULL);
    }
    randdouble = *(r + i);
    randdouble *= rmaxint;
    randint = (UINT)randdouble;

    // xor the integers
    if (rbase == 2)
      result = pointint ^ randint; // for base 2 use C's XOR
    else {
      result = 0;
      k = 1;
      for (j = 0; j < rmaxcoeff; j++) {
        result += (((pointint + randint) % rbase) * k);
        pointint /= rbase;
        randint /= rbase;
        k = k * rbase;
      }

      // if result / rmaxint = 0.9999... then result is 1 mod 1 = 0
      if (result >= rmaxint - 1)
        result = 0;
    }

    // write the new point
    *(rpoint + i) = (double)result;
    *(rpoint + i) /= rmaxint;
  }

  return (rpoint);
}

// -----------------------------------------------------------------------

double *AppNoRandom(double *point, __attribute__((unused))double *r)
/* Purpose: to apply no randomization to point
       Pre: none.
      Post: nothing is done to point. */
{
  return (point);
}

// -----------------------------------------------------------------------

double RPoint(int currdim)
/* Purpose: to generate randomized points in dimensions higher then
            randdim as needed.
       Pre: currdim is the currdim that is being accessed.
            lastpoint is past by reference and must contain the
            last UNRANDOMIZED point.
            rpoint is the vector of randomized points.
      Post: if rpoint contains the dimension needed then return the
            number at that dimension.
            otherwise call the method NextDimQMC to get a new number.
            lastpoint will equal this new value.
            rpoint will be the randomized version of lastpoint. */
{
  double result;
  double randnum;

  // if the dimension is greater than the randdim then
  if (currdim >= randdimen) {
    // get the point for the next dimension, and save it in lastpoint
    if (NextDimQMC == 0) {
      fprintf(stderr, "Cannot use random dimension with this method.\n");
      return (0.0);
    }
    result = NextDimQMC(currdim);

    // set the state of the generator back to original state
    if (currdim == randdimen) {
#ifdef MRG
      Rand_SetSeed(generator, state);
#else
      rqmc_xsubi[0] = saved_xsubi[0];
      rqmc_xsubi[1] = saved_xsubi[1];
      rqmc_xsubi[2] = saved_xsubi[2];
#endif
    }

    // generate the randomization
#ifdef MRG
    randnum = Rand_RandU01(generator);
#else
    randnum = erand48(rqmc_xsubi);
#endif

    // apply add shift
    result = result + randnum;

    // modulo 1
    if (result >= 1.0)
      result -= 1.0;
  } else {
    result = *(rpoint + currdim);
  }

  return (result);
}

// -----------------------------------------------------------------------
void FreeRandom(void)

/* Purpose: to free the randomization generator. The memory allocated
            by it, and to delete the random number generator.
   Pre: the randomization generator must have been initialized.
   Post: the randomization generator is free. */
{
  // free the shifted point used.
  if (rpoint != NULL)
    free(rpoint);
  rpoint = NULL;

  // free the random vecotr used.
  if (rvector != NULL)
    free(rvector);
  rvector = NULL;

  // delete the random number generator.
#ifdef MRG
  Rand_DeleteStream(&generator);
#endif

  return;
}

// -----------------------------------------------------------------------
