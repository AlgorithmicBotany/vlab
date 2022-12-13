#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define X 0
#define Y 1
#define Z 2

#include "sky.h"

struct data_type {
  float distrF, intensity;
};

typedef struct data_type data_type;

int samples[2];
int dimension;

double from[2] = {0, 0}, to[2] = {M_PI / 2, 2 * M_PI}; /* angle boundaries */

data_type *data = NULL;

/*************************************************************************/
void FreeSkyData(void) {
  if (data != NULL)
    free(data);
}

/*************************************************************************/
int ReadSkyFile(char *name) {
  FILE *fp;
  char line[1024];
  char *token;
  int x, y;

  if ((fp = fopen(name, "r")) == NULL) {
    fprintf(stderr, "Cannot open sky file %s.\n", name);
    return 0;
  }

  /* get the first non-comment line */
  do
    if (fgets(line, sizeof(line), fp) == NULL) {
      fprintf(stderr, "Unexpected end of file %s. File empty?\n", name);
      return 0;
    }
  while (line[0] == '#');

  if ((token = strtok(line, " ,x\n")) == NULL) {
    fprintf(stderr, "Cannot read the number of samples from file %s.\n", name);
    return 0;
  }

  /* get the number of samples */
  samples[0] = atoi(token);

  if ((token = strtok(NULL, " ,x\n")) == NULL) {
    dimension = 1;
    samples[1] = 1;
  } else {
    dimension = 2;
    samples[1] = atoi(token);
  }

  /* allocate the memory for samples */
  if ((data = (data_type *)malloc(samples[0] * samples[1] *
                                  sizeof(data_type))) == NULL) {
    fprintf(stderr, "Cannot allocate memory for %dx%d samples!\n", samples[0],
            samples[1]);
    return 0;
  }

  for (y = 0; y < samples[1]; y++) {
    for (x = 0; x < samples[0]; x++) {
      do {
        if (fgets(line, sizeof(line), fp) == NULL) {
          fprintf(stderr, "Sample [%d,%d] not found!\n", x, y);
          return 0;
        }
      } while (line[0] == '\n' || line[0] == '#');

      if ((token = strtok(line, " ,\n")) == NULL) {
        fprintf(stderr, "Sample [%d,%d] line empty:%s?\n", x, y, line);
        return 0;
      }

      data[y * samples[0] + x].distrF = atof(token);

      if ((token = strtok(NULL, " ,\n")) != NULL) {
        /* intensities present */
        fprintf(stderr, "Intensity token:%s.\n", token);
        data[y * samples[0] + x].intensity = atof(token);
      } else
        data[y * samples[0] + x].intensity = 1;
    }
  }

  /* all data in */
  fclose(fp);

  return 1;
}

/*************************************************************************/
void GetRandomSkyDirection(float *dir, float *xvec, float *yvec) {
  float angle[2];
  int x, y, yi;
  float rnd;
  float aux, sina, cosa;

  /* choose angle[0] according to F(x,max) - y=samples[1]-1 */
  y = (samples[1] - 1) * samples[0];

  rnd = data[y].distrF +
        drand48() * (data[y + samples[0] - 1].distrF - data[y].distrF);

  /* find rnd in the last row */
  x = 1;

  while (rnd >= data[y + x].distrF)
    x++;

  /* rnd is in the interval <data[y+x-1], data[y + x]) */

  /* approximate between values x-1 and x */
  rnd = (rnd - data[y + x - 1].distrF) /
        (data[y + x].distrF - data[y + x - 1].distrF);

  angle[0] =
      from[0] + (to[0] - from[0]) * ((float)x + rnd) / (float)(samples[0] - 1);

  /* which column is closer? */
  if (rnd < 0.5)
    x--;

  /* choose row y in the column x */
  rnd = data[x].distrF +
        drand48() *
            (data[x + (samples[1] - 1) * samples[0]].distrF - data[x].distrF);

  /* find rnd in the column x */
  y = samples[0];
  yi = 1;

  while (rnd >= data[y + x].distrF) {
    y += samples[0];
    yi++;
  }

  /* rnd is in the interval <data[y-samples[0] + x], data[y + x]) */

  rnd = (rnd - data[y - samples[0] + x].distrF) /
        (data[y + x].distrF - data[y - samples[0] + x].distrF);

  angle[1] =
      from[1] + (to[1] - from[1]) * ((float)yi + rnd) / (float)(samples[1] - 1);

  /* found the two angles */
  /* angle[0] - zenith angle -> from positive y axis down
     angle[1] - azimuth angle -> from positive z axis (north) towards
                positive x axis (east) */

  /* determine ray direction dir */
  dir[Y] = cos(angle[0]);
  aux = sin(angle[0]);

  dir[Z] = aux * (cosa = cos(angle[1]));
  dir[X] = aux * (sina = sin(angle[1]));

  /* get two vectors perpendicular to dir */
  /* yvec is pointing "up" */
  yvec[Y] = aux;
  yvec[Z] = -dir[Y] * cosa;
  yvec[X] = -dir[Y] * sina;

  /* xvec is perpendicular both to dir and yvec */
  xvec[Y] = 0;
  xvec[Z] = -sina;
  xvec[X] = cosa;
}

/*************************************************************************/
float SkyIntensityInDir(float *dir) {
  float angle[2];
  int x, y;

  /* but there should be a ground */
  if (dir[Y] < 0)
    dir[Y] = -dir[Y];

  /* angle[0] - angle from zenith */
  angle[0] = acos(dir[Y]);

  /* angle[1] - azimuth angle: from positive z axis (north) towards
                positive x axis (east) */
  angle[1] = 0;

  x = (angle[0] - from[0]) / (to[0] - from[0]) * (float)(samples[0] - 1);
  y = (angle[1] - from[1]) / (to[1] - from[1]) * (float)(samples[1] - 1);
  if (samples[0] * samples[1] * sizeof(data_type) < (size_t)( x + y * samples[0])) {
    fprintf(stderr, "Warning overflow in SkyIntensityInDir \n");
    return 0;
  }
  if (samples[0] * samples[1] * sizeof(data_type) < (size_t) (x + y * samples[0])) {
    fprintf(stderr, "Warning overflow in SkyIntensityInDir \n");
    return 0;
  }

  return data[x + y * samples[0]].intensity;
}
