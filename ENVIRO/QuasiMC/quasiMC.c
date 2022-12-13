// quasiMC.c - environmental program quasiMC

#include "quasiMC.h"
#include "qmc.h"
#include "randquasimc.h"

static char proc_name[32] = "QuasiMC";

const float MAX_SCATTERING_EXPONENT = 100.f;

// parameters for 'quasiMC' set in specification/configuration file
// all of these variables are declared in main.cpp
extern int verbose;
extern int remove_objects;
extern int rays_from_objects;
extern int spectrum_samples;
extern int reflectance_model;
extern unsigned int max_depth;
extern int num_samples;
extern int use_sky_model;
extern int one_ray_per_spectrum;
extern int no_direct_light;
extern int return_var;
extern int material_parameter;
extern int number_of_rays;
extern float sensor_fov;
extern int return_type[MAX_SPECTRUM_SAMPLES];
extern float total_spectrum_flux[MAX_SPECTRUM_SAMPLES];
extern char output_filename[64];
extern char RQMC_method[32];
extern SOURCESPECTRUM source_spectrum[MAX_SPECTRUM_SAMPLES];
extern RUSSIANROULETTE russian_roulette;
extern QUERIES queries;
extern SCENE scene;
extern double *pointqmc, *rpointqmc, *randqmc; // RQMC points
extern double npoints;
extern int numSides;

// local prototypes

int ReadPolygons(int master, unsigned long module_id, int polygon_num,
                 Cmodule_type *two_modules, CTURTLE *turtle);
void ShootRaysFromObjects(QUERY *query, Cmodule_type *comm_module);
void ShootAllRays(void);

int TraceRay(RAY *ray, int spectrum);
int ResolveIntersection(RAY *ray, float *normal, float mindist,
                        PRIMITIVE *intersected, int spectrum);
int ResolveIntersectionOne(RAY *ray, float *normal, float mindist,
                           PRIMITIVE *intersected);
float GenerateRandomDirection_mPhong(float *incoming, float n, float *d,
                                     float *normal);
float GenerateRandomDirection_Phong(float *incoming, float n, float *d,
                                    float *normal);
float GenerateRandomDirection_Lambertian(float *incoming, float n, float *d,
                                         float *normal, float fov);
void GenerateRandomRay(RAY *ray);
void GenerateRayFromSensor(RAY *ray, float *ldir, QUERY *query, int spectrum);

/* ------------------------------------------------------------------------- */
int ReadPolygons(int master, unsigned long module_id, int polygon_num,
                 Cmodule_type *two_modules, CTURTLE *turtle) {
  char line[128];
  float vertex[MAX_VERTICES][3];
  int str_master, input, input_lines;
  int prim_index, vertices, v;
  QUERY *query = NULL;

  // first check if a polygon is being sent from cpfg  - function CSGetString
  // returns zero when no strings from cpfg are coming
  input_lines = 0;
  while (CSGetString(&str_master, line, sizeof(line))) {
    ++input_lines;

    // check if the next few lines specifiy vertices of a polygon
    while (!strncmp(line, "polygon", sizeof("polygon"))) {
      // if yes, read in the vertices and store them in an array (called vertex)
      input = vertices = 0;
      while ((input = CSGetString(&str_master, line, sizeof(line))) != 0) {
        // if there are no more vertices to read, check for further polygons
        if (isalpha(line[0]) || input == 0)
          break;

        sscanf(line, "%f %f %f", &vertex[vertices][0], &vertex[vertices][1],
               &vertex[vertices][2]);

        if (++vertices == MAX_VERTICES) {
          fprintf(stderr,
                  "%s - ERROR: maximum number (%d) of vertices per polygon "
                  "reached\n",
                  proc_name, MAX_VERTICES);
          return (0);
        }
      }

      // if at least three vertices were specified, add the polygon to the scene
      // (triangulate if necessary)
      if (vertices >= 3) {
        // triangulate the polygon
        for (v = 0; v <= vertices - 3; v++) {
          // copy the starting vertex - for the next triangle
          if (v > 0) {
            vertex[v][0] = vertex[0][0];
            vertex[v][1] = vertex[0][1];
            vertex[v][2] = vertex[0][2];
          }

          // Add the triangle
          if ((prim_index = AddTriangle(&scene, vertex[v], material_parameter,
                                        turtle->up, &two_modules[1])) != -1) {
            // if ?E has parameters then the intensity must be returned for
            // primitve if (two_modules[0].num_params > 0) // this works OK with
            // cpfg but not with lpfg
            if (two_modules[0].params[0].value >= 0) {
              CheckQueryArraySize(&queries, vertices - 2);

              query = &queries.query[queries.num_queries];
              query->primitive_index = 0;
              query->id = module_id;
              query->master = master;
              query->num_params = two_modules[0].num_params;
              query->in_polygon = polygon_num;
              query->surface_size = 1;
              // OLD CODE: you could specify number of rays to be shot from
              // sensor for (i = 0; i < spectrum_samples; i++)
              //  query->num_rays[i] = (int) two_modules[0].params[i].value;
              query->primitive_index = prim_index;
              query->area = scene.primitives[prim_index].area;
              queries.num_queries++;
            }
          } else {
            fprintf(stderr, "%s - polygon triangle was not added.\n",
                    proc_name);
            fprintf(stderr,
                    "%s - The triangulation may have caused a degenerate "
                    "triangle. Check specification of vertices.\n",
                    proc_name);
          }
        }
      } else {
        fprintf(stderr,
                "%s - ERROR: at least 3 vertices per polygon required\n",
                proc_name);
      }
    }
  }
  if (input_lines > 0)
    return (1);
  else
    return (0);
}

/* ------------------------------------------------------------------------- */

void StoreQuery(int master, unsigned long module_id, int polygon_num,
                Cmodule_type *two_modules, CTURTLE *turtle)
/* stores one query in the list of all queries made by the plant model */
{
  int i, prim_index;
  QUERY *query = NULL;

  // if polygons are sent from cpfg, read them in and return
  if (ReadPolygons(master, module_id, polygon_num, two_modules, turtle))
    return;

  if (turtle->positionC < 3) {
    fprintf(stderr, "%s - turtle position wasn't sent to the environment.\n",
            proc_name);
    return;
  }

  if (turtle->headingC < 3) {
    fprintf(stderr, "%s - turtle heading wasn't sent to the environment.\n",
            proc_name);
    return;
  }

  if (turtle->upC < 3) {
    fprintf(stderr, "%s - turtle up vector wasn't sent to the environment.\n",
            proc_name);
    return;
  }

  // change several QuasiMC parameters using the 'Q' module
  if (two_modules[1].symbol[0] == 'Q') {
    // first parameter changes the number of rays
    // but don't reset if number of rays hasn't changed
    if (two_modules[1].num_params >= 1 &&
        fabs(npoints - (double)two_modules[1].params[0].value) > 0.1) {
      FreeQMC();

      npoints = (double)two_modules[1].params[0].value;
      SetRQMC(RQMC_method, max_depth, num_samples, &npoints);

      if (!InitQMC()) {
        fprintf(stderr, "QuasiMC - RQMC method '%s' cannot be initialzed!\n",
                RQMC_method);
        return;
      }
    }

    // the second, third, and fourth parameters change the grid size
    if (two_modules[1].num_params >= 4) {
      if (two_modules[1].params[1].value > 0 &&
          two_modules[1].params[2].value > 0 &&
          two_modules[1].params[3].value > 0) {
        ResetGrid(&scene, (int)two_modules[1].params[1].value,
                  (int)two_modules[1].params[2].value,
                  (int)two_modules[1].params[3].value);
      } else {
        fprintf(stderr, "QuasiMC - incorrent grid size in 'Q' module!\n");
      }
    }
    return;
  }

  // check if module changes QuasiMC's configuration (e.g. J() for julian day)
  if (two_modules[1].symbol[0] == 'J') {
    i = (int)two_modules[1].params[0].value;
    if (i < 1 || i > 366)
      fprintf(stderr, "QuasiMC - Julian day out of range (1-366)\n");
    else
      SetJulianDay(i);
    if (two_modules[1].num_params >= 3) {
      if (two_modules[1].params[1].value >= 0. &&
          two_modules[1].params[1].value <= 1. &&
          two_modules[1].params[2].value >= 0.)
        SetWeather(two_modules[1].params[1].value,
                   two_modules[1].params[2].value);
      else
        fprintf(stderr, "QuasiMC - in module J() clear days is out of range (0 "
                        "to 1) or turbidity < 0\n");
    }

    InitSkyModel();
    return;
  }

  // determine if any parameters were sent with comm. module
  // if (two_modules[0].num_params > 0) // this works OK with cpfg but not with
  // lpfg
  if (two_modules[0].params[0].value >= 0) {
    // store the query
    CheckQueryArraySize(&queries, 1);
    query = &queries.query[queries.num_queries];

    query->primitive_index = 0;
    query->id = module_id;
    query->master = master;
    query->num_params = two_modules[0].num_params;
    query->in_polygon = 0;
    query->surface_size =
        1; // surface size must be at least one, (for one triangle, etc.)

    // OLD VERSION: the params of ?E module could be used to specify number of
    // rays
    //              to shoot from the sensor
    // NEW VERSION: the params of ?E module specify the scattering exponent per
    // wavelength
    // for (i = 0; i < spectrum_samples; i++)
    //{
    //  query->num_rays[i] = (int) two_modules[0].params[i].value;
    //  query->radiant_flux[i] = 0.f;
    //}
    for (i = 0; i < spectrum_samples; i++) {
      query->num_rays[i] = 0;
      query->radiant_flux[i] = 0.f;
      query->exponent[i] = two_modules[0].params[i].value;
    }

    if (rays_from_objects) {
      // store two vectors used for generating rays and up vector
      if (turtle->leftC < 3) {
        fprintf(stderr,
                "%s - turtle left vector was not sent to the environment.\n",
                proc_name);
        return;
      }

      if (two_modules[1].symbol[0] != 'P') {
        fprintf(stderr, "%s - rays-from-object mode only allows 'P' modules.\n",
                proc_name);
        return;
      }

      query->area =
          two_modules[1].params[0].value * two_modules[1].params[1].value;

      for (i = X; i <= Z; i++) {
        if (turtle->up[i] > -0.0001f && turtle->up[i] < 0.0001f)
          query->up[i] = 0.f;
        else
          query->up[i] = turtle->up[i];
        query->pos[i] = turtle->position[i];
        query->edge1[i] =
            two_modules[1].params[0].value * turtle->heading[i] +
            two_modules[1].params[1].value * 0.5f * turtle->left[i];
        query->edge2[i] =
            two_modules[1].params[0].value * turtle->heading[i] -
            two_modules[1].params[1].value * 0.5f * turtle->left[i];
      }

      BoundQueryObject(&scene, query);

      ++queries.num_queries;
      return; // 'P' modules are not stored in scene
    } else
      ++queries.num_queries;
  }

  if ((prim_index = AddObject(&scene, turtle, &two_modules[1],
                              material_parameter)) != -1) {
    // if a surface was added, save the number of patches, which is used for
    // summing up irradiance of the triangles of a single surface (need to
    // muliple by 2 because there are 2 triangles per patch)
    if (two_modules[1].symbol[0] == 'S' && !rays_from_objects)
      query->surface_size =
          2 *
          SurfacePatches(&scene.surfaces[(int)two_modules[1].params[0].value]);

    // if adding an internode, save the number of triangles
    if (two_modules[1].symbol[0] == 'I' && !rays_from_objects)
      query->surface_size = numSides * 2;

    // if ?E has parameters then the intensity must be returned for primitive
    // if (two_modules[0].num_params > 0) // This works OK with cpfg, but not
    // lpfg
    // if ?E has parameters >= 0, the intensity must be returned for primitive
    if (two_modules[0].params[0].value >= 0)
      query->primitive_index = prim_index;
  } else {
    // set this query to have no primitive associated with it, but first
    // check if the radiant energy even has to be returned for the primitive
    if (two_modules[0].params[0].value >= 0) {
      query->surface_size = 0;
      query->primitive_index = -1;
      if (verbose >= 3)
        fprintf(stderr, "%s - object was not added\n", proc_name);
    }
  }

  return;
}

