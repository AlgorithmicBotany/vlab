/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifndef NOGRAPHICS

#include "opengl.h"
#include "turtle.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef GLshort Colorindex;
typedef GLfloat Matrix[4][4];
typedef GLfloat Coord;
typedef GLuint Screencoord;

#ifndef TRUE
#define TRUE GL_TRUE
#endif

#ifndef FALSE
#define FALSE GL_FALSE
#endif

#define CIRCLE_SLICES 24 /* for wireframe of a circles */

/* old GL splines */
#define crv(x) Message("Splines not supported!\n")

#define patchbasis(x, y)                                                       \
  {                                                                            \
    if (clp.warnings)                                                          \
      Message("Patches not supported!\n");                                     \
  }
#define patchprecision(x, y)                                                   \
  {                                                                            \
    if (clp.warnings)                                                          \
      Message("Patches not supported!\n");                                     \
  }
#define patchcurves(x, y)                                                      \
  {                                                                            \
    if (clp.warnings)                                                          \
      Message("Patches not supported!\n");                                     \
  }

#define patch(x, y, z)                                                         \
  {                                                                            \
    if (clp.warnings)                                                          \
      Message("Patches not supported!\n");                                     \
  }

/* old GL depthcueing */
#define getdcm() glIsEnabled(GL_FOG)
#define depthcue(x)      {if(x) glEnable(GL_FOG); else glDisable(GL_FOG);}
#define lshaderange(l, h, n, f)                                                \
  {                                                                            \
    glFogf(GL_FOG_START, (GLfloat)n);                                          \
    glFogf(GL_FOG_END, (GLfloat)f);                                            \
    glFogf(GL_FOG_INDEX, (GLfloat)(h - (l)));                                  \
    glIndexi((GLint)l);                                                        \
  }

/**** extension of graphics routines ****/
void setcursor(short, Colorindex, Colorindex);
void my_color(GLint index, GLint index_back);
void text_color(GLint, GLint);

void my_ringbell(void);
void my_swapbuffers(void);
void my_SetBufferType(char double_buffer_on);

#else /* NO_GRAPHICS */

#define TRUE 1
#define FALSE 0

typedef short Colorindex;
typedef float Matrix[4][4];
typedef float Coord;
typedef unsigned int Screencoord;

#endif /* NOGRAPHICS */

/* MATERIALS and COLORMAPS */
struct material_type {
  float ambient[4];
  float diffuse[4];
  float specular[4];
  float emissive[4];
  float shininess;
};
typedef struct material_type material_type;

typedef unsigned char colormap_item_type[4];

/* LIGHTS */
#define NUMLIGHTS 8

struct light_parms {
  float ambient[4];
  float diffuse[4];
  float specular[4];
  float position[4];
  float spot_direction[3];
  float spot_exponent;
  float spot_cutoff;
  float constant_attenuation;
  float linear_attenuation;
  float quadratic_attenuation;
};
typedef struct light_parms light_parms;

light_parms *get_light(int index);

#define DIRECTIONLIGHTCOEF 1000 /* to make point light out of directional */

/* windowing */
int InitializeGraphics(
#ifdef LINUX
    int, char **
#else
    void
#endif
);
void Change_Resolution(void);
void GetWindowOrigin(int *xorg, int *yorg);
int InitializeOpenGLGraphics();
void SetGraphics(void);
int Update_Rotation(void);
int Update_Scale(void);
int Update_Pan(void);
void GetTextExtent(char *str, int *width, int *ascent, int *descent);
void DrawString(char *str, const TURTLE *tu);
void CopyFromPixmap(void);
void makeRasterFont(void);
void FreeGraphics(void);
void Message(const char *, ...);

#ifdef LINUX
void GetWindowSize(int *width, int *height);
#endif

#ifdef WIN32
void SetIdle(void);
#endif

#ifndef NOGRAPHICS
#ifdef __cplusplus
}
#endif
#endif

#endif
