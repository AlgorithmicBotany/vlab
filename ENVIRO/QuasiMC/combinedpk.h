/*
  This is the header file for the polynomial korobov rule. It defines all the
  functions which are used by the korobov polynomial method.

  Written by: Kris Luttmer
  Date: August 9, 2001

*/

#ifndef _P_KOROBOV_H_
#define _P_KOROBOV_H_

int InitCpKorobov(int s, double points, int num_gens, double *gen_params);
double *CpKorobov(void);
void ResetCpKorobov(void);
void FreeCpKorobov(void);
double NextDimCpKorobov(int start_dim);
#endif