/* ---------------------------------------------------------------------------
 */

void DetermineResponse(void)
/* send modules with new values back to the plant model */
// it is too bad that the area calculations are done per wavelength!!!
{
  Cmodule_type comm_symbol;
  STATSQMC outer_blocks[MAX_SPECTRUM_SAMPLES];
  STATSQMC outer_blocks_ratios[MAX_SPECTRUM_SAMPLES];
  STATSQMC sunlit_blocks[MAX_SPECTRUM_SAMPLES];
  double light_flux[MAX_SPECTRUM_SAMPLES];
  double sunlit_area[MAX_SPECTRUM_SAMPLES];
  int i, j, k, spectrum;
  float area, polygon_area, polygon_mean_angle, polygon_num;
  float normal[3];
  FILE *output = NULL;
  int over_estimate_issue = 0; // number of times the results for a primitive
                               // was determined to be oversampled
  static int outfile_num =
      1; // adjusts name of output file for each environmental step

  for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
    outer_blocks[spectrum] = InitStat(queries.num_queries);
    outer_blocks_ratios[spectrum] = InitStat(queries.num_queries);
    sunlit_blocks[spectrum] = InitStat(queries.num_queries);
  }

  // the following function calls were already made in main.cpp
  // FindBoundingSphere (&scene);
  // FillGrid (&scene);

  if (!rays_from_objects) {

    // for each sample, do...
    for (i = 0; i < num_samples; i++) {
      if (verbose >= 1)
        fprintf(stderr, "\n%s - running sample #%d\n", proc_name, i + 1);

      // reset values that are computed for each run
      for (j = 0; j < scene.num_primitives; j++) {
        scene.primitives[j].avg_cos_angle = 0.0;
        for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
          scene.primitives[j].absorbed_flux[spectrum] = 0.0;
          scene.primitives[j].incident_flux[spectrum] = 0.0;
          scene.primitives[j].direct_hits[spectrum] = 0.0;
        }
      }

      ShootAllRays();

      for (j = 0; j < queries.num_queries; j++) {
        for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
          // compute the light flux reaching a primitive object
          light_flux[spectrum] = 0.f;
          sunlit_area[spectrum] = 0.f;
          area = 0.f;
          polygon_mean_angle = 0.f;
          normal[X] = normal[Y] = normal[Z] = 0.f;

          // must loop over all of the triangles in a surface but
          // the same code works for just a single triangle or parallelogram
          for (k = 0; k < queries.query[j].surface_size; k++) {
            // sum the absorbed flux or incident flux for this primitive
            // depending on which one the user wants.
            if (return_type[spectrum] == UP_INCIDENT_IRRADIANCE ||
                return_type[spectrum] == LW_INCIDENT_IRRADIANCE)
              light_flux[spectrum] +=
                  scene.primitives[queries.query[j].primitive_index - k]
                      .incident_flux[spectrum];
            else
              light_flux[spectrum] +=
                  scene.primitives[queries.query[j].primitive_index - k]
                      .absorbed_flux[spectrum];

            // sum of direct intersections used to compute sunlit leaf area
            sunlit_area[spectrum] +=
                scene.primitives[queries.query[j].primitive_index - k]
                    .direct_hits[spectrum];

            // sum of the area
            area += scene.primitives[queries.query[j].primitive_index - k].area;

            // save normal
            normal[X] += scene.primitives[queries.query[j].primitive_index - k]
                             .normal[X];
            normal[Y] += scene.primitives[queries.query[j].primitive_index - k]
                             .normal[Y];
            normal[Z] += scene.primitives[queries.query[j].primitive_index - k]
                             .normal[Z];

            // save the cosine angle
            polygon_mean_angle +=
                scene.primitives[queries.query[j].primitive_index - k]
                    .avg_cos_angle;
          }

          // save area of the primitive and save normal (average for surface),
          // both are used for output
          queries.query[j].area = area;
          queries.query[j].up[X] =
              normal[X] / (float)queries.query[j].surface_size;
          queries.query[j].up[Y] =
              normal[Y] / (float)queries.query[j].surface_size;
          queries.query[j].up[Z] =
              normal[Z] / (float)queries.query[j].surface_size;
          queries.query[j].avg_cos_angle =
              polygon_mean_angle / (float)queries.query[j].surface_size;

          // NEW CODE: extended here depending on what user whats returned
          // first divide by ray density (total radiant flux per spectrum /
          // total area) to get normalized flux ray density is the radiant flux
          // per unit area
          if (total_spectrum_flux[spectrum] > 0.0) {
            light_flux[spectrum] *=
                (M_PIf * scene.grid.bsph_radius * scene.grid.bsph_radius) /
                (total_spectrum_flux[spectrum]) *
                source_spectrum[spectrum].weight;
            sunlit_area[spectrum] *=
                (M_PIf * scene.grid.bsph_radius * scene.grid.bsph_radius) /
                npoints;
          }
          switch (return_type[spectrum]) {
          case ABSORBED_FLUX:
            break;
          case ABSORBED_IRRADIANCE:
          case UP_INCIDENT_IRRADIANCE:
          case LW_INCIDENT_IRRADIANCE:
            if (area > 0.f) {
              // if the radiant energy is greater than radiant energy available
              // in primitive's area an overestimation has occured (e.g., all of
              // the rays hit a triangle and none miss one) one possible fix
              // that sometimes works is to increase the number of rays!
              // another fix might be to slighty increase the radius of the
              // bounding sphere
              if (light_flux[spectrum] >
                  area * source_spectrum[spectrum].weight) {
                ++over_estimate_issue;
                light_flux[spectrum] = source_spectrum[spectrum].weight;
              } else
                light_flux[spectrum] /= area;
            } else
              light_flux[spectrum] = 0.0;
            break;
          default:
            fprintf(stderr, "%s - unknown return type specified\n", proc_name);
            break;
          }

          StatUpdate1(outer_blocks[spectrum], j, light_flux[spectrum]);
          StatUpdate1(sunlit_blocks[spectrum], j, sunlit_area[spectrum]);
        }
        // update the value for the ratio of spectrum 0 to 1
        if (spectrum_samples == 2 && light_flux[1] > 0.0)
          StatUpdate1(outer_blocks_ratios[0], j, light_flux[0] / light_flux[1]);
      }
    }
  } else {
    for (i = 0; i < num_samples; i++) {
      if (verbose >= 1)
        fprintf(stderr, "\n%s - running sample #%d\n", proc_name, i + 1);

      for (j = 0; j < queries.num_queries; j++) {
        ShootRaysFromObjects(queries.query + j, &comm_symbol);

        for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
          StatUpdate1(outer_blocks[spectrum], j,
                      comm_symbol.params[spectrum].value);
        // update the value for the ratio of spectrum 0 to 1
        if (spectrum_samples >= 2 && comm_symbol.params[1].value > 0.f)
          StatUpdate1(outer_blocks_ratios[0], j,
                      comm_symbol.params[0].value /
                          comm_symbol.params[1].value);
      }
    }
  }

  if (verbose >= 1 && over_estimate_issue > 0)
    fprintf(stderr, "%s - clamping occured %d times from %d querie(s)\n",
            proc_name, over_estimate_issue, queries.num_queries);

  // send results back to cpfg
  if (verbose >= 2)
    fprintf(stderr, "\n%s - starting to answer %d querie(s)\n", proc_name,
            queries.num_queries);

  // if return variance is set, output a file with mean and variance for all
  // primitives
  if (return_var) {
    // try to open a file for writing, if no file name is given, use a default.
    if (strlen(output_filename) == 0)
      sprintf(output_filename, "%s%d.%d.%d.dat", RQMC_method, outfile_num,
              num_samples, one_ray_per_spectrum);
    output = fopen(output_filename, "w");
    if (output == NULL) {
      fprintf(stderr,
              "%s - ERROR: could not open output file for writing. Output file "
              "disabled!\n",
              proc_name);
      return_var = 0;
    } else {
      // output a header to the file
      fprintf(output,
              "actual_leaf_area mean_projected_area sunlit_area shaded_area ");
      for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
        fprintf(output, "mean_flux%d rel_mean_flux%d variance%d ", spectrum,
                spectrum, spectrum);
      if (spectrum_samples == 2)
        fprintf(output, "spectrum01 var01\n");
      else
        fprintf(output, "\n");
      ++outfile_num;

      // compute the average cosine angle for each object and incoming ray
      // direction (used to calculate projected area)
      for (i = 0; i < queries.num_queries; i++) {
        if (!use_sky_model) {
          // if not using sky model, for each object and light source, compute
          // the average cosine angle
          queries.query[i].avg_cos_angle = 0.0f;
          for (j = 0; j < scene.num_lights; j++) {
            // take absolute value of dot product because it doesn't matter if
            // front or back side of surface is "seen"
            queries.query[i].avg_cos_angle += (float)fabs(
                DotProduct(scene.lights[j].dir, queries.query[i].up));
          }
          queries.query[i].avg_cos_angle /= (float)scene.num_lights;
        } else {
          // careful not to take average ray dir and assign it to mean_ray_dir!
          // DON'T do following "mean_ray_dir /= (num_samples*npoints)", because
          // mean_ray_dir is used per object in the loop!
          if (!one_ray_per_spectrum)
            queries.query[i].avg_cos_angle /= spectrum_samples;
        }
      }
    }
  }

  // for all queries - careful the index 'i' is modified within the block
  // statement not just the for loop!!!
  for (i = 0; i < queries.num_queries; i++) {
    // check if this query module belongs to one polygon
    // REUSE the light_flux variable to store the variance for each polygon
    // REUSE the sunlit area variable to store the average sunlit leaf area over
    // the polygon
    if (queries.query[i].in_polygon != 0) {
      // add contribution from the first primitive in the polygon - this will
      // initialize the irradiance
      for (spectrum = 0; spectrum < queries.query[i].num_params; spectrum++) {
        polygon_area = queries.query[i].area;
        polygon_num = 1.f;
        polygon_mean_angle = queries.query[i].avg_cos_angle;
        if (spectrum < spectrum_samples) {
          comm_symbol.params[spectrum].value =
              (float)StatAverage(outer_blocks[spectrum], i, RANDVARONE) *
              queries.query[i].area;
          light_flux[spectrum] =
              (float)StatVariance(outer_blocks[spectrum], i, RANDVARONE);
          comm_symbol.params[spectrum].set = 1;

          sunlit_area[spectrum] =
              StatAverage(sunlit_blocks[spectrum], i, RANDVARONE);
        } else {
          comm_symbol.params[spectrum].value = 0.0;
          comm_symbol.params[spectrum].set = 0;

          sunlit_area[spectrum] = 0.0;
        }
      }

      j = i; // save the index of the first query forming the polygon (used to
             // save radiant flux)

      // add contribution from the rest of the primitives that make up the poly
      while (i + 1 < queries.num_queries &&
             queries.query[i].in_polygon == queries.query[i + 1].in_polygon) {
        polygon_area += queries.query[i + 1].area;
        polygon_num += 1.f;
        polygon_mean_angle += queries.query[i + 1].avg_cos_angle;

        for (spectrum = 0; spectrum < queries.query[i + 1].num_params;
             spectrum++)
          if (spectrum < spectrum_samples) {
            comm_symbol.params[spectrum].value +=
                (float)StatAverage(outer_blocks[spectrum], i + 1, RANDVARONE) *
                queries.query[i + 1].area;
            light_flux[spectrum] +=
                StatVariance(outer_blocks[spectrum], i, RANDVARONE);
            comm_symbol.params[spectrum].set = 1;

            sunlit_area[spectrum] +=
                StatAverage(sunlit_blocks[spectrum], i, RANDVARONE);
          } else {
            comm_symbol.params[spectrum].value += 0.0;
            comm_symbol.params[spectrum].set = 0;

            sunlit_area[spectrum] += 0.0;
          }
        ++i;
      }
      comm_symbol.num_params = queries.query[i].num_params;
      // divide by the total area of the polygon, and take the mean variance
      for (spectrum = 0; spectrum < comm_symbol.num_params; spectrum++) {
        if (spectrum < spectrum_samples) {
          comm_symbol.params[spectrum].value /= polygon_area;
          light_flux[spectrum] /= polygon_num;
        }
        // save radiant flux for visualization
        queries.query[j].radiant_flux[spectrum] =
            comm_symbol.params[spectrum].value;
      }
      // get average cosine angle
      polygon_mean_angle /= polygon_num;
    } else // the query belongs to a triangle or parallelogram or surface
    {
      // store the area, for output
      polygon_area = queries.query[i].area;
      polygon_mean_angle = queries.query[i].avg_cos_angle;

      for (spectrum = 0; spectrum < queries.query[i].num_params; spectrum++) {
        if (spectrum < spectrum_samples) {
          comm_symbol.params[spectrum].value =
              (float)StatAverage(outer_blocks[spectrum], i, RANDVARONE);
          light_flux[spectrum] =
              (float)StatVariance(outer_blocks[spectrum], i, RANDVARONE);
          comm_symbol.params[spectrum].set = 1;

          sunlit_area[spectrum] =
              StatAverage(sunlit_blocks[spectrum], i, RANDVARONE);
        } else {
          comm_symbol.params[spectrum].value = 0.0;
          comm_symbol.params[spectrum].set = 0;

          sunlit_area[spectrum] = 0.0;
        }
        // save radiant flux for visualization
        queries.query[i].radiant_flux[spectrum] =
            comm_symbol.params[spectrum].value;
      }
      comm_symbol.num_params = queries.query[i].num_params;
    }

    // output results file with leaf areas and absorbed light + variance
    if (return_var) {
      // 1. output the actual leaf area and projected leaf area
      area =
          polygon_area *
          polygon_mean_angle; // this is the projected area, REUSE area variable
      fprintf(output, "%f %f ", polygon_area, area);

      // 2. compute the sunlit_area over the spectrum (since we may have several
      // wavelengths this is necessary) REUSE the polygon_area variable
      if (one_ray_per_spectrum)
        polygon_area =
            (float)sunlit_area[0]; // if only one ray for all wavelengths, the
                                   // values are all the same so use [0]
      else {
        // compute average over the different wavelengths
        polygon_area = 0.f;
        for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
          polygon_area += (float)sunlit_area[spectrum];
        polygon_area /= (float)spectrum_samples;
      }

      // 3. output sunlit area and shaded area - check to see if sunlit leaf
      // area is > projected leaf area this is another oversampling error I
      // think...
      if (polygon_area > area)
        polygon_area = area;
      fprintf(output, "%f %f ", polygon_area, area - polygon_area);

      // 4. output the values associated for each wavelength (if samples > 1)
      // first, compute normalization factor for horizontal surface at top of
      // canopy
      polygon_mean_angle =
          0.0f; // use this as temp variable to compute normalizing factor
      polygon_area = 0.0f; // use this as temp variable to store sum of light
                           // source weights
      normal[X] = normal[Z] = 0.0f;
      normal[Y] = 1.0f;
      if (!use_sky_model) {
        for (j = 0; j < scene.num_lights; j++) {
          polygon_mean_angle +=
              (float)fabs(DotProduct(scene.lights[j].dir, normal)) *
              scene.lights[j].weight;
          polygon_area += scene.lights[j].weight;
        }
        polygon_mean_angle /= polygon_area;
      } else {
        polygon_mean_angle = 0.f;
      }

      for (spectrum = 0; spectrum < queries.query[i].num_params; spectrum++)
        if (spectrum < spectrum_samples) {

          // light flux according to return type
          fprintf(output, "%lg ", comm_symbol.params[spectrum].value);

          // compute light flux relative to horizontal surface at top of canopy
          if (polygon_mean_angle != 0.0) {
            // do not clamp the relative value to 1.0. if it is > 1.0, the
            // object gets more light relative to horizontal surface.
            fprintf(output, "%lg ",
                    comm_symbol.params[spectrum].value / polygon_mean_angle);
          } else
            fprintf(output, "0.0 "); // in this case, the value would be 0.0
                                     // anyway, so just print that
          // estimated variance
          if (num_samples > 1)
            fprintf(output, "%lg ", light_flux[spectrum]);
          else
            fprintf(output, "0.0 ");
        }
      // also output the mean and variance for the ratios of spectrum 0 and 1
      // this does not work correctly if a polygon is used. only T and P work
      if (spectrum_samples == 2) {
        fprintf(output, "%lg ",
                StatAverage(outer_blocks_ratios[0], i, RANDVARONE));
        if (num_samples > 1)
          fprintf(output, "%lg",
                  StatVariance(outer_blocks_ratios[0], i, RANDVARONE));
        else
          fprintf(output, "0.0");
      }
      fprintf(output, "\n");
    }

    CSSendData(queries.query[i].master, queries.query[i].id, &comm_symbol);
  }

  if (return_var)
    fclose(output);

  for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
    FreeStat(outer_blocks[spectrum]);
    FreeStat(outer_blocks_ratios[spectrum]);
    FreeStat(sunlit_blocks[spectrum]);
  }

  return;
}

