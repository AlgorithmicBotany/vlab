/*
  The korobov polynomial rule.

  Written by: Kris Luttmer and Mik Cieslak
  Date: NOvember 24, 2001



*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "combinedpk.h"

#define L 32
#define MAX ((double)4294967296.) // 2^32

struct pk_generator {
  unsigned int poly; /* the polynomial */
  unsigned int u;    /* the bits of the current point */
  unsigned int c;    /* a special bit mask */
  unsigned int z;    /* another bit mask used in computing the next u */
  unsigned int n;    /* the total points allowed by this generator */
  int v;             /* a special variable given by the user */
  int *q;
  int *bi;
  int k; /* the degree of the polynomial */
  int m; /* the number of non zero coefficients -2 */
};

static unsigned int bits = 0; /* the bits that make up a point */
static int dim = 0;           /* the dimension of the point */
static int n_gens = 0;        /* the number of generators needed */
static double n = 0;          /* the number of points to generate */
static double count;          /* a count of the total number of points */
static double state_count =
    0;                /* the number of points before changing generators */
static int STATE = 1; /* a bit mask to ecide what generator to use */

// things that must be calculated
static double *point = NULL; // the point that will be returned
static struct pk_generator *gens = NULL;

// functions to compute the variables that we need
static int Init_gen(struct pk_generator *gen);
static void ResetCpGenerator(struct pk_generator *gen);
static void
compute_next_bits(); /* the bits value will be stored in global "bits" */

static int compute_Ck(unsigned int poly);
static int compute_Cm(unsigned int poly);
static int *compute_Cq(unsigned int poly, int k, int m);
static unsigned int compute_Cc(int k);
static unsigned int compute_Cz();
static void compute_Cu(struct pk_generator *gen);
static unsigned int compute_pkorobov_points(unsigned int poly);

/**************************************************************************
       This is the function to intialize the combined korobov polynomial method
**************************************************************************/
int InitCpKorobov(int s, double points, int num_gens, double *gen_params) {
  int i;

  // testing ploynomials and parameters
  /*
  0x00002289 v=4 c=FFF80000 Decimal = 8841
  0x00008017 v=8 c=FFFF0000 Decimal = 32791
  0x0000082B v=4 c=FFF00000 Decimal = 2091
  0x00000083 v=1 c=FF000000 Decimal = 131
  */

  if (gens != NULL) {
    fprintf(
        stderr,
        "Cannot reinitialize the Korobov generator without freeing it first\n");
    return 0;
  }

  if (num_gens > 3 || num_gens <= 0) {
    fprintf(stderr, "The number of generators requested for combined "
                    "polynomial korobov is invalid\n");
    return 0;
  }

  count = 0;
  n = points;
  dim = s;
  n_gens = num_gens;

  gens = calloc(n_gens, sizeof(struct pk_generator));

  if (gen_params == NULL) {
    fprintf(stderr,
            "No parameters for polynomial korobov generators specified\n");
    return 0;
  }

  for (i = 0; i < n_gens; i++) {
    gens[i].poly = (int)gen_params[2 * i];
    gens[i].v = (int)gen_params[2 * i + 1];
  }

  state_count = compute_pkorobov_points(gens[0].poly) - 1;

  for (i = 0; i < n_gens; i++) {
    if (Init_gen(&gens[i]) == 0) {
      FreeCpKorobov();
      return 0;
    }
  }

  point = calloc(dim, sizeof(double));

  if (point == NULL)
    return 0;

  return 1;
}

