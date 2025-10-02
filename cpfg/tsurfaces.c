/***************************************************************************
  surfaces specified as a set of triangles in a rayshade format

*/

#ifdef WIN32
#include "warningset.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "platform.h"
#include "interpret.h"
#include "patch.h"
#include "tsurfaces.h"
#include "utility.h"
#include "indices.h"
#include "comlineparam.h"

#include "test_malloc.h"

/* Maximum number of tsurfaces */
#define TSURFACES 20

#ifndef absf
#define absf(x) (((x) < 0.) ? -(x) : (x))
#endif

static char tsurface_identifier[TSURFACES];

int tsurface_number;

struct triangle_type {
  float vertex[3][PITEM]; /* stores 3 coordinates for the vertex,
                             3 for the normal, 2 for texture, etc.
                             (see control.h) */
};
typedef struct triangle_type triangle_type;

struct tsurface_type {
  triangle_type *triangles;
  int num_triangles;
} tsurfaces[TSURFACES];

/********************************************************************/
static void determine_vertex_shading(float *vertex, Colorindex *color_index,
                                     Colorindex *color_index_back,
                                     const TURTLE *tu, const DRAWPARAM *dr,
                                     VIEWPARAM *vw);

/********************************************************************/
void InitializeTsurfaces(void) { tsurface_number = 0; }

/********************************************************************/
void FreeTsurfaces(void) {
  int tsurface_index;

  for (tsurface_index = 0; tsurface_index < tsurface_number; tsurface_index++) {
    if (tsurfaces[tsurface_index].triangles != NULL) {
      Free(tsurfaces[tsurface_index].triangles);
      tsurfaces[tsurface_index].triangles = NULL;
    }
  }

  tsurface_number = 0;
}

/********************************************************************/
/* returns -1 if not found */
int FindTSurfaceIndex(char surface_id) {
  int tsurface_index;

  for (tsurface_index = 0; tsurface_index < tsurface_number; tsurface_index++) {
    if (tsurface_identifier[tsurface_index] == surface_id)
      return tsurface_index;
  }

  return -1;
}

/********************************************************************/
int parse_file(char *name, float surface_size) {
  char line[1024], *token;
  int num_triangles;
  FILE *fp;
  int values = -1, vertex = -1, expect_value;

  if ((fp = fopen(name, "r")) == NULL) {
    Message("Cannot open tsurface file %s!\n", name);
    return 0;
  }

  /* the first pass: count the triangles */
  num_triangles = 0;

  while (!feof(fp)) {
    if (fgets(line, sizeof(line), fp) == NULL)
      break;

    if ((token = strtok(line, "\t ,:\n")) == NULL)
      continue;
    if (!strncmp(token, "triangle", 8))
      num_triangles++;
  }

  tsurfaces[tsurface_number].num_triangles = num_triangles;

  /* allocate the memory */
  if ((tsurfaces[tsurface_number].triangles = (triangle_type *)Malloc(
           num_triangles * sizeof(triangle_type))) == NULL) {
    Message("Cannot allocate the memory for tsurface triangles!\n");
    return 0;
  }

  /* read in the triangles */
  num_triangles = 0;
  expect_value = 0;

  rewind(fp);

  while (!feof(fp)) {
    if (fgets(line, sizeof(line), fp) == NULL)
      break;

    if ((token = strtok(line, "\t ,:\n")) == NULL)
      continue;

    do {
      if (!expect_value) {
        /* expect command "triangle" */
        if (!strncmp(token, "triangle", 8)) {
          expect_value = 1;
          vertex = 0;
          values = 0;

          /* this shouldn't happend but what if */
          if (num_triangles == tsurfaces[tsurface_number].num_triangles) {
            Message("Error in parsing tsurface file %s. Tsurface"
                    " ignored!\n",
                    name);
            /* something is wrong, better return 0 */
            return 0;
          }
        }
      } else {
        /* value expected */
        tsurfaces[tsurface_number]
            .triangles[num_triangles]
            .vertex[vertex][values] = atof(token);

        /* scale the surface points */
        if (values < 3)
          tsurfaces[tsurface_number]
              .triangles[num_triangles]
              .vertex[vertex][values] *= surface_size;

        if (++values > 8)
          break; /* ignore the rest on the line */
      }
      token = strtok(NULL, "\t ,:\n");
    } while (token != NULL);

    if (values >= 3) {
      /* vertex finished */
      values = 0;
      if (++vertex == 3) {
        /* triangle finished */
        expect_value = 0;
        num_triangles++;
      }
    }
  }

  if (num_triangles != tsurfaces[tsurface_number].num_triangles) {
    Message("Error in parsing tsurface file %s. Tsurface"
            " ignored!\n",
            name);
    return 0;
  }

  tsurface_number++;

  return 1;
}