/* ------------------------------------------------------------------------- */

void ShootRaysFromObjects(QUERY *query, Cmodule_type *comm_module)
/* shoot rays from the polygon in "query" and determine amount of light reaching
   it */
{
  RAY ray;
  float ldir[3]; // light direction
  unsigned int i, hits;
  float num, temp;
  int spectrum;

  // NOTE: virtual sensors should not have material properties, only a
  // scattering exponent! because virtual sensors do not absorb light! so return
  // type cannot be F or D, as those are for absorbed light

  // determine the number of rays
  num = (float)npoints;

  hits = 0;
  for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
    comm_module->params[spectrum].value = 0;
    total_spectrum_flux[spectrum] = 0.f;
  }

  if (one_ray_per_spectrum) {
    ResetQMC();
    randqmc = GenRandom();

    for (i = (unsigned int)num; i > 0; i--) {
      pointqmc = QMC();
      rpointqmc = AppRandom(pointqmc, randqmc);
      ResetU01();

      GenerateRayFromSensor(&ray, ldir, query, 0);
      temp = ray.intensity[0];
      // save total available light and set ray intensity
      for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
        if (return_type[spectrum] != LW_INCIDENT_IRRADIANCE)
          total_spectrum_flux[spectrum] += temp;
        else
          temp = 1.0;

        // adjust ray intensity according to angle between its direction and
        // surface's normal this is the normalizing term so a flat surface at
        // top of canopy gets 1 W/m^2 taking the abs() is necessary to avoid
        // multiplying by negative numbers when the light is below the surface
        // (this could change if we are only interested in the top side of the
        // sensor)
        ray.intensity[spectrum] = temp *
                                  (float)fabs(DotProduct(ray.dir, query->up)) *
                                  source_spectrum[spectrum].weight;
      }

      // if (FindBoxIntersection (&scene, &ray))
      {
        if (verbose >= 3) {
          fprintf(stderr, "\n%s - tracing ray %d for %d spectrum samples:\n",
                  proc_name, i, spectrum);
          fprintf(stderr, "\t(%g, %g, %g) + t*(%g, %g, %g)\n", ray.pt[X],
                  ray.pt[Y], ray.pt[Z], ray.dir[X], ray.dir[Y], ray.dir[Z]);
          glDisplayRay(ray);
        }
        // the 2nd parameter is arbitrary because all wavelengths are evaluated
        hits += TraceRay(&ray, 0);
      }
      for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
        if (return_type[spectrum] != LW_INCIDENT_IRRADIANCE) {
          comm_module->params[spectrum].value +=
              ray.intensity[spectrum] * (float)fabs(DotProduct(ldir, ray.dir));
        } else {
          if (use_sky_model) {
            total_spectrum_flux[spectrum] = 1.0; // GetSkyIntensity (ray.dir);
            comm_module->params[spectrum].value +=
                ray.intensity[spectrum] * GetSkyIntensity(ray.dir);
          } else
            fprintf(stderr,
                    "QuasiMC - shoot rays from lower surface of virtual sensor "
                    "not implemented for light source\n");
        }

      if (verbose >= 3)
        fprintf(stderr, "%s - ray %d killed\n", proc_name, i);
    }
  } else {
    // for each wavelength
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
      ResetQMC();
      randqmc = GenRandom();

      // for number of rays
      for (i = (unsigned int)num; i > 0; i--) {
        pointqmc = QMC();
        rpointqmc = AppRandom(pointqmc, randqmc);
        ResetU01();

        GenerateRayFromSensor(&ray, ldir, query, spectrum);
        if (return_type[spectrum] != LW_INCIDENT_IRRADIANCE)
          total_spectrum_flux[spectrum] += ray.intensity[spectrum];
        else
          ray.intensity[spectrum] = 1.0;
        // adjust ray intensity according to angle between its direction and
        // surface's normal taking the abs() is necessary to avoid multiplying
        // by negative numbers when the light is below the surface (this could
        // change if we are only interested in the top side of the sensor)
        ray.intensity[spectrum] *= (float)fabs(DotProduct(ray.dir, query->up)) *
                                   source_spectrum[spectrum].weight;

        // I turned off findboxintersection for rays_from_objects because
        // it didn't work.  instead i've included the sensors in the bounding
        // box and sphere of the scene, so rays generated from sensors already
        // intersect the bounding box.
        // BUT do the sensors really have to be included in bounding box?
        // The senors are never tested for intersection anyway...
        // if (FindBoxIntersection (&scene, &ray))
        //{
        if (verbose >= 3) {
          fprintf(stderr, "\n%s - tracing ray %d for spectrum %d:\n", proc_name,
                  i, spectrum + 1);
          fprintf(stderr, "\t(%g, %g, %g) + t*(%g, %g, %g)\n", ray.pt[X],
                  ray.pt[Y], ray.pt[Z], ray.dir[X], ray.dir[Y], ray.dir[Z]);
          fprintf(stderr, "\tlight dir: %g, %g, %g\n", ldir[X], ldir[Y],
                  ldir[Z]);
          fprintf(stderr, "\tintensity: %g\n", ray.intensity[spectrum]);
          glDisplayRay(ray);
        }

        hits += TraceRay(&ray, spectrum);
        if (return_type[spectrum] != LW_INCIDENT_IRRADIANCE) {
          // adjust ray intensity according to angle between final ray direction
          // and the original shooting direction
          comm_module->params[spectrum].value +=
              ray.intensity[spectrum] * (float)fabs(DotProduct(ldir, ray.dir));
        } else {
          if (use_sky_model) {
            total_spectrum_flux[spectrum] = 1.0; // GetSkyIntensity (ray.dir);
            comm_module->params[spectrum].value +=
                ray.intensity[spectrum] * GetSkyIntensity(ray.dir);
          } else
            fprintf(stderr,
                    "QuasiMC - shoot rays from lower surface of virtual sensor "
                    "not implemented for light source\n");
        }

        if (verbose >= 3)
          fprintf(stderr, "%s - ray %d killed\n", proc_name, i);
      }
    }
  }

  for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
    // save the radiant flux for this query module so it can be used in the
    // visualization
    query->radiant_flux[spectrum] = comm_module->params[spectrum].value;

    if (total_spectrum_flux[spectrum] > 0.f) {
      // check if QuasiMC should return absorbed irradiance or number of
      // intersections
      if (return_type[spectrum] == NUM_INTERSECTIONS)
        comm_module->params[spectrum].value =
            hits / total_spectrum_flux[spectrum];
      else if (return_type[spectrum] == ABSORBED_IRRADIANCE)
        comm_module->params[spectrum].value /= total_spectrum_flux[spectrum];
      else {
        comm_module->params[spectrum].value /= total_spectrum_flux[spectrum];
        // fprintf (stderr, "%s - warning: 'return type' should be 'D' (absorbed
        // flux density) for sensor\n", proc_name);
        // the return type for a sensor cannot be 'absorbed' anything! sensors
        // don't absorb light.
      }
    } else
      comm_module->params[spectrum].value = 0.f;
  }

  return;
}

