/* quasiMC.h - interface for the environmental program quasiMC */

#ifndef _QUASIMC_H
#define _QUASIMC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

#define NEW_SKY_MODEL 0

// the maximum number of wavelenths to simulate
// NOTE: must be defined before #include scene3d.h and ray.h
// this number must match the communication library (see EA20 in lpfg)
#define MAX_SPECTRUM_SAMPLES 20

#include "comm_lib.h"
#include "matrix.h"
#include "surface.h"
#include "ray.h" // must be before query.h
#include "query.h"
#include "scene3d.h"
#include "sky.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define MAX_VERTICES 64

// number of samples to take from sky model
#define AZIMUTH_SAMPLES 32  // 64
#define ALTITUDE_SAMPLES 32 // 32

// define the type of information that QuasiMC will return to the plant
// simulator
#define ABSORBED_FLUX 1
#define ABSORBED_IRRADIANCE 2
#define UP_INCIDENT_IRRADIANCE 3
#define LW_INCIDENT_IRRADIANCE 4
#define NUM_INTERSECTIONS 5

// define the BRDFs that are available to the user
#define BLINN_PHONG 1
#define PHONG 2
#define LAMBERTIAN 3

#define EPSILON 0.00001 /* 1x10^(-5) */

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
#ifndef M_PIf
#define M_PIf 3.1415926535897932f
#endif

typedef struct tagSOURCESPECTRUM {
  float wavelength;
  float weight;
} SOURCESPECTRUM;

typedef struct tagVISUALIZATION {
  bool show_light;
  bool show_box;
  bool show_wireframe;
  float background[3];
  float colour_mod[3];
} VISUALIZATION;

#ifdef __cplusplus
extern "C" {
#endif
// implemented in args.c
int ProcessArguments(int argc, char **argv);
int StrCaseCmp(char *str1, char *str2);

// implemented in quasimc.c
void StoreQuery(int master, unsigned long module_id, int polygon_num,
                Cmodule_type *two_modules, CTURTLE *turtle);
void DetermineResponse(void);

// implemented in gldisplay.cpp
void glDisplayRay(RAY ray);

#ifdef __cplusplus
}
#endif
#endif
