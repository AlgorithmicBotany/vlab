/* ======================================================================== *
**
**             T E X T U R E S
*/
/* ------------------------------------------------------------------------ */

// to avoid GLu Warning due to deprecated function on MacOs
#define GL_SILENCE_DEPRECATION

#ifdef WIN32
#include "warningset.h"
#endif

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "control.h"
#include "interpret.h"

#ifdef WIN32
#include <image.h>
#else
#include "image.h"
#endif

#include "textures.h"
#include "utility.h"
#include "targa.h"
#include "rle.h"

#include "test_malloc.h"

#include "lodepng.h"

#ifdef IRIX
#define glDeleteTextures glDeleteTexturesEXT
#define glGenTextures glGenTexturesEXT
#define glBindTexture glBindTextureEXT
#endif
#define WARNING_LVL 0
/* TEXTURE ********************************/
static TEXTURET textures[NUMTEXTURES];
extern DRAWPARAM drawparam;

int gl_numtextures;
int current_texture;
int GenTexturesCount = 0;

/* ------------------------------------------------------------------------ */

void FreeTextureSpace(void) {
  int i;

  for (i = 0; i < gl_numtextures; i++) {
    Free(textures[i].mipmapImage);
    textures[i].mipmapImage = NULL;
    Free(textures[i].filename);
    textures[i].filename = NULL;
    glDeleteTextures(1, &(textures[i].id));
    GenTexturesCount--;
  }
  gl_numtextures = 0;
  current_texture = -1;
}

/* ------------------------------------------------------------------------ */
TEXTURET *GetTexture(int index) {
  if (!is_valid_texture_index(index))
    return NULL;

  return textures + index;
}

/* ------------------------------------------------------------------------ */
int is_valid_texture_index(int index) {
  return (index >= 0) && (index < gl_numtextures);
}

/* ------------------------------------------------------------------------ */
int texture_type(int index) {
  if (!is_valid_texture_index(index))
    return 0;

  return textures[index].per_surface ? TEXELS_PER_SURFACE : TEXELS_PER_PATCH;
}

/* ------------------------------------------------------------------------ */
int find_texture_per_patch(void) {
  int i;

  for (i = 0; i < gl_numtextures; i++) {
    if (!textures[i].per_surface)
      return i;
  }
  return -1;
}

/* ------------------------------------------------------------------------ */
/* returns the increase in tex_t parameter according to the width and length
   of the movement
*/
float update_segment_texture(int index, float line_width, float line_lenght) {
  if (is_valid_texture_index(index)) {
    if (line_width == 0)
      return 0.0f;
    else
      return (float)(line_lenght / (M_PI * line_width) * textures[index].width /
                     textures[index].height / textures[index].ratio);
  } else
    return 0.0f;
}

static int LoadTGATexture(const char *fname, int index) {
  int x, y;
  int xsize, ysize;
  unsigned char *cbuf;
  targa_params_type TGAspec;

  if (!loadTGAhead(fname, &TGAspec)) {
    Message("Texture: cannot open input targa image %s.\n", fname);
    return 0;
  }

  /* make it closest lower power of two (6->4, 4->4, 7->4) */
  xsize = 1;
  while (TGAspec.Xres >= xsize)
    xsize <<= 1;
  xsize >>= 1;

  ysize = 1;
  while (TGAspec.Yres >= ysize)
    ysize <<= 1;
  ysize >>= 1;

  if (clp.warnings)
    if ((TGAspec.Xres != xsize) || (TGAspec.Yres != ysize))
      Message("Texture image has been clipped to have sides "
              "of power of two!\n");

  if ((cbuf = (unsigned char *)Malloc(TGAspec.Xres * 3 *
                                      sizeof(unsigned char))) == NULL) {
    Message("Texture: cannot allocate memory for image %s!\n", fname);
    return 0;
  }

  if ((textures[index].mipmapImage =
           (GLubyte *)Malloc(xsize * ysize * 3 * sizeof(GLubyte))) == NULL) {
    Message("Texture: cannot allocate memory for image %s!\n", fname);
    return 0;
  }

  /* read the image in */
  /* image is stored in rows of R, G, and B. We need R,G,B for each
  pixel */
  for (y = TGAspec.Yres - 1; y >= ysize; y--)
    loadTGArow(&TGAspec, y, cbuf);

  for (y = ysize - 1; y >= 0; y--) {
    loadTGArow(&TGAspec, y, cbuf);

    for (x = 0; x < xsize; x++) {
      textures[index].mipmapImage[y * xsize * 3 + x * 3 + 2] = cbuf[3 * x];
      textures[index].mipmapImage[y * xsize * 3 + x * 3 + 1] = cbuf[3 * x + 1];
      textures[index].mipmapImage[y * xsize * 3 + x * 3 + 0] = cbuf[3 * x + 2];
    }
  }
  Free(cbuf);
  cbuf = NULL;
  loadTGAfinish(&TGAspec);

  textures[index].width = xsize;
  textures[index].height = ysize;
  return 1;
}

