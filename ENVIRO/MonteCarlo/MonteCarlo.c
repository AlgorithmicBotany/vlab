/*
  Environmental process - Monte Carlo radiosity
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "MonteCarlo.h"
#include "comm_lib.h"
#include "scene3d.h"
#include "matrix.h"
#include "sky.h"

/**** field specific variables ****/
#define QUERY_ARRAY_SIZE 1000

struct item_type {
  unsigned long dist;
  int master;
  int num_rays[MAX_SPECTRUM_SAMPLES];
  double mean[MAX_SPECTRUM_SAMPLES];
  double var[MAX_SPECTRUM_SAMPLES];
  double ratio_mean;
  double ratio_var;
  int num_params;
  float pos[3], up[3], edge1[3], edge2[3];
  float exponent;
  float area;
  primitive_type *primitive; /* corresponding primitive  */
};
typedef struct item_type item_type;

item_type *queries;
int num_queries;
int query_array_size;

grid_type grid;

char verbose, periodic_canopy, remove_objects;
int max_depth;
float ray_density; /* number of rays per unit area */

#define NUM_MATERIALS 256

material_type materials[NUM_MATERIALS];
/* each material has top and bottom */
int num_materials;
int spectrum_samples;
int material_parameter; /* which parameter of the symbol specifies the material
                         */

char proc_name[] = "MonteCarlo";

unsigned long ray_signature;

#define BLINN_PHONG 1
#define PHONG 2
#define PARCINOPY 3

char reflectance_model;

struct russian_roulette_type {
  float threshold;
  float p;
} russian_roulette;

struct source_spectrum_type {
  float wavelength;
  float weight;
} source_spectrum[MAX_SPECTRUM_SAMPLES];

struct ray_spectrum_type {
  float intensity;
  float prob;
};

typedef struct ray_spectrum_type ray_spectrum_type;

/****** light sources *******/
#define MAX_LIGHT_SOURCES 50

struct light_source_type {
  float direction[3];
  float xvec[3], yvec[3];
  float weight;
} light_sources[MAX_LIGHT_SOURCES];

int num_light_sources;

char use_sky_file;
char rays_from_objects;
char one_ray_per_spectrum;
float from_object_exp;
char stratified_sampling;
unsigned long all_hits;
int num_of_runs;
char no_direct;
float version;

/****************************************************************************/
void FreeFieldStructures(void) {
  OBJECT_LIST_TYPE *ptr, *next;
  int i;

  if (grid.data != NULL) {
    for (i = grid.size[X] * grid.size[Y] * grid.size[Z] - 1; i >= 0; i--) {
      ptr = grid.data[i].list;
      while (ptr != NULL) {
        next = ptr->next;
        free(ptr);
        ptr = next;
      }
    }
    free(grid.data);
  }
  grid.data = NULL;

  if (queries != NULL)
    free(queries);
  queries = NULL;
}

/****************************************************************************/
void InitializeFieldStructures(void) {

  FreeFieldStructures();
  num_queries = 0;
  query_array_size = QUERY_ARRAY_SIZE;
  if ((queries = (item_type *)malloc(query_array_size * sizeof(item_type))) ==
      NULL) {
    fprintf(stderr, "%s - cannot allocate memory for querries.\n", proc_name);
    exit(0);
  }

  if ((grid.data = (CELL_TYPE *)malloc(grid.size[X] * grid.size[Y] *
                                       grid.size[Z] * sizeof(CELL_TYPE))) ==
      NULL) {
    fprintf(stderr, "%s - cannot allocate memory for the grid!\n", proc_name);
    exit(0);
  }
}

/****************************************************************************/
CELL_TYPE *GetCell(grid_type *grid, int x, int y, int z) {
  if (x < 0)
    x = 0;
  if (x >= grid->size[X])
    x = grid->size[X] - 1;

  if (y < 0)
    y = 0;
  if (y >= grid->size[Y])
    y = grid->size[Y] - 1;

  if (z < 0)
    z = 0;
  if (z >= grid->size[Z])
    z = grid->size[Z] - 1;

  return grid->data + z * grid->size[Y] * grid->size[X] + y * grid->size[X] + x;
}

/****************************************************************************/
void InitializeLightSources(void) {
  int i, c;
  float weights;
  float up[3] = {0, 1, 0};

  if (num_light_sources == 0)
    return;

  weights = 0;

  for (i = 0; i < num_light_sources; i++) {
    weights += light_sources[i].weight;

    Normalize(light_sources[i].direction);

    for (c = 0; c < 3; c++)
      light_sources[i].direction[c] *= -1;

    /* create the two perpendicular vectors (xvec, yvec) */
    if (fabs(light_sources[i].direction[Y]) > 0.999) {
      light_sources[i].xvec[X] = 0;
      light_sources[i].xvec[Y] = 0;
      light_sources[i].xvec[Z] = 1;
    } else {
      CrossProduct(light_sources[i].direction, up, light_sources[i].xvec);
      Normalize(light_sources[i].xvec);
    }

    CrossProduct(light_sources[i].direction, light_sources[i].xvec,
                 light_sources[i].yvec);
  }

  for (i = 0; i < num_light_sources; i++)
    light_sources[i].weight /= weights;
}

/****************************************************************************/
void CheckQueryArraySize(int num) {

  if (num_queries + num > query_array_size) {
    /* reallocate */
    do
      query_array_size *= 2;
    while (num_queries + num > query_array_size);

    if ((queries = (item_type *)realloc(
             queries, query_array_size * sizeof(item_type))) == NULL) {
      fprintf(stderr, "%s - cannot reallocate memory for querries.\n",
              proc_name);
      exit(0);
    }
    if (verbose)
      fprintf(stderr, "%s - queries reallocated. New size is %d.\n", proc_name,
              query_array_size);
  }
}

