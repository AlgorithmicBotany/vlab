/* args.c - implements the function that processes the arguments in the
            field specification file */

#include "quasiMC.h"

static char proc_name[32] = "QuasiMC";
// all these extern variables are declared in main.cpp
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
extern int return_hits;
extern float sensor_fov;
extern char output_filename[64];
extern char RQMC_method[32];
extern int return_type[MAX_SPECTRUM_SAMPLES];
extern SOURCESPECTRUM source_spectrum[MAX_SPECTRUM_SAMPLES];
extern RUSSIANROULETTE russian_roulette;
extern SCENE scene;
extern int numSides;

VISUALIZATION visualization;

static char *keywords[] = {"verbose",
                           "remove objects",
                           "rays from objects",
                           "spectrum samples",
                           "local light model",
                           "maximum depth",
                           "number of runs",
                           "one ray per spectrum",
                           "ignore direct light",
                           "return variance",
                           "output file",
                           "material parameter",
                           "source spectrum",
                           "leaf material (top)",
                           "leaf material (bottom)",
                           "material",
                           "light source",
                           "Russian roulette",
                           "grid size",
                           "sampling method",
                           "location",
                           "weather",
                           "growth period",
                           "surface",
                           "number of rays",
                           "julian day",
                           "sky file",
                           "return type",
                           "light source file",
                           "visualization",
                           "cylinder sides",
                           NULL};

void DefaultSettings(void);
void ProcessKeyword(FILE *infile, int index, int *line_num);
void ReadMaterial(FILE *infile, int index, int *line_num);

/* ------------------------------------------------------------------------- */

int StrCaseCmp(char *s1, char *s2) {
  while ((*s1 != '\0') &&
         (tolower(*(unsigned char *)s1) == tolower(*(unsigned char *)s2))) {
    ++s1;
    ++s2;
  }
  return tolower(*(unsigned char *)s1) - tolower(*(unsigned char *)s2);
}

/* ------------------------------------------------------------------------- */

int ProcessArguments(int argc, char **argv)
/* sets the parameters of 'quasiMC' aaccording to input file */
{
  static char input_line[128];
  LIGHT light;
  FILE *infile;
  char *token;
  int i, line_num;

  DefaultSettings();

  /* read parameters from the file (if any) */
  if (argc == 1) {
    fprintf(stderr,
            "%s - not enough arguments provided\n"
            "\tUSAGE: %s -e environment_file specification_file\n",
            proc_name, proc_name);
    return (0);
  }

  if ((infile = fopen(argv[1], "r")) == NULL)
    fprintf(stderr,
            "%s - cannot open specification file %s\n"
            "\t(using default arguments)\n",
            proc_name, argv[1]);
  else {
    line_num = 0;
    while (!feof(infile)) {
      ++line_num;

      if (fgets(input_line, sizeof(input_line), infile) == NULL)
        break;

      token = strtok(input_line, "\t:");

      if (!strcmp(token, "\n"))
        continue;
      if (token[0] == '/' && token[1] == '/')
        continue;

      i = 0;
      while (keywords[i] != NULL)
        if (!StrCaseCmp(keywords[i], token))
          break;
        else
          ++i;

      if (keywords[i] != NULL)
        ProcessKeyword(infile, i, &line_num);
      else {
        fprintf(
            stderr,
            "%s - (line: %d) unknown directive %s in the specification file\n",
            proc_name, line_num, token);
      }
    }
    fclose(infile);
  }

  /* add a default light source */
  if ((scene.num_lights == 0) && (use_sky_model == 0)) {
    light.dir[X] = 0.0;
    light.dir[Y] = -1.0;
    light.dir[Z] = 0.0;
    light.weight = 1.0;
    AddLight(&scene, &light);
  }

  /* if a light is specified don't use the sky model */
  if (scene.num_lights > 0)
    use_sky_model = 0;
  return (1);
}

/* ------------------------------------------------------------------------- */

