#include "quasiMC.h"

static void Subdivide(SURFACE *surface, int patch, int numsubdivides);
static void SubBezier(float *points, int num_pts, int offset);

/* ------------------------------------------------------------------------- */

void InitSurface(SURFACE *surface) {
  surface->max[0] = surface->max[1] = surface->max[2] = 1.0;
  surface->min[0] = surface->min[1] = surface->min[2] = -1.0;
  surface->num_patches = 0;
}

/* ------------------------------------------------------------------------- */

int LoadSurface(SURFACE *surface, char *filename, int numsubdivides) {
  FILE *in;
  float x, y, z;
  char buf[1024];
  char token[32];
  float transform[16];
  float leftVector[3];
  int sizefound, i, j;

  if ((in = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "quasiMC - Could not open surface file %s.\n", filename);
    return (0);
  }

  /* init patches */
  surface->num_patches = 0;
  sizefound = 0;

  /* bounding box */
  fgets(buf, 1024, in);
  sscanf(buf, "%f %f %f %f %f %f", &surface->min[0], &surface->max[0],
         &surface->min[1], &surface->max[1], &surface->min[2],
         &surface->max[2]);

  while (fgets(buf, 1024, in) != NULL) {
    sscanf(buf, "%s", token);

    if (!StrCaseCmp(token, "PRECISION")) {
      fgets(buf, 1024, in);
    } else if (!StrCaseCmp(token, "CONTACT")) {
      sscanf(buf, "%s %s %s %f %s %f %s %f", token, token, token,
             &surface->connectionPoint[0], token, &surface->connectionPoint[1],
             token, &surface->connectionPoint[2]);
    } else if (!StrCaseCmp(token, "END")) {
      sscanf(buf, "%s %s %s %f %s %f %s %f", token, token, token,
             &surface->endPoint[0], token, &surface->endPoint[1], token,
             &surface->endPoint[2]);
    } else if (!StrCaseCmp(token, "HEADING")) {
      sscanf(buf, "%s %s %f %s %f %s %f", token, token,
             &surface->headingVector[0], token, &surface->headingVector[1],
             token, &surface->headingVector[2]);
    } else if (!StrCaseCmp(token, "UP")) {
      sscanf(buf, "%s %s %f %s %f %s %f", token, token, &surface->upVector[0],
             token, &surface->upVector[1], token, &surface->upVector[2]);
    } else if (!StrCaseCmp(token, "SIZE:")) {
      sscanf(buf, "%s %f", token, &surface->scale);
      sizefound = 1;
    } else {
      /* get the patch name */
      strcpy(surface->patches[surface->num_patches].name, buf);

      /* discard colour info if it exists and read patch connection AL,A,AR */
      if (fgets(buf, 1024, in) == NULL) {
        fprintf(stderr, "ERROR: (1a) cannot read patch %s\n", filename);
        return (0);
      }
      while (buf[0] != 'A' && buf[0] != 'a')
        fgets(buf, 1024, in);

      /* read in the patch connections - for L,R and BL,B,BR */
      if (fgets(buf, 1024, in) == NULL) {
        fprintf(stderr, "ERROR: (1b) cannot read patch %s\n", filename);
        return (0);
      }
      if (fgets(buf, 1024, in) == NULL) {
        fprintf(stderr, "ERROR: (1c) cannot read patch %s\n", filename);
        return (0);
      }

      /* create the transformation matrix */
      MakeUnitMatrix(transform);
      CrossProduct(surface->headingVector, surface->upVector, leftVector);
      for (i = 0; i < 3; i++) {
        transform[access(i, 0)] = surface->upVector[i];
        transform[access(i, 1)] = surface->headingVector[i];
        transform[access(i, 2)] = leftVector[i];
        // transform[access(3,i)] = surface->connectionPoint[i];
        // transforming position has to been done separately (before rotation)
      }

      /* read in the coordinates */
      for (i = 0; i < 16; i++) {
        fscanf(in, "%f %f %f", &leftVector[0], &leftVector[1], &leftVector[2]);
        leftVector[0] -= surface->connectionPoint[0];
        leftVector[1] -= surface->connectionPoint[1];
        leftVector[2] -= surface->connectionPoint[2];
        Transform3Point(leftVector, transform,
                        surface->patches[surface->num_patches].points[i]);
      }
      fgets(buf, 1024, in);
      surface->num_patches++;
    }
  }

  if (!sizefound) {
    x = surface->max[0] - surface->min[0];
    y = surface->max[1] - surface->min[1];
    z = surface->max[2] - surface->min[2];

    surface->scale = x;
    if (y > surface->scale)
      surface->scale = y;
    if (z > surface->scale)
      surface->scale = z;
  }

  /* scale the patches */
  for (i = 0; i < surface->num_patches; i++)
    for (j = 0; j < 16; j++) {
      surface->patches[i].points[j][0] /= surface->scale;
      surface->patches[i].points[j][1] /= surface->scale;
      surface->patches[i].points[j][2] /= surface->scale;
    }
  surface->max[0] /= surface->scale;
  surface->max[1] /= surface->scale;
  surface->max[2] /= surface->scale;
  surface->min[0] /= surface->scale;
  surface->min[1] /= surface->scale;
  surface->min[2] /= surface->scale;

  surface->num_subdivides = numsubdivides;
  for (i = 0; i < surface->num_patches; i++)
    Subdivide(surface, i, numsubdivides);

  return (1);
}