/****************************************************************************/
void StoreInput(grid_type *grid, int master, unsigned long dist,
                Cmodule_type *two_modules, CTURTLE *tu) {
#define MAXVERTICES 20
  primitive_type *ptr;
  int top_mat, bot_mat;
  int str_master, vertices, v;
  char line[2048], *token;
  float vertex[MAXVERTICES * 3];
  char in, input;
  int c, spectrum;

  /* first check whether triangles are coming from cpfg */
  if (CSGetString(&str_master, line, sizeof(line))) {
    /* triangles are coming from cpfg */

    if (verbose)
      fprintf(stderr, "%s - expecting triangles.\n", proc_name);

    in = 0;
    input = 0;

    do {
      if (!strncmp(line, "polygon", sizeof("polygon"))) {

        vertices = 0;

        while ((input = CSGetString(&str_master, line, sizeof(line))!=0)) {
          if (isalpha(line[0]))
            break;

          if ((token = strtok(line, " \t")) == NULL)
            break;
          vertex[vertices * 3 + 0] = atof(token);

          if ((token = strtok(NULL, " \t")) == NULL)
            break;
          vertex[vertices * 3 + 1] = atof(token);

          if ((token = strtok(NULL, " \t")) == NULL)
            break;
          vertex[vertices * 3 + 2] = atof(token);

          if (++vertices == MAXVERTICES) {
            fprintf(stderr,
                    "%s - WARNING! Maximum number (20) of triangles "
                    "per module reached!\n",
                    proc_name);
            break;
          }
        }
        if (vertices >= 3) {
          in = 1; /* at least one triangle */

          /* there will be vertices-2 triangles */
          CheckQueryArraySize(vertices - 2);

          for (v = 0; v <= vertices - 3; v++) {

            queries[num_queries].primitive = NULL;
            queries[num_queries].dist = dist;
            queries[num_queries].master = master;
            queries[num_queries].num_params = two_modules[0].num_params;
            for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
              if (spectrum < two_modules[0].num_params)
                queries[num_queries].num_rays[spectrum] =
                    two_modules[0].params[spectrum].value;
              else
                queries[num_queries].num_rays[spectrum] = 0;

            if (v > 0)
              /* copy the vertex[0] to vertex[v] */
              for (c = 0; c < 3; c++)
                vertex[v * 3 + c] = vertex[c];

            if ((ptr = AddTriangle(grid, vertex + v * 3, &two_modules[1],
                                   material_parameter, &top_mat, &bot_mat)) !=
                NULL) {
              if (top_mat < 0 || top_mat >= num_materials)
                ptr->material[0] = &materials[0]; /* default top material */
              else
                ptr->material[0] = &materials[top_mat];

              if (bot_mat < 0 || bot_mat >= num_materials)
                ptr->material[1] = ptr->material[0]; /* the same as top
                                                        material */
              else
                ptr->material[1] = &materials[bot_mat];

              if (two_modules[0].num_params > 0)
                queries[num_queries].primitive = ptr;

              num_queries++;
            } else
              fprintf(stderr,
                      "%s - didn't manage to add a triangle to "
                      "the grid.\n",
                      proc_name);

            /* triangle added */
          }
          /* all triangles stored */
        }
      } else
        /* else skip the line */
        input = CSGetString(&str_master, line, sizeof(line));
    } while (input);

    if (in)
      return;
    else if (verbose)
      fprintf(stderr,
              "%s - no triangles read. Trying to process the module"
              " behind ?E.\n",
              proc_name);
  }

  if (tu->positionC < 3) {
    fprintf(stderr, "%s - turtle position wasn't sent to the environment.\n",
            proc_name);
    return;
  }

  if (tu->headingC < 3) {
    fprintf(stderr, "%s - turtle heading wasn't sent to the environment.\n",
            proc_name);
    return;
  }

  if (tu->upC < 3) {
    fprintf(stderr, "%s - turtle up vector wasn't sent to the environment.\n",
            proc_name);
    return;
  }

  if (two_modules[0].num_params > 0) {
    /* store the query */
    CheckQueryArraySize(1);

    queries[num_queries].primitive = NULL;
    queries[num_queries].dist = dist;
    queries[num_queries].master = master;
    queries[num_queries].num_params = two_modules[0].num_params;
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      if (spectrum < two_modules[0].num_params)
        queries[num_queries].num_rays[spectrum] =
            two_modules[0].params[spectrum].value;
      else
        queries[num_queries].num_rays[spectrum] = 0;

    if (rays_from_objects) {
      /* store also two vector used for generating rays (plus up vector) */
      if (tu->leftC < 3) {
        fprintf(stderr,
                "%s - turtle left vector wasn't sent to the environment.\n",
                proc_name);
        return;
      }

      if (two_modules[1].symbol[0] != 'P') {
        fprintf(stderr,
                "%s - only 'P' can follow a query in rays-from-object mode.\n",
                proc_name);
        return;
      }

      queries[num_queries].area =
          two_modules[1].params[0].value * two_modules[1].params[1].value;

      for (c = X; c <= Z; c++) {
        queries[num_queries].up[c] = tu->up[c];
        queries[num_queries].pos[c] = tu->position[c];
        queries[num_queries].edge1[c] =
            two_modules[1].params[0].value * tu->heading[c] +
            two_modules[1].params[1].value * 0.5 * tu->left[c];
        queries[num_queries].edge2[c] =
            two_modules[1].params[0].value * tu->heading[c] -
            two_modules[1].params[1].value * 0.5 * tu->left[c];
      }

      if (two_modules[1].num_params == 3)
        queries[num_queries].exponent = two_modules[1].params[2].value;
      else
        queries[num_queries].exponent = from_object_exp;
    }

    num_queries++;

    if (rays_from_objects)
      /* query polygons are not stored */
      return;
  }

  /* if the second module is L, set the light parameters and exit */
  if (two_modules[1].symbol[0] == 'L') {
    fprintf(stderr, "Changing light spectrum to: ");

    for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
      if (spectrum >= two_modules[1].num_params)
        /* setting only as many values as there are parameters */
        break;
      source_spectrum[spectrum].weight = two_modules[1].params[spectrum].value;
      fprintf(stderr, "%g, ", source_spectrum[spectrum].weight);
    }
    fprintf(stderr, "\n");

    return;
  }

  if ((ptr = AddObject(grid, tu, &two_modules[1], &top_mat, &bot_mat)) !=
      NULL) {
    if (top_mat < 0 || top_mat >= num_materials)
      ptr->material[0] = &materials[0]; /* default top material */
    else
      ptr->material[0] = &materials[top_mat];

    if (bot_mat < 0 || bot_mat >= num_materials)
      ptr->material[1] = ptr->material[0]; /* the same as top material */
    else
      ptr->material[1] = &materials[bot_mat];

    if (two_modules[0].num_params > 0)
      queries[num_queries - 1].primitive = ptr;
  } else
    num_queries--;
}

/****************************************************************************/
int cell_step[3];
float node_size[3];

void SetGlobalParameters(void) {
  int c;

  cell_step[X] = 1;
  cell_step[Y] = grid.size[X];
  cell_step[Z] = grid.size[X] * grid.size[Y];

  for (c = X; c <= Z; c++) {
    /* half size of a node */
    node_size[c] = 0.5 * grid.range[c] / (float)grid.size[c];
  }

  for (c = X; c <= Z; c++) {
    grid.bbox[c] = grid.pos[c] + grid.range[c];
    grid.bbox[3 + c] = grid.pos[c];

    grid.bsph_C[c] = 0;
  }
  grid.bsph_r = -1;

  grid.maxdist = 2 * (grid.range[X] + grid.range[Y] + grid.range[Z]);

  for (c = X; c <= Z; c++)
    grid.grid_to_real[c] = (double)grid.range[c] / (double)grid.size[c];
}

/****************************************************************************/
/* This routine is modified ggDielectricReflectance given by
   Gladimir Baranoski */
float Reflectance(float cosAi, float cosAt, float Nt) {
  float nn, cc, Nt2, ci2, ct2;
  float num, denom, Nit, cit;

  nn = Nt;
  cc = cosAt * cosAi;
  Nt2 = Nt * Nt;
  ci2 = cosAi * cosAi;
  ct2 = cosAt * cosAt;
  Nit = 1.0 - Nt2;
  cit = ci2 - ct2;
  num = Nit * Nit * cc * cc + cit * cit * nn * nn;
  denom = cc * (1.0 + Nt2) + nn * (ci2 + ct2);
  denom = denom * denom;
  if ((denom < 0.000001) && (num < 0.000001))
    return 1.0;
  else
    return num / denom;
}

/****************************************************************************/
/* returns angle beta used */
float GenerateRandomDirection(float *ray, float n, float *dir) {
  float alpha, beta, len, sinb;
  float vec[3], x[3], y[3];
  int c;

  if (n < 0) {
    for (c = X; c <= Z; c++)
      dir[c] = ray[c];
    return 0.0;
  }

  /* Blinn-Phong Model Based - Gladimir's notes + Shirley and Wang*/
  alpha = 2.0 * M_PI * drand48();

  switch (reflectance_model) {
  case BLINN_PHONG:
    beta = 2.0 * acos(pow(1.0 - drand48(), 1.0 / (n + 2)));
    break;

  case PHONG:
    beta = acos(pow(1.0 - drand48(), 1.0 / (n + 1)));
    break;

  case PARCINOPY:
    beta = 0.5 * acos(1.0 - 2.0 * drand48());
    /* although in Parcinopy code the power is omitted */
    break;
  }

  /* get two unit vectors perpendicular to ray[] */
  /* find a coordinate not close to 0 */
  for (c = X; c <= Z; c++)
    if (fabs(ray[c]) >= 0.5) {
      x[c] = -ray[(c + 1) % 3];
      x[(c + 1) % 3] = ray[c];
      x[(c + 2) % 3] = 0.0;
      break;
    }

  if (c > Z) {
    fprintf(stderr, "radiance - internal error 1e.\n");
    exit(0);
  }

  len = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
  for (c = X; c <= Z; c++)
    x[c] /= len;

  CrossProduct(ray, x, y);

  vec[0] = cos((double)alpha) * (sinb = sin((double)beta));
  vec[2] = sin((double)alpha) * sinb;
  vec[1] = cos((double)beta);

  if (verbose == 2)
    fprintf(stderr, "%s - alpha=%g, beta=%g, vec=(%g, %g, %g).\n", proc_name,
            alpha / M_PI * 180, beta / M_PI * 180, vec[0], vec[1], vec[2]);

  for (c = X; c <= Z; c++)
    dir[c] = vec[0] * x[c] + vec[1] * ray[c] + vec[2] * y[c];

  return beta;
}

