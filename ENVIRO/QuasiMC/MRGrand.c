/******************************************************************************

Environnement       :  GCC et CC sur Solaris et Redhat Linux.
-------------
Programmeurs        :
-------------
     Richard Simard (cmrg.c)

     Modifie par Francis Picard et Jean-Sebastien Senecal, aout 1999

Copyright: Pierre L'Ecuyer, University of Montreal.

Notice: This code can be used freely for personal, academic or non-commerical
purposes. For commercial purposes, please contact P. L'Ecuyer at
lecuyer@iro.umontreal.ca.

*******************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "MRGrand.h"

struct InfoStream {
  double Cg[6], Bg[6], Ig[6];
  int anti;
  int doubleGen;
  char name[32];
  RngStream next;
  RngStream prev;
};
/*   Information on a stream.
The arrays {Cg, Bg, Ig} contain the current state of the
stream, the starting state of the current block in the state,
and the starting state of the stream.
This stream generates antithetic variates if and only if
{anti = 1}.  The array {name} contains the name of the
stream, given at creation.  The pointers {next} and {prev} point
to the two neighbors in the list of existing streams.  */

/*---------------------------------------------------------------------*/
/* Private part.                                                       */
/*---------------------------------------------------------------------*/
#define norm 2.328306549837828827e-10
#define m1 4294967087.0
#define m2 4294944443.0
#define a12 1403580.0
#define a13n 810728.0
#define a21 527612.0
#define a23n 1370589.0

#define two17 131072.0
#define two53 9007199254740992.0
#define invtwo24 5.9604644775390625e-8

/* Head and tail of list of existing streams. */
/* static RngStream GenListHead = NULL, GenListTail = NULL;
 */

/* boolean: = 1, the first stream. */
static int First = 1;

/* Default initial seed of the package. */
static double defseed[6] = {12345, 12345, 12345, 12345, 12345, 12345};

/* Initial seed of the last created stream (if any). */
static double lastseed[6] = {12345, 12345, 12345, 12345, 12345, 12345};

/* The following are the transition matrices of the two MRG components */
/* (in matrix form), raised to the powers -1, 1, 2^76, and 2^127, resp.*/
static double InvA1[3][3] = {/* Inverse of A1p0 */
                             {184888585.0, 0.0, 1945170933.0},
                             {1.0, 0.0, 0.0},
                             {0.0, 1.0, 0.0}};

static double InvA2[3][3] = {/* Inverse of A2p0 */
                             {0.0, 360363334.0, 4225571728.0},
                             {1.0, 0.0, 0.0},
                             {0.0, 1.0, 0.0}};