/**************************************************************************
   This is the function to generate the points for the combined korbov
polynomial rule
**************************************************************************/
double *CpKorobov() {
  int i;

  if (count >= n) {
    fprintf(stderr,
            "Polynomial Korobov rule cannot generate any more points\n");
    return (NULL);
  }

  if (count == 0) {
    ++count;
    return point;
  }

  if (state_count == 0) {
    ++STATE;
    state_count = 1;
    for (i = 0; i < n_gens; i++) {
      if (STATE & (0x01 << i))
        state_count = state_count * (compute_pkorobov_points(gens[i].poly) - 1);
    }
    compute_next_bits();
    point[0] = ((double)bits) / MAX;
    for (i = 1; i < dim; i++) {
      compute_next_bits();
      point[i] = ((double)bits) / MAX;
    }

    ++count;
    --state_count;
    return point;
  }

  if (count == 1) {
    compute_next_bits();
    point[0] = ((double)bits) / MAX;

    for (i = 1; i < dim; i++) {
      compute_next_bits();
      point[i] = ((double)bits) / MAX;
    }

    state_count--;
    count++;
    return point;
  }

  for (i = 0; i < dim - 1; i++)
    point[i] = point[i + 1];

  compute_next_bits();
  point[dim - 1] = ((double)bits) / MAX;
  count++;
  state_count--;
  return point;
}
/**************************************************************************
       This is the function to reset the combined korobov polynomial method
**************************************************************************/
void ResetCpKorobov() {
  int limit;
  int i, j, l;
  unsigned int temp;
  unsigned int b = 0;

  count = 0;

  if (point != NULL)
    memset(point, 0, sizeof(double) * dim);

  if (gens != NULL) {
    for (l = 0; l < n_gens; l++) {
      gens[l].u = 0x80000000;
      limit = (int)ceil((double)(L - gens[l].k) / (gens[l].k - gens[l].q[0]));

      for (j = 0; j < limit; j++) {
        b = 0;
        for (i = 0; i < gens[l].m; i++) {
          gens[l].bi[i] = gens[l].u << gens[l].q[i];
          b ^= gens[l].bi[i];
        }

        b ^= gens[l].u;
        b >>= gens[l].k;
        temp = gens[l].z;
        temp >>= (gens[l].k - gens[l].q[0]) * j;
        gens[l].u ^= (b & temp);
      }
    }
  }

  state_count = compute_pkorobov_points(gens[0].poly);
  STATE = 1;
  bits = 0;
}
/**************************************************************************
       This is the function to free the combined korobov polynomial method
**************************************************************************/
void FreeCpKorobov() {
  int i;

  if (gens != NULL) {
    for (i = 0; i < n_gens; i++) {
      if (gens[i].bi)
        free(gens[i].bi);

      if (gens[i].q)
        free(gens[i].q);
    }

    free(gens);
  }

  if (point)
    free(point);

  state_count = 0;
  STATE = 0;
  point = NULL;
  gens = NULL;
}
/**************************************************************************
       This function will generate the next dimension of a point if the
       generator is using random dimension.
**************************************************************************/
double NextDimCpKorobov(int start_dim) {
  static struct pk_generator new_gens[3];
  static struct pk_generator old_gens[3];
  double result;
  int i;

  if (count == 1)
    return (0.0);

  if (dim == start_dim) {
    for (i = 0; i < n_gens; i++)
      new_gens[i] = old_gens[i] = gens[i];
  } else {
    for (i = 0; i < n_gens; i++)
      gens[i] = new_gens[i];
  }

  compute_next_bits();

  for (i = 0; i < n_gens; i++)
    new_gens[i] = gens[i];

  result = ((double)bits) / MAX;

  for (i = 0; i < n_gens; i++)
    gens[i] = old_gens[i];

  return (result);
}