static int LoadRLETexture(const char *fname, int index) {
  int x, y;
  int xsize, ysize;
  RLE_params_type RLEspec;
  unsigned char *cbuf;
  if (!loadRLEhead(fname, &RLEspec)) {
    Message("Texture: cannot open input rle image %s.\n", fname);
    return 0;
  }

  /* make it closest lower power of two (6->4, 4->4, 7->4) */
  xsize = 1;
  while (RLEspec.Xres >= xsize)
    xsize <<= 1;
  xsize >>= 1;

  ysize = 1;
  while (RLEspec.Yres >= ysize)
    ysize <<= 1;
  ysize >>= 1;

  if (clp.warnings)
    if ((RLEspec.Xres != xsize) || (RLEspec.Yres != ysize))
      Message("Texture image has been clipped to have sides "
              "of power of two!\n");
  if ((cbuf = (unsigned char *)Malloc(RLEspec.Xres * 3 *
                                      sizeof(unsigned char))) == NULL) {
    Message("Texture: cannot allocate memory for image %s!\n", fname);
    return 0;
  }

  if ((textures[index].mipmapImage =
           (GLubyte *)Malloc(xsize * ysize * 3 * sizeof(GLubyte))) == NULL) {
    Message("Texture: cannot allocate memory for image %s!\n", fname);
    return 0;
  }

  /* read the image in */
  /* image is stored in rows of R, G, and B. We need R,G,B for each
  pixel */
  for (y = 0; y < ysize; y++) {
    loadRLErow(&RLEspec, y, cbuf);

    for (x = 0; x < xsize; x++) {
      textures[index].mipmapImage[y * xsize * 3 + x * 3 + 0] = cbuf[x];
      textures[index].mipmapImage[y * xsize * 3 + x * 3 + 1] =
          cbuf[x + RLEspec.Xres];
      textures[index].mipmapImage[y * xsize * 3 + x * 3 + 2] =
          cbuf[x + 2 * RLEspec.Xres];
    }
  }
  Free(cbuf);
  cbuf = NULL;
  loadRLEfinish(&RLEspec);

  textures[index].width = xsize;
  textures[index].height = ysize;
  return 1;
}