/* ------------------------------------------------------------------------- */

void GenerateRayFromSensor(RAY *ray, float *ldir, QUERY *query, int spectrum)
// generate a ray from a virtual sensor
// if directional light source is used, generate a ray towards the light source
// if the sky model is used, generate a ray in the direction of the sensor's
// normal WHAT ABOUT DIFFERENT LOCAL LIGHT MODELS?
{
  LIGHT *light;
  float uvec[3], vvec[3];
  float r1, r2;

  ray->depth = 0;

  r1 = RandU01(RANDQMC_START);
  r2 = RandU01(RANDQMC_START);
  ray->pt[X] = query->pos[X] + r1 * query->edge1[X] + r2 * query->edge2[X];
  ray->pt[Y] = query->pos[Y] + r1 * query->edge1[Y] + r2 * query->edge2[Y];
  ray->pt[Z] = query->pos[Z] + r1 * query->edge1[Z] + r2 * query->edge2[Z];

  if (use_sky_model) {
    if (return_type[spectrum] == LW_INCIDENT_IRRADIANCE) {
      ldir[0] = -query->up[0];
      ldir[1] = -query->up[1];
      ldir[2] = -query->up[2];
    } else
      ray->intensity[spectrum] = GetSkyDirection(ldir, uvec, vvec);
    if (reflectance_model == LAMBERTIAN) {
      GenerateRandomDirection_Lambertian(ldir, query->exponent[spectrum],
                                         ray->dir, ldir, sensor_fov);
      ray->intensity[spectrum] = GetSkyIntensity(ray->dir);
    } else {
      // ensure the scattering exponent is within a reasonable range
      if (query->exponent[spectrum] >= 0.f &&
          query->exponent[spectrum] < MAX_SCATTERING_EXPONENT) {
        if (reflectance_model == BLINN_PHONG)
          GenerateRandomDirection_mPhong(ldir, query->exponent[spectrum],
                                         ray->dir, query->up);
        else if (reflectance_model == PHONG)
          GenerateRandomDirection_Phong(ldir, query->exponent[spectrum],
                                        ray->dir, query->up);
        ray->intensity[spectrum] = GetSkyIntensity(ray->dir);
      } else {
        ray->dir[0] = ldir[0];
        ray->dir[1] = ldir[1];
        ray->dir[2] = ldir[2];
      }
    }
  } else {
    light = RandomLight(&scene);
    ldir[0] = -light->dir[0];
    ldir[1] = -light->dir[1];
    ldir[2] = -light->dir[2];
    Normalize(ldir);
    if (reflectance_model == LAMBERTIAN) {
      GenerateRandomDirection_Lambertian(ldir, query->exponent[spectrum],
                                         ray->dir, ldir, sensor_fov);
      // if the ray's direction is below the surface, set the intensity to zero
      if (DotProduct(ray->dir, query->up) <= 0.0)
        ray->intensity[spectrum] = 0.0;
      else
        // adjust ray intensity by the angle between its direction and the new
        // random direction about it because in the direction of the light the
        // intensity is 1 otherwise it is less than 1
        ray->intensity[spectrum] = (float)fabs(DotProduct(ldir, ray->dir));
    } else {
      // Generate a random ray direction based on the light ray and scattering
      // exponent
      if (query->exponent[spectrum] >= 0.f &&
          query->exponent[spectrum] < MAX_SCATTERING_EXPONENT) {
        if (reflectance_model == BLINN_PHONG)
          GenerateRandomDirection_mPhong(ldir, query->exponent[spectrum],
                                         ray->dir, query->up);
        else if (reflectance_model == PHONG)
          GenerateRandomDirection_Phong(ldir, query->exponent[spectrum],
                                        ray->dir, query->up);

        // if the ray's direction is below the surface, set the intensity to
        // zero
        if (DotProduct(ray->dir, query->up) <= 0.0)
          ray->intensity[spectrum] = 0.0;
        else
          // adjust ray intensity by the angle between its direction and the
          // random direction about it because pointing straight at light has
          // intensity 1 otherwise it is less than 1
          ray->intensity[spectrum] = (float)fabs(DotProduct(ldir, ray->dir));
      } else {
        // generate a ray in direction of light
        ray->dir[0] = ldir[0];
        ray->dir[1] = ldir[1];
        ray->dir[2] = ldir[2];
        ray->intensity[spectrum] = 1.0;
      }
    }
  }
  return;
}