static double A1p0[3][3] = {
    {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {-810728.0, 1403580.0, 0.0}};

static double A2p0[3][3] = {
    {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}, {-1370589.0, 0.0, 527612.0}};

static double A1p76[3][3] = {{82758667.0, 1871391091.0, 4127413238.0},
                             {3672831523.0, 69195019.0, 1871391091.0},
                             {3672091415.0, 3528743235.0, 69195019.0}};

static double A2p76[3][3] = {{1511326704.0, 3759209742.0, 1610795712.0},
                             {4292754251.0, 1511326704.0, 3889917532.0},
                             {3859662829.0, 4292754251.0, 3708466080.0}};

static double A1p127[3][3] = {{2427906178.0, 3580155704.0, 949770784.0},
                              {226153695.0, 1230515664.0, 3580155704.0},
                              {1988835001.0, 986791581.0, 1230515664.0}};

static double A2p127[3][3] = {{1464411153.0, 277697599.0, 1610723613.0},
                              {32183930.0, 1464411153.0, 1022607788.0},
                              {2824425944.0, 32183930.0, 2093834863.0}};

static double
MultModM(double a, double s, double c,
         double m) { /* Compute (a*s + c) MOD m ; m must be < 2^35 */
  /* Works also for s, c < 0 */
  double v;
  long a1;
  v = a * s + c;
  if (fabs(v) >= two53) {
    a1 = (long)(a / two17);
    a -= a1 * two17;
    v = a1 * s;
    a1 = (long)(v / m);
    v -= a1 * m;
    v = v * two17 + a * s + c;
  }
  a1 = (long)(v / m);
  if ((v -= a1 * m) < 0.0)
    return v += m;
  else
    return v;
}

static void MatVecModM(double A[3][3], double s[3], double v[3], double m)
/* Returns v = A*s MOD m.  Assumes that -m < s[i] < m. */
/* Works even if v = s.                                */
{
  int i;
  double x[3];
  for (i = 0; i <= 2; ++i) {
    x[i] = MultModM(A[i][0], s[0], 0.0, m);
    x[i] = MultModM(A[i][1], s[1], x[i], m);
    x[i] = MultModM(A[i][2], s[2], x[i], m);
  }
  for (i = 0; i <= 2; ++i)
    v[i] = x[i];
}

static void MatMatModM(double A[3][3], double B[3][3], double C[3][3], double m)
/* Returns C = A*B MOD m */
/* Note: work even if A = C or B = C or A = B = C.         */
{
  int i, j;
  double V[3], W[3][3];
  for (i = 0; i <= 2; ++i) {
    for (j = 0; j <= 2; ++j)
      V[j] = B[j][i];
    MatVecModM(A, V, V, m);
    for (j = 0; j <= 2; ++j)
      W[j][i] = V[j];
  }
  for (i = 0; i <= 2; ++i) {
    for (j = 0; j <= 2; ++j)
      C[i][j] = W[i][j];
  }
}

static void MatTwoPowModM(double A[3][3], double B[3][3], double m, long e)
/* Compute matrix B = (A^(2^e) Mod m);  works even if A = B */
{
  int i, j;

  /* initialize: B = A */
  if (A != B) {
    for (i = 0; i <= 2; i++) {
      for (j = 0; j <= 2; ++j)
        B[i][j] = A[i][j];
    }
  }
  /* Compute B = A^{2^e} */
  for (i = 0; i < e; i++)
    MatMatModM(B, B, B, m);
}

static void MatPowModM(double A[3][3], double B[3][3], double m, long c)
/* Compute matrix D = A^c Mod m ;  works even if A = B */
{
  int i, j, n = c;
  double W[3][3];

  /* initialize: W = A; B = I */
  for (i = 0; i <= 2; i++) {
    for (j = 0; j <= 2; ++j) {
      W[i][j] = A[i][j];
      B[i][j] = 0.0;
    }
  }
  for (j = 0; j <= 2; ++j)
    B[j][j] = 1.0;

  /* Compute B = A^c mod m using the binary decomposition of c */
  while (n > 0) {
    if (n % 2)
      MatMatModM(W, B, B, m);
    MatMatModM(W, W, W, m);
    n /= 2;
  }
}

static double U01(RngStream g) {
  register long k;
  register double p1, p2, u;
  /* Component 1 */
  p1 = a12 * g->Cg[1] - a13n * g->Cg[0];
  k = (long)(p1 / m1);
  p1 -= k * m1;
  if (p1 < 0.0)
    p1 += m1;
  g->Cg[0] = g->Cg[1];
  g->Cg[1] = g->Cg[2];
  g->Cg[2] = p1;
  /* Component 2 */
  p2 = a21 * g->Cg[5] - a23n * g->Cg[3];
  k = (long)(p2 / m2);
  p2 -= k * m2;
  if (p2 < 0.0)
    p2 += m2;
  g->Cg[3] = g->Cg[4];
  g->Cg[4] = g->Cg[5];
  g->Cg[5] = p2;
  /* Combination */
  u = ((p1 > p2) ? (p1 - p2) * norm : (p1 - p2 + m1) * norm);
  return (g->anti == 1) ? (1 - u) : u;
}

static double U01d(RngStream g) {
  double u;
  if (g->anti != 1) {
    u = U01(g) + U01(g) * invtwo24;
    return (u < 1.0) ? u : (u - 1.0);
  } else {
    u = (U01(g) + (U01(g) - 1.0) * invtwo24);
    return (u < 0.0) ? u + 1.0 : u;
  }
}

/*---------------------------------------------------------------------*/
/* Public part.                                                        */
/*---------------------------------------------------------------------*/

void Rand_CreateStream(RngStream *pg, char name[32]) {
  int i;
  RngStream g;
  ulongint V[6];
  g = (RngStream)malloc(sizeof(struct InfoStream));
  if (!g) {
    printf("CreateStream: No more memory\n\n");
    exit(1);
  }
  g->anti = 0;
  for (i = 0; i < 32; ++i)
    g->name[i] = name[i];
  if (First) /* the first stream */
  {
    for (i = 0; i < 6; ++i)
      V[i] = (ulongint)defseed[i];
    Rand_SetSeed(g, V);
    for (i = 0; i < 6; ++i)
      lastseed[i] = defseed[i];
    First = 0;
  } else {
    MatVecModM(A1p127, lastseed, lastseed, m1);
    MatVecModM(A2p127, &lastseed[3], &lastseed[3], m2);
    for (i = 0; i < 6; ++i)
      V[i] = (ulongint)lastseed[i];
    Rand_SetSeed(g, V);
  }
  *pg = g;
}

void Rand_DeleteStream(RngStream *pg) {
  if (*pg == NULL)
    return;
  free(*pg);
  *pg = NULL;
}

void Rand_ResetStream(RngStream g, SeedType where) {
  int i;
  switch (where) {
  case StartStream:
    for (i = 0; i <= 5; ++i)
      g->Cg[i] = g->Bg[i] = g->Ig[i];
    break;
  case StartBlock:
    for (i = 0; i <= 5; ++i)
      g->Cg[i] = g->Bg[i];
    break;
  case NextBlock:
    MatVecModM(A1p76, g->Bg, g->Bg, m1);
    MatVecModM(A2p76, &g->Bg[3], &g->Bg[3], m2);
    for (i = 0; i <= 5; ++i)
      g->Cg[i] = g->Bg[i];
    break;
  }
}

void Rand_SetPackageSeed(ulongint seed[6]) {
  int i;

  /* Update the default seeds (used by CreateGen).  */
  for (i = 0; i <= 5; ++i)
    lastseed[i] = defseed[i] = seed[i];
  First = 1;
}

void Rand_AdvanceState(RngStream g, long e, long c) {
  double B1[3][3], C1[3][3], B2[3][3], C2[3][3];

  if (e > 0) {
    MatTwoPowModM(A1p0, B1, m1, e);
    MatTwoPowModM(A2p0, B2, m2, e);
  } else if (e < 0) {
    MatTwoPowModM(InvA1, B1, m1, -e);
    MatTwoPowModM(InvA2, B2, m2, -e);
  }

  if (c >= 0) {
    MatPowModM(A1p0, C1, m1, c);
    MatPowModM(A2p0, C2, m2, c);
  } else if (c < 0) {
    MatPowModM(InvA1, C1, m1, -c);
    MatPowModM(InvA2, C2, m2, -c);
  }

  if (e) {
    MatMatModM(B1, C1, C1, m1);
    MatMatModM(B2, C2, C2, m2);
  }
  MatVecModM(C1, g->Cg, g->Cg, m1);
  MatVecModM(C2, &g->Cg[3], &g->Cg[3], m2);
}

void Rand_SetSeed(RngStream g, ulongint seed[6]) {
  int i;
  for (i = 0; i <= 2; ++i) {
    if (seed[i] >= m1) {
      printf("Error: Seed[%d] >= m1\n", i);
      exit(1);
    }
  }
  for (i = 3; i <= 5; ++i) {
    if (seed[i] >= m2) {
      printf("Error: Seed[%d] >= m2\n", i);
      exit(1);
    }
  }
  for (i = 0; i <= 5; ++i)
    g->Cg[i] = g->Bg[i] = g->Ig[i] = seed[i];
}

void Rand_GetState(RngStream g, ulongint seed[6]) {
  int i;
  for (i = 0; i <= 5; ++i)
    seed[i] = (ulongint)g->Cg[i];
}

void Rand_WriteState(RngStream g) {
  int i;
  printf("\n State of RngStream  %s :", g->name);
  for (i = 0; i < 6; i++)
    printf("\n   Cg[%u] = %14lu", i, (ulongint)g->Cg[i]);
  printf("\n\n");
}

void Rand_DoubleGenerator(RngStream g, int d) { g->doubleGen = d; }

void Rand_SetAntithetic(RngStream g, int a) { g->anti = a; }

double Rand_RandU01(RngStream g) {
  if (g->doubleGen)
    return U01d(g);
  else
    return U01(g);
}

long Rand_RandInt(RngStream g, long i, long j) {
  double u;
  u = Rand_RandU01(g);
  return i + (long)(u * (j - i + 1));
}