#ifdef _WINDOWS
static int LoadBMPTexture(const char *fname, int index) {
  int x, y;
  int xsize, ysize;
  unsigned char *cbuf;
  int mipmapImageAllocSize;
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER bmih;
  union {
    WORD bfType;
    char bfc[2];
  } chcktp;
  FILE *fp;

  fp = fopen(fname, "rb");
  if (NULL == fp) {
    Message("Texture: cannot open image %s\n", fname);
    return 0;
  }
  fread(&bmfh, sizeof(BITMAPFILEHEADER), 1, fp);
  chcktp.bfc[0] = 'B';
  chcktp.bfc[1] = 'M';
  if (chcktp.bfType != bmfh.bfType) {
    Message("File %s doesn't seem to be a valid BMP file\n", fname);
    fclose(fp);
    return 0;
  }

  fread(&(bmih.biSize), sizeof(DWORD), 1, fp);
  if (bmih.biSize > sizeof(BITMAPINFOHEADER)) {
    Message("File %s doesn't seem to be a valid BMP file\n", fname);
    fclose(fp);
    return 0;
  }

  fread(&(bmih.biWidth), bmih.biSize - sizeof(DWORD), 1, fp);

  xsize = 1;
  while (xsize < bmih.biWidth)
    xsize <<= 1;
  if (xsize != bmih.biWidth) {
    Message("Invalid dimensions of file %s\n", fname);
    fclose(fp);
    return 0;
  }
  ysize = 1;
  while (ysize < bmih.biHeight)
    ysize <<= 1;
  if (ysize != bmih.biHeight) {
    Message("Invalid dimensions of file %s\n", fname);
    fclose(fp);
    return 0;
  }
  switch (bmih.biBitCount) {
  case 1:
  case 4:
  case 8: {
    Message("Texture file %s is a colormap image.\n", fname);
    fclose(fp);
    return 0;
  } break;
  case 24: {
    int bfsz = 3 * xsize;
    int ix;
    assert(0 == bfsz % 4);
    /*
    int pad = 4- (bfsz % 4);
    if (4 == pad)
    pad = 0;
    buf = Malloc(bfsz+pad);
    */
    cbuf = Malloc(bfsz);
    if (NULL == cbuf) {
      Message("Out of memory while reading texture file %s\n", fname);
      fclose(fp);
      return 0;
    }
    mipmapImageAllocSize = xsize * ysize * 3 * sizeof(GLubyte);
    textures[index].mipmapImage = (GLubyte *)Malloc(mipmapImageAllocSize);
    if (NULL == textures[index].mipmapImage) {
      Message("Out of memory while reading texture file %s\n", fname);
      Free(cbuf);
      cbuf = NULL;
      fclose(fp);
      fp = NULL;
      return 0;
    }

    for (y = 0; y < ysize; y++) {
      fread(cbuf, 3 * sizeof(unsigned char), xsize, fp);
      ix = 0;
      for (x = 0; x < xsize; x++) {
        textures[index].mipmapImage[y * xsize * 3 + x * 3 + 2] = cbuf[ix];
        ix++;
        textures[index].mipmapImage[y * xsize * 3 + x * 3 + 1] = cbuf[ix];
        ix++;
        textures[index].mipmapImage[y * xsize * 3 + x * 3 + 0] = cbuf[ix];
        ix++;
      }
    }
    Free(cbuf);
    cbuf = NULL;
  } break;
  }

  fclose(fp);
  textures[index].width = xsize;
  textures[index].height = ysize;
  return 1;
}
#endif

static int LoadPNGTexture(const char *filename, int index) {
  unsigned error;
  unsigned char *image;
  unsigned width, height;
  int mipmapImageAllocSize;
  size_t jump = 1;
  unsigned int xsize = 1;
  unsigned int ysize = 1;
  int count = 0;
  unsigned int x, y;
    
  error = lodepng_decode32_file(&image, &width, &height, filename);
  
  while ((error != 0) && (count < 100)) {
    error = lodepng_decode32_file(&image, &width, &height, filename);
    count++;
  }
  if (count == 100)
    return 0;

  /*use image here*/
  textures[index].type = RGBAtexture;
  mipmapImageAllocSize = width * height * 4 * sizeof(GLubyte);

  if ((textures[index].mipmapImage = (GLubyte *)Malloc(mipmapImageAllocSize)) ==
      NULL) {
    Message("Texture: Cannot allocate memory for image %s!\n", filename);
    return 0;
  }
  if (width / 4096 >= jump)
    jump = width / 4096 + 1;
  if (height / 4096 >= jump)
    jump = height / 4096 + 1;
  while (xsize < width)
    xsize <<= 1;

  while (ysize < height)
    ysize <<= 1;
#ifdef _WINDOWS
  if ((height != ysize) || (width != xsize)) {
    if (drawparam.openGL_2 == 0) {
      Message("Warning! OpenGL 2.0 is not supported, PNG texture must be a "
              "power of 2 \n",WARNING_LVL);
    }
  }
#endif

  /*plot the pixels of the PNG file*/
  for (y = 0; y + jump - 1 < height; y += jump) {
    for (x = 0; x + jump - 1 < width; x += jump) {
      /*get RGBA components*/
      // For some reason, the image from lodepng is flipped and then it should
      // be return to get the proper texture in cpfg
      textures[index].mipmapImage[4 * y * width + 4 * x + 0] =
          image[4 * (height - y - jump) * width + 4 * (x) + 0]; /* r */
      textures[index].mipmapImage[4 * y * width + 4 * x + 1] =
          image[4 * (height - y - jump) * width + 4 * (x) + 1]; /* g */
      textures[index].mipmapImage[4 * y * width + 4 * x + 2] =
          image[4 * (height - y - jump) * width + 4 * (x) + 2]; /* b */
      textures[index].mipmapImage[4 * y * width + 4 * x + 3] =
          image[4 * (height - y - jump) * width + 4 * (x) + 3]; /* alpha */
    }
  }

  free(image);
  image = NULL;
  textures[index].width = width;
  textures[index].height = height;
  return 1;
}

