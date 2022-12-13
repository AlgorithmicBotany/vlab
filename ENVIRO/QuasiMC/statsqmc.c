// FILE: statsqmc.c

/* Implemention of the statsqmc.h
   For a description of the functions read statsqmc.
   There are two static functions declared. They are used to determine
   the confidence interval. */

// Mik Cieslak and Kris Luttmer
// May 22, 2001

#include "qmc.h"

// defines for InvStudentDist
#define P0 (-3.22232431088E-1)
#define Q0 9.9348462606E-2
#define P1 (-1.0)
#define Q1 5.88581570495E-1
#define P2 (-3.42242088547E-1)
#define Q2 5.31103462366E-1
#define P3 (-2.04231210245E-2)
#define Q3 1.0353775285E-1
#define P4 (-4.53642210148E-5)
#define Q4 3.8560700634E-3

// ========================================================================

/*
  Programmeurs       :
  ------------
   Pierre L'Ecuyer, Pierre Poulin, Gaetan Perron, Jean Belanger, Denis Alain

   Adapte a ANSI C par Francis Picard et Jean-Sebastien Senecal, aout 1999
*/

// ------------------------------------------------------------------------

static double InvNormalDist(double U) {
  double Z;
  double Y;

  if (U > 0.5)
    Y = sqrt(-(2.0 * log(1.0 - U)));
  else
    Y = sqrt(-(2.0 * log(U)));

  Z = Y + ((((Y * P4 + P3) * Y + P2) * Y + P1) * Y + P0) /
              ((((Y * Q4 + Q3) * Y + Q2) * Y + Q1) * Y + Q0);

  if (U < 0.5)
    Z = -Z;

  return Z;
} /* end InvNormalDist() */

// ------------------------------------------------------------------------

static double InvStudentDist(long n, double u) {
  double y;
  double x;
  double t;
  double p;
  double e;
  double d;
  double c;
  double b;
  double a;

  e = (double)n;

  if (u > 0.5)
    p = 2.0 * (1.0 - u);
  else
    p = 2.0 * u;

  if (n < 1)
    return (10);

  if (p <= 2.E-20)
    return (9);

  if (n == 1) {
    t = fabs(cos((3.1415926 * p) / 2.0) / sin((3.1415926 * p) / 2.0));
  } else if (n == 2)
    t = sqrt(2.0 / (p * (2.0 - p)) - 2.0);
  else {
    /* n > 2  */
    a = 1.0 / (e - 0.5);
    b = 48.0 / (a * a);
    c = (((20700.0 / b) * a - 98.0) * a - 16.0) * a + 96.36;
    d = e * sqrt((a * 3.1415926) / 2.0) * ((94.5 / (b + c) - 3.0) / b + 1.0);
    y = pow(d * p, 2.0 / e);
    if (y > a + 0.05) {
      if (p == 1.0)
        x = 0.0;
      else
        x = InvNormalDist(p * 0.5);
      y = x * x;
      if (n < 5l)
        c = c + 0.3 * (e - 4.5) * (x + 0.6);
      c = (((0.05 * d * x - 5.0) * x - 7.0) * x - 2.0) * x + b + c;
      y = ((((((0.4 * y + 6.3) * y + 36.0) * y + 94.5) / c - y) - 3.0) / b +
           1.0) *
          x;
      y = a * y * y;
      if (y > 0.002)
        y = exp(y) - 1.0;
      else
        y = 0.5 * y * y + y;
    } else {
      y = (((1.0 / ((((e + 6.0) / (e * y) - 0.089 * d) - 0.822) * (e + 2.0) *
                    3.0) +
             0.5 / (e + 4.0)) *
                y -
            1.0) *
           (e + 1.0)) /
              (e + 2.0) +
          1.0 / y;
    }
    t = sqrt(e * y);
  }
  if (u < 0.5)
    return (-t);
  else
    return (t);

  return (0);
} /* end InvStudentDist() */

// ========================================================================

STATSQMC InitStat(int numblocks)
/* Purpose: to return a STATSQMC type with numblocks blocks
       Pre: block must be > 0
      Post: upon success a stats type is returned else NULL */
{
  STATSQMC stats;

  if (numblocks < 1) {
    fprintf(stderr, "Cannot initialize stats with %d blocks.\n", numblocks);
    return (NULL);
  }

  stats = (STATSQMC)malloc(sizeof(int) * sizeof(int));

  stats->numblocks = numblocks;

  stats->blocks = (BLOCKQMC *)malloc(sizeof(BLOCKQMC) * numblocks);

  memset(stats->blocks, 0, sizeof(BLOCKQMC) * numblocks);

  return (stats);
}