void DefaultSettings(void)
/* sets default values for the parameters */
{
  MATERIAL mat;
  int i;

  /* set the default values */
  verbose = 2;
  remove_objects = 1;
  rays_from_objects = 0;
  spectrum_samples = 1;
  reflectance_model = LAMBERTIAN;
  max_depth = 5;
  num_samples = 1;
  use_sky_model = 0;
  one_ray_per_spectrum = 1;
  no_direct_light = 0;
  return_var = 0;
  material_parameter = 1;
  number_of_rays = 1;
  sensor_fov = 0.0;
  for (i = 0; i < MAX_SPECTRUM_SAMPLES; i++) {
    return_type[i] = ABSORBED_FLUX;

    source_spectrum[i].weight = 1.0;
    source_spectrum[i].wavelength = 660.0;

    mat.reflectance[i] = 0.25;
    mat.spec_power[i] = 10.0;
    mat.transmittance[i] = 0.25;
    mat.trans_power[i] = 10.0;
    mat.Nt[i] = 1.0;
  }
  russian_roulette.threshold = 0.0;
  russian_roulette.prob = 0.7f;

  AddMaterial(&scene, &mat); /* need to add the leaf material (top) */
  /* commented out the next line because if bottom material was not specified
     it would use 'mat' instead of whatever top material was.
     AddMaterial (&scene, &mat);*/ /* and the leaf material (bottom) */

  scene.grid.size[X] = 1;
  scene.grid.size[Y] = 1;
  scene.grid.size[Z] = 1;

  strcpy(RQMC_method, "monte carlo");

  visualization.show_light = true;
  visualization.show_box = false;
  visualization.show_wireframe = false;
  visualization.background[0] = 0.f;
  visualization.background[1] = 0.f;
  visualization.background[2] = 0.f;
  visualization.colour_mod[0] = 1.f;
  visualization.colour_mod[1] = 1.f;
  visualization.colour_mod[2] = 1.f;

  numSides = 4;

  return;
}

/* ------------------------------------------------------------------------- */