static int LoadRGBTexture(const char *fname, int index) {
  int x, y;
  int xsize, ysize;
  IMAGE *image;
  unsigned short *buf;
  int mipmapImageAllocSize;
  image = iopen(fname, "r", 0, 0, 0, 0, 0);
  if (NULL == image) {
    Message("Texture: Cannot open image %s!\n", fname);
    return 0;
  }

  /* make it closest lower power of two (6->4, 4->4, 7->4) */
  xsize = 1;
  while (image->xsize >= xsize)
    xsize <<= 1;
  xsize >>= 1;

  ysize = 1;
  while (image->ysize >= ysize)
    ysize <<= 1;
  ysize >>= 1;

  if (clp.warnings) {
#ifdef _WINDOWS
    if ((image->xsize != xsize) || (image->ysize != ysize))
      Message("Texture image has been clipped to have sides "
              "of power of two!\n");
#endif
  }
  if ((buf = (unsigned short *)Malloc(image->xsize * sizeof(unsigned short))) ==
      NULL) {
    Message("Texture: Cannot allocate memory for image %s!\n", fname);
    return 0;
  }

  if (image->zsize == 4) {
    textures[index].type = RGBAtexture;
    mipmapImageAllocSize = xsize * ysize * 4 * sizeof(GLubyte);
  } else {
    textures[index].type = RGBtexture;
    mipmapImageAllocSize = xsize * ysize * 3 * sizeof(GLubyte);
  }

  if ((textures[index].mipmapImage = (GLubyte *)Malloc(mipmapImageAllocSize)) ==
      NULL) {
    Message("Texture: Cannot allocate memory for image %s!\n", fname);
    return 0;
  }

  /* read the image in */
  /* image is stored in rows of R, G, and B. We need R,G,B for each pixel
   */

  for (y = 0; y < ysize; y++) {
    if (0 != getrow(image, buf, y, 0))
      Message("Error reading texture file token\n");
    for (x = 0; x < xsize; x++)
      textures[index]
          .mipmapImage[y * xsize * image->zsize + x * image->zsize + 0] =
          buf[x];

    if (0 != getrow(image, buf, y, 1))
      Message("Error reading texture file token\n");
    for (x = 0; x < xsize; x++)
      textures[index]
          .mipmapImage[y * xsize * image->zsize + x * image->zsize + 1] =
          buf[x];

    if (0 != getrow(image, buf, y, 2))
      Message("Error reading texture file token\n");
    for (x = 0; x < xsize; x++)
      textures[index]
          .mipmapImage[y * xsize * image->zsize + x * image->zsize + 2] =
          buf[x];

    if (RGBAtexture == textures[index].type) {
      if (0 != getrow(image, buf, y, 3))
        Message("Error reading texture file token\n");
      for (x = 0; x < xsize; x++)
        textures[index]
            .mipmapImage[y * xsize * image->zsize + x * image->zsize + 3] =
            buf[x];
    }
  }

  Free(buf);
  buf = NULL;
  iclose(image);
  image = NULL;
  textures[index].width = xsize;
  textures[index].height = ysize;
  return 1;
}