// ------------------------------------------------------------------------

void StatUpdate1(STATSQMC stats, int block, double entry)
/* Purpose: to Update the random variables with entry at statistic
            block with index block.
       Pre: block must be > 0 and < stats->numblocks.
      Post: the statistic block has been updated. */
{
  if (stats == NULL) {
    fprintf(stderr, "Cannot return stats for non-existing structure.\n");
    return;
  }

  if ((block < 0) || (block >= stats->numblocks)) {
    fprintf(stderr, "Cannot update stats for block %d.\n", block);
    return;
  }

  stats->blocks[block].average[0] =
      ((stats->blocks[block].average[0] * stats->blocks[block].observations) +
       entry) /
      (stats->blocks[block].observations + 1);

  stats->blocks[block].squares[0] =
      ((stats->blocks[block].squares[0] * stats->blocks[block].observations) +
       (entry * entry)) /
      (stats->blocks[block].observations + 1);

  stats->blocks[block].observations += 1.0;

  return;
}

// ------------------------------------------------------------------------

void StatUpdate2(STATSQMC stats, int block, double entry1, double entry2)
/* Purpose: to Update the random variables with entry at statistic
            block with index block.
       Pre: block must be > 0 and < stats->numblocks.
      Post: the statistic block has been updated. */
{
  if (stats == NULL) {
    fprintf(stderr, "Cannot return stats for non-existing structure.\n");
    return;
  }

  if ((block < 0) || (block >= stats->numblocks)) {
    fprintf(stderr, "Cannot update stats for block %d.\n", block);
    return;
  }

  StatUpdate1(stats, block, entry1);
  stats->blocks[block].observations -= 1.0;

  stats->blocks[block].average[1] =
      ((stats->blocks[block].average[1] * stats->blocks[block].observations) +
       entry2) /
      (stats->blocks[block].observations + 1);

  stats->blocks[block].squares[1] =
      ((stats->blocks[block].squares[1] * stats->blocks[block].observations) +
       (entry2 * entry2)) /
      (stats->blocks[block].observations + 1);

  stats->blocks[block].totalaverage =
      ((stats->blocks[block].totalaverage * stats->blocks[block].observations) +
       (entry1 * entry2)) /
      (stats->blocks[block].observations + 1);

  stats->blocks[block].observations += 1.0;

  return;
}

// ------------------------------------------------------------------------

double StatAverage(STATSQMC stats, int block, int randvar)
/* Purpose: to return the average of the random variable, randvar, at
            block with index block.
       Pre: block must exist.
            randvar must be < 2.
      Post: the average is returned. */
{
  if (stats == NULL) {
    fprintf(stderr, "Cannot return stats for non-existing structure.\n");
    return (0.0);
  }

  if ((block < 0) || (block >= stats->numblocks)) {
    fprintf(stderr, "Cannot find average for block %d.\n", block);
    return (0.0);
  }

  if ((randvar < 0) || (randvar > 1)) {
    fprintf(stderr, "Cannot return average for random variable %d.\n", randvar);
    return (0.0);
  }

  return (stats->blocks[block].average[randvar]);
}

// ------------------------------------------------------------------------

double StatVariance(STATSQMC stats, int block, int randvar)
/* Purpose: to return the variance of the block at index block for
            the random variable randvar.
       Pre: block must exist
            randvar must be < 2.
      Post: the variance is returned. */
{
  if (stats == NULL) {
    fprintf(stderr, "Cannot return stats for non-existing structure.\n");
    return (0.0);
  }

  if ((block < 0) || (block >= stats->numblocks)) {
    fprintf(stderr, "Cannot find variance for block %d.\n", block);
    return (0.0);
  }

  if ((randvar < 0) || (randvar > 1)) {
    fprintf(stderr, "Cannot return variance for random variable %d.\n",
            randvar);
    return (0.0);
  }

  if (stats->blocks[block].observations < 2)
    return (0.0);

  return ((stats->blocks[block].squares[randvar] -
           (stats->blocks[block].average[randvar] *
            stats->blocks[block].average[randvar])) *
          stats->blocks[block].observations /
          (stats->blocks[block].observations - 1));
}