/* ------------------------------------------------------------------------- */

void ShootAllRays(void)
/* shoot rays from the light sources */
{
  RAY ray;
  float num;
  int spectrum;
  unsigned int i, hits, rays;

  hits = rays = 0;

  if (one_ray_per_spectrum) {
    ResetQMC();
    randqmc = GenRandom();

    /* determine the number of rays */
    num = (float)npoints;
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      total_spectrum_flux[spectrum] = 0.f;

    for (i = 0; i < (unsigned int)num; i++) {
      pointqmc = QMC();
      rpointqmc = AppRandom(pointqmc, randqmc);
      ResetU01();

      GenerateRandomRay(&ray);
      for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
        total_spectrum_flux[spectrum] += ray.intensity[spectrum];
      ++rays;

      if (FindBoxIntersection(&scene, &ray)) {
        if (verbose >= 3) {
          fprintf(stderr, "\n%s - tracing ray %d for %d spectrum samples:\n",
                  proc_name, i, spectrum_samples);
          fprintf(stderr, "\t(%g, %g, %g) + t*(%g, %g, %g)\n", ray.pt[X],
                  ray.pt[Y], ray.pt[Z], ray.dir[X], ray.dir[Y], ray.dir[Z]);
          glDisplayRay(ray);
        }

        // the 2nd parameter is not used because one ray for entire spectrums is
        // used
        hits += TraceRay(&ray, 0);

        if (verbose >= 3)
          fprintf(stderr, "%s - ray %d killed\n", proc_name, i);
      } else if (verbose >= 3)
        fprintf(stderr, "%s - ray %d does not intersect bounding box\n",
                proc_name, i);
    }
    // OLD CODE: this division is now done in the DetermineResponse function
    // for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
    //  spectrum_density[spectrum] /= M_PIf * scene.grid.bsph_radius *
    //  scene.grid.bsph_radius;
  } else {
    num = (float)npoints;

    // for each wavelength
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
      ResetQMC();
      randqmc = GenRandom();

      /* FROM OLD VERSION: determine the ray 'density' according to  */
      // spectrum_density[spectrum] = num / (M_PIf * scene.grid.bsph_radius *
      // scene.grid.bsph_radius);
      /* THIS IS THE NEW VERSION: */
      // spectrum density is initialized to zero, and incremented by the amount
      // of available light
      total_spectrum_flux[spectrum] = 0.f;

      // for the number of rays specfied by user do
      for (i = 0; i < (unsigned int)num; i++) {
        pointqmc = QMC();
        rpointqmc = AppRandom(pointqmc, randqmc);
        ResetU01();

        GenerateRandomRay(&ray);
        // increment the spectrum density by amount of available light
        // NOTE: do not incorporate direction of incoming light with respect to
        // zenith for multiple lights, because the returned radiant flux is
        // averaged over the directional light sources
        total_spectrum_flux[spectrum] += ray.intensity[spectrum];
        ++rays;

        if (FindBoxIntersection(&scene, &ray)) {
          if (verbose >= 3) {
            fprintf(stderr,
                    "\n%s - tracing ray %d for spectrum %d, with energy %g:\n",
                    proc_name, i, spectrum + 1, ray.intensity[spectrum]);
            fprintf(stderr, "\t(%g, %g, %g) + t*(%g, %g, %g)\n", ray.pt[X],
                    ray.pt[Y], ray.pt[Z], ray.dir[X], ray.dir[Y], ray.dir[Z]);
            glDisplayRay(ray);
          }

          hits += TraceRay(&ray, spectrum);

          if (verbose >= 3)
            fprintf(stderr, "%s - ray %d killed\n", proc_name, i);
        } else if (verbose >= 3)
          fprintf(stderr, "%s - ray %d does not intersect bounding box\n",
                  proc_name, i);
      }
      // OLD CODE:divide available light by area of projected bounding sphere
      // this is now done in the DetermineResponse function
      // spectrum_density[spectrum] /= M_PIf * scene.grid.bsph_radius *
      // scene.grid.bsph_radius;
    }
  }

  if (verbose >= 1)
    fprintf(stderr, "%s - number of intersections %d from %g rays\n", proc_name,
            hits, num);

  return;
}

/* ------------------------------------------------------------------------- */