/****************************************************************************/
/* resolving intersection for a ray carrying all wavelengths */
/* returns 0, if no new ray is shot */
int ResolveIntersectionOne(float *pt, float *dir,
                           ray_spectrum_type *ray_spectrum, int depth,
                           float mindist, float *norm,
                           primitive_type *intersected) {
  int c, side;
  int spectrum, chosen;
  float d[MAX_SPECTRUM_SAMPLES], R[MAX_SPECTRUM_SAMPLES],
      T[MAX_SPECTRUM_SAMPLES];
  float dr, n, n2, sumR, sumT, rnd, theta, intensity;
  float cosAi, ray[3];

  cosAi = -norm[0] * dir[0] - norm[1] * dir[1] - norm[2] * dir[2];

  side = cosAi > 0 ? 0 : 1;

  all_hits++;
  if (verbose == 2) {
    fprintf(stderr, "%s - hit at depth %d\n", proc_name, depth);
    fprintf(stderr, "%s - side: %d\n", proc_name, side);
  }

  for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
    R[spectrum] = intersected->material[side]->reflectance[spectrum];
    T[spectrum] = intersected->material[side]->transmittance[spectrum];

    d[spectrum] = R[spectrum] + T[spectrum];
  }

  if (russian_roulette.threshold > 0) {
    intensity = 0;
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      intensity += ray_spectrum[spectrum].intensity;

    /* Russian roulette */
    if (intensity <= russian_roulette.threshold * (float)spectrum_samples){
      if (drand48() < russian_roulette.p) {
        /* ray is all absorbed */
        for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
          d[spectrum] = 0;
      } else {
        /* ray continues */
        /* but with bigger probability weight */
        for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
          ray_spectrum[spectrum].intensity /= 1.0 - russian_roulette.p;
      }
    }
  }

  /* absorb pertinent part of the intensity */
  if (!no_direct || depth > 1)
    /* do not absorb if no_direct && depth==1 */
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
      intersected->intensity[spectrum] +=
          ray_spectrum[spectrum].intensity * (1 - d[spectrum]);
    }

  if (max_depth >= 0)
    if (depth >= max_depth)
      return 0;

  /* choose a between reflected or transmitted ray based on the current
     probability per wavelentg.
     consider only wavelengths with nonzero d[spectrum] */

  /* sum the probabilities multiplied by reflectance */
  /* sum the probabilities multiplied by transmittance */
  sumR = sumT = 0;
  for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
    if (d[spectrum] > 0) {
      sumR += ray_spectrum[spectrum].intensity * R[spectrum];
      sumT += ray_spectrum[spectrum].intensity * T[spectrum];
    }

  if (sumT + sumR <= 0)
    /* no reflected or transmitted ray */
    return 0;

  for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
    /* adjust the intensities after the absorption */
    ray_spectrum[spectrum].intensity *= d[spectrum];

  /* get the new ray origin */
  for (c = X; c <= Z; c++)
    pt[c] += mindist * dir[c];

  /* switch normal if it points the same way as dir[] */
  if (cosAi < 0) {
    cosAi = -cosAi;
    for (c = X; c <= Z; c++)
      norm[c] = -norm[c];
  }

  if ((sumR + sumT) * drand48() < sumR) {
    /* continue reflected ray */
    for (c = X; c <= Z; c++)
      ray[c] = dir[c] + 2 * norm[c] * cosAi;

    /* choose n based on weighted reflected intensities */
    rnd = sumR * drand48();
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      if (d[spectrum] > 0)
        if ((rnd -= ray_spectrum[spectrum].intensity * R[spectrum]) <= 0)
          break;

    chosen = spectrum;
    n = intersected->material[side]->spec_power[chosen];

    for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
      ray_spectrum[spectrum].prob *= R[spectrum];
      if (d[spectrum] > 0)
        ray_spectrum[spectrum].intensity *=
            (R[spectrum] / d[spectrum]) / (sumR / (sumR + sumT));
    }

    dr = 1;
  } else {
    if (verbose == 2)
      fprintf(stderr, "%s - transmitted ray.\n", proc_name);

    /* continue transmitted ray */
    for (c = X; c <= Z; c++)
      ray[c] = dir[c];

    /* choose n based on weighted reflected intensities */
    rnd = sumT * drand48();
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      if (d[spectrum] > 0)
        if ((rnd -= ray_spectrum[spectrum].intensity * T[spectrum]) <= 0)
          break;

    chosen = spectrum;
    n = intersected->material[side]->trans_power[chosen];

    for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
      ray_spectrum[spectrum].prob *= T[spectrum];
      if (d[spectrum] > 0)
        ray_spectrum[spectrum].intensity *=
            (T[spectrum] / d[spectrum]) / (sumT / (sumR + sumT));
    }
    dr = -1;
  }

  if (n >= 0) {
    /* generate a new random direction */
    if (reflectance_model == PARCINOPY) {
      theta = GenerateRandomDirection(norm, n, dir);
      dir[0] *= dr;
      dir[1] *= dr;
      dir[2] *= dr;
    } else{
      do {
        theta = GenerateRandomDirection(ray, n, dir);
      } while (dr * (dir[0] * norm[0] + dir[1] * norm[1] + dir[2] * norm[2]) <=
               0);
    }
  } else{
    theta = 0;
    for (c = X; c <= Z; c++){
      dir[c] = ray[c];
    }
  }

  theta = cos(theta);

  /* adjust intensity due to the difference in exponent */
  for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
    if (dr == 1)
      /* reflected ray */
      n2 = intersected->material[side]->spec_power[spectrum];
    else
      n2 = intersected->material[side]->trans_power[spectrum];

    if (n != n2)
      ray_spectrum[spectrum].intensity *=
          (n2 + 2) * pow(theta, n2 - n) / (n + 2);
  }

  if (verbose == 2)
    fprintf(stderr,
            "%s - spawning a new ray "
            "(%g, %g, %g) (%g, %g, %g).\n",
            proc_name, pt[X], pt[Y], pt[Z], dir[X], dir[Y], dir[Z]);

  return 1;
}

/****************************************************************************/
/* returns 0, if no new ray is shot */
int ResolveIntersection(float *pt, float *dir, int spectrum,
                        ray_spectrum_type *ray_spectrum, int depth,
                        float mindist, float *norm,
                        primitive_type *intersected) {
  int c, side;
  float d, R, rnd, n;
  float cosAi, ray[3];

  cosAi = -norm[0] * dir[0] - norm[1] * dir[1] - norm[2] * dir[2];

  side = cosAi > 0 ? 0 : 1;

  all_hits++;
  if (verbose == 2) {
    fprintf(stderr, "%s - hit at depth %d\n", proc_name, depth);
    fprintf(stderr, "%s - side: %d\n", proc_name, side);
  }

#ifdef GLADIMIR
  /* Fresnel's equation */
  sinAt = sqrt(1 - cosAi * cosAi) / intersected->material[side]->Nt[spectrum];

  cosAt = sqrt(1 - sinAt * sinAt);

  R = Reflectance(cosAi, cosAt, intersected->material[side]->Nt[spectrum]);

  if (verbose == 2)
    fprintf(stderr, "%s - reflectance computed: %g.\n", proc_name, R);
#else
  R = intersected->material[side]->reflectance[spectrum];
#endif

  d = R + intersected->material[side]->transmittance[spectrum];

  rnd = drand48();

  if (russian_roulette.threshold > 0) {
    /* Russian roulette */
    if (ray_spectrum->intensity /* * ray_spectrum->prob */
        <= russian_roulette.threshold){
      if (drand48() < russian_roulette.p) {
        /* ray is all absorbed */
        d = 0;
      } else {
        /* ray continues */
        /* but with bigger probability weight */
        /* ray_spectrum->prob /= 1.0 - russian_roulette.p; */
        ray_spectrum->intensity /= 1.0 - russian_roulette.p;
      }
    }
  }

  /* absorb pertinent part of the intensity */
  if (!no_direct || depth > 1)
    /* do not absorb if no_direct && depth==1 */
    intersected->intensity[spectrum] += ray_spectrum->intensity * (1 - d);

  if (max_depth >= 0)
    if (depth >= max_depth)
      return 0;

  if (d == 0)
    return 0;

  /* the rest will go to the reflected or transmitted ray as before*/
  ray_spectrum->intensity *= d;

  /* the random number must be scaled to interval <0,d) */
  rnd *= d;

  /* get the new ray origin */
  for (c = X; c <= Z; c++)
    pt[c] += mindist * dir[c];

  /* switch normal if it points the same way as dir[] */
  if (cosAi < 0) {
    cosAi = -cosAi;
    for (c = X; c <= Z; c++)
      norm[c] = -norm[c];
  }

  if (rnd < R) {
    /* continue reflected ray */
    for (c = X; c <= Z; c++)
      ray[c] = dir[c] + 2 * norm[c] * cosAi;

    n = intersected->material[side]->spec_power[spectrum];

    ray_spectrum->prob *= R;
    d = 1;
  } else {
    if (verbose == 2)
      fprintf(stderr, "%s - transmitted ray.\n", proc_name);

    /* continue transmitted ray */
    for (c = X; c <= Z; c++)
      ray[c] = dir[c];

    n = intersected->material[side]->trans_power[spectrum];

    ray_spectrum->prob *= intersected->material[side]->transmittance[spectrum];
    d = -1;
  }

  if (n >= 0) {
    /* generate a new random direction */
    if (reflectance_model == PARCINOPY) {
      GenerateRandomDirection(norm, n, dir);
      dir[0] *= d;
      dir[1] *= d;
      dir[2] *= d;
    } else
      do {
        GenerateRandomDirection(ray, n, dir);
      } while (d * (dir[0] * norm[0] + dir[1] * norm[1] + dir[2] * norm[2]) <=
               0);
  } else
    for (c = X; c <= Z; c++)
      dir[c] = ray[c];

  if (verbose == 2)
    fprintf(stderr,
            "%s - spawning a new ray "
            "(%g, %g, %g) (%g, %g, %g).\n",
            proc_name, pt[X], pt[Y], pt[Z], dir[X], dir[Y], dir[Z]);

  return 1;
}