// 0 if false
// 1 if true
static int IsPowerOfTwo(GLsizei x) {
  if (x < 0) {
    return 0;
  }
  while (x > 1 && (x % 2) == 0) {
    x /= 2;
  }
  return x == 1;
}

static int LoadFile(const char *fname, int index) {
  /* file name - necessary */
  int c;
  int status = 0;

  c = strlen(fname);
  if (c <= 3)
    return 0;

  textures[index].filename = (char *)Malloc(strlen(fname) + 1);
  if (textures[index].filename == NULL) {
    Message("Texture: not enough memory!\n");
    return 0;
  }
  strcpy(textures[index].filename, fname);

  if (strcmp(fname + c - 3, "tga") == 0)
    status = LoadTGATexture(fname, index);
  else if (strcmp(fname + c - 3, "rle") == 0)
    status = LoadRLETexture(fname, index);
#ifdef _WINDOWS
  else if (strcmp(fname + c - 3, "bmp") == 0)
    status = LoadBMPTexture(fname, index);
#endif
  /* otherwise load in rgb image */
  else if (strcmp(fname + c - 3, "rgb") == 0)
    status = LoadRGBTexture(fname, index);
  /*or in png format */
  else if (strcmp(fname + c - 3, "png") == 0) {
    status = LoadPNGTexture(fname, index);
  }
  if (!status) {
    return status;
  }
#ifdef _WINDOWS
  if (!IsPowerOfTwo(textures[index].width) ||
      !IsPowerOfTwo(textures[index].height)) {
    Message("\"%s\" texture dimensions are not a power-of-two!\n",
            textures[index].filename);
  }
  #endif

  return 1;
}

