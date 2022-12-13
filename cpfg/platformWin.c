/* PLATFORM SPECIFIC FUNCTIONS */
/* MS Windows */

/* May 98 : Radek Karwowski */

#ifdef WIN32
#endif

#ifndef WIN32
#error This file to be compiled in MS Windows version only
#endif

#include "warningset.h"

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include <windows.h>

#ifndef NOGRAPHICS
#include <gl/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "shaders.h" // MC - July 2016 - added shadow mapping
#endif               /* NOGRAPHICS */

#define TIMEOUT_TIME 100 /* [msec] timeout for checking the server socket */

#include "platform.h"
#include "platformmenu.h"
#include "control.h"
#include "interpret.h"
#include "utility.h"
#include "turtleDrawDispatcher.h"
#include "irisGL.h"
#include "comlineparam.h"

#ifdef RESEARCH_VER
#include "server.h"
#endif

#include "test_malloc.h"

#include "wininterface.h"

static int screen_height;

#ifdef WIN32
int popupmenuon;
#endif

/*extern COMLINEPARAM clp;*/
extern VIEWPARAM viewparam;
extern DRAWPARAM drawparam;

extern int animateFlag;
extern int updateFlag;

extern void MyExit(int status);

int InitializeOpenGLGraphics(void);

void SetView(VIEWPARAM *viewPtr);

static char Idle_Func(void *ptr) {
  return (char)IdleFunction(); /* return 1 if finished */
}

int initialized = FALSE;
int first_run = FALSE;
int double_buffering = 1;

void my_ringbell(void) { MessageBeep(0xFFFFFFFF); }

void SetIdle(void) { SetWindowsIdleFunc(Idle_Func); }

void GetTextExtent(char *str, int *width, int *ascent, int *descent) {
  *width = 50;
  *ascent = 10;
  *descent = 10;
}

void my_getmcolor(Colorindex colindex, short *red, short *green, short *blue) {
  material_type *mat;
  colormap_item_type *item;

  if (clp.iscolormapfile) {
    my_getcolormapitem(colindex, &item);

    *red = (*item)[0];
    *green = (*item)[1];
    *blue = (*item)[2];
    return;
  }

  if (clp.ismaterialfile) {
    my_getmaterial(colindex, &mat);

    *red = (short)(255.0 * mat->diffuse[0]);
    *green = (short)(255.0 * mat->diffuse[1]);
    *blue = (short)(255.0 * mat->diffuse[2]);
    return;
  }
}

void my_SetBufferType(char double_buffer_on) {
  if (clp.graphics_output) {
    if (double_buffer_on) {
      glDrawBuffer(GL_BACK);
      double_buffering = 1;
    }
    if (!double_buffer_on) {
      glDrawBuffer(GL_FRONT);
      double_buffering = 0;
    }
  }
}

void init_window(void) {
  if (clp.iscolormapfile)
    load_in_colormaps();

  if (clp.ismaterialfile)
    load_in_materials();
}

void OnSKeyDown(void) {
  extern int swapinterval;
  animateFlag = STOP;
  swapinterval = 0;
}

void OnLButtonDown(int pos_x, int pos_y) {
  updateFlag = UPDATE_ROTATION;
  viewparam.xStart = (short)pos_x;
  viewparam.yStart = (short)pos_y;
  viewparam.xLast = (short)viewparam.xRotation;
  viewparam.yLast = (short)viewparam.yRotation;

  SetIdle(); /* rotation is done by working procedure */
}

void OnMButtonDown(int pos_x, int pos_y) {
  updateFlag = UPDATE_SCALE;
  viewparam.yStart = (short)pos_y;
  SetIdle();
}

void StartPan(int pos_x, int pos_y) {
  updateFlag = UPDATE_PAN;
  viewparam.xStart = (short)pos_x;
  viewparam.yStart = (short)pos_y;
  SetIdle();
}

void OnButtonUp(void) {
  if (eTriggered == viewparam.GenerateOnViewChange)
    RegenerateString();

  updateFlag = NO_UPDATE;
}