/****************************************************************************/
/* assuming that dir is of unit length - necessary for IsIntersection() !!! */
/* returns 1 if the ray hit any object or
   the intensity reaching the object for rays-from-objects mode */
int TraceRay(float *pt, float *dir, int spectrum,
             ray_spectrum_type *ray_spectrum, grid_type *grid) {
  double smallest[3], smallest_step[3];
  float len, rnorm[3] = {0, 0, 0}, norm[3], sky_int;
  OBJECT_LIST_TYPE *ptr;
  int c, node[3], ind, cstep[3], spec;
  CELL_TYPE *cell;
  primitive_type *last_primitive = NULL; /* pointer to the primitive of the
                                            last intersection */
  float mindist;                         /* so far the closest intersection */
  primitive_type *intersected;
  int any_intersection = 0;
  int depth; /* depth of the current ray */

  for (c = X; c <= Z; c++) {
    /* initial voxel */
    node[c] = floor((pt[c] - grid->pos[c]) / grid->grid_to_real[c]);

    if ((node[c] < 0) || (node[c] >= grid->size[c]))
      return 0;
  }

  cell = grid->data + node[Z] * grid->size[Y] * grid->size[X] +
         node[Y] * grid->size[X] + node[X];

  depth = 1;

  for (;;) {

    mindist = grid->maxdist;
    if (periodic_canopy)
      /* 5 should be a parameter */
      mindist = 5 * grid->maxdist;

    /* increase the current ray signature */
    if (++ray_signature == 0) {
      ray_signature = 1;

      /* all signatures of obstacles and objects should be set to 0 */
    }

    for (c = X; c <= Z; c++) {
      /* adjusted according to the direction */
      if (dir[c] < 0) {
        cstep[c] = -cell_step[c];

        if (dir[c] < 0.0000001) {
          /* distance to the cube side[c] */
          smallest[c] =
              (grid->pos[c] + ((double)node[c] + 0.5) * grid->grid_to_real[c] -
               node_size[c] - pt[c]) /
              (double)dir[c];
          smallest_step[c] = -2 * (double)node_size[c] / (double)dir[c];
        } else {
          smallest[c] = mindist;
          smallest_step[c] = 0;
        }
      } else {
        cstep[c] = cell_step[c];

        if (dir[c] > 0.0000001) {
          /* distance to the cube side[c] */
          smallest[c] =
              (grid->pos[c] + ((double)node[c] + 0.5) * grid->grid_to_real[c] +
               node_size[c] - pt[c]) /
              (double)dir[c];
          smallest_step[c] = 2 * (double)node_size[c] / (double)dir[c];
        } else {
          smallest[c] = mindist;
          smallest_step[c] = 0;
        }
      }
    }

    intersected = NULL;

    for (;;) {
      /* go through the list associated with the node and check for
         intersection with each object */
      ptr = cell->list;

      while (ptr != NULL) {
        if (ptr->prim->ray_signature != ray_signature &&
            ptr->prim != last_primitive) {
          /* make sure that the direction is of unit lenght! */
          if ((len = IsIntersection(pt, dir, mindist, ptr->prim, rnorm)) >= 0) {
            ptr->prim->ray_signature = ray_signature;

            if (len < mindist) {
              mindist = len;
              intersected = ptr->prim;
              for (c = X; c <= Z; c++)
                norm[c] = rnorm[c];
            }
          }
        }
        ptr = ptr->next;
      }

      /* determine the next node intersected by the ray */

      /* find the intersected (closest) face */
      if (smallest[X] < smallest[Y])
        ind = smallest[X] < smallest[Z] ? X : Z;
      else
        ind = smallest[Y] < smallest[Z] ? Y : Z;

      /* compare the distance for the intersection with the voxel and */
      /* mindist */
      if (smallest[ind] + 0.00001 > mindist) {
        /* doesn't have to go further */

        if (intersected) { /* this condition should be always true */

          any_intersection = 1;
          last_primitive = intersected;

          if (one_ray_per_spectrum) {
            if (ResolveIntersectionOne(pt, dir, ray_spectrum, depth++, mindist,
                                       norm, intersected))
              break; /* break the second for(;;) to get into the first one */
          } else if (ResolveIntersection(pt, dir, spectrum, ray_spectrum,
                                         depth++, mindist, norm, intersected))
            break; /* break the second for(;;) to get into the first one */
        } else if (!periodic_canopy)
          fprintf(stderr, "%s - internal error 1.\n", proc_name);

        if (verbose == 2)
          fprintf(stderr, "%s - end of ray.\n", proc_name);

        if (rays_from_objects)
          /* if the ray finishes inside the grid, the contributing intensity
             is 0 */
          return 0;
        else
          return any_intersection;
      }

      if (dir[ind] > 0) {
        /* if the ray leaves the grid, go out of the loop */
        if (++node[ind] >= grid->size[ind]) {

          if (periodic_canopy && (ind == X || ind == Z)) {
            /* wrapping the ray */
            node[ind] = 0;
            pt[ind] -= grid->range[ind];

            cell = grid->data + node[Z] * grid->size[Y] * grid->size[X] +
                   node[Y] * grid->size[X] + node[X] -
                   cstep[ind]; /* because cstep is added later */
          } else {
          out:
            if (intersected) {
              fprintf(stderr, "%s - maybe an object sticks out of the grid!\n",
                      proc_name);
              fprintf(stderr,
                      "  depth=%d, ind=%d, node[ind]=%d, "
                      "smallest[0]=%f, mindist=%f\n",
                      depth, ind, node[ind], smallest[0], mindist);
              fprintf(stderr, "  pt=(%g,%g,%g)\n", pt[0], pt[1], pt[2]);
              fprintf(stderr, "  dir=(%g,%g,%g)\n", dir[0], dir[1], dir[2]);
            }

#ifdef FULL_VERBOSE
            if (verbose == 2)
              fprintf(stderr, "%s - end of ray.\n", proc_name);
#endif
            if (rays_from_objects) {
              /* determine the sky intensity in the given direction */
              sky_int = SkyIntensityInDir(dir);

              if (one_ray_per_spectrum)
                /* whole spectrum */
                for (spec = 0; spec < spectrum_samples; spec++)
                  ray_spectrum[spec].intensity *= sky_int;
              else
                ray_spectrum[0].intensity *= sky_int;
            }
            return any_intersection;
          }
        }
      } else if (--node[ind] < 0) {
        if (periodic_canopy && (ind == X || ind == Z)) {
          /* wrapping the ray */
          node[ind] = grid->size[ind] - 1;
          pt[ind] += grid->range[ind];

          cell = grid->data + node[Z] * grid->size[Y] * grid->size[X] +
                 node[Y] * grid->size[X] + node[X] -
                 cstep[ind]; /* because cstep is added later */
        } else
          goto out;
      }

      smallest[ind] += smallest_step[ind];

      cell += cstep[ind];
    }
  }
}