int TraceRay(RAY *ray, int spectrum)
// trace the ray through the scene
{
  PRIMITIVE *intersected, *prim;
  CELL *cell;
  float norm[3], minnorm[3];
  float dist, mindist;
  int i, done, num_intersections, last_intersected_index;

  done = num_intersections = 0;
  last_intersected_index = -1;
  while (!done) {
    prim = NULL;
    intersected = NULL;
    mindist = 1e30f;

    cell = FindFirstCell(&scene, ray);
    if (++ray->signature == 0)
      ray->signature = 1;

    // loop through all the cells (voxels) in the subdivided space
    while (cell != NULL) {
      // for each primitive in this cell
      for (i = 0; i < cell->num_primitives; i++) {
        // first ensure we don't check the last intersected primitive (from a
        // reflection or a transmission)
        if (last_intersected_index != cell->list[i]) {
          // check for an intersection and compute the distance between starting
          // point and intersection point
          dist =
              IsIntersection(&scene, ray, cell->list[i], mindist, norm, &prim);

          // if newly computed distance is the minimum, save the primitive as
          // the "intersected" primitive
          if (dist >= 0.0 && dist < mindist) {
            mindist = dist;
            intersected = prim;
            last_intersected_index = cell->list[i];
            minnorm[X] = norm[X];
            minnorm[Y] = norm[Y];
            minnorm[Z] = norm[Z];
          }
        }
      }
      // march to the next cell (voxel) in the subdivided space and get list of
      // primitives
      cell = FindNextCell(&scene, ray);
    }

    // if there was an intersection, resolve it by generating
    // reflected/transmitted ray
    if (intersected) {
      // save intersection info
      ++num_intersections;

      if (one_ray_per_spectrum) {
        if (!ResolveIntersectionOne(ray, minnorm, mindist, intersected))
          done = 1;
      } else {
        if (!ResolveIntersection(ray, minnorm, mindist, intersected, spectrum))
          done = 1;
      }
    } else
      done = 1;
  }

  return (num_intersections);
}

/* ------------------------------------------------------------------------- */

int ResolveIntersection(RAY *ray, float *normal, float mindist,
                        PRIMITIVE *intersected, int spectrum)
/* generates a new ray for the intersection of ray and object
   returns 0 if ray is dead else 1 */
{
  float dir[3];
  float cosAi, reflected, radiant, n;
  int side;

  // find side that was hit
  cosAi = -DotProduct(normal, ray->dir);
  side = (cosAi > 0.0) ? 0 : 1;

  // save material properties
  reflected = intersected->material[side]->reflectance[spectrum];
  radiant = reflected + intersected->material[side]->transmittance[spectrum];

  if (verbose >= 3) {
    fprintf(stderr, "%s - hit at depth %d\n", proc_name, ray->depth);
    fprintf(stderr, "%s - hit material side: %d, with absorptance: %g\n",
            proc_name, side, 1.0 - radiant);
    fprintf(stderr, "%s - ray intensity: %g\n", proc_name,
            ray->intensity[spectrum]);
  }

  // play Russian roulette - Arvo and Kirk
  // having both max depth test and Russian roulette is a bit strange, since the
  // latter works in the limit anyway this must be before max depth test,
  // because it adjust the intensity values
  if (russian_roulette.threshold > 0.0)
    if (ray->intensity[spectrum] <= russian_roulette.threshold) {
      if (RandU01(RANDQMC_RR) < russian_roulette.prob)
        // radiant = 0.f; // This was incorrect!
        // Do not add the radiant flux of the ray to the object if it is
        // terminated
        return (0);
      else
        ray->intensity[spectrum] /= (1.0f - russian_roulette.prob);
    }

  // absorb some of the light
  if ((!no_direct_light) || (ray->depth > 0)) {
    intersected->absorbed_flux[spectrum] +=
        ray->intensity[spectrum] * (1.0f - radiant);
    if ((return_type[spectrum] == UP_INCIDENT_IRRADIANCE && side == 0) ||
        (return_type[spectrum] == LW_INCIDENT_IRRADIANCE && side == 1))
      intersected->incident_flux[spectrum] += ray->intensity[spectrum];
    // count direct hits
    if (ray->depth == 0)
      intersected->direct_hits[spectrum] += 1.0f;
  }

  // the ray's new intensity = the intensity that was not absorbed
  ray->intensity[spectrum] *= radiant;

  if (radiant <= 0.0)
    return (0);

  // after setting flux above, check if we need to generate a
  // reflected/transmitted ray
  ++ray->depth;
  // check max depth of ray
  // if (max_depth >= 0) // this check is now done in args.c
  if (ray->depth >= max_depth)
    return (0);

  // get new ray origin  - added a boundary check
  ray->pt[X] = fabs(ray->pt[X] + mindist * ray->dir[X]) < EPSILON
                   ? 0.0f
                   : ray->pt[X] + mindist * ray->dir[X];
  ray->pt[Y] = fabs(ray->pt[Y] + mindist * ray->dir[Y]) < EPSILON
                   ? 0.0f
                   : ray->pt[Y] + mindist * ray->dir[Y];
  ray->pt[Z] = fabs(ray->pt[Z] + mindist * ray->dir[Z]) < EPSILON
                   ? 0.0f
                   : ray->pt[Z] + mindist * ray->dir[Z];

  // switch normal if it points the same way as ray dir
  if (cosAi < 0.0) {
    cosAi = -cosAi;
    normal[X] = -normal[X];
    normal[Y] = -normal[Y];
    normal[Z] = -normal[Z];
  }

  // choose reflected or transmitted ray
  if ((RandU01(RANDQMC_RT) * radiant) < reflected) {
    if (verbose >= 3)
      fprintf(stderr, "%s - spawning reflected ray:\n", proc_name);

    n = intersected->material[side]->spec_power[spectrum];

    // ideal reflection
    ray->dir[X] += 2.0f * normal[X] * cosAi;
    ray->dir[Y] += 2.0f * normal[Y] * cosAi;
    ray->dir[Z] += 2.0f * normal[Z] * cosAi;
  } else {
    if (verbose >= 3)
      fprintf(stderr, "%s - spawning transmitted ray:\n", proc_name);

    n = intersected->material[side]->trans_power[spectrum];

    normal[X] = -normal[X];
    normal[Y] = -normal[Y];
    normal[Z] = -normal[Z];

    // since no refraction is assumed, ray direction stays the same
  }

  // if the scattering exponent is in an acceptable range, generate a new
  // direction
  if (n >= 0.f && n < MAX_SCATTERING_EXPONENT) {
    // apply the local light model
    if (reflectance_model == BLINN_PHONG)
      GenerateRandomDirection_mPhong(ray->dir, n, dir, normal);
    else if (reflectance_model == PHONG)
      GenerateRandomDirection_Phong(ray->dir, n, dir, normal);
    else
      GenerateRandomDirection_Lambertian(ray->dir, n, dir, normal, 0.0);

    if (DotProduct(dir, normal) < 0.0) {
      fprintf(stderr,
              "%s - Precision error! Did not generate correct ray after "
              "intersection\n",
              proc_name);
    }

    ray->dir[X] = dir[X];
    ray->dir[Y] = dir[Y];
    ray->dir[Z] = dir[Z];
  }

  if (verbose >= 3) {
    fprintf(stderr, "%s - new ray intensity: %g\n", proc_name,
            ray->intensity[spectrum]);
    fprintf(stderr, "(%g, %g, %g) + %g * (%g, %g, %g)\n", ray->pt[X],
            ray->pt[Y], ray->pt[Z], mindist, ray->dir[X], ray->dir[Y],
            ray->dir[Z]);
  }

  return (1);
}

/* ------------------------------------------------------------------------- */

int ResolveIntersectionOne(RAY *ray, float *normal, float mindist,
                           PRIMITIVE *intersected)