void Reshape(int width, int height) {
  // int setview = SETVIEWOFF;
  extern char *currentString;

  if ((width == clp.xsize) && (height == clp.ysize))
    return;
  clp.xsize = width;
  clp.ysize = height; /* do not remove. Used in SetView() */

  if (!initialized) {
    initialized = TRUE;

    if (!first_run) {
      /* first cpfg run */
      FirstRun();
      first_run = TRUE;
    }

    InitializeOpenGLGraphics(); // should be after FirstRun() because view
                                // parameters are read in FirstRun()

    InitializeDispatcher(); /* should be after FirstRun() */

    CalculateViewVolume(currentString, &drawparam, &viewparam);
  }

  SetView(&viewparam); /* view volume is the same */
}

int InitializeOpenGLGraphics(void) {
  GLenum err;

  SetGraphics();

  if (clp.doublebuffer)
    glDrawBuffer(GL_FRONT);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (clp.doublebuffer) {
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  /* for proper orientation of normals */
  glFrontFace(GL_CW);

  // MC - July 2016 - support for shadow mapping using GLSL 1.2 shaders
  // first some housekeeping, obtain the addresses of the OpenGL extension entry
  // points (these are not provided on Windows)
  err = glewInit();
  if (GLEW_OK != err) {
    Message("Warning! the 'render mode: shadows' view option will not work.\n"
            "Cannot initialize OpenGL extensions: %s\n",
            glewGetErrorString(err));
    drawparam.render_mode = RM_SHADED;
  }
  drawparam.openGL_2 = 1;
  if (!glewIsSupported("GL_VERSION_2_0")) {
    drawparam.openGL_2 = 0;
    Message("Warning! the 'render mode: shadows' view option will not work.\n"
            "OpenGL 2.0 is not supported and shadows will not be rendered.\n");
    drawparam.render_mode = RM_SHADED;
  }

  // MIK must change the following code

  // check if shaders are supported (GL version >= 2.0) and if frame buffers are
  // supported (GL extension)
  if (!glewIsSupported("GL_VERSION_2_0 GL_ARB_framebuffer_object")) {
    Message("Warning! the 'render mode: shadows' view option will not work.\n"
            "OpenGL 2.0 or the frame buffer extension is not supported and "
            "shadows will not be rendered.\n");
    drawparam.render_mode = RM_SHADED;
  } else {
    // even if "render mode: shadows" is not specified when the OpenGL context
    // is first created, the shaders are loaded in case the render mode is
    // changed in the view file later
    char path[_MAX_PATH + 1];
    size_t l;
    GetModuleFileName(NULL, path, _MAX_PATH); // get path to cpfg.exe
    l = strlen(path);
    // remove 'cpfg.exe' from the path
    while (path[l - 1] != '\\' && l > 0)
      --l;
    path[l - 1] = '\0';
    // add '\'
    strcat(path, "\\");
    ++l;
    // init the shadow map
    initShadowMap(path);
  }

  return 1;
}

void draw_scene_callback() {
  unsigned int rflags = 0;

#ifdef WIN32
  if (!popupmenuon)
#endif
    setcursor(CURSOR_HOURGLASS, 0, 0);

  if (!initialized) {
    initialized = TRUE;

    if (!first_run) {
      /* first cpfg run */
      FirstRun();
      first_run = TRUE;
      rflags = rfScale | rfViewVol;
    }

    InitializeOpenGLGraphics(); // should be after FirstRun() because view
                                // parameters are read in FirstRun()

    InitializeDispatcher(); /* should be after FirstRun() */
  }
  // MC July 2016 - if in 'shadows' mode, set flag to generate shadow map
  if (drawparam.render_mode == RM_SHADOWS)
    rflags = rflags | rfShadows;

  Redraw(rflags | rfClear);
  setcursor(CURSOR_ARROW, 0, 0);
}

void setcursor(short c, Colorindex f, Colorindex b) {
  switch (c) {
  case CURSOR_ARROW:
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    break;
  case CURSOR_HOURGLASS:
    SetCursor(LoadCursor(NULL, IDC_WAIT));
    break;
  }
}

float bgclr[3];

void SetGraphics(void) {
  extern DRAWPARAM drawparam;
  short red, green, blue;
  material_type *mat;

  glDisable(GL_LIGHTING);
  glDisable(GL_NORMALIZE);

  if (clp.iscolormapfile) { /* RGBA mode */
    my_getmcolor((short)clp.colormap, &red, &green, &blue);

    glClearColor((GLclampf)(red) / 256.0f, (GLclampf)(green) / 256.0f,
                 (GLclampf)(blue) / 256.0f, 1.0);
    bgclr[0] = red / 256.0f;
    bgclr[1] = green / 256.0f;
    bgclr[2] = blue / 256.0f;
  } else if (clp.ismaterialfile) {
    /* background is the emissive color of material 0 (wrt reference */
    /* colormap) */
    my_getmaterial((short)clp.colormap, &mat);
    glClearColor((GLclampf)(mat->emissive[0]), (GLclampf)(mat->emissive[1]),
                 (GLclampf)(mat->emissive[2]), 1.0);
    bgclr[0] = mat->emissive[0];
    bgclr[1] = mat->emissive[1];
    bgclr[2] = mat->emissive[2];

    if ((drawparam.render_mode == RM_FLAT) ||
        (drawparam.render_mode == RM_SHADED) ||
        (drawparam.render_mode == RM_SHADOWS)) {

      /* set lighting mode */
      glEnable(GL_LIGHTING);
      glEnable(GL_NORMALIZE);
    }
  } else /* index mode */
    glClearIndex((GLfloat)clp.colormap);
}

void my_swapbuffers(void) {
  if (clp.graphics_output)
    if (clp.doublebuffer && double_buffering)
      WinSwapBuffers();
}

void FreeGraphics(void) {
  freeShadowMap(); // MC - July 2016 - free shadow map buffer
  WinFreeGraphics();
  FreeGL();
}

int InitializeGraphics(void) {
  WInitializeGraphics();
  return 0;
}

void my_color(GLint index, GLint index_back) {
  short red, green, blue;
  material_type *mat;
  GLenum face;

  if (clp.iscolormapfile) {
    my_getmcolor((short)index, &red, &green, &blue);
    glColor3ub((unsigned char)red, (unsigned char)green, (unsigned char)blue);
  } else if (clp.ismaterialfile) {
    face = drawparam.double_sided ? GL_FRONT : GL_FRONT_AND_BACK;

    my_getmaterial((short)index, &mat);

    if (drawparam.gllighting) {
      glMaterialfv(face, GL_AMBIENT, mat->ambient);
      glMaterialfv(face, GL_DIFFUSE, mat->diffuse);
      glMaterialfv(face, GL_EMISSION, mat->emissive);
      glMaterialfv(face, GL_SPECULAR, mat->specular);
      glMaterialf(face, GL_SHININESS, mat->shininess);

      if (drawparam.double_sided) {
        my_getmaterial((short)index_back, &mat);

        glMaterialfv(GL_BACK, GL_AMBIENT, mat->ambient);
        glMaterialfv(GL_BACK, GL_DIFFUSE, mat->diffuse);
        glMaterialfv(GL_BACK, GL_EMISSION, mat->emissive);
        glMaterialfv(GL_BACK, GL_SPECULAR, mat->specular);
        glMaterialf(GL_BACK, GL_SHININESS, mat->shininess);
      }
    } else {
      glColor4fv(mat->diffuse);
    }
  } else
    glIndexi(index);
}

void text_color(GLint index, GLint index_back) {
  short red, green, blue;
  material_type *mat;
  GLenum face;

  if (clp.iscolormapfile) {
    my_getmcolor((short)index, &red, &green, &blue);
    glColor3ub((unsigned char)red, (unsigned char)green, (unsigned char)blue);
  } else if (clp.ismaterialfile) {
    face = drawparam.double_sided ? GL_FRONT : GL_FRONT_AND_BACK;

    my_getmaterial((short)index, &mat);

    assert(!drawparam.gllighting);
    glColor4fv(mat->emissive);
  } else
    glIndexi(index);
}

int Update_Rotation(void) {
  int pos_x, pos_y;

  WinGetMousePosition(&pos_x, &pos_y);
  /* if moved */
  if ((pos_y != viewparam.yStart) || (pos_x != viewparam.xStart)) {
    /* update rotation parameters */
    viewparam.xRotation += 2 * (pos_y - viewparam.yStart);
    viewparam.yRotation += 2 * (pos_x - viewparam.xStart);

    viewparam.xStart = (short)pos_x;
    viewparam.yStart = (short)pos_y;

    if (eOn == viewparam.GenerateOnViewChange)
      RegenerateString();

    return 1; /* perform a rotation */
  }
  return 0; /* no rotation necessary */
}

int Update_Pan(void) {
  int pos_x, pos_y;
  WinGetMousePosition(&pos_x, &pos_y);

  if ((pos_x != viewparam.xStart) || (pos_y != viewparam.yStart)) {
    viewparam.xPan += (viewparam.xStart - pos_x) / viewparam.real_to_pixel;
    viewparam.yPan += (pos_y - viewparam.yStart) / viewparam.real_to_pixel;
    viewparam.xStart = (short)pos_x;
    viewparam.yStart = (short)pos_y;
    if (eOn == viewparam.GenerateOnViewChange)
      RegenerateString();

    return 1; /* perform a translation */
  }
  return 0;
}

#define UPDATE_SCALE_PAR 0.8 /* scale rate per 100-pixel mouse movement */

int Update_Scale(void) {
  int pos_x, pos_y;

  /* get mouse position */
  WinGetMousePosition(&pos_x, &pos_y);
  /* if moved */
  if (pos_y != viewparam.yStart) {
    /* update rotation parameters */
    if (viewparam.parallel_projection_on) {
      viewparam.scale = viewparam.scale *
                        (float)pow(UPDATE_SCALE_PAR,
                                   (double)(pos_y - viewparam.yStart) * 0.01);
    } else
      viewparam.scale = viewparam.scale *
                        (float)pow(UPDATE_SCALE_PAR,
                                   (double)(pos_y - viewparam.yStart) * 0.01);

    viewparam.yStart = (short)pos_y;

    if (viewparam.scale < viewparam.zoom_min)
      viewparam.scale = viewparam.zoom_min;
    else if (viewparam.scale > viewparam.zoom_max)
      viewparam.scale = viewparam.zoom_max;

    if (eOn == viewparam.GenerateOnViewChange)
      RegenerateString();

    return 1; /* perform a scale change */
  }
  return 0; /* no scale change necessary */
}

unsigned int font_list_base = 0;

void makeRasterFont(void) {
  if (0 != font_list_base)
    glDeleteLists(font_list_base, 256);
  WinMakeRasterFont(&font_list_base, &drawparam);
}

void DrawString(char *buffer) {
  glPushAttrib(GL_LIST_BIT);

  glListBase(font_list_base);

  glCallLists(strlen(buffer), GL_UNSIGNED_BYTE, buffer);

  glPopAttrib();
}

#ifdef DIB_CMPRS
static int _Compress(BYTE *, int);

void SaveAsBmp(const char *fname) {
  GLint xsize, ysize;
  unsigned char *rowr = NULL;
  unsigned char *rowg = NULL;
  unsigned char *rowb = NULL;
  BYTE *aCompressed = NULL;
  int x, y, i;
  int rowpad;
  static char fnm[256];
  FILE *fp = NULL;
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER bmih;
  static RGBQUAD colormap[256];

  strcpy(fnm, clp.savedir);
  strcat(fnm, fname);

  if (!clp.graphics_output)
    return;
  else {
    GLint tmp[4];
    glGetIntegerv(GL_VIEWPORT, tmp);
    xsize = tmp[2];
    ysize = tmp[3];
  }

  setcursor(CURSOR_HOURGLASS, 0, 0);

  rowr = (unsigned char *)Malloc(xsize * sizeof(char));
  if (NULL == rowr)
    goto CleanUp;

  rowg = (unsigned char *)Malloc(xsize * sizeof(char));
  if (NULL == rowg)
    goto CleanUp;

  rowb = (unsigned char *)Malloc(xsize * sizeof(char));
  if (NULL == rowb)
    goto CleanUp;

  if (clp.iscolormapfile) {
    aCompressed = (BYTE *)Malloc(2 * xsize * sizeof(BYTE));
    if (NULL == aCompressed)
      goto CleanUp;
  }

  fp = fopen(fname, "wb");

  if (NULL == fp)
    goto CleanUp;

  if (clp.iscolormapfile)
    rowpad = 4 - (xsize % 4);
  else
    rowpad = 4 - ((3 * xsize) % 4);

  if (4 == rowpad)
    rowpad = 0;

  {
    union {
      WORD w;
      char c[2];
    } u;
    u.c[0] = 'B';
    u.c[1] = 'M';
    bmfh.bfType = u.w;

    if (clp.iscolormapfile)
      bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                    256 * sizeof(RGBQUAD) + (xsize + rowpad) * ysize;
    else
      bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                    (3 * xsize + rowpad) * ysize;

    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    if (clp.iscolormapfile)
      bmfh.bfOffBits += 256 * sizeof(RGBQUAD);
  }

  fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, fp);

  {
    memset(&bmih, 0, sizeof(BITMAPINFOHEADER));
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = xsize;
    bmih.biHeight = ysize;
    bmih.biPlanes = 1;
    if (clp.iscolormapfile) {
      bmih.biBitCount = 8;
      bmih.biCompression = BI_RLE8;
    } else
      bmih.biBitCount = 24;
  }

  fwrite(&bmih, sizeof(BITMAPINFOHEADER), 1, fp);

  if (clp.iscolormapfile) {
    for (i = 0; i < 256; i++) {
      short srgb[3];
      my_getmcolor((short)(256 + i), &(srgb[0]), &(srgb[1]), &(srgb[2]));
      colormap[i].rgbRed = (BYTE)srgb[0];
      colormap[i].rgbGreen = (BYTE)srgb[1];
      colormap[i].rgbBlue = (BYTE)srgb[2];
    }
    fwrite(colormap, sizeof(RGBQUAD), 256, fp);
  }

  glReadBuffer(GL_FRONT);

  for (y = 0; y < ysize; y++) {
    BYTE rgb[3];
    glReadPixels(0, y, xsize, 1, GL_RED, GL_UNSIGNED_BYTE, rowr);
    glReadPixels(0, y, xsize, 1, GL_GREEN, GL_UNSIGNED_BYTE, rowg);
    glReadPixels(0, y, xsize, 1, GL_BLUE, GL_UNSIGNED_BYTE, rowb);
    for (x = 0; x < xsize; x++) {
      if (clp.iscolormapfile) {
        RGBQUAD rgbq;
        BYTE ix = 0;
        rgbq.rgbRed = rowr[x];
        rgbq.rgbGreen = rowg[x];
        rgbq.rgbBlue = rowb[x];
        for (i = 0; i < 256; i++) {
          if (rgbq.rgbRed == colormap[i].rgbRed &&
              rgbq.rgbGreen == colormap[i].rgbGreen &&
              rgbq.rgbBlue == colormap[i].rgbBlue) {
            ix = (BYTE)i;
            /*fwrite(&ix, sizeof(BYTE), 1, fp);*/
            aCompressed[x] = ix;
            break;
          }
        }
        if (256 == i)
          /*fwrite(&ix, sizeof(BYTE), 1, fp);*/
          aCompressed[x] = 0;
      } else {
        rgb[2] = rowr[x];
        rgb[1] = rowg[x];
        rgb[0] = rowb[x];
        fwrite(rgb, sizeof(BYTE), 3, fp);
      }
    }
    if (clp.iscolormapfile) {
      int RowLength = _Compress(aCompressed, xsize);
      fwrite(aCompressed, sizeof(BYTE), RowLength, fp);
      rowpad = 4 - (RowLength % 4);
      if (4 == rowpad)
        rowpad = 0;
    }
    if (rowpad != 0)
      fwrite(rgb, sizeof(BYTE), rowpad, fp);
  }

CleanUp:
  setcursor(CURSOR_ARROW, 0, 0);
  if (NULL != aCompressed) {
    Free(aCompressed);
    aCompressed = NULL;
  }
  if (NULL != fp) {
    fclose(fp);
    fp = NULL;
  }
  if (NULL != rowb) {
    Free(rowb);
    rowb = NULL;
  }
  if (NULL != rowg) {
    Free(rowg);
    rowg = NULL;
  }
  if (NULL != rowr) {
    Free(rowr);
    rowr = NULL;
  }
}