/****************************************************************************/
void ShootAllRaysOld(grid_type *grid) {
  unsigned long i, hits, num, rays;
  float pt[3], dir[3];
  float range[3], step[2];
  int size[2];
  int spectrum;
  ray_spectrum_type ray_spectrum[MAX_SPECTRUM_SAMPLES];

  range[X] = grid->bbox[3 + X] - grid->bbox[X];
  range[Z] = grid->bbox[3 + Z] - grid->bbox[Z];

  /* determine the number of rays */
  num = range[X] * range[Z] * ray_density;

  if (verbose)
    fprintf(stderr, "All primitives are in box: (%g,%g,%g) to (%g,%g,%g)\n",
            grid->bbox[0], grid->bbox[1], grid->bbox[2], grid->bbox[3],
            grid->bbox[4], grid->bbox[5]);

  if (verbose) {
    fprintf(stderr, "%s - shooting %lu rays.\n", proc_name, num);
  }

  hits = rays = 0;

  if (one_ray_per_spectrum) {
    /* each ray has associated whole spectrum */

    if (stratified_sampling) {
      double interm;

      interm = sqrt(range[X] * range[Z]);

      /* size of the grid for stratified sampling */
      size[X] = (int)((range[X] * sqrt(num)) / interm);
      size[Y] = (int)((range[Z] * sqrt(num)) / interm);

      step[X] = range[X] / (float)size[X];
      step[Y] = range[Z] / (float)size[Y];
    }

    /* for num rays */
    for (i = 0; i < num; i++) {
      pt[Y] = grid->pos[Y] + grid->range[Y] - 0.0001;

      if (stratified_sampling && (int)i < size[X] * size[Y]) {
        pt[X] = grid->bbox[X] + (i % size[X]) * step[X] + drand48() * step[X];
        pt[Z] = grid->bbox[Z] + (i / size[X]) * step[Y] + drand48() * step[Y];
      } else {
        /* uniform sampling */
        pt[X] = grid->bbox[X] + drand48() * range[X];
        pt[Z] = grid->bbox[Z] + drand48() * range[Z];
      }

      /* all rays go down */
      dir[Y] = -1;
      dir[X] = dir[Z] = 0;

      if (verbose == 2)
        fprintf(stderr,
                "%s - tracing ray %lu from (%g, %g, %g) in dir (%g, %g, %g)\n",
                proc_name, i, pt[X], pt[Y], pt[Z], dir[X], dir[Y], dir[Z]);

      for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
        ray_spectrum[spectrum].prob = 1.0;
        ray_spectrum[spectrum].intensity = source_spectrum[spectrum].weight;
      }

      hits += TraceRay(pt, dir, 0.0, ray_spectrum, grid);
      rays++;
    }
  } else
    /* for each wavelength */
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
      if (stratified_sampling) {
        double interm;

        interm = sqrt(range[X] * range[Z]);

        /* size of the grid for stratified sampling */
        size[X] =
            (int)((range[X] * sqrt(num * source_spectrum[spectrum].weight)) /
                  interm);
        size[Y] =
            (int)((range[Z] * sqrt(num * source_spectrum[spectrum].weight)) /
                  interm);

        step[X] = range[X] / (float)size[X];
        step[Y] = range[Z] / (float)size[Y];
      }

      for (i = 0; i < num * source_spectrum[spectrum].weight; i++) {
        pt[Y] = grid->pos[Y] + grid->range[Y] - 0.0001;

        if (stratified_sampling && (int)i < size[X] * size[Y]) {
          pt[X] = grid->bbox[X] + (i % size[X]) * step[X] + drand48() * step[X];
          pt[Z] = grid->bbox[Z] + (i / size[X]) * step[Y] + drand48() * step[Y];
        } else {
          /* uniform sampling */
          pt[X] = grid->bbox[X] + drand48() * range[X];
          pt[Z] = grid->bbox[Z] + drand48() * range[Z];
        }

        /* all rays go down */
        dir[Y] = -1;
        dir[X] = dir[Z] = 0;

        /* all rays have intensity 1.0 */

        if (verbose == 2)
          fprintf(stderr, "%s - shooting ray %lu from (%g, %g, %g)\n", proc_name,
                  i, pt[X], pt[Y], pt[Z]);

        ray_spectrum[0].intensity = 1.0;
        ray_spectrum[0].prob = 1.0;

        hits += TraceRay(pt, dir, spectrum, ray_spectrum, grid);
        rays++;
      }
    }

  if (verbose > 0) {
    fprintf(stderr, "%s - %.2f%% rays hit an object (%ld out of %ld rays).\n",
            proc_name, (float)hits / (float)num * 100.0, hits, rays);
  }
}

/****************************************************************************/
/* returns normalized ray direction and two normalized vectors perpendicular to
   it. */
void ChooseRayDirection(float *dir, float *xvec, float *yvec) {
  float rnd;
  int i, c;

  if (use_sky_file) {
    GetRandomSkyDirection(dir, xvec, yvec);
  } else {
    /* directional light sources */
    rnd = drand48();

    i = 0;
    while ((rnd -= light_sources[i].weight) >= 0)
      if (++i >= num_light_sources - 1)
        break;

    for (c = 0; c < 3; c++) {
      dir[c] = light_sources[i].direction[c];
      xvec[c] = light_sources[i].xvec[c];
      yvec[c] = light_sources[i].yvec[c];
    }
  }
}

/****************************************************************************/
/* finds intersection of the ray with the bounding box, returns 0 if not found
   pt is inside the bounding sphere */
int FindBoxIntersection(grid_type *grid, float *pt, float *dir) {
  float dist[3];
  int c, ind;

  /* find distance to the closest X,Y,Z face */
  for (c = X; c <= Z; c++) {
    if (dir[c] < -1e-10) {
      dist[c] =
          (grid->pos[c] + grid->range[c] - (pt[c] -= grid->maxdist * dir[c])) /
          dir[c];
    } else if (dir[c] > 1e-10)
      dist[c] = (grid->pos[c] - (pt[c] -= grid->maxdist * dir[c])) / dir[c];
    else
      /* dir[c] is basically zero */
      dist[c] = 0;
  }

  /* pick the biggest of the three distances */
  if (dist[X] > dist[Y])
    ind = dist[X] > dist[Z] ? X : Z;
  else
    ind = dist[Y] > dist[Z] ? Y : Z;

  if (dist[ind] < 0)
    /* this shouldn't happen */
    return 0;

  for (c = X; c <= Z; c++)
    pt[c] += dir[c] * dist[ind] * 1.000001;

  /* check if pt is on the box */
  for (c = X; c <= Z; c++)
    if (c != ind)
      if (pt[c] < grid->pos[c] || pt[c] > grid->pos[c] + grid->range[c])
        return 0;

  return 1;
}

/****************************************************************************/
void GeneratePointOnDisk(float rad, float *x, float *y) {
  double lx, ly;

  /* rejection sampling */
  do {
    lx = drand48() * 2 - 1;
    ly = drand48() * 2 - 1;
  } while (lx * lx + ly * ly >= 1);

  *x = rad * lx;
  *y = rad * ly;
}