/* ------------------------------------------------------------------------ */
int read_texture(char *input_line) {

  char *token;
  int index = gl_numtextures;
  int i = 0;

  if (index >= NUMTEXTURES) {
    Message("Too many textures!\n");
    return 0;
  }

  /* defaults */

  textures[index].min_filter = GL_NEAREST;
  textures[index].mag_filter = GL_NEAREST;
  textures[index].wrap_s = GL_REPEAT;
  textures[index].wrap_t = GL_REPEAT;
  textures[index].width = 0;
  textures[index].height = 0;
  textures[index].ratio = 1.0;
  textures[index].env_mode = GL_MODULATE;
  textures[index].per_surface = 0;
  textures[index].mipmap = 0; // AJB no mipmaps
  textures[index].type = RGBtexture;

  token = strtok(input_line, " \t:");

  for (;;) {
    ++i;
    if (token == NULL)
      break;

    switch (token[0]) {
    case 'F':
      token = strtok(NULL, " \t:\n,");
      strcpy(clp.textureFiles[index], token);
      if (0 == LoadFile(token, index))
        return 0;
      clp.initTexture = 1;
      break;

    case 'L': /* min filter */
      token = strtok(NULL, " \t:\n,");
      switch (token[0]) {
      case 'n':
        textures[index].min_filter = GL_NEAREST;
        break;
      case 'l':
        textures[index].min_filter = GL_LINEAR;
        break;
        /* AJB adding support for mipmaps */
      case 'm':
        textures[index].min_filter = GL_LINEAR_MIPMAP_LINEAR;
        textures[index].mipmap = 1;

        /* Radek - more control */
        if (token[1] != 0) {
          switch (token[1]) {
          case 'n':
            textures[index].min_filter = token[2] == 'n'
                                             ? GL_NEAREST_MIPMAP_NEAREST
                                             : GL_NEAREST_MIPMAP_LINEAR;
            break;
          case 'l':
            textures[index].min_filter = token[2] == 'n'
                                             ? GL_LINEAR_MIPMAP_NEAREST
                                             : GL_LINEAR_MIPMAP_LINEAR;
            break;
          }
        }
        break;
      }
      break;

    case 'H': /* mag filter */
      token = strtok(NULL, " \t:\n,");
      switch (token[0]) {
      case 'n':
        textures[index].mag_filter = GL_NEAREST;
        break;
      case 'l':
        textures[index].mag_filter = GL_LINEAR;
        break;
      }
      break;

    case 'E': /* env_mode */
      token = strtok(NULL, " \t:\n,");
      switch (token[0]) {
      case 'm':
        textures[index].env_mode = GL_MODULATE;
        break;
      case 'd':
        textures[index].env_mode = GL_DECAL;
        break;
      case 'b':
        textures[index].env_mode = GL_BLEND;
        break;
      }
      break;

    case 'S': /* texture per surface, not per patch */
      textures[index].per_surface = 1;
      break;

    case 'R': /* H/W ratio for cylindrical textures */
      token = strtok(NULL, " \t:\n,");
      if (token != NULL)
        textures[index].ratio = atof(token);
      break;

    default:
      Message("Texture: unknown command '%c'.\n", token[0]);
      return 0;
    }
    token = strtok(NULL, " \t:\n");
  }

  /* parsing OK */

  if ((textures[index].width == 0) || (textures[index].height == 0))
    return 0;

/* Textures binding, added: RadekK 14-Apr-99 */
// MC - Dec. 2015 - these calls cannot be made if there is no active OpenGL
// context because glGenTextures causes a segmentation fault in that case
// However, on Windows there seems to be an active OpenGL context but not on Mac
#ifndef WIN32
  if (!clp.graphics_output) {
    Message("Textures are not supported in off-screen rendering mode.\n");
  } else
#endif
  {
    glGenTextures(1, &(textures[index].id));
    GenTexturesCount++;
    glBindTexture(GL_TEXTURE_2D, textures[index].id);

    /* The stuff below moved here from iGLStartTexture */

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, textures[index].env_mode);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    textures[index].min_filter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    textures[index].mag_filter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textures[index].wrap_s);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textures[index].wrap_t);

    /** AJB added stuff for mipmapping  4/26/98 **/
    if (textures[index].mipmap) {
      if (RGBtexture == textures[index].type) {
        if ((gluBuild2DMipmaps(GL_TEXTURE_2D, 3, /* always RGB	 */
                               textures[index].width, textures[index].height,
                               GL_RGB, GL_UNSIGNED_BYTE,
                               textures[index].mipmapImage)) != 0)
          Message("MIPMAPS failed");
      } else {
        if ((gluBuild2DMipmaps(GL_TEXTURE_2D, 4, /* unless RGBA */
                               textures[index].width, textures[index].height,
                               GL_RGBA, GL_UNSIGNED_BYTE,
                               textures[index].mipmapImage)) != 0)
          Message("MIPMAPS failed");
      }
    } else {
      if (RGBtexture == textures[index].type) {
        glTexImage2D(GL_TEXTURE_2D, 0, /* not more than one level */
                     GL_RGB,                /* always R,G,B */
                     textures[index].width, textures[index].height,
                     0, /* no border */
                     GL_RGB, GL_UNSIGNED_BYTE, textures[index].mipmapImage);
      } else {
        glTexImage2D(GL_TEXTURE_2D, 0, /* not more than one level */
                     GL_RGBA,                /* unless RGBA */
                     textures[index].width, textures[index].height,
                     0, /* no border */
                     GL_RGBA, GL_UNSIGNED_BYTE, textures[index].mipmapImage);
      }
    }

    glEnable(GL_TEXTURE_2D);

    glPopMatrix(); /* see above */
    glMatrixMode(GL_MODELVIEW);

    glBindTexture(GL_TEXTURE_2D, 0);
    // set active texture unit's default texture environment
    // back to GL_MODULATE
    // This is needed because shadows mode uses a default white texture.
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    gl_numtextures++;
  }

  current_texture = -1;

  return 1;
}