void ProcessKeyword(FILE *infile, int index, int *line_num)
/* process the keyword at index */
{
  LIGHT light;
  float lat, lon;
  float period[6];
  char *token, *filename;
  int i;

  switch (index) {
  case 0: /* verbose */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    verbose = atoi(token);
    break;

  case 1: /* remove_objects */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    if (!strcmp(token, "yes"))
      remove_objects = 1;
    else if (!strcmp(token, "no"))
      remove_objects = 0;
    else
      fprintf(stderr, "%s - (line: %d) unknown parameter %s for directive %s\n",
              proc_name, *line_num, token, keywords[index]);
    break;

  case 2: /* rays_from_objects */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    if (!strcmp(token, "yes")) {
      rays_from_objects = 1;
      if ((token = strtok(NULL, ",; \t:\n")) == NULL)
        sensor_fov = 0.0; // 90 degrees field of view
      else if (isdigit(token[0])) {
        sensor_fov = (float)atof(token);
        if (sensor_fov < 0.0 || sensor_fov > 90.0) {
          fprintf(stderr,
                  "%s - (line: %d) sensor field of view out of range [0,90)\n",
                  proc_name, *line_num);
          sensor_fov = 0.0;
        } else {
          sensor_fov *= M_PIf / 180.0f;
          sensor_fov = cosf(sensor_fov) * cosf(sensor_fov);
          sensor_fov = sensor_fov < 0.00001f ? 0.0f : sensor_fov;
        }
      } else // the second token is not a number
        sensor_fov = 0.0;
    } else if (!strcmp(token, "no"))
      rays_from_objects = 0;
    else {
      fprintf(stderr, "%s - (line: %d) unknown parameter %s for directive %s\n",
              proc_name, *line_num, token, keywords[index]);
      break;
    }
    break;

  case 3: /* spectrum_samples */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    spectrum_samples = atoi(token);
    break;

  case 4: /* reflectance_model - now called local light model since it is used
             for BRDF and BTDF */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    if (!strcmp(token, "modified"))
      reflectance_model = BLINN_PHONG;
    else if (!strcmp(token, "Phong"))
      reflectance_model = PHONG;
    else if (!strcmp(token, "Lambertian"))
      reflectance_model = LAMBERTIAN;
    else
      fprintf(stderr, "%s - (line: %d) unknown reflectance model %s\n",
              proc_name, *line_num, token);
    break;

  case 5: /* max_depth */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    max_depth = (unsigned int)atoi(token);
    if (max_depth < 0)
      max_depth = 0;
    break;

  case 6: /* number of runs */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    num_samples = atoi(token);
    break;

  case 7: /* one ray per spectrum */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    if (!strcmp(token, "yes"))
      one_ray_per_spectrum = 1;
    else if (!strcmp(token, "no"))
      one_ray_per_spectrum = 0;
    else
      fprintf(stderr, "%s - (line: %d) unknown parameter %s for directive %s\n",
              proc_name, *line_num, token, keywords[index]);
    break;

  case 8: /* no direct light */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    if (!strcmp(token, "yes"))
      no_direct_light = 1;
    else if (!strcmp(token, "no"))
      no_direct_light = 0;
    else
      fprintf(stderr, "%s - (line: %d) unknown parameter %s for directive %s\n",
              proc_name, *line_num, token, keywords[index]);
    break;

  case 9:  /* return variance */
  case 10: /* output file */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    if (!strcmp(token, "yes"))
      return_var = 1;
    else if (!strcmp(token, "no"))
      return_var = 0;
    else
      fprintf(stderr, "%s - (line: %d) unknown parameter %s for directive %s\n",
              proc_name, *line_num, token, keywords[index]);
    // check for filename
    token = strtok(NULL, ",; \t:\n\"");
    if (token != NULL) {
      strcpy(output_filename, token);
    }
    break;

  case 11: /* material parameter */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    material_parameter = atoi(token);
    break;

  case 12: /* source spectrum */
    for (i = 0; i < spectrum_samples; i++) {
      if ((token = strtok(NULL, ",; \t:\n")) == NULL) {
        fprintf(stderr,
                "%s - (line: %d) wavelength not specified for "
                "source spectrum %d\n",
                proc_name, *line_num, i + 1);
        break;
      }
      source_spectrum[i].wavelength = (float)atof(token);

      if ((token = strtok(NULL, ",; \t:\n")) == NULL) {
        fprintf(stderr,
                "%s - (line: %d) weight not specified for "
                "source spectrum %d\n",
                proc_name, *line_num, i + 1);
        break;
      }
      source_spectrum[i].weight = (float)atof(token);
    }
    break;

  case 13: /* leaf material (top) */
    ReadMaterial(infile, 0, line_num);
    break;

  case 14: /* leaf material (bottom) */
    ReadMaterial(infile, 1, line_num);
    break;

  case 15: /* material */
    ReadMaterial(infile, 2, line_num);
    break;

  case 16: /* light source */
    light.dir[X] = 0.0;
    light.dir[Y] = 0.0;
    light.dir[Z] = 0.0;
    light.weight = 1.0;

    for (i = X; i <= Z; i++) {
      if ((token = strtok(NULL, ",; \t:\n")) == NULL) {
        fprintf(stderr,
                "%s - (line: %d) coordinate %d missing for light source"
                " direction\n",
                proc_name, *line_num, i + 1);
        break;
      }
      light.dir[i] = (float)atof(token);
    }

    if ((token = strtok(NULL, "x,; \t:\n")) == NULL) {
      fprintf(stderr, "%s - (line: %d) weight missing for light source\n",
              proc_name, *line_num);
      break;
    }
    light.weight = (float)atof(token);

    AddLight(&scene, &light);
    break;

  case 17: /* Russian Roulette */
    if ((token = strtok(NULL, ",; \t:\n")) == NULL) {
      fprintf(stderr,
              "%s - (line: %d) threshold not specified for Russian roulette\n",
              proc_name, *line_num);
      break;
    }
    russian_roulette.threshold = (float)atof(token);

    if ((token = strtok(NULL, ",; \t:\n")) == NULL) {
      fprintf(
          stderr,
          "%s - (line: %d) probability not specified for Russian roulette\n",
          proc_name, *line_num);
      break;
    }
    russian_roulette.prob = (float)atof(token);
    break;

  case 18: /* grid size */
    token = strtok(NULL, "x,; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) first parameter of grid size missing\n",
              proc_name, *line_num);
      break;
    }
    scene.grid.size[X] = atoi(token);
    token = strtok(NULL, "x,; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) second parameter of grid size missing\n",
              proc_name, *line_num);
      break;
    }
    scene.grid.size[Y] = atoi(token);
    token = strtok(NULL, "x,; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) third parameter of grid size missing\n",
              proc_name, *line_num);
      break;
    }
    scene.grid.size[Z] = atoi(token);

    /* check the size of the grid */
    if (scene.grid.size[X] * scene.grid.size[Y] * scene.grid.size[Z] <= 0) {
      fprintf(stderr, "%s - (line: %d) wrong size of the grid! using 1x1x1\n",
              proc_name, *line_num);
      scene.grid.size[X] = 1;
      scene.grid.size[Y] = 1;
      scene.grid.size[Z] = 1;
    }
    // save default size
    scene.grid.default_size[X] = scene.grid.size[X];
    scene.grid.default_size[Y] = scene.grid.size[Y];
    scene.grid.default_size[Z] = scene.grid.size[Z];
    break;

  case 19: /* sampling method */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) RQMC method name is missing\n",
              proc_name, *line_num);
      break;
    }
    strcpy(RQMC_method, token);
    break;

  case 20: /* location - sky model */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(
          stderr,
          "%s - (line: %d) first parameter (latitude) of location missing\n",
          proc_name, *line_num);
      break;
    }
    lat = (float)atof(token);
    if (lat < -90.0f || lat > 90.0f) {
      fprintf(stderr, "%s - (line: %d) latitude is out of range (-90 to 90)\n",
              proc_name, *line_num);
      break;
    }

    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(
          stderr,
          "%s - (line: %d) second parameter (longitude) of location missing\n",
          proc_name, *line_num);
      break;
    }
    lon = (float)atof(token);
    if (lon < -180.0f || lon > 180.0f) {
      fprintf(stderr,
              "%s - (line: %d) longitude is out of range (-180 to 180)\n",
              proc_name, *line_num);
      break;
    }
    SetLocation(lat, lon);
    use_sky_model = 1;
    break;

  case 21: /* weather - sky model */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(
          stderr,
          "%s - (line: %d) first parameter (clear days) of weather missing\n",
          proc_name, *line_num);
      break;
    }
    lat = (float)atof(token);
    if (lat < 0.0f || lat > 1.0f) {
      fprintf(stderr, "%s - (line: %d) clear days is out of range (0 to 1)\n",
              proc_name, *line_num);
      break;
    }

    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(
          stderr,
          "%s - (line: %d) second parameter (turbidity) of weather missing\n",
          proc_name, *line_num);
      break;
    }
    lon = (float)atof(token);
    if (lon < 0.0f || lon > 100.0f) {
      fprintf(stderr, "%s - (line: %d) longitude is out of range (0 to 100)\n",
              proc_name, *line_num);
      break;
    }
    SetWeather(lat, lon);
    use_sky_model = 1;
    break;

  case 22: /* growth period - sky model */
    for (i = 0; i < 2; i++) {
      token = strtok(NULL, ",; \t:\n");
      if (token == NULL) {
        fprintf(stderr,
                "%s - (line: %d) parameter of growth period (0 to 24 hours) "
                "missing\n",
                proc_name, *line_num);
        i = -1;
        break;
      }
      period[i] = (float)atof(token);
    }
    if (i != -1) {
      if (period[0] > period[1])
        fprintf(stderr,
                "%s - (line: %d) 1st parameter of growth period should be <= "
                "to 2nd parameter\n",
                proc_name, *line_num);
      period[0] = period[0] < 0 ? 0 : period[0];
      period[1] = period[1] > 24 ? 24 : period[1];
      SetGrowthPeriod(period);
      use_sky_model = 1;
    }
    break;

  case 23: /* surface */
    if (scene.num_surfaces + 1 >= MAX_SURFACES) {
      fprintf(stderr,
              "%s - (line: %d) Too many surfaces specified (MAX = %d)\n",
              proc_name, *line_num, MAX_SURFACES);
      break;
    }
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) file name for surface %d is missing\n",
              proc_name, *line_num, scene.num_surfaces);
      break;
    }
    filename = token;

    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr,
              "%s - (line: %d) subdivisions for surface %d is missing\n",
              proc_name, *line_num, scene.num_surfaces);
      break;
    }
    i = atoi(token);

    InitSurface(&scene.surfaces[scene.num_surfaces]);
    if (LoadSurface(&scene.surfaces[scene.num_surfaces], filename, i))
      ++scene.num_surfaces;
    break;

  case 24: /* number of rays */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    number_of_rays = atoi(token);
    break;

  case 25: /* sky model - julian day */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    i = atoi(token);
    if (i < 1)
      i = 1;
    if (i > 365)
      i = 365;
    SetJulianDay(i);
    use_sky_model = 1;
    break;

  case 26: /* sky file */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    SetSkyFile(token);
    use_sky_model = 1;
    break;

  case 27: /* return type */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    i = 0;
    while (token != NULL && i < MAX_SPECTRUM_SAMPLES) {
      if (!strcmp(token, "F"))
        return_type[i] = ABSORBED_FLUX;
      else if (!strcmp(token, "D"))
        return_type[i] = ABSORBED_IRRADIANCE;
      else if (!strcmp(token, "U"))
        return_type[i] = UP_INCIDENT_IRRADIANCE;
      else if (!strcmp(token, "L"))
        return_type[i] = LW_INCIDENT_IRRADIANCE;
      else if (!strcmp(token, "H"))
        return_type[i] = NUM_INTERSECTIONS;
      else
        fprintf(stderr,
                "%s - (line: %d) unknown parameter %s for directive %s\n",
                proc_name, *line_num, token, keywords[index]);
      token = strtok(NULL, ",; \t:\n");
      ++i;
    }
    break;

  case 28: /* light source file */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    if (SetLightSourceFile(&scene, token) == 0) {
      fprintf(stderr, "%s - (line: %d) could not read %s, %s\n", proc_name,
              *line_num, keywords[index], token);
      break;
    }
    break;

  case 29: /* visualization */
    // show light source
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(
          stderr,
          "%s - (line: %d) show light source not specified for directive %s\n",
          proc_name, *line_num, keywords[index]);
      break;
    }
    if (!strcmp(token, "yes"))
      visualization.show_light = true;
    else if (!strcmp(token, "no"))
      visualization.show_light = false;
    else
      fprintf(stderr, "%s - (line: %d) unknown parameter %s for directive %s\n",
              proc_name, *line_num, token, keywords[index]);
    // show bounding box
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(
          stderr,
          "%s - (line: %d) show bounding box not specified for directive %s\n",
          proc_name, *line_num, keywords[index]);
      break;
    }
    if (!strcmp(token, "yes"))
      visualization.show_box = true;
    else if (!strcmp(token, "no"))
      visualization.show_box = false;
    else
      fprintf(stderr, "%s - (line: %d) unknown parameter %s for directive %s\n",
              proc_name, *line_num, token, keywords[index]);
    // show wireframe
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr,
              "%s - (line: %d) show wireframe not specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    if (!strcmp(token, "yes"))
      visualization.show_wireframe = true;
    else if (!strcmp(token, "no"))
      visualization.show_wireframe = false;
    else
      fprintf(stderr, "%s - (line: %d) unknown parameter %s for directive %s\n",
              proc_name, *line_num, token, keywords[index]);
    // background rgb
    for (i = 0; i < 3; i++) {
      token = strtok(NULL, ",; \t:\n");
      if (token == NULL) {
        fprintf(stderr,
                "%s - (line: %d) parameter of background colour missing\n",
                proc_name, *line_num);
        i = -1;
        break;
      }
      visualization.background[i] = (float)atof(token);
    }
    if (i != -1) {
      visualization.background[0] =
          fminf(fmaxf(visualization.background[0], 0.f), 1.f);
      visualization.background[1] =
          fminf(fmaxf(visualization.background[1], 0.f), 1.f);
      visualization.background[2] =
          fminf(fmaxf(visualization.background[2], 0.f), 1.f);
    }
    // colour modifier for objects
    for (i = 0; i < 3; i++) {
      token = strtok(NULL, ",; \t:\n");
      if (token == NULL) {
        // fprintf (stderr, "%s - (line: %d) parameter of object colour
        // missing\n",
        //         proc_name, *line_num);
        i = -1;
        break;
      }
      visualization.colour_mod[i] = (float)atof(token);
    }
    if (i != -1) {
      visualization.colour_mod[0] =
          fminf(fmaxf(visualization.colour_mod[0], 0.f), 1.f);
      visualization.colour_mod[1] =
          fminf(fmaxf(visualization.colour_mod[1], 0.f), 1.f);
      visualization.colour_mod[2] =
          fminf(fmaxf(visualization.colour_mod[2], 0.f), 1.f);
    }

    break;

  case 30: /* number of sides on the triangulated cylinder */
    token = strtok(NULL, ",; \t:\n");
    if (token == NULL) {
      fprintf(stderr, "%s - (line: %d) nothing specified for directive %s\n",
              proc_name, *line_num, keywords[index]);
      break;
    }
    numSides = atoi(token);
    if (numSides < 3)
      numSides = 3;
    if (numSides > 128)
      numSides = 128;
    break;
  }
  return;
}