/****************************************************************************/
void ShootAllRays(grid_type *grid) {
  unsigned long i, hits, rays;
  float pt[3], dir[3], xvec[3], yvec[3];
  float range[3];
  float rad, num, x, y;
  int spectrum, c;
  ray_spectrum_type ray_spectrum[MAX_SPECTRUM_SAMPLES];

  /* compare the bounding sphere computed incrementaly with a sphere
     enclosing the bounding box and pick up the smaller one */
  for (c = X; c <= Z; c++)
    xvec[c] = grid->bbox[3 + c] - grid->bbox[c];

  rad = sqrt(xvec[X] * xvec[X] + xvec[Y] * xvec[Y] + xvec[Z] * xvec[Z]) / 2.0;

  if (rad < grid->bsph_r) {
    if (verbose)
      fprintf(stderr,
              "%s - sphere around bounding box is smaller than "
              "the bounding sphere\n",
              proc_name);

    grid->bsph_r = rad;
    for (c = X; c <= Z; c++)
      grid->bsph_C[c] = 0.5 * (grid->bbox[3 + c] + grid->bbox[c]);
  } else if (verbose)
    fprintf(stderr,
            "%s - bounding sphere is smaller than "
            "the sphere around bounding box \n",
            proc_name);

  if (verbose)
    fprintf(stderr, "All primitives are in sphere at: (%g,%g,%g) rad %g\n",
            grid->bsph_C[0], grid->bsph_C[1], grid->bsph_C[2], grid->bsph_r);

  /* determine the number of rays */
  num = M_PI * grid->bsph_r * grid->bsph_r * ray_density;

  range[X] = grid->bbox[3 + X] - grid->bbox[X];
  range[Z] = grid->bbox[3 + Z] - grid->bbox[Z];

  rays = hits = 0;

  /* for each wavelength */
  for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
    /* for num*weight rays */
    for (i = (int)(num * source_spectrum[spectrum].weight); i > 0; i--) {
      ChooseRayDirection(dir, xvec, yvec);

      GeneratePointOnDisk(grid->bsph_r, &x, &y);

      for (c = X; c <= Z; c++)
        pt[c] = grid->bsph_C[c] + x * xvec[c] + y * yvec[c];

      if (FindBoxIntersection(grid, pt, dir)) {
        /* intersection found */

        if (verbose == 2)
          fprintf(stderr, "%s - tracing ray %lu from (%g, %g, %g)\n", proc_name,
                  i, pt[X], pt[Y], pt[Z]);

        ray_spectrum[0].intensity = 1.0;
        ray_spectrum[0].prob = 1.0;

        hits += TraceRay(pt, dir, spectrum, ray_spectrum, grid);
        rays++;
      }
    }

  if (verbose)
    fprintf(stderr, "%s - %.2f%% rays hit an object (%ld out of %ld rays).\n",
            proc_name, (float)hits / rays * 100.0, hits, rays);
}

/****************************************************************************/
/* rays originate not from the sky/light source(s) but from the objects stored
   as queries.
   */
void ShootRaysFromObjects(grid_type *grid, item_type *query,
                          Cmodule_type *comm_symbol) {
  unsigned long i, hits, rays;
  float pt[3], dir[3];
  int spectrum, c;
  ray_spectrum_type ray_spectrum[MAX_SPECTRUM_SAMPLES];
  double max, num;

  /* determine the number of rays */

  if (version == 1.0) {
    max = query->area * ray_density * M_PI;
    /* don't aks me why M_PI, but it was there */
    num = max;
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      query->num_rays[spectrum] = max;
  } else {
    max = 0;

    for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
      if (query->num_rays[spectrum] > max)
        max = query->num_rays[spectrum];
    }

    if (max < 1) {
      num = query->area * ray_density;
      for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
        query->num_rays[spectrum] = num;
    } else
      num = max;
  }

  if (verbose)
    fprintf(stderr, "%s - shooting %d rays.\n", proc_name, (int)num);

  hits = rays = 0;

  for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
    comm_symbol->params[spectrum].value = 0;

  if (one_ray_per_spectrum) {
    /* each ray has associated whole spectrum */

    /* for num rays */
    for (i = (int)(num); i > 0; i--) {
      GenerateRandomDirection(query->up, query->exponent, dir);

      for (c = X; c <= Z; c++)
        pt[c] = query->pos[c] + drand48() * query->edge1[c] +
                drand48() * query->edge2[c] + dir[c] * 0.00001;

      if (verbose == 2)
        fprintf(stderr,
                "%s - tracing ray %lu from (%g, %g, %g) in dir (%g, %g, %g)\n",
                proc_name, i, pt[X], pt[Y], pt[Z], dir[X], dir[Y], dir[Z]);

      for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
        ray_spectrum[spectrum].prob = 1.0;
        ray_spectrum[spectrum].intensity = source_spectrum[spectrum].weight *
                                           (double)query->num_rays[spectrum] /
                                           num;
      }

      hits += TraceRay(pt, dir, 0.0, ray_spectrum, grid);
      rays++;

      for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
        comm_symbol->params[spectrum].value += ray_spectrum[spectrum].intensity;
      }
    }
  } else
    /* for each wavelength */
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      /* for num*weight rays */
      for (i = (int)(query->num_rays[spectrum] *
                     source_spectrum[spectrum].weight);
           i > 0; i--) {
        GenerateRandomDirection(query->up, query->exponent, dir);

        for (c = X; c <= Z; c++)
          pt[c] = query->pos[c] + drand48() * query->edge1[c] +
                  drand48() * query->edge2[c] + dir[c] * 0.00001;

        if (verbose == 2)
          fprintf(stderr,
                  "%s - tracing ray %lu from (%g, %g, %g) in dir (%g, %g, %g)\n",
                  proc_name, i, pt[X], pt[Y], pt[Z], dir[X], dir[Y], dir[Z]);

        ray_spectrum[0].intensity = 1.0;
        ray_spectrum[0].prob = 1.0;

        hits += TraceRay(pt, dir, spectrum, ray_spectrum, grid);
        rays++;

        comm_symbol->params[spectrum].value += ray_spectrum[0].intensity;
      }

  for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
    if (max < 1)
      comm_symbol->params[spectrum].value /= ray_density;
    else if (query->num_rays[spectrum] >= 1)
      comm_symbol->params[spectrum].value /= query->num_rays[spectrum];

    comm_symbol->params[spectrum].set = 1;
  }

  if (verbose > 0)
    fprintf(stderr, "%s - %.2f%% rays hit an object (%ld out of %ld rays).\n",
            proc_name, (float)hits / rays * 100.0, hits, rays);
}

/****************************************************************************/
void DetermineResponse(grid_type *grid) {
  int q, q2, spectrum, run;
  Cmodule_type comm_symbol, comm_symbol2;
  float aux[MAX_SPECTRUM_SAMPLES];

  /* initializing stats */
  for (q = 0; q < num_queries; q++) {
    queries[q].ratio_mean = 0;
    queries[q].ratio_var = 0;
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
      queries[q].mean[spectrum] = 0;
      queries[q].var[spectrum] = 0;
    }
  }

  for (run = 0; run < num_of_runs; run++) {
    for (q = 0; q < num_queries; q++)
      for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
        if (queries[q].primitive != NULL)
          queries[q].primitive->intensity[spectrum] = 0;

    /* run the radiosity*/
    if (!rays_from_objects){
      if (!use_sky_file && num_light_sources == 0)
        /* backward compatibility */
        ShootAllRaysOld(grid);
      else
        ShootAllRays(grid);
    }
    if (verbose) {
      fprintf(stderr, "%s - starting answering %d querie(s).\n", proc_name,
              num_queries);
    }

    /* send results back to cpfg */

    /* for all queries */
    for (q = 0; q < num_queries; q++) {
      q2 = q;

      for (;;) {
        if (rays_from_objects) {
          ShootRaysFromObjects(grid, queries + q, &comm_symbol2);

          for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
            aux[spectrum] = comm_symbol2.params[spectrum].value;

            queries[q2].mean[spectrum] += aux[spectrum];
            queries[q2].var[spectrum] += aux[spectrum] * aux[spectrum];
          }
        } else
          for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
            queries[q2].mean[spectrum] +=
                (aux[spectrum] =
                     queries[q].primitive->intensity[spectrum] / ray_density);
            queries[q2].var[spectrum] += aux[spectrum] * aux[spectrum];
          }

        if (num_of_runs > 1)
          if (spectrum_samples > 1 && aux[1] > 0) {
            queries[q2].ratio_mean += aux[0] / aux[1];
            queries[q2].ratio_var += aux[0] * aux[0] / (aux[1] * aux[1]);
          }

        if (queries[q + 1].dist == queries[q].dist &&
            queries[q + 1].master == queries[q].master && q < num_queries - 1)
          q++; /* there are more primitives representing one object */
        else
          break;
      }
    }
  }

  if (num_of_runs == 1)
    for (q = 0; q < num_queries; q++) {
      for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
        comm_symbol.params[spectrum].value = queries[q].mean[spectrum];
        comm_symbol.params[spectrum].set = 1;
      }
      comm_symbol.num_params = queries[q].num_params;

      if (verbose) {
        fprintf(stderr, "%s - intensity reaching primitive %d is %g\n",
                proc_name, q, comm_symbol.params[0].value);
      }

      CSSendData(queries[q].master, queries[q].dist, &comm_symbol);

      for (;;)
        if (queries[q + 1].dist == queries[q].dist &&
            queries[q + 1].master == queries[q].master && q < num_queries - 1)
          q++; /* there are more primitives representing one object */
        else
          break;
    }
  else if (spectrum_samples > 0)
    for (q = 0; q < num_queries; q++) {
      if ((comm_symbol.num_params = queries[q].num_params) >= 2) {
        queries[q].ratio_mean /= (float)num_of_runs;
        comm_symbol.params[0].value = queries[q].ratio_mean;
        comm_symbol.params[0].set = 1;

        queries[q].ratio_var /= (float)num_of_runs;
        comm_symbol.params[1].value =
            sqrt(fabs(queries[q].ratio_var -
                      queries[q].ratio_mean * queries[q].ratio_mean));
        comm_symbol.params[1].set = 1;

        if (comm_symbol.num_params >= 2 + 2 * spectrum_samples) {
          for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
            queries[q].mean[spectrum] /= (float)num_of_runs;
            comm_symbol.params[2 + spectrum * 2].value =
                queries[q].mean[spectrum];
            queries[q].var[spectrum] /= (float)num_of_runs;
            comm_symbol.params[3 + spectrum * 2].value = sqrt(
                fabs(queries[q].var[spectrum] -
                     queries[q].mean[spectrum] * queries[q].mean[spectrum]));
            comm_symbol.params[2 + spectrum * 2].set = 1;
            comm_symbol.params[3 + spectrum * 2].set = 1;
          }
        }

        CSSendData(queries[q].master, queries[q].dist, &comm_symbol);
      }

      for (;;)
        if (queries[q + 1].dist == queries[q].dist &&
            queries[q + 1].master == queries[q].master && q < num_queries - 1)
          q++; /* there are more primitives representing one object */
        else
          break;
    }
}