/* generates a new ray for the intersection of ray and object
   but the ray is carrying all of the wavelengths
   returns 0 if rays is dead else 1 */
{
  float dir[3];
  float cosAi, n, /*n2,*/ sumR, sumT, theta, rnd;
  float reflected[MAX_SPECTRUM_SAMPLES];
  float transmitted[MAX_SPECTRUM_SAMPLES];
  float radiant[MAX_SPECTRUM_SAMPLES];
  int side, spectrum, ray_type; // ray type = 0 reflected or 1 is transmitted
  /* find side that was hit */
  cosAi = -DotProduct(normal, ray->dir);
  side = cosAi > 0 ? 0 : 1;

  if (verbose >= 3) {
    fprintf(stderr, "%s - hit at depth %d\n", proc_name, ray->depth);
    fprintf(stderr, "%s - hit material side: %d\n", proc_name, side);
    fprintf(stderr, "%s - ray intensities: ", proc_name);
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      fprintf(stderr, "%g ", ray->intensity[spectrum]);
    fprintf(stderr, "\n");
  }

  for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
    reflected[spectrum] = intersected->material[side]->reflectance[spectrum];
    transmitted[spectrum] =
        intersected->material[side]->transmittance[spectrum];

    radiant[spectrum] = reflected[spectrum] + transmitted[spectrum];
  }

  // play Russian roulette
  if (russian_roulette.threshold > 0.0) {
    n = 0.0;
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      n += ray->intensity[spectrum];

    if (n <= russian_roulette.threshold * ((float)spectrum_samples)) {
      if (RandU01(RANDQMC_RR) < russian_roulette.prob) {
        // Do not add the radiant flux of the ray to the object if it is
        // terminated
        // memset (radiant, 0, sizeof(float) * MAX_SPECTRUM_SAMPLES);
        return (0);
      } else
        for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
          ray->intensity[spectrum] /= (1 - russian_roulette.prob);
    }
  }

  // absorb some of the intensity
  if ((!no_direct_light) || (ray->depth > 0))
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++) {
      intersected->absorbed_flux[spectrum] +=
          ray->intensity[spectrum] * (1.0f - radiant[spectrum]);
      if ((return_type[spectrum] == UP_INCIDENT_IRRADIANCE && side == 0) ||
          (return_type[spectrum] == LW_INCIDENT_IRRADIANCE && side == 1))
        intersected->incident_flux[spectrum] += ray->intensity[spectrum];
      // count direct hits
      if (ray->depth == 0)
        intersected->direct_hits[spectrum] += 1.0f;
    }

  // adjust the intensities after the absorption
  for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
    ray->intensity[spectrum] *= radiant[spectrum];

  // after setting flux above, check if we need to generate a
  // reflected/transmitted ray
  ++ray->depth;
  // check max depth of ray
  // if (max_depth >= 0) // this is done in args.c
  if (ray->depth >= max_depth)
    return (0);

  sumR = sumT = 0.0;
  for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
    if (radiant[spectrum] > 0.0) {
      sumR += ray->intensity[spectrum] * reflected[spectrum];
      sumT += ray->intensity[spectrum] * transmitted[spectrum];
    }

  if (sumT + sumR <= 0.0)
    // no reflected or transmitted ray
    return (0);

  // get new ray origin
  ray->pt[X] = ray->pt[X] + mindist * ray->dir[X];
  ray->pt[Y] = ray->pt[Y] + mindist * ray->dir[Y];
  ray->pt[Z] = ray->pt[Z] + mindist * ray->dir[Z];

  // switch normal if it points the same way as ray dir
  if (cosAi < 0.0) {
    cosAi = -cosAi;
    normal[X] = -normal[X];
    normal[Y] = -normal[Y];
    normal[Z] = -normal[Z];
  }

  // choose reflected or transmitted ray
  if ((RandU01(RANDQMC_RT) * (sumR + sumT)) < sumR) {
    if (verbose >= 3)
      fprintf(stderr, "%s - spawning reflected ray:\n", proc_name);

    ray_type = 0;

    /* choose n based on weighted reflected intensities */
    rnd = sumR * RandU01(RANDQMC_N);
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      if (radiant[spectrum] > 0.0)
        if ((rnd -= (ray->intensity[spectrum] * reflected[spectrum])) <= 0.0)
          break;

    n = intersected->material[side]->spec_power[spectrum];

    // adjust ray intensity according to optical properties of both wavelengths
    // (ignore intensity that was transmitted) PROBLEM: is it safe to ignore
    // this light energy? ANSWER: NO, so that is why there is a division by
    // probability factor: sumR/(sumR+sumT) to account for ignored radiant
    // energy
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      if (radiant[spectrum] > 0.0)
        ray->intensity[spectrum] *=
            (reflected[spectrum] / radiant[spectrum]) / (sumR / (sumR + sumT));

    ray->dir[X] += 2.0f * normal[X] * cosAi;
    ray->dir[Y] += 2.0f * normal[Y] * cosAi;
    ray->dir[Z] += 2.0f * normal[Z] * cosAi;
  } else {
    if (verbose >= 3)
      fprintf(stderr, "%s - spawning transmitted ray:\n", proc_name);

    ray_type = 1;

    /* choose n based on weighted refractive intensities */
    rnd = sumT * RandU01(RANDQMC_N);
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      if (radiant[spectrum] > 0.0) {
        if ((rnd -= (ray->intensity[spectrum] * transmitted[spectrum])) <= 0.0)
          break;
      }
    // save the scattering exponent, n, that was chosen
    n = intersected->material[side]->trans_power[spectrum];

    normal[X] = -normal[X];
    normal[Y] = -normal[Y];
    normal[Z] = -normal[Z];

    // adjust ray intensity according to optical properties of both wavelengths
    // (ignore intensity that was reflected) PROBLEM: is it safe to ignorre this
    // light energy? ANSWER: NO, so that is why there is a division by
    // probability factor: sumR/(sumR+sumT) to account for ignored radiant
    // energy
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      if (radiant[spectrum] > 0.0)
        ray->intensity[spectrum] *=
            (transmitted[spectrum] / radiant[spectrum]) /
            (sumT / (sumR + sumT));

    /* since no refraction is assumed, ray direction stays the same */
  }

  // if the scattering exponent is in an acceptable range, generate a new
  // direction
  if (n >= 0.f && n < MAX_SCATTERING_EXPONENT) {
    // apply the local light model
    if (reflectance_model == BLINN_PHONG)
      theta = GenerateRandomDirection_mPhong(ray->dir, n, dir, normal);
    else if (reflectance_model == PHONG)
      theta = GenerateRandomDirection_Phong(ray->dir, n, dir, normal);
    else
      theta = GenerateRandomDirection_Lambertian(ray->dir, n, dir, normal, 0.0);

    if (DotProduct(dir, normal) < 0.0) {
      fprintf(stderr,
              "%s - Precision error. Did not generate correct ray after "
              "intersection\n",
              proc_name);
    }

    ray->dir[X] = dir[X];
    ray->dir[Y] = dir[Y];
    ray->dir[Z] = dir[Z];
  } else {
    theta = 0.f;
  }


  if (verbose >= 3) {
    fprintf(stderr, "%s - new ray intensities: ", proc_name);
    for (spectrum = 0; spectrum < spectrum_samples; spectrum++)
      fprintf(stderr, "%g ", ray->intensity[spectrum]);
    fprintf(stderr, "\n(%g, %g, %g) + t*(%g, %g, %g)\n", ray->pt[X], ray->pt[Y],
            ray->pt[Z], ray->dir[X], ray->dir[Y], ray->dir[Z]);
  }

  return (1);
}

/* ------------------------------------------------------------------------- */

float GenerateRandomDirection_mPhong(float *outgoing, float n, float *dir,
                                     float *normal)
/* generate random direction for ray based on ideally reflected ray */
{
  float theta, phi, psi, udotn, sin_theta, cos_theta, r1, angleON;
  static float vec[3], u[3], v[3], w[3];

  // if exponent is < 0, do nothing
  if (n < 0.f)
    return (0.f);

  // form a basis
  w[0] = outgoing[0];
  w[1] = outgoing[1];
  w[2] = outgoing[2];
  Normalize(w);

  angleON = DotProduct(w, normal);
  // check if normal is not more than 90 degrees away from outgoing ray
  if (angleON < 0.f) {
    normal[0] = -normal[0];
    normal[1] = -normal[1];
    normal[2] = -normal[2];
    angleON = DotProduct(w, normal);
  }
  if (angleON > 1.f)
    angleON = 1.f;

  angleON = M_PIf / 2.0f - acosf(angleON);
  psi = DotProduct(w, normal);

  // find the left and up vectors of the basis at point of intersection
  if ((psi >= 1.f - EPSILON) && (psi <= 1.f + EPSILON)) {
    vec[0] = w[0];
    vec[1] = w[1];
    vec[2] = w[2];
    vec[MIN3Di(fabs(w[0]), fabs(w[1]), fabs(w[2]))] = 1.f;
    CrossProduct(vec, w, u);
    Normalize(u);
    CrossProduct(w, u, v);
  } else {
    CrossProduct(w, normal, v);
    Normalize(v);
    CrossProduct(w, v, u);
  }

  udotn = DotProduct(u, normal);

  // modified Phong-BRDF - Shirely and Wang
  r1 = RandU01(RANDQMC_THETA) *
       (1.0f - powf(cosf(0.5f * (M_PIf - angleON)), n + 2.f));
  // OLD version:  theta = (2.f * acosf(powf(1.f - r1, 1.f / (n + 2.f))));
  // NEW version: checks to ensure the parameter to acosf is between -1 and 1
  theta = powf(1.f - r1, 1.f / (n + 2.f));
  theta = theta < -1.f ? -1.f : theta > 1.f ? 1.f : theta;
  theta = 2.0f * (acosf(theta));

  sin_theta = sinf(theta);
  cos_theta = cosf(theta);

  if (theta < angleON)
    phi = 2.f * M_PIf * RandU01(RANDQMC_PHI);
  else {
    // OLD version: psi = acosf(tanf(angleON) * cos_theta / sin_theta);
    // NEW version: checks to ensure the parameter to acosf is between -1 and 1
    psi = tanf(angleON) * cos_theta / sin_theta;
    psi = psi < -1.f ? -1.f : psi > 1.f ? 1.f : psi;
    psi = acosf(psi);

    phi = 2.f * (M_PIf - psi) * RandU01(RANDQMC_PHI) + psi;
    if (udotn > 0.0)
      phi -= M_PIf;
  }

  // change to cartesian coordinates
  vec[X] = cosf(phi) * sin_theta;
  vec[Y] = sinf(phi) * sin_theta;
  vec[Z] = cos_theta;

  if (verbose >= 3)
    fprintf(stderr,
            "\tangleON = %g, phi = %g, theta = %g\n\tvec = (%g, %g, %g).\n",
            angleON / M_PI * 180.0, phi / M_PI * 180.0, theta / M_PI * 180.0,
            vec[0], vec[1], vec[2]);

  dir[X] = vec[X] * u[X] + vec[Y] * v[X] + vec[Z] * w[X];
  dir[Y] = vec[X] * u[Y] + vec[Y] * v[Y] + vec[Z] * w[Y];
  dir[Z] = vec[X] * u[Z] + vec[Y] * v[Z] + vec[Z] * w[Z];

  return (theta);
}

