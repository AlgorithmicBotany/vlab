/* randquasimc.h - interface for generating RQMC points */

#ifndef _RANDQUASIMC_H
#define _RANDQUASIMC_H

#define RANDQMC_START                                                          \
  0 // first two, three or four dimensions (one light source, several light
    // sources, sky model)
#define RANDQMC_THETA                                                          \
  1 // next max_depth dimensions for generating theta direction of
    // reflected/transmitted ray
#define RANDQMC_PHI                                                            \
  2 // next max_depth dimensions for generating phi direction of
    // reflected/transmitted ray
#define RANDQMC_RR 3 // next max_depth dimensions for russian roulette
#define RANDQMC_RT                                                             \
  4 // next max_depth dimensions for choosing reflected/transmitted ray
#define RANDQMC_N                                                              \
  5 // next max_depth dimensions for choosing scattering exponent
#define RANDQMC_MAX 6 /* number of blocks in RQMC point */

#ifdef __cplusplus
extern "C" {
#endif
void ResetU01(void);
float RandU01(int index);
int SetRQMC(char *RQMC_method, unsigned int max_depth, int num_samples,
            double *npoints);
#ifdef __cplusplus
}
#endif
#endif
