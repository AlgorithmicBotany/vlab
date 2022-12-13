/* query.h - handles the modules that come from the plant model */

#ifndef _QUERY_H
#define _QUERY_H


#define DEFAULT_QUERY_SIZE 512

typedef struct tagQUERY {
  unsigned long id;
  int master;
  int num_params;
  float pos[3], up[3], edge1[3],
      edge2[3]; // these are only used for rays from object mode
  float exponent[MAX_SPECTRUM_SAMPLES];
  float area; // area is only used in shooting from sensors, for output in case
              // of ShootAllRays(), and summing polygon area
  float avg_cos_angle; // use in DetermineResponse to calculate average cosine
                       // angle between all light sources and this object
  /* QQQ */ // float weighted_avg_cos_angle; // same as above but weighted
            // according to "intensity" of the light source (DOES NOT WORK WITH
            // SKY MODEL!)
  float radiant_flux[MAX_SPECTRUM_SAMPLES]; /* used in visualization, to shade
                                               parallelogram associated with the
                                               query module */
  int num_rays[MAX_SPECTRUM_SAMPLES];
  int primitive_index;
  int in_polygon;
  int surface_size;
} QUERY;

typedef struct tagQUERIES {
  QUERY *query;
  int num_queries;
  int query_array_size;
} QUERIES;

#ifdef __cplusplus
extern "C" {
#endif
void CheckQueryArraySize(QUERIES *queries, int num_add);
void InitQuery(QUERIES *queries);
void FreeQuery(QUERIES *queries);
void ResetQuery(QUERIES *queries);
#ifdef __cplusplus
}
#endif

#endif
