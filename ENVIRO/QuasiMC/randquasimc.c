/* rand.c - implementation for getting RQMC points */

#include "quasiMC.h"
#include "qmc.h"
#include "randquasimc.h"

extern int use_sky_model;
extern unsigned int max_depth;
extern double *rpointqmc;
extern SCENE scene;
static int startdim[RANDQMC_MAX];

/* ------------------------------------------------------------------------- */

void ResetU01(void)
// first two QMC points are used for generating random pt to trace ray
// if sky model, next two allocated to generating sample from sky (THETA starts
// at dim 4) if more than one directional light sources, next one allocated for
// picking light (THETA starts at dim 3) if only one light source, THETA starts
// at dim 2
{
  startdim[RANDQMC_START] = 0;
  startdim[RANDQMC_THETA] = 2;
  if (use_sky_model)
    startdim[RANDQMC_THETA] += 2;
  else if (scene.num_lights > 1)
    startdim[RANDQMC_THETA] += 1;
  startdim[RANDQMC_PHI] = startdim[RANDQMC_PHI - 1] + max_depth;
  startdim[RANDQMC_RR] = startdim[RANDQMC_RR - 1] + max_depth;
  startdim[RANDQMC_RT] = startdim[RANDQMC_RT - 1] + max_depth;
  startdim[RANDQMC_N] = startdim[RANDQMC_N - 1] + max_depth;

  return;
}

/* ------------------------------------------------------------------------- */

float RandU01(int index) {
  float value;

  value = (float)rpointqmc[startdim[index]];
  startdim[index]++;

  return (value);
}

/* ------------------------------------------------------------------------- */

int SetRQMC(char *RQMC_method, unsigned int max_depth, int num_samples,
            double *npoints) {
  static int power_n[14][2] = {
      {128, 11},         {1024, 139},       {2048, 519},      {4096, 1081},
      {8192, 1289},      {16384, 2961},     {32768, 2149},    {65536, 21553},
      {131072, 27383},   {262144, 3597},    {524288, 120079}, {1048576, 172565},
      {2097152, 886269}, {4194304, 3067221}};
  static int poly_korobov[7][8] = {
      {1024, 2, 11, 1, 137, 4, 0, 0},   {2048, 2, 37, 3, 67, 5, 0, 0},
      {4096, 2, 37, 1, 131, 2, 0, 0},   {8192, 2, 67, 5, 131, 4, 0, 0},
      {16384, 3, 11, 1, 19, 1, 137, 2}, {32768, 3, 11, 2, 37, 3, 131, 2},
      {65536, 2, 11, 2, 8245, 5, 0, 0}};

  float min_diff;
  int i, min_index, dimen;

  min_index = 0;
  dimen = 2;
  if (use_sky_model)
    dimen += 2;
  else if (scene.num_lights > 1)
    dimen += 1;

  dimen += (RANDQMC_MAX - 1) * max_depth;

  // set the RQMC library parameters according to the method
  if (!StrCaseCmp(RQMC_method, "sobol")) {
    SetQMC(QMC_NPARAMS, 0);
    SetQMC(QMC_METHOD, SOBOL_METHOD);
    // SetQMC (QMC_DIMEN, RANDQMC_MAX * max_depth);
    SetQMC(QMC_DIMEN, dimen); // fix to use only the exact dimension
    SetQMC(QMC_NPOINTS, *npoints);
    SetQMC(QMC_RMETHOD, XORSHIFT);
    SetQMC(QMC_RANDS, num_samples);
    SetQMC(QMC_RBASE, 2);
  } else if (!StrCaseCmp(RQMC_method, "korobov")) {
    SetQMC(QMC_NPARAMS, 2);
    SetQMC(QMC_METHOD, KOROBOV_METHOD);
    // SetQMC (QMC_DIMEN, RANDQMC_MAX * max_depth);
    SetQMC(QMC_DIMEN, dimen);

    // npoints has to be set correctly to match the generator 'a'
    min_diff = 1e30f;
    for (i = 0; i < 14; i++)
      if (fabs(*npoints - (double)power_n[i][0]) < min_diff) {
        min_diff = (float)fabs(*npoints - (double)power_n[i][0]);
        min_index = i;
      }
    if (*npoints != (double)power_n[min_index][0]) {
      fprintf(stderr, "quasiMC - the number of rays for the Korobov method ");
      fprintf(stderr, "must be a power of 2. Using the closest value: %d.\n",
              power_n[min_index][0]);
      *npoints = (double)power_n[min_index][0];
    }

    SetQMC(QMC_NPOINTS, *npoints);
    SetQMC(QMC_NPARAMS + 1, (double)power_n[min_index][1]);
    SetQMC(QMC_NPARAMS + 2, KOROBOV_POWER_OF_2);

    SetQMC(QMC_RMETHOD, ADDSHIFT);
    SetQMC(QMC_RANDS, num_samples);
    SetQMC(QMC_RBASE, 2);
  } else if (!StrCaseCmp(RQMC_method, "poly-korobov")) {
    // npoints has to be set correctly to match the generator's polynomials
    min_diff = 1e30f;
    for (i = 0; i < 7; i++)
      if (fabs(*npoints - (double)poly_korobov[i][0]) < min_diff) {
        min_diff = (float)fabs(*npoints - (double)poly_korobov[i][0]);
        min_index = i;
      }
    if (*npoints != (double)poly_korobov[min_index][0]) {
      fprintf(stderr, "quasiMC - the number of rays for the poly-Korobov ");
      fprintf(stderr, "method must be a power of 2 in [1024, 65536]. ");
      fprintf(stderr, "Using the closest value: %d.\n",
              poly_korobov[min_index][0]);
      *npoints = (double)poly_korobov[min_index][0];
    }

    SetQMC(QMC_NPARAMS, (double)(poly_korobov[min_index][1] * 2 + 1));
    SetQMC(QMC_METHOD, PKOROBOV_METHOD);
    // SetQMC (QMC_DIMEN, RANDQMC_MAX * max_depth);
    SetQMC(QMC_DIMEN, dimen);

    SetQMC(QMC_NPOINTS, *npoints);
    SetQMC(QMC_NPARAMS + 1, (double)poly_korobov[min_index][1]);

    for (i = 0; i < poly_korobov[min_index][1]; i++) {
      SetQMC(QMC_NPARAMS + 2 * (i + 1),
             (double)poly_korobov[min_index][2 * (i + 1)]);
      SetQMC(QMC_NPARAMS + 2 * (i + 1) + 1,
             (double)poly_korobov[min_index][2 * (i + 1) + 1]);
    }

    SetQMC(QMC_RMETHOD, ADDSHIFT);
    SetQMC(QMC_RANDS, num_samples);
    SetQMC(QMC_RBASE, 2);
  } else {
    /* RQMC_method is compared to "monte" because the "carlo" part is cut out
       of RQMC_method in args.c by the function strtok() */
    if (StrCaseCmp(RQMC_method, "monte"))
      fprintf(
          stderr,
          "quasiMC - unknown sampling method specified. Using Monte Carlo.\n");

    SetQMC(QMC_NPARAMS, 0);
    SetQMC(QMC_METHOD, MONTECARLO_METHOD);
    // SetQMC (QMC_DIMEN, RANDQMC_MAX * max_depth);
    SetQMC(QMC_DIMEN, dimen);
    SetQMC(QMC_NPOINTS, *npoints);
    SetQMC(QMC_RMETHOD, NOSHIFT);
    SetQMC(QMC_RANDS, num_samples);
    SetQMC(QMC_RBASE, 2);
  }

  return (1);
}

/* ------------------------------------------------------------------------- */