/********************************************************************/
int read_tsurface(char *name, float surface_size, char surface_id) {
  printf("Read tsurfaces.c %s\n",name);
  /* Check that array size is not exceeded */
  if (tsurface_number >= TSURFACES) {
    Message("Maximum number of tsurfaces (%d) exceeded.\n", TSURFACES);
    Message("Change TSURFACES in tsurfaces.c and recompile for more.\n");
    Message("The tsurface ignored.\n");
    return 0;
  }

  if (FindSurfaceIndex(surface_id) != -1) {
    Message("Tsurface id %c already used (for a bicubic surface)!"
            "The tsurface ignored!\n");
    return 0;
  }

  if (FindTSurfaceIndex(surface_id) != -1) {
    Message("Tsurface id %c already used! The tsurface ignored!\n");
    return 0;
  }

  tsurface_identifier[tsurface_number] = surface_id;

  if (!parse_file(name, surface_size))
    return 0;

  /* set texture coordinates */

  VERBOSE("Tsurface %c initialized.\n", surface_id);

  return 1;
}

/**************************************************************************/
/* draw_surface_tmesh passes tmesh vertices, composed of points           */
/* if necessary, color, normal, or texture coordinates are computed for   */
/* each vertex                                                            */
/**************************************************************************/

void draw_tsurface(const TURTLE *tu, char desired_surface, DRAWPARAM *dr,
                   VIEWPARAM *vw) {
  int triangle, vertex;
  int surface_index;
  int tex_index;
  Colorindex color_index, color_index_back;

  /* Determine index of desired surface, if none return */
  if ((surface_index = FindTSurfaceIndex(desired_surface)) == -1)
    return;

  tex_index = tu->texture;

  dr->texture = dr->tdd->StartTexture(tex_index);

  color_index = tu->color_index;
  color_index_back = tu->color_index_back;

  VERBOSE("Drawing tsurface %c (%d triangle(s)).\n", desired_surface,
          tsurfaces[surface_index].num_triangles);

  /* loop over all triangles */
  for (triangle = 0; triangle < tsurfaces[surface_index].num_triangles;
       triangle++) {
    for (vertex = 0; vertex < 3; vertex++) {
      if (dr->ourlighting)
        determine_vertex_shading(
            tsurfaces[surface_index].triangles[triangle].vertex[vertex],
            &color_index, &color_index_back, tu, dr, vw);

      tsurfaces[surface_index].triangles[triangle].vertex[vertex][COLOR_FRONT] =
          color_index;
      tsurfaces[surface_index].triangles[triangle].vertex[vertex][COLOR_BACK] =
          color_index_back;
    }

    (*dr->tdd->RenderTriangle)(
        tsurfaces[surface_index].triangles[triangle].vertex[0],
        tsurfaces[surface_index].triangles[triangle].vertex[1],
        tsurfaces[surface_index].triangles[triangle].vertex[2], dr);
  }

  if (dr->texture)
    dr->tdd->EndTexture(tex_index);
}