/****************************************************************************/
void MainLoop(void) {
  Cmodule_type two_modules[2];
  unsigned long module_id;
  int master;
  CTURTLE turtle;
  int i;
  int in;

  /* infinite loop - until signal 'exit' comes */
  for (;;) {
    in = 0;
    all_hits = 0;

    if (verbose)
      fprintf(stderr, "%s - start processing data.\n", proc_name);

    if (remove_objects) {
      InitializeFieldStructures();
      InitializeObjects();
      FillGrid(&grid);
    }
    /*
      else
        set all intensities to 0
        deal with removed or moved leaves
    */

    CSBeginTransmission();

    /* process the data */
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {
      in = 1;
      if (verbose) {
        fprintf(stderr, "%s - comm. symbol has %d parameters:\n      ",
                proc_name, two_modules[0].num_params);
        for (i = 0; i < two_modules[0].num_params; i++)
          fprintf(stderr, " %g", two_modules[0].params[i].value);
        fprintf(stderr, "\n");

        fprintf(stderr, "%s - next symbol '%s' has %d parameters:\n      ",
                proc_name, two_modules[1].symbol, two_modules[1].num_params);
        for (i = 0; i < two_modules[1].num_params; i++)
          fprintf(stderr, " %g", two_modules[1].params[i].value);
        fprintf(stderr, "\n");
      }

      StoreInput(&grid, master, module_id, two_modules, &turtle);
    }

    if (in) {
      DetermineResponse(&grid);

      if (verbose) {
        fprintf(stderr, "%s - total number of hits: %ld\n", proc_name,
                all_hits);
        fprintf(stderr, "%s - data processed.\n", proc_name);
      }
    } else if (verbose)
      fprintf(stderr, "%s - no data input.\n", proc_name);

    /* End transmission returns 1 when the process is requested to exit */
    if (CSEndTransmission())
      break;
  }
}

/****************************************************************************/
void ReadMaterial(FILE *fp, int i) {
  int spectrum;
  char *token, input_line[255];

  /* there is a line per spectrum */
  for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
    if (spectrum > 0) {
      /* get new line */
      if (fgets(input_line, 255, fp) == NULL)
        break;

      /* get over the colon */
      token = strtok(input_line, "\t:");
    }

    if ((token = strtok(NULL, ",; ()\t:\n")) == NULL) {
      fprintf(stderr, "Error reading reflectance[%d].\n", spectrum);
      break;
    }
    materials[i].reflectance[spectrum] = atof(token);

    if ((token = strtok(NULL, ",; ()\t:\n")) == NULL) {
      fprintf(stderr, "Error reading spec. power[%d].\n", spectrum);
      break;
    }
    materials[i].spec_power[spectrum] = atof(token);

    if ((token = strtok(NULL, ",; ()\t:\n")) == NULL) {
      fprintf(stderr, "Error reading transmittance[%d].\n", spectrum);
      break;
    }
    materials[i].transmittance[spectrum] = atof(token);

    if ((token = strtok(NULL, ",; ()\t:\n")) == NULL) {
      fprintf(stderr, "Error reading trans_power[%d].\n", spectrum);
      break;
    }
    materials[i].trans_power[spectrum] = atof(token);

    if ((token = strtok(NULL, ",; ()\t:\n")) == NULL) {
      fprintf(stderr, "Error reading Nt[%d].\n", spectrum);
      break;
    }
    materials[i].Nt[spectrum] = atof(token);

    if (materials[i].reflectance[spectrum] > 1) {
      fprintf(stderr,
              "%s - reflectance cannot be more than 1!. "
              "Adjusted to 1.\n",
              proc_name);
      materials[i].reflectance[spectrum] = 1;
    }

    if (materials[i].reflectance[spectrum] < 0) {
      fprintf(stderr,
              "%s - reflectance cannot be less than 0!. "
              "Adjusted to 0.\n",
              proc_name);
      materials[i].reflectance[spectrum] = 0;
    }

    if (materials[i].transmittance[spectrum] > 1) {
      fprintf(stderr,
              "%s - transmittance cannot be more than 1!. "
              "Adjusted to 1.\n",
              proc_name);
      materials[i].transmittance[spectrum] = 1;
    }

    if (materials[i].transmittance[spectrum] < 0) {
      fprintf(stderr,
              "%s - transmittance cannot be less than 0!. "
              "Adjusted to 0.\n",
              proc_name);
      materials[i].transmittance[spectrum] = 0;
    }

    if (materials[i].transmittance[spectrum] +
            materials[i].reflectance[spectrum] >
        1) {
      fprintf(stderr, " sum of transmittance and reflectance cannot"
                      " be more than 1. Transmittance adjusted to "
                      "1-reflectance.\n");
      materials[i].transmittance[spectrum] =
          1 - materials[i].reflectance[spectrum];
    }
  }
}

