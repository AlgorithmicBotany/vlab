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



#ifndef __TEXTURES_H__
#define __TEXTURES_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "comlineparam.h"
#define NOTEXTURE -1
#define NO_TEXELS_YET 0
#define TEXELS_PER_PATCH 1
#define TEXELS_PER_SURFACE 2

/* TEXTURE ********************************/

enum TextureType { RGBtexture, RGBAtexture };

typedef enum TextureType TextureType;

struct TEXTURET {
  char *filename;
  GLsizei width;
  GLsizei height;
  GLfloat ratio;        /* H/W ratio. Used for cylidrical textures */
  GLubyte *mipmapImage; /* just one level */
  /* RGB or RGBA depending on type field */
  char mipmap;        /* AJB does the user want mitmapping */
  GLfloat min_filter; /* for glTexParameter */
  GLfloat mag_filter;
  GLfloat wrap_s;
  GLfloat wrap_t;
  char per_surface; /* image is mapped on the whole surface or on each
patch */
  /* border ignored */

  GLfloat env_mode;
  TextureType type;
  /* For texture binding : RadekK 15-Apr-99 */
  GLuint id;
};

typedef struct TEXTURET TEXTURET;

/* prototypes */
void FreeTextureSpace(void);
int is_valid_texture_index(int index);
int texture_type(int index);
int find_texture_per_patch(void);
int read_texture(char *input_line);
int SaveTextureToRLE(char *filename, int index);
TEXTURET *GetTexture(int index);
float update_segment_texture(int index, float line_width, float line_lenght);
void SetPolygonTextureParams(POLYGON *polygon, double *s_axis, double *t_axis,
                             double *min_s, double *min_t, double *del_s,
                             double *del_t);

int iGLStartTexture(int index);
void iGLEndTexture(int index);
int voidStartTexture(int index);
void voidEndTexture(int index);

#ifdef __cplusplus
}
#endif

#else
#error File already included
#endif
