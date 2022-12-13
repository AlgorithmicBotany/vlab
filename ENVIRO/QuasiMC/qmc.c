// FILE: qmc.c

/* Implementation of the QMC Library.
   See the Header file qmc.h for a full description of the functions.
   Only one function is implemented, InitQMC, because the rest of the
   functions are just function pointers.  InitQMC assigns those
   functions the appropriate QMC method funtions.
   The parameters for InitQMC is an unsigned integer array of size
   MAXARGS.  There are defines in qmc.h that describe the contents
   of each index. */

// Kris Luttmer and Mik Cieslak | March 11, 2001
// Updated February 25th,  2002

#include "qmc.h"
#include "mc.h"
#include "korobov.h"
#include "sobol.h"
#include "combinedpk.h"
#include "randqmc.h"

static double *args = NULL;

// -----------------------------------------------------------------------
int InitQMC(void)
/* Purpose: to prepare the correct QMC method for generating points.
       Pre: parameters[MAXARGS] must contain correct values as described
            by the defines for it in qmc.h.
      Post: the choosen method is initialized with the correct parameters.
            the three function pointers are set to the correct methods.
            Upon success 1 is returned else 0. */
{
  int retvalue = 0; // the return value

  switch ((int)args[QMC_METHOD]) {
  case MONTECARLO_METHOD:
    retvalue = InitMC((unsigned int)args[QMC_DIMEN]);
    QMC = MC;
    ResetQMC = ResetMC;
    FreeMethod = FreeMC;
    break;

  case KOROBOV_METHOD:
    retvalue =
        InitKorobov((int)args[QMC_DIMEN], (int)args[QMC_NPOINTS],
                    (int)args[QMC_NPARAMS + 1], (int)args[QMC_NPARAMS + 2]);
    QMC = Korobov;
    ResetQMC = ResetKorobov;
    FreeMethod = FreeKorobov;
    break;

  case PKOROBOV_METHOD:
    retvalue =
        InitCpKorobov((int)args[QMC_DIMEN], args[QMC_NPOINTS],
                      (int)args[QMC_NPARAMS + 1], &args[QMC_NPARAMS + 2]);
    QMC = CpKorobov;
    ResetQMC = ResetCpKorobov;
    FreeMethod = FreeCpKorobov;
    break;

  case SOBOL_METHOD:
    retvalue = InitSobol((int)args[QMC_DIMEN], (int)args[QMC_NPOINTS]);
    QMC = Sobol;
    ResetQMC = ResetSobol;
    FreeMethod = FreeSobol;
    break;
  }

  if (retvalue == 0) {
    fprintf(stderr, "Cannot Initialize Quasi-Monte Carlo Method\n");
    return (0);
  }

  retvalue = InitRandom(args);

  if (retvalue == 0) {
    fprintf(stderr, "Cannot Initialize Random Method\n");
    FreeMethod();
  }

  return (retvalue);
}

// -----------------------------------------------------------------------
void FreeQMC() {
  if (FreeMethod != NULL) {
    FreeMethod();
    FreeMethod = NULL;
  }

  FreeRandom();

  if (args != NULL) {
    free(args);
    args = NULL;
  }
}

// -----------------------------------------------------------------------

double GetQMC(int request) {
  if (args == NULL) {
    fprintf(stderr,
            "Cannot get QMC parameter without loading input file first\n");
    return 0;
  }

  if (request < QMC_STATIC_PARAMS + args[QMC_NPARAMS])
    return args[request];

  fprintf(stderr, "QMC Parameter requested is out of range\n");
  return 0;
}

int SetQMC(int request, double value) {
  if (args == NULL) {
    if (request != QMC_NPARAMS) {
      fprintf(stderr, "Must set the number of extra parameters first\n");
      return 0;
    }

    args = malloc(sizeof(double) * ((int)value + QMC_STATIC_PARAMS));
    args[QMC_NPARAMS] = value;
    return 1;
  }

  if (request < QMC_STATIC_PARAMS + args[QMC_NPARAMS]) {
    args[request] = value;
    return 1;
  }

  fprintf(stderr, "QMC Parameter requested is out of range\n");
  return 0;
}