/****************************************************************************/
void ProcessArguments(int argc, char **argv) {
  FILE *fp;
  int i, spectrum;
  char *keywords[] = {
      "domain size",            /*  0 */
      "position",               /*  1 */
      "verbose",                /*  2 */
      "grid size",              /*  3 */
      "seed",                   /*  4 */
      "obstacles",              /*  5 */
      "periodic canopy",        /*  6 */
      "remove objects",         /*  7 */
      "maximum depth",          /*  8 */
      "ray density",            /*  9 */
      "leaf material (top)",    /* 10 */
      "leaf material (bottom)", /* 11 */

      "Russian roulette",  /* 12 */
      "reflectance model", /* 13 */

      "spectrum samples", /* 14 */
      "source spectrum",  /* 15 */

      "sky file",             /* 16 */
      "rays from objects",    /* 17 */
      "one ray per spectrum", /* 18 */

      "material",            /* 19 */
      "material parameter",  /* 20 */
      "stratified sampling", /* 21 */
      "number of runs",      /* 22 */
      "no direct light",     /* 23 */

      "version", /* 24 */

      "light source", /* 25 */

      NULL /* the last item must be NULL! */
  };
  char *token, input_line[255];

  /* defaults */
  verbose = 0;
  remove_objects = 1;
  periodic_canopy = 0;
  material_parameter = -1;

  for (spectrum = 0; spectrum < MAX_SPECTRUM_SAMPLES; spectrum++) {
    for (i = 0; i < 2; i++) {
      materials[i].reflectance[spectrum] = 0.35;
      materials[i].spec_power[spectrum] = 0; /* distribute in all directions */
      materials[i].transmittance[spectrum] = 0.55;
      materials[i].trans_power[spectrum] = 0; /* distribute in all directions */
      materials[i].Nt[spectrum] = 1.4;
    }
    source_spectrum[spectrum].wavelength = 660; /* [nm] - red */
    source_spectrum[spectrum].weight = 1.0;
  }

  num_materials = 2;
  spectrum_samples = 1;

  num_light_sources = 0; /* meaning that the light is coming only from the
                            top */
  use_sky_file = 0;
  rays_from_objects = 0;
  from_object_exp = 0.0;
  one_ray_per_spectrum = 0;

  max_depth = 3;
  ray_density = 500;

  russian_roulette.threshold = 0; /* don't use Russian roulette */
  russian_roulette.p = 0.7;       /* as in Michael Chelle's Parcinopy */

  reflectance_model = PARCINOPY;

  stratified_sampling = 0;

  num_of_runs = 1;
  no_direct = 0; /* if 1, no direct light is considered (the first hit) */

  grid.size[X] = 1;
  grid.size[Y] = 1;
  grid.size[Z] = 1;

  grid.range[X] = 2.0;
  grid.range[Y] = 2.0;
  grid.range[Z] = 2.0;
  grid.pos[X] = -1.0;
  grid.pos[Y] = -1.0;
  grid.pos[Z] = -1.0;

  version = 1.0;

  if (argc == 1) {
    printf("%s - not enough arguments!\n"
           "USAGE: %s -e environment_file specification_file\n",
           proc_name, proc_name);

    exit(0);
  }

  /* read in environment file */
  if ((fp = fopen(argv[1], "r")) == NULL)
    fprintf(stderr, "%s - cannot open specification file %s.\n", proc_name,
            argv[1]);
  else {
    /* process the file line by line */
    while (!feof(fp)) {
      /* get the whole line */
      if (fgets(input_line, sizeof(input_line), fp) == NULL)
        break;

      /* get the keyword */
      token = strtok(input_line, "\t:");

      /* look for a keyword in the table */
      i = 0;
      while (keywords[i] != NULL) {
        if (strcmp(keywords[i], token) == 0)
          break;
        i++;
      }

      if (keywords[i] == NULL) {
        fprintf(stderr,
                "%s - unknown directive %s in the specification file.\n",
                proc_name, token);
        continue;
      }

      switch (i) {
      case 0: /* domain size - range */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.range[X] = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.range[Y] = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.range[Z] = atof(token);
        break;

      case 1: /* position */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.pos[X] = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.pos[Y] = atof(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.pos[Z] = atof(token);
        break;

      case 2: /* verbose */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        verbose = atoi(token);
        break;

      case 3: /* number of nodes - size */
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.size[X] = atoi(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.size[Y] = atoi(token);
        token = strtok(NULL, "x,; \t:\n");
        if (token == NULL)
          break;
        grid.size[Z] = atoi(token);

        /* allocate the grid */
        if (grid.size[X] * grid.size[Y] * grid.size[Z] == 0) {
          fprintf(stderr, "%s - wrong size of the grid! Size 1x1x1 used.\n",
                  proc_name);
          grid.size[X] = 1;
          grid.size[Y] = 1;
          grid.size[Z] = 1;
        }
        break;

      case 4: /* seed */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        srand48(atol(token));
        break;

      case 5: /* obstacles */
        /* file input */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        ReadPrimitiveFile(token, 1);
        break;

      case 6: /* periodic canopy */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        if (!strcmp(token, "on") || !strcmp(token, "yes"))
          periodic_canopy = 1;
        break;

      case 7: /* remove objects */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        if (strcmp(token, "off") == 0 || strcmp(token, "no") == 0)
          remove_objects = 0;
        break;

      case 8: /* maximum depth */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        max_depth = atoi(token);
        break;

      case 9: /* ray density */
        token = strtok(NULL, ",; \t:\n");
        if (token == NULL)
          break;
        ray_density = atof(token);
        break;

      case 10: /* leaf material (top)*/
        ReadMaterial(fp, 0);
        break;

      case 11: /* leaf material (bottom)*/
        ReadMaterial(fp, 1);
        break;

      case 12: /* Russian roulette */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        russian_roulette.threshold = atof(token);

        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        russian_roulette.p = atof(token);
        break;

      case 13: /* reflectance model */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (!strcmp(token, "blinn"))
          reflectance_model = BLINN_PHONG;
        if (!strcmp(token, "phong"))
          reflectance_model = PHONG;
        if (!strcmp(token, "parcinopy"))
          reflectance_model = PARCINOPY;
        break;

      case 14: /* spectrum samples */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        spectrum_samples = atoi(token);
        break;

      case 15: /* source spectrum */
        for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
          if ((token = strtok(NULL, ",; \t:\n")) == NULL)
            break;
          source_spectrum[spectrum].wavelength = atof(token);

          if ((token = strtok(NULL, ",; \t:\n")) == NULL)
            break;
          source_spectrum[spectrum].weight = atof(token);
        }
        break;

      case 16: /* sky file */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (ReadSkyFile(token))
          use_sky_file = 1;
        break;

      case 17: /* rays from objects */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (!strcmp(token, "yes"))
          rays_from_objects = 1;

        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        from_object_exp = atof(token);
        break;

      case 18: /* one ray per spectrum */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (!strcmp(token, "yes"))
          one_ray_per_spectrum = 1;
        break;

      case 19: /* material - index 2 and higher */
        if (num_materials >= NUM_MATERIALS) {
          fprintf(stderr, "cannot add new material.\n");
          break;
        }
        ReadMaterial(fp, num_materials++);
        break;

      case 20: /* material parameter */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        material_parameter = atoi(token) - 1;
        break;

      case 21: /* stratified sampling */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (!strcmp(token, "yes") || !strcmp(token, "on"))
          stratified_sampling = 1;

        break;

      case 22: /* number of runs */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        num_of_runs = atoi(token);
        if (num_of_runs < 1)
          num_of_runs = 1;

      case 23: /* no direct light */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        if (!strcmp(token, "yes") || !strcmp(token, "on"))
          no_direct = 1;
        break;

      case 24: /* version */
        if ((token = strtok(NULL, ",; \t:\n")) == NULL)
          break;
        version = atof(token);
        break;

      case 25: /* light sources */
        if (num_light_sources >= MAX_LIGHT_SOURCES) {
          fprintf(stderr, "%s - too many light sources. Ignored.\n", proc_name);
          break;
        }

        /* default */
        for (i = 0; i < 3; i++)
          light_sources[num_light_sources].direction[i] = 0;
        light_sources[num_light_sources].weight = 1;

        for (i = 0; i < 3; i++) {
          if ((token = strtok(NULL, ",; \t:\n")) == NULL)
            break;
          light_sources[num_light_sources].direction[i] = atof(token);
        }

        if ((token = strtok(NULL, "x,; \t:\n")) == NULL)
          break;
        light_sources[num_light_sources].weight = atof(token);

        num_light_sources++;
        break;
      }
    }
  }

  SetGlobalParameters();

  InitializeFieldStructures();
  InitializeObjects();
  InitializeLightSources();
  FillGrid(&grid);

  if (verbose) {
    fprintf(stderr, "%s - domain size:     %gx%gx%g\n", proc_name,
            grid.range[X], grid.range[Y], grid.range[Z]);
    fprintf(stderr, "%s - position:       (%g,%g,%g)\n", proc_name, grid.pos[X],
            grid.pos[Y], grid.pos[Z]);
    fprintf(stderr, "%s - number of nodes: %dx%dx%d\n", proc_name, grid.size[X],
            grid.size[Y], grid.size[Z]);
    fprintf(stderr, "material_parameter = %d\n", material_parameter);

    fprintf(stderr, "\n%s - specification file processed.\n\n", proc_name);
  }
}

/****************************************************************************/
int main(int argc, char **argv) {
  char *process_name = strdup(argv[0]);

  /* initialize the communication as the very first thing */
  CSInitialize(&argc, &argv);

  ProcessArguments(argc, argv);

  fprintf(stderr, "Field process %s initialized.\n", process_name);

  MainLoop();

  FreeFieldStructures();

  fprintf(stderr, "Field process %s exiting.\n", process_name);

  /* should be the last function called */
  CTerminate();

  return 1;
}