/***************************************************************************/
/* Set shading parameters for this vertex (Iris GL version) */
/* probably should go to rendering routines */
static void determine_vertex_shading(float *vertex, Colorindex *color_index,
                                     Colorindex *color_index_back,
                                     const TURTLE *tu, const DRAWPARAM *dr,
                                     VIEWPARAM *vw) {
  int color_value;
  int hue;
  float intensity;
  float diffuse;
  float vertex_lamp /*, vertex_view*/;
  float n_x, n_y, n_z, distance;
  /*int side;*/
  Colorindex index;

  /* use turtle colour if simple fill */
  if (dr->render_mode == RM_FILLED) {
    *color_index = tu->color_index;
    *color_index_back = tu->color_index_back;
    return;
  }

  /* otherwise, Determine shading at the vertex */

  /* Transform the normal, then perform the shading calculation */
  n_x = vertex[NORMAL_X] * tu->left[eX] + vertex[NORMAL_Y] * tu->heading[eX] +
        vertex[NORMAL_Z] * tu->up[eX];

  n_y = vertex[NORMAL_X] * tu->left[eY] + vertex[NORMAL_Y] * tu->heading[eY] +
        vertex[NORMAL_Z] * tu->up[eY];

  n_z = vertex[NORMAL_X] * tu->left[eZ] + vertex[NORMAL_Y] * tu->heading[eZ] +
        vertex[NORMAL_Z] * tu->up[eZ];

  distance = 1.0 / (float)sqrt((double)(n_x * n_x + n_y * n_y + n_z * n_z));
  n_x *= distance;
  n_y *= distance;
  n_z *= distance;

  /* Calculate dot product of vertex normal and lamp normal */
  vertex_lamp = n_x * dr->light_dir[eX] + n_y * dr->light_dir[eY] +
                n_z * dr->light_dir[eZ];

  /* Determine color value on scale of 0-255 */
  color_value = tu->color_index - clp.colormap;

  /* Determine hue of color */
  hue = (int)(color_value / 64);

  /* Determine intensity of color requested on a scale of 0-1 */
  intensity = (color_value - hue * 64.0) / 64.0;

  /* Use given diffuse */
  diffuse = dr->diffuse;

  /* Determine color indices based on intensity,
  ** ambient light, and the contribution of diffuse
  ** light at this viewing angle. */

  index = (int)(64. * intensity * (dr->ambient + diffuse * absf(vertex_lamp)));

  if (index < 1)
    index = 0;
  if (index > 63)
    index = 63;

  index += hue * 64 + clp.colormap;

  *color_index = index;

  if (dr->double_sided) {
    /* Determine color value on scale of 0-255 using polygon color if
     ** non-zero */
    color_value = tu->color_index_back - clp.colormap;

    /* Determine hue of color */
    hue = (int)(color_value / 64);

    /* Determine intensity of color requested on a scale of 0-1 */
    intensity = (color_value - hue * 64.0) / 64.0;

    /* Use given diffuse if not zero */
    diffuse = dr->diffuse;

    /* Determine color indices based on intensity,
    ** ambient light, and the contribution of diffuse
    ** light at this viewing angle. */

    index =
        (int)(64. * intensity * (dr->ambient + diffuse * absf(vertex_lamp)));

    if (index < 1)
      index = 0;
    if (index > 63)
      index = 63;

    index += hue * 64 + clp.colormap;
  }

  *color_index_back = index;
}

/* Output the tsurfaces as rayshade objects. Each time it is drawn by
the turtle, an instantiation of the object will be added to the file */

/***********************************************************************/
void r_tsurfaces(FILE *fp, TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw) {
  int tsurface_index;
  char gll, ourl, tutex;

  for (tsurface_index = 0; tsurface_index < tsurface_number; tsurface_index++) {
    /* identify surface */
    fprintf(fp, "name %c grid 20 20 20\n", tsurface_identifier[tsurface_index]);
    /* store the changed values */
    gll = dr->gllighting;
    ourl = dr->ourlighting;
    tutex = tu->texture;
    dr->ourlighting = dr->ourlighting | dr->gllighting;
    dr->gllighting = 1;
    dr->ourlighting = 0;
    tu->texture = 1;

    draw_tsurface(tu, tsurface_identifier[tsurface_index], dr, vw);
    dr->gllighting = gll;
    dr->ourlighting = ourl;
    tu->texture = tutex;

    /* complete definition */
    fprintf(fp, "end\n");

    fprintf(fp, "\n");
  }
}