/**************************************************************************
  This is the function to initialize a generator
  Before this function is called all that is required is that
  the ploynomial be set and the parameter v be set in the generator
**************************************************************************/
int Init_gen(struct pk_generator *gen) {
  gen->u = 0x80000000;
  gen->k = compute_Ck(gen->poly);
  gen->m = compute_Cm(gen->poly);
  gen->q = compute_Cq(gen->poly, gen->k, gen->m);
  gen->z = compute_Cz(gen->k, gen->q);
  gen->n = compute_pkorobov_points(gen->poly);
  gen->c = compute_Cc(gen->k);
  gen->bi = malloc(sizeof(int) * gen->m);

  if (gen->m < 1) {
    fprintf(stderr, "\nThis polynomial is invalid\n");
    return 0;
  }

  ResetCpGenerator(gen);
  return 1;
}
/**************************************************************************
        This is the function to reset a ploynomial korobov generator to it's
        original state
**************************************************************************/
void ResetCpGenerator(struct pk_generator *gen) {

  int i, j;
  unsigned int temp;
  unsigned int b = 0;
  int limit;

  gen->u = 0x80000000;
  limit = (int)ceil((double)(L - gen->k) / (gen->k - gen->q[0]));

  for (j = 0; j < limit; j++) {
    b = 0;
    for (i = 0; i < gen->m; i++) {
      gen->bi[i] = gen->u << gen->q[i];
      b ^= gen->bi[i];
    }

    b ^= gen->u;
    b >>= gen->k;
    temp = gen->z;
    temp >>= (gen->k - gen->q[0]) * j;
    gen->u ^= (b & temp);
  }
}
/**************************************************************************
  This is the function to compute the next value of "bits" to determine the
  next point.
**************************************************************************/
void compute_next_bits() {
  int i;
  bits = 0;

  for (i = 0; i < n_gens; i++) {
    if (STATE & (0x01 << i)) {
      bits ^= gens[i].u;
      compute_Cu(&gens[i]);
    }
  }
}
/**************************************************************************
  This is the function to compute the value of the next u
**************************************************************************/

void compute_Cu(struct pk_generator *gen) {
  int i;
  unsigned int b = 0;

  for (i = 0; i < gen->m; i++) {
    gen->bi[i] = gen->u << gen->q[i];
    b ^= gen->bi[i];
  }
  b ^= gen->u;
  b >>= gen->k - gen->v;
  gen->u = gen->u & gen->c;
  gen->u <<= gen->v;
  gen->u ^= b;
}

/**************************************************************************
  This is the function to compute the value of k for the given polynomial
  K is the degree of the polynomial
**************************************************************************/

int compute_Ck(unsigned int poly) {
  int temp = 0;
  unsigned int mask = 0x80000000;

  while (!(poly & (mask >> temp)) && temp < L - 1)
    temp++;

  return L - temp - 1;
}

/**************************************************************************
  This is the function to compute the value of m for the given polynomial
  m is the total number of non-zero coefficients not including the first and
  last non zero coefficients.
**************************************************************************/
int compute_Cm(unsigned int poly) {
  int temp = -2;
  int i;
  unsigned int mask = 0x80000000;

  for (i = 0; i < L; i++) {
    if (poly & (mask >> i))
      temp++;
  }

  return temp;
}
/**************************************************************************
  This is the function to compute the vector of q for the given polynomial
  q is the coefficients that are non zero excluding the first and last there
  will always be m of them
**************************************************************************/
int *compute_Cq(unsigned int poly, int k, int m) {
  int *temp;
  int mask = 0x00000001;
  int i = 0;
  int j = 0;

  mask <<= k - 1;
  temp = malloc(sizeof(int) * m);

  for (j = 0; j < m; j++) {
    while (!((mask >> i) & poly))
      i++;
    if (j == 0)
      temp[j] = k - i - 1;
    else
      temp[j] = temp[j - 1] - i - 1;

    mask = mask >> (i + 1);
    i = 0;
  }

  return temp;
}
/**************************************************************************
  This is the function to compute the value that we need for c.
  C is used when generating more coordinates
 **************************************************************************/
unsigned int compute_Cc(int k) {
  int mask = 0x80000000;
  int i;
  unsigned int temp = 0;

  for (i = 0; i < k; i++) {
    temp |= mask >> i;
  }

  return temp;
}
/**************************************************************************
  This is the function to compute the value that we need for z.
  z is used in the initialization. This function must be called after cmpute_q()
**************************************************************************/
unsigned int compute_Cz(int k, int *q) {
  unsigned int z = 0;
  unsigned int mask = 0x80000000;
  int i;
  int bits = k - q[0];

  for (i = 0; i < bits; i++) {
    z |= (mask >> i);
  }

  z >>= k;
  return z;
}

/*******************************************************************************
        This is a method to compute the number of points for any single
generator
********************************************************************************/
unsigned int compute_pkorobov_points(unsigned int poly) {
  int temp = 0;
  unsigned int mask = 0x80000000;

  while (!(poly & (mask >> temp)) && temp < L - 1)
    temp++;

  return 0x01 << (L - temp - 1);
}