/* ------------------------------------------------------------------------- */

float GenerateRandomDirection_Phong(float *outgoing, float n, float *dir,
                                    float *normal)
/* generate random direction for ray based on ideally reflected ray */
{
  float theta, phi, psi, udotn, sin_theta, cos_theta, r1, angleON;
  static float vec[3], u[3], v[3], w[3];

  // if exponent is < 0, do nothing
  if (n < 0.f)
    return (0.f);

  // form a basis
  w[0] = outgoing[0];
  w[1] = outgoing[1];
  w[2] = outgoing[2];
  Normalize(w);

  // check if normal is not more than 90 degrees away from outgoing ray
  if (DotProduct(w, normal) < 0.f) {
    normal[0] = -normal[0];
    normal[1] = -normal[1];
    normal[2] = -normal[2];
  }

  // find the left and up vectors of the basis at point of intersection
  angleON = DotProduct(w, normal);
  if ((angleON >= 1.f - EPSILON) && (angleON <= 1.f + EPSILON)) {
    vec[0] = w[0];
    vec[1] = w[1];
    vec[2] = w[2];
    vec[MIN3Di(fabs(w[0]), fabs(w[1]), fabs(w[2]))] = 1.f;
    CrossProduct(vec, w, u);
    Normalize(u);
    CrossProduct(w, u, v);
  } else {
    CrossProduct(w, normal, v);
    Normalize(v);
    CrossProduct(w, v, u);
  }

  udotn = DotProduct(u, normal);

  // THIS DOESN'T WORK WHEN N = 0???
  // Phong pdf: (n+1)/2pi * cos(theta)^n
  //  theta = acosf(powf(1.f- RandU01(RANDQMC_THETA), 1.f/(n+1.f)));
  r1 = RandU01(RANDQMC_THETA) * (1.0f - powf(cosf(M_PIf - angleON), n + 1.f));
  theta = powf(1.f - r1, 1.f / (n + 1.f));
  theta = theta < -1.f ? -1.f : theta > 1.f ? 1.f : theta;
  theta = acosf(theta);

  sin_theta = sinf(theta);
  cos_theta = cosf(theta);

  if (theta < angleON)
    phi = 2.f * M_PIf * RandU01(RANDQMC_PHI);
  else {
    psi = tanf(angleON) * cos_theta / sin_theta;
    psi = psi < -1.f ? -1.f : psi > 1.f ? 1.f : psi;
    psi = acosf(psi);

    phi = 2.f * (M_PIf - psi) * RandU01(RANDQMC_PHI) + psi;
    if (udotn > 0.0)
      phi -= M_PIf;
  }

  // change to cartesian coordinates
  vec[X] = cosf(phi) * sin_theta;
  vec[Y] = sinf(phi) * sin_theta;
  vec[Z] = cos_theta;

  if (verbose >= 3)
    fprintf(stderr,
            "\tangleON = %g, phi = %g, theta = %g\n\tvec = (%g, %g, %g).\n",
            angleON / M_PI * 180.0, phi / M_PI * 180.0, theta / M_PI * 180.0,
            vec[0], vec[1], vec[2]);

  dir[X] = vec[X] * u[X] + vec[Y] * v[X] + vec[Z] * w[X];
  dir[Y] = vec[X] * u[Y] + vec[Y] * v[Y] + vec[Z] * w[Y];
  dir[Z] = vec[X] * u[Z] + vec[Y] * v[Z] + vec[Z] * w[Z];

  return (theta);
}

/* ------------------------------------------------------------------------- */

float GenerateRandomDirection_Lambertian(__attribute__((unused))float *outgoing, __attribute__((unused))float n, float *dir,
                                         float *normal, float fov)
/* generate random direction for ray based on ideally reflected ray */
// fov is the field of view if a virtual sensor is used (should be 0.0
// otherwise)
{
  static float vec[3], u[3], v[3], w[3];
  float e1, e2;

  // use normal for outgoing ray of the Lambertian model
  w[0] = normal[0];
  w[1] = normal[1];
  w[2] = normal[2];
  Normalize(w);

  // find the left and up vectors of the basis at point of intersection
  vec[0] = w[0];
  vec[1] = w[1];
  vec[2] = w[2];
  vec[MIN3Di(fabs(w[0]), fabs(w[1]), fabs(w[2]))] = 1.f;
  CrossProduct(vec, w, u);
  Normalize(u);
  CrossProduct(w, u, v);

  // Lambertian local light model
  e1 = RandU01(RANDQMC_THETA) * (1.f - fov) + fov;
  e2 = RandU01(RANDQMC_PHI);
  vec[X] = sqrtf(1.f - e1) * cosf(2.f * M_PIf * e2);
  vec[Y] = sqrtf(1.f - e1) * sinf(2.f * M_PIf * e2);
  vec[Z] = sqrtf(e1);

  dir[X] = vec[X] * u[X] + vec[Y] * v[X] + vec[Z] * w[X];
  dir[Y] = vec[X] * u[Y] + vec[Y] * v[Y] + vec[Z] * w[Y];
  dir[Z] = vec[X] * u[Z] + vec[Y] * v[Z] + vec[Z] * w[Z];

  return (0);
}

/* ------------------------------------------------------------------------- */

void ToUnitDisk(float *u, float *v)
/* concentric mapping from [-1,1]^2 to unit disk - from Realistic Ray Tracing */
{
  float a, b, r, phi;

  a = 2.0f * RandU01(RANDQMC_START) - 1.0f;
  b = 2.0f * RandU01(RANDQMC_START) - 1.0f;

  if (a > -b) {
    if (a > b) {
      r = a;
      phi = (M_PIf / 4.0f) * (b / a);
    } else {
      r = b;
      phi = (M_PIf / 4.0f) * (2.0f - a / b);
    }
  } else {
    if (a < b) {
      r = -a;
      phi = (M_PIf / 4.0f) * (4.0f + b / a);
    } else {
      r = -b;
      if (b != 0.0)
        phi = (M_PIf / 4.0f) * (6.0f - a / b);
      else
        phi = 0.0;
    }
  }

  *u = r * (float)cos(phi);
  *v = r * (float)sin(phi);
  return;
}

/* ------------------------------------------------------------------------- */

void GenerateRandomRay(RAY *ray)
/* generates a random ray from either light sources or the sky */
{
  LIGHT *light;
  float uvec[3], vvec[3];
  float u, v, intensity;
  int i;

  // set ray direction
  if (use_sky_model) {
    intensity = GetSkyDirection(ray->dir, uvec, vvec);
    ray->dir[X] *= -1.0f;
    ray->dir[Y] *= -1.0f;
    ray->dir[Z] *= -1.0f;

    // from this point, the ray is treated as if it came from a directional
    // light source, so the sky model is really just a large collection of
    // weighted directional light sources. that is (see below) a random pt is
    // selected on the projected disk (made of perpendicular vectors 'uvec' and
    // 'vvec') and the ray is traced as if it come directly from that point like
    // for a directional light source. I'm not sure this is the best way to do
    // it. Would treating it as a point light source be more appropriate? That
    // would require generating the ray's direction using Lambertian
    // distribution around ray->dir, and adjusting its intensity.

    // if the projected leaf area should be returned, need to save some info
    if (return_var) {
      // method 1: save the cosine angle for each object, but this is very slow!
      // scene.primitives[i].avg_cos_angle is reset to zero in
      // DetermineResponse() for every "run"
      for (i = 0; i < scene.num_primitives; i++)
        scene.primitives[i].avg_cos_angle +=
            (float)fabs(DotProduct(ray->dir, scene.primitives[i].normal)) *
            intensity;
      // method 2: save the average ray direction, but this doesn't give the
      // exact same result as the above? This is strange because vector dot
      // product is distributive... anyway, there is only a small numerical
      // difference.
    }
  } else {
    light = RandomLight(&scene);
    intensity = 1.f;

    // get ray direction
    ray->dir[X] = light->dir[X];
    ray->dir[Y] = light->dir[Y];
    ray->dir[Z] = light->dir[Z];

    uvec[X] = light->u[X];
    uvec[Y] = light->u[Y];
    uvec[Z] = light->u[Z];
    vvec[X] = light->v[X];
    vvec[Y] = light->v[Y];
    vvec[Z] = light->v[Z];
  }

  // set ray location
  ToUnitDisk(&u, &v);

  u *= scene.grid.bsph_radius;
  v *= scene.grid.bsph_radius;

  ray->pt[X] = scene.grid.bsph_centre[X] + u * uvec[X] + v * vvec[X];
  ray->pt[Y] = scene.grid.bsph_centre[Y] + u * uvec[Y] + v * vvec[Y];
  ray->pt[Z] = scene.grid.bsph_centre[Z] + u * uvec[Z] + v * vvec[Z];

  // initialize ray
  ray->depth = 0;
  for (i = 0; i < spectrum_samples; i++)
    if (one_ray_per_spectrum)
      ray->intensity[i] = source_spectrum[i].weight * intensity;
    else
      ray->intensity[i] = 1.f * intensity;
  return;
}

/* ------------------------------------------------------------------------- */