static int _Compress(BYTE *arr, int l) {
  BYTE first;
  int count = 1;
  int current = 1;

  first = arr[0];
  while (arr[current] == first && count < 255 && current < l) {
    current++;
    count++;
  }

  if (current > 2) {
    arr[0] = count;
    memmove(arr + 2, arr + current, l - current + 2);
    return 2 + _Compress(arr + 2, l - current + 2);
  } else {
    BYTE last = arr[0];
    while (arr[current] != last && count < 255 && current < l) {
      current++;
      count++;
      last = arr[current];
    }
  }
  return 0;
}

#else

void SaveAsBmp(const char *fname) {
  GLint xsize, ysize;
  unsigned char *rowr = NULL;
  unsigned char *rowg = NULL;
  unsigned char *rowb = NULL;
  int x, y;
  int rowpad;
  FILE *fp = NULL;
  BITMAPFILEHEADER bmfh;
  BITMAPINFOHEADER bmih;
  static char fnm[256];

  strcpy(fnm, clp.savedir);
  strcat(fnm, fname);

  if (!clp.graphics_output)
    return;
  else {
    GLint tmp[4];
    glGetIntegerv(GL_VIEWPORT, tmp);
    xsize = tmp[2];
    ysize = tmp[3];
  }

  setcursor(CURSOR_HOURGLASS, 0, 0);
  rowr = (unsigned char *)Malloc(xsize * sizeof(char));
  if (NULL == rowr)
    goto CleanUp;

  rowg = (unsigned char *)Malloc(xsize * sizeof(char));
  if (NULL == rowg)
    goto CleanUp;

  rowb = (unsigned char *)Malloc(xsize * sizeof(char));
  if (NULL == rowb)
    goto CleanUp;

  fp = fopen(fnm, "wb");

  if (NULL == fp)
    goto CleanUp;

  rowpad = 4 - ((3 * xsize) % 4);

  if (4 == rowpad)
    rowpad = 0;

  {
    union {
      WORD w;
      char c[2];
    } u;
    u.c[0] = 'B';
    u.c[1] = 'M';
    bmfh.bfType = u.w;

    bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                  (3 * xsize + rowpad) * ysize;

    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  }

  fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, fp);

  {
    memset(&bmih, 0, sizeof(BITMAPINFOHEADER));
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = xsize;
    bmih.biHeight = ysize;
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
  }

  fwrite(&bmih, sizeof(BITMAPINFOHEADER), 1, fp);

  glReadBuffer(GL_FRONT);

  for (y = 0; y < ysize; y++) {
    BYTE rgb[3];
    glReadPixels(0, y, xsize, 1, GL_RED, GL_UNSIGNED_BYTE, rowr);
    glReadPixels(0, y, xsize, 1, GL_GREEN, GL_UNSIGNED_BYTE, rowg);
    glReadPixels(0, y, xsize, 1, GL_BLUE, GL_UNSIGNED_BYTE, rowb);
    for (x = 0; x < xsize; x++) {
      rgb[2] = rowr[x];
      rgb[1] = rowg[x];
      rgb[0] = rowb[x];
      fwrite(rgb, sizeof(BYTE), 3, fp);
    }
    if (rowpad != 0)
      fwrite(rgb, sizeof(BYTE), rowpad, fp);
  }

CleanUp:
  setcursor(CURSOR_ARROW, 0, 0);
  if (NULL != fp) {
    fclose(fp);
    fp = NULL;
  }
  if (NULL != rowb) {
    Free(rowb);
    rowb = NULL;
  }
  if (NULL != rowg) {
    Free(rowg);
    rowg = NULL;
  }
  if (NULL != rowr) {
    Free(rowr);
    rowr = NULL;
  }
}

#endif