/* ------------------------------------------------------------------------- */

void ReadMaterial(FILE *infile, int index, int *line_num)
/* reads material properties for the entire spectrum */
{
  static char input_line[128];
  MATERIAL material;
  char *token;
  int i, j;

  for (i = 0; i < spectrum_samples; i++) {
    /* since we already read the first line, we can't read it from the file
     * again */
    if (i > 0) {
      *line_num += 1;

      /* get new line */
      if (fgets(input_line, sizeof(input_line), infile) == NULL) {
        fprintf(stderr,
                "%s - (line: %d) material parameters missing for spectrum %d\n",
                proc_name, *line_num, i + 1);
        break;
      }

      /* get over the colon - hack to find the first colon */
      // if it isn't in the string, write error message
      // this breaks if one of the lines is commented out!
      if (!strchr(input_line, ':')) {
        fprintf(stderr,
                "%s - (line: %d) material parameters missing for spectrum %d\n",
                proc_name, *line_num, i + 1);
        break;
      }

      token = strtok(strchr(input_line, ':'), " ");
      j = 0;
      while (keywords[j] != NULL)
        if (!strcmp(keywords[j], token))
          break;
        else
          ++j;
      if (keywords[j] != NULL) {
        fprintf(stderr,
                "%s - (line: %d) material parameters missing for spectrum %d\n",
                proc_name, *line_num, i + 1);
        break;
      }
    }

    if ((token = strtok(NULL, ",; ()\t:\n")) == NULL) {
      fprintf(stderr,
              "%s - (line: %d) reflectance parameter missing for spectrum %d\n",
              proc_name, *line_num, i + 1);
      break;
    }
    material.reflectance[i] = (float)atof(token);

    if ((token = strtok(NULL, ",; ()\t:\n")) == NULL) {
      fprintf(stderr,
              "%s - (line: %d) spec. power parameter missing for spectrum %d\n",
              proc_name, *line_num, i + 1);
      break;
    }
    material.spec_power[i] = (float)atof(token);

    if ((token = strtok(NULL, ",; ()\t:\n")) == NULL) {
      fprintf(
          stderr,
          "%s - (line: %d) transmittance parameter missing for spectrum %d\n",
          proc_name, *line_num, i + 1);
      break;
    }
    material.transmittance[i] = (float)atof(token);

    if ((token = strtok(NULL, ",; ()\t:\n")) == NULL) {
      fprintf(
          stderr,
          "%s - (line: %d) trans. power parameter missing for spectrum %d\n",
          proc_name, *line_num, i + 1);
      break;
    }
    material.trans_power[i] = (float)atof(token);

    if ((token = strtok(NULL, ",; ()\t:\n")) == NULL) {
      fprintf(stderr, "%s - (line: %d) Nt parameter missing for spectrum %d\n",
              proc_name, *line_num, i + 1);
      break;
    }
    material.Nt[i] = (float)atof(token);

    /* error checking */
    if (material.reflectance[i] > 1.0) {
      fprintf(stderr,
              "%s - (line: %d) reflectance cannot be more than 1 (using 1)\n",
              proc_name, *line_num);
      material.reflectance[i] = 1.0;
    }

    if (material.reflectance[i] < 0.0) {
      fprintf(stderr,
              "%s - (line: %d) reflectance cannot be less than 0 (using 0)\n",
              proc_name, *line_num);
      material.reflectance[i] = 0.0;
    }

    if (material.transmittance[i] > 1.0) {
      fprintf(stderr,
              "%s - (line: %d) transmittance cannot be more than 1 (using 1)\n",
              proc_name, *line_num);
      material.transmittance[i] = 1.0;
    }

    if (material.transmittance[i] < 0.0) {
      fprintf(stderr,
              "%s - (line: %d) transmittance cannot be less than 0 (using 0)\n",
              proc_name, *line_num);
      material.transmittance[i] = 0.0;
    }

    if (material.transmittance[i] + material.reflectance[i] > 1.0) {
      fprintf(stderr,
              "%s - (line: %d) sum of transmittance and reflectance cannot"
              " be more than 1. Transmittance adjusted to "
              "1-reflectance.\n",
              proc_name, *line_num);
      material.transmittance[i] = 1.0f - material.reflectance[i];
    }
  }

  /* add the material */
  if (index >= 1)
    AddMaterial(&scene, &material);
  else
    ChangeMaterial(&scene, &material, index);

  return;
}

/* ------------------------------------------------------------------------- */