/* ------------------------------------------------------------------------- */

int SurfacePatches(SURFACE *surface) {
  int i, patches;
  patches = 3;
  for (i = 0; i < surface->num_subdivides; i++)
    patches *= 2;
  return (surface->num_patches * patches * patches);
}

/* ------------------------------------------------------------------------- */

static void Subdivide(SURFACE *surface, int patch, int numsubdivides) {
  SUBSURFACE *sub_s;
  int i, j, num_pts;

  /* the new number of points is 3*2^n + 1 */
  num_pts = 3 * (1 << numsubdivides) + 1;

  sub_s = &surface->subsurfaces[patch];

  sub_s->num_pts = 4;
  sub_s->points = (float *)malloc(sizeof(float) * num_pts * num_pts * 3);
  memset(sub_s->points, 0, sizeof(float) * num_pts * num_pts * 3);

  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++) {
      sub_s->points[i * num_pts * 3 + j * 3 + 0] =
          surface->patches[patch].points[i * 4 + j][0];
      sub_s->points[i * num_pts * 3 + j * 3 + 1] =
          surface->patches[patch].points[i * 4 + j][1];
      sub_s->points[i * num_pts * 3 + j * 3 + 2] =
          surface->patches[patch].points[i * 4 + j][2];
    }

  for (i = 0; i < numsubdivides; i++) {
    /* subdivide all u curves */
    for (j = 0; j < sub_s->num_pts; j++)
      SubBezier(&sub_s->points[j * num_pts * 3], sub_s->num_pts, 1);

    /* subdivide all v curves */
    for (j = 0; j < num_pts; j++)
      SubBezier(&sub_s->points[j * 3], sub_s->num_pts, num_pts);

    /* update the number of points */
    sub_s->num_pts = 2 * sub_s->num_pts - 1;
  }
}

/* ------------------------------------------------------------------------- */

static void SubBezier(float *points, int num_pts, int offset)
/* apply subdivision technique to bezier curve, just once */
{
  float *new_pts;
  int num, i, j;

  /* there will be 2*n - 1 new points, after the subdivision */
  num = 2 * num_pts - 1;
  new_pts = (float *)malloc(sizeof(float) * num * 3);
  memset(new_pts, 0, sizeof(float) * num * 3);

  /* evenly distribute the current points among the new points */
  for (i = 0; i < num_pts; i++) {
    new_pts[i * 6 + 0] = points[i * 3 * offset + 0];
    new_pts[i * 6 + 1] = points[i * 3 * offset + 1];
    new_pts[i * 6 + 2] = points[i * 3 * offset + 2];
  }

  /* the results of the first subdivision are placed into the gaps */
  for (i = 0; i < (num_pts - 1); i++)
    for (j = i; j < (num - 1 - i); j += 2) {
      /* Q(x+1,y+1,z+1) = (Q(x,y,z) + Q(x+2,y+2,z+2)) / 2.0 */
      new_pts[(j + 1) * 3 + 0] =
          (new_pts[j * 3 + 0] + new_pts[(j + 2) * 3 + 0]) / 2.0f;
      new_pts[(j + 1) * 3 + 1] =
          (new_pts[j * 3 + 1] + new_pts[(j + 2) * 3 + 1]) / 2.0f;
      new_pts[(j + 1) * 3 + 2] =
          (new_pts[j * 3 + 2] + new_pts[(j + 2) * 3 + 2]) / 2.0f;
    }

  /* replace the bezier points with the new points */
  for (i = 0; i < num; i++) {
    points[i * 3 * offset + 0] = new_pts[i * 3 + 0];
    points[i * 3 * offset + 1] = new_pts[i * 3 + 1];
    points[i * 3 * offset + 2] = new_pts[i * 3 + 2];
  }

  free(new_pts);
  return;
}

/* ------------------------------------------------------------------------- */