/* ------------------------------------------------------------------------ */
int SaveTextureToRLE(char *filename, int index) {
  RLE_params_type RLEspec = {0};
  unsigned char *row;
  int x, y;

  if (!is_valid_texture_index(index))
    return 0;

  if ((RLEspec.fp = fopen(filename, "wb")) == NULL) {
    Message("Cannot open file %s for writing!\n", filename);
    return 0;
  }

  RLEspec.Xres = textures[index].width;
  RLEspec.Yres = textures[index].height;

  if ((row = (unsigned char *)Malloc(RLEspec.Xres * 3 *
                                     sizeof(unsigned char))) == NULL) {
    Message("Texture: Cannot allocate memory for image row!\n");
    return 0;
  }

  saveRLEhead(&RLEspec);

  for (y = 0; y < RLEspec.Yres; y++) {
    for (x = 0; x < RLEspec.Xres; x++)
      row[x] = textures[index].mipmapImage[y * RLEspec.Xres * 3 + x * 3 + 0];

    for (x = 0; x < RLEspec.Xres; x++)
      row[x + RLEspec.Xres] =
          textures[index].mipmapImage[y * RLEspec.Xres * 3 + x * 3 + 1];

    for (x = 0; x < RLEspec.Xres; x++)
      row[x + 2 * RLEspec.Xres] =
          textures[index].mipmapImage[y * RLEspec.Xres * 3 + x * 3 + 2];

    saveRLErow(&RLEspec, y, row);
  }

  Free(row);
  row = NULL;

  saveRLEfinish(&RLEspec);
  return 1;
}

/* ------------------------------------------------------------------------ */
/* returns 0 if index out of range */

int iGLStartTexture(int index) {
  if (!is_valid_texture_index(index))
    return 0;

  if (current_texture == index)
    return 1;

  glBindTexture(GL_TEXTURE_2D, textures[index].id);
  glEnable(GL_TEXTURE_2D);

  /* All the stuff previously here moved to read_texture */
  // was put back in Jan 2022 because the texturing parameters can differ

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, textures[index].env_mode);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  textures[index].min_filter);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  textures[index].mag_filter);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textures[index].wrap_s);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textures[index].wrap_t);

  current_texture = index;

  return 1;
}

/* ------------------------------------------------------------------------ */
void iGLEndTexture(int index) {
  glDisable(GL_TEXTURE_2D);
  // MC Oct. 2015 - need to bind default texture for shadow mapping
  glBindTexture(GL_TEXTURE_2D,0);
  current_texture = -1;
}

/* ------------------------------------------------------------------------ */
/* Given a polygon, set texture coordinates:
   unit length axes s and t,
   min_s: distance from line  vec(s_axis).vec(X) = 0 corresponding to s=0
   min_t: distance from line  vec(t_axis).vec(X) = 0 corresponding to t=0
   del_s: 1.0/step along the s_axis corresponding to unit step in 's'
   del_t: 1.0/step along the t_axis corresponding to unit step in 't'
*/
void SetPolygonTextureParams(POLYGON *polygon, double *s_axis, double *t_axis,
                             double *min_s, double *min_t, double *del_s,
                             double *del_t) {
  double s, t, mins, maxs, mint, maxt;
  int i;

  t_axis[0] = polygon->vertex[0].heading[0];
  t_axis[1] = polygon->vertex[0].heading[1];
  t_axis[2] = polygon->vertex[0].heading[2];

  s_axis[0] = -polygon->vertex[0].left[0];
  s_axis[1] = -polygon->vertex[0].left[1];
  s_axis[2] = -polygon->vertex[0].left[2];

  /* t_axis is normal for a line going along s texture coordinates */
  mint = maxt = DDotProduct(polygon->vertex[0].position, t_axis);

  /* s_axis is normal for a line going along t texture coordinates */
  mins = maxs = DDotProduct(polygon->vertex[0].position, s_axis);

  for (i = 1; i < polygon->edge_count; i++) {
    t = DDotProduct(polygon->vertex[i].position, t_axis);

    if (t < mint)
      mint = t;
    if (t > maxt)
      maxt = t;

    s = DDotProduct(polygon->vertex[i].position, s_axis);

    if (s < mins)
      mins = s;
    if (s > maxs)
      maxs = s;
  }

  *del_s = 1.0 / (maxs - mins);
  *del_t = 1.0 / (maxt - mint);

  *min_s = mins;
  *min_t = mint;
}

/* ------------------------------------------------------------------------ */
int voidStartTexture(int index) {
  return 0; /* no texture */
}

void voidEndTexture(int index) {}
