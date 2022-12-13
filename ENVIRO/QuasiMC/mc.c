#include "qmc.h"
#include "mc.h"

#ifdef MRG
#include "MRGrand.h"
#else
#include "rand.h"
#endif

// the random number generator's successive storage array
#ifdef MRG
static RngStream mc_generator = NULL; // the random number generator
// static unsigned long mc_state[6];     // the state of the generator
#else
static unsigned short mc_xsubi[3] = {0x123E, 0x456E, 0x789E};
#endif

static double *point;
static UINT dim = 0;

int InitMC(UINT param_dim) {
  dim = param_dim;
  point = (double *)malloc(sizeof(double) * dim);

#ifdef MRG
  Rand_CreateStream(&mc_generator, "mc_generator");
#endif

  if (point == NULL)
    return 0;

  memset(point, 0, sizeof(double) * dim);

  return 1;
}

void FreeMC() {
#ifdef MRG
  Rand_DeleteStream(&mc_generator);
#endif
  if (point)
    free(point);
}

double *MC() {
  unsigned int i;
  if (point == NULL) {
    fprintf(stderr, "Point was not intialized, Please try again.\n");
    return (NULL);
  }

#ifdef MRG
  for (i = 0; i < dim; i++)
    *(point + i) = Rand_RandU01(mc_generator);
#else
  for (i = 0; i < dim; i++)
    *(point + i) = erand48(mc_xsubi);
#endif

  return point;
}

void ResetMC() { return; }

// ------------------------------------------------------------------

double NextDimMC()
// returns the value for the nextdim
{
#ifdef MRG
  return (Rand_RandU01(mc_generator));
#else
  return (erand48(mc_xsubi));
#endif
}

// ------------------------------------------------------------------