// ------------------------------------------------------------------------

CONFIDENCEINTERVAL StatConfIntval(STATSQMC stats, int block, int randvar,
                                  double level)
/* Purpose: to return the confidence interval with level, level, at block
            with index block for random variable randvar.
       Pre: block must be < MAXSTATSBLOCK.
            0 < level <= 1.
            0 <= randvar must be < 2.
      Post: the confidence interval is returned */
{
  CONFIDENCEINTERVAL cf;
  double result;
  double U;

  cf.low = 0;
  cf.high = 0;

  if (stats == NULL) {
    fprintf(stderr, "Cannot return stats for non-existing structure.\n");
    return (cf);
  }

  if ((block < 0) || (block >= stats->numblocks)) {
    fprintf(stderr, "Cannot return confidence interval for block, %d.\n",
            block);
    return (cf);
  }

  if ((randvar < 0) || (randvar > 1)) {
    fprintf(stderr, "Cannot return confidence interval for random variable ");
    fprintf(stderr, "%d \n", randvar);
    return (cf);
  }

  if (stats->blocks[block].observations < 2)
    return (cf);

  if (level == 0) {
    fprintf(stderr, "Cannot return confidence interval for level = 0.\n");
    return (cf);
  }

  result = 1.0 - (level / 2.0);

  U = InvStudentDist((long)stats->blocks[block].observations - 1, result) *
      sqrt(StatVariance(stats, block, randvar) /
           stats->blocks[block].observations);

  cf.low = (stats->blocks[block].average[randvar]) - U;
  cf.high = (stats->blocks[block].average[randvar]) + U;

  return (cf);
}

// -----------------------------------------------------------------------

double StatCovariance(STATSQMC stats, int block)
/* Purpose: to return the covariance of the statistical block.
       Pre: block must exist.
            the number of random variables must be 2.
      Post: the covariance is returned. */
{
  if (stats == NULL) {
    fprintf(stderr, "Cannot return stats for non-existing structure.\n");
    return (0.0);
  }

  if ((block < 0) || (block >= stats->numblocks)) {
    fprintf(stderr, "Cannot find covariance for block, %d", block);
    return (0.0);
  }

  if (stats->blocks[block].observations < 2)
    return (0.0);

  return (
      (stats->blocks[block].totalaverage -
       (stats->blocks[block].average[0] * stats->blocks[block].average[1])) *
      stats->blocks[block].observations /
      (stats->blocks[block].observations - 1));
}

// -----------------------------------------------------------------------

double StatCorrelation(STATSQMC stats, int block)
/* Purpose: to return the correlation between the two random variables
       Pre: block must exist
            the number of random variables must be > 1
      Post: the correlation is returned. */
{
  double totalvariance;

  if (stats == NULL) {
    fprintf(stderr, "Cannot return stats for non-existing structure.\n");
    return (0.0);
  }

  if ((block < 0) || (block >= stats->numblocks)) {
    fprintf(stderr, "Cannot find covariance for block, %d", block);
    return (0.0);
  }

  totalvariance = StatVariance(stats, block, 0) * StatVariance(stats, block, 1);

  if (totalvariance != 0) {
    totalvariance = sqrt(totalvariance);
    return (StatCovariance(stats, block) / totalvariance);
  } else
    return (0.0);
}

// -----------------------------------------------------------------------

void ResetStat(STATSQMC stats, int block)
/* Purpose: to reset the statistic block, at index block.
       Pre: block must be >= 0.
      Post: the statistical block, at index block, has been reset. */
{
  if (stats == NULL) {
    fprintf(stderr, "Cannot reset stats for non-existing structure.\n");
    return;
  }

  if ((block < 0) || (block >= stats->numblocks)) {
    fprintf(stderr, "Cannot reset block, %d.\n", block);
    return;
  }

  memset(&stats->blocks[block], 0, sizeof(BLOCKQMC));

  return;
}

// -----------------------------------------------------------------------

void FreeStat(STATSQMC stats)
/* Purpose: to free all of the blocks in the stats data type.
       Pre: stats must be not be NULL.
      Post: the blocks are free. */
{
  if (stats != NULL) {
    if (stats->blocks != NULL) {
      free(stats->blocks);
      stats->blocks = NULL;
    }
    free(stats);
    stats = NULL;
  }

  return;
}

// -----------------------------------------------------------------------
