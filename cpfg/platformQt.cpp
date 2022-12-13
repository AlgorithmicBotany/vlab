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



/* PLATFORM SPECIFIC FUNCTIONS */
/* currently Motif windowing */

/* Oct 94:  Radomir Mech */

#include <cassert>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <string>
#include <vector>

#include <QApplication>
#include <QPainter>

#include "GLWindow.h"
#include <QTimer>

#include "drawparam.h"
#include "control.h"
#include "utility.h"
#include "irisGL.h"
#include "comlineparam.h"
#include "animparam.h"
#include "sgiFormat.h"

#include <QDesktopWidget>

#ifdef TEST_MALLOC
#include "test_malloc.h"
#endif

#include <qgl.h>

using namespace std;
QApplication *app = 0;

extern VIEWPARAM viewparam;
extern DRAWPARAM drawparam;
extern ANIMPARAM animparam;

extern int relativeSize;
extern int relativePos;

extern void MyExit(int status);

void Initialize_Menus(void);

static QFont _font;

void makeRasterFont() {
  char font_spec[100];
  extern DRAWPARAM drawparam;

  strcpy(font_spec, drawparam.fontname);

  /* font not loaded, try default font */
  std::string message = std::string("Font ") + std::string(drawparam.fontname)
    + std::string("not found - trying -*-*-*-r-*-*-12-*-*-*-*-*-*-*\n");
  char *cstr = new char[message.length() + 1];
  strcpy(cstr, message.c_str());
  Warning(cstr,0);
  strcpy(font_spec, "-*-*-*-r-*-*-12-*-*-*-*-*-*-*");

  _font = QFont("Times", 48);
}

/************************************************************************/
/*
  rings a bell
*/
void my_ringbell(void) {}

void Message(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
}

extern "C" void setcursor(short, Colorindex, Colorindex) {}

extern "C" void SetMainMenu() {}

extern "C" void SetAnimateMenu() {}

extern "C" void Change_Resolution() {}

extern "C" void Change_Filename(int) {}

extern "C" void Dialog_Box(char *, char *, int) {}

extern "C" void GetWindowOrigin(int *pX, int *pY) {
  *pX = 10;
  *pY = 10;
}

/************************************************************************/
/*
  get a color. In RGBA mode the color is picked from a local colormap.
*/
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

    *red = short(255.0 * mat->diffuse[0]);
    *green = short(255.0 * mat->diffuse[1]);
    *blue = short(255.0 * mat->diffuse[2]);
    return;
  }

  fprintf(stderr, "*** WARNING *** WARNING *** WARNING *** WARNING ***\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "   FIX-ME: some glut code removed but not replaced...\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "*** WARNING *** WARNING *** WARNING *** WARNING ***\n");

  *red = 0;
  *green = 0;
  *blue = 0;
}

int double_buffering = 1;

void my_SetBufferType(char double_buffer_on) {
#ifndef NOGRAPHICS
  if (clp.graphics_output) {
    if (double_buffer_on) {
      double_buffering = 1;
    }
    if (!double_buffer_on) {
      double_buffering = 0;
    }
  }
#endif
}

/*****************************************************/
/******************* WINDOWING ***********************/
/*****************************************************/

extern int animateFlag;
extern int updateFlag;

extern int validLsystem;

int pixmaps_exist = FALSE;

/* ------------------------ FUNCTION DECLARATIONS -------------------------- */
int InitializeOpenGLGraphics(void);
void SetView(VIEWPARAM *viewPtr);

/***************************************************************************/
void FreeGraphics(void) { FreeGL(); }

/***************************************************************************/

void swap_buffers();
void my_swapbuffers(void) { swap_buffers(); }

GLWindow *win_gl = NULL;
int InitializeGraphics(int argc, char **argv) {
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  app = new QApplication(argc, argv);


  // setting up relative position and relative size of the window
  int cxscr, cyscr;
  GetWindowSize(&cxscr, &cyscr);
  if (relativeSize) {
    // [PASCAL] Przemek asked to change it back to a number between 0 and 1
    if ((clp.xsize > 1)&&(clp.ysize > 1)){
      clp.xsize *= (cxscr / 100.);
      clp.ysize *= (cyscr / 100.);
    }
    else{
      clp.xsize *= cxscr;
      clp.ysize *= cyscr;
    }
  }
  if (relativePos) {
    if ((clp.xpos > 1)&&(clp.ypos > 1)){
      clp.xpos *= (cxscr / 100.);
      clp.ypos *= (cyscr / 100.);
    }
    else{
      clp.xpos *= cxscr;
      clp.ypos *= cyscr;
    }

  }
  if (clp.iscolormapfile)
    load_in_colormaps();

  if (clp.ismaterialfile)
    load_in_materials();

  // what was the point of this condition?
  // if ((clp.graphics_output)||(!clp.graphics_output))
  if (1)
    {
    if (!DoubleBuffer(&animparam))
      double_buffering = 0;

    win_gl = new GLWindow(NULL,clp.xsize,clp.ysize);
    //    win_gl->resize(clp.xsize, clp.ysize);
    win_gl->move(clp.xpos, clp.ypos);
    char *vlab_obj_name = getenv("VLAB_OBJECT_NAME");
    if (vlab_obj_name != NULL) {
      win_gl->setWindowTitle(QString("cpfg: %1").arg(vlab_obj_name));
      win_gl->setTitle(win_gl->windowTitle());
    }

    /*
      The following code was used in an old version of QT.
      It seems to mess up the sizing of the windows in High DPI.
    QTimer delay_show; // This timer ensure Qt has the time to create the OpenGL
                       // context before showing the window
    delay_show.setSingleShot(true);
    QObject::connect(&delay_show, SIGNAL(timeout()), win_gl, SLOT(show()));
    QObject::connect(&delay_show, SIGNAL(timeout()), win_gl, SLOT(raise()));
    delay_show.start(100);
    */
    win_gl->show();
    std::string iconFname = "./icon";

    // try to load in the icon from the object directory
    QImage iconPicture = QImage(iconFname.c_str());
    // if unsuccessful, silenty ignore it and use the default icon
    if (iconPicture.isNull()) {
      iconPicture = readSGI(iconFname.c_str());
      if (iconPicture.isNull()) {
        iconFname = ":/default-icon.png";
        iconPicture = QImage(iconFname.c_str());
      }
    }

    // read icon from SGI file
    QPixmap icon = QPixmap::fromImage(iconPicture);

    QPainter painter(&icon);
    painter.fillRect(0, icon.height() - 53, 53, 53,
                     QColor::fromRgbF(0, 0, 0, 1));
    painter.setPen(Qt::red);
    painter.setFont(QFont("Times", 50));

    painter.drawText(3, icon.height() - 3, QString("C"));

    QTimer::singleShot(0, win_gl, SLOT(raise()));

    app->setWindowIcon(icon);
#ifdef __APPLE__
    // TODO: check why the icon was set to empty QPixmap()?
    win_gl->setWindowIcon(QPixmap());
#else
    win_gl->setWindowIcon(icon);
#endif
    app->setQuitOnLastWindowClosed(true);
  
    return app->exec();
  }

  else
    return 0;
}

void GLWindow::closeEvent(QCloseEvent *pEv) {
  canvas->exit();
  pEv->accept();
  app->quit();
  QWidget::closeEvent(pEv);
}

void swap_buffers(void) {
  if (clp.graphics_output) {
    if (clp.doublebuffer && double_buffering) {
    }
  }
}

void GetTextExtent(char *, int *width, int *ascent, int *descent) {
  *width = 0;
  *ascent = 0;
  *descent = 0;
}

void DrawString(char *str, const TURTLE *tu) {
  short red, green, blue;

  my_getmcolor((short)tu->color_index, &red, &green, &blue);
  QColor qcolor((int)(red), (int)(green), (int)blue);

  char font_spec[100];
  extern DRAWPARAM drawparam;

  strcpy(font_spec, drawparam.fontname);

  QFont font = QFont();

  std::vector<std::string> vector_font;

  char delim = '-';
  string work = string(font_spec);
  string buf = "";
  unsigned int i = 0;
  while (i < work.length()) {
    if (work[i] != delim)
      buf += work[i];
    else {
      if (buf.length() > 0) {
        vector_font.push_back(buf);
        buf = "";
      } else {
        buf += "*";
        vector_font.push_back(buf);
        buf = "";
      }
    }
    i++;
  }
  if (!buf.empty())
    vector_font.push_back(buf);

  if (vector_font.size() >= 7) {

    std::istringstream buffer(vector_font[7]);
    int font_size;
    buffer >> font_size; // value = 45

    if ((font_size > 100) || (font_size <= 0)) {
      // try to read the size at a different position
      std::istringstream buffer(vector_font[6]);
      buffer >> font_size; // value = 45
    }

    std::string weight = vector_font[3];
    std::string slant = vector_font[4];

    int w = QFont::Normal;
    if (!weight.compare("light"))
      w = QFont::Light;
    else if (!weight.compare("normal"))
      w = QFont::Normal;
    else if (!weight.compare("medium"))
      w = QFont::DemiBold;
    else if (!weight.compare("bold"))
      w = QFont::Bold;
    else if (!weight.compare("black"))
      w = QFont::Black;

    bool italic = false;
    if (!slant.compare("italic"))
      italic = true;

    delim = '_';
    work = vector_font[2];

    buf = "";
    i = 0;
    std::vector<std::string> vector_name_font;
    while (i < work.length()) {
      if (work[i] != delim)
        buf += work[i];
      else if (buf.length() > 0) {
        vector_name_font.push_back(buf);
        buf = "";
      }
      i++;
    }
    if (!buf.empty())
      vector_name_font.push_back(buf);

    string name_font = "";
    for (unsigned int i = 0; i < vector_name_font.size() - 1; i++)
      name_font += vector_name_font[i] + " ";

    name_font += vector_name_font[vector_name_font.size() - 1];

    font = QFont(QString::fromStdString(name_font), font_size, w,
                 italic); // used for printing postscript text
    drawparam.font_name = new char[name_font.size() + 1];
    drawparam.font_name[name_font.size()] = 0;
    memcpy(drawparam.font_name, name_font.c_str(), name_font.size());

    drawparam.font_size = font_size;
    drawparam.italic = 0;
    if (italic)
      drawparam.italic = 1;
    drawparam.bold = 0;
    if (!weight.compare("bold"))
      drawparam.bold = 1;
  }
  win_gl->canvas->renderText(tu->position[0], tu->position[1], tu->position[2],
                             str, qcolor, font);
}

/********************** DRAWING WINDOW *******************/

int initialized = FALSE;
int first_run = FALSE;

/*************************************************************************/
#ifdef LINUX
void GetWindowSize(int *width, int *height) {
  QDesktopWidget widget;
  QRect mainScreenSize = widget.availableGeometry(widget.primaryScreen());

  *width = mainScreenSize.width();
  *height = mainScreenSize.height();
}
#endif

/*************************************************************************/
/* used in case of pixmas - here empty */
void CopyFromPixmap(void) {}

/*************************************************************************/
/* here comes everything that should be changed after slecting NewView
 */
void SetGraphics(void) {

  extern DRAWPARAM drawparam;
  short red, green, blue;
  material_type *mat;
  glDisable(GL_LIGHTING);
  glDisable(GL_NORMALIZE);
  float alpha = 1.;
  alpha = 0;
  if (clp.iscolormapfile) { /* RGBA mode */
    
    my_getmcolor((short)clp.colormap, &red, &green, &blue);
    glClearColor((GLclampf)(red) / 256.0, (GLclampf)(green) / 256.0,
                 (GLclampf)(blue) / 256.0, alpha);
    if (ClearBetweenFrames(&animparam)) {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

  } else {
    if (clp.ismaterialfile) {
      /* background is the emissive color of material 0 (wrt reference */
      /* colormap) */
      my_getmaterial(clp.colormap, &mat);

      glClearColor((GLclampf)(mat->emissive[0]), (GLclampf)(mat->emissive[1]),
                   (GLclampf)(mat->emissive[2]), alpha);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      if (((int)drawparam.render_mode == RM_FLAT) ||
          ((int)drawparam.render_mode == RM_SHADED) ||
          ((int)drawparam.render_mode == RM_SHADOWS)) {

        /* set lighting mode */

        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE);
      }
    } else /* index mode */
      glClearIndex((GLfloat)clp.colormap);
  }
}
/*************************************************************************/
// initGL()
int InitializeOpenGLGraphics(void) {

  SetGraphics();

  if (clp.doublebuffer && double_buffering) {
  }

  /* for proper orientation of normals */
  glFrontFace(GL_CW);

  //[PASCAL] Normally by default, face culling is disabled.  by January
  // 2019, it looks that on Mac Os it's not
  // the case so we need to disable it
  glDisable(GL_CULL_FACE);
  return 1;
}

/*************************************************************************/
/* necessary to differentiate for RGBA and index mode
 */
void my_color(GLint index, GLint index_back) {
  short red, green, blue;
  extern DRAWPARAM drawparam;
  material_type *mat = NULL;
  GLenum face;

  if (clp.iscolormapfile) {
    my_getmcolor(index, &red, &green, &blue);
    glColor3ub((unsigned char)red, (unsigned char)green, (unsigned char)blue);
  } else if (clp.ismaterialfile) {
    face = drawparam.double_sided ? GL_FRONT : GL_FRONT_AND_BACK;

    // if (is_material(index)){
    my_getmaterial(index, &mat);
    if (mat != NULL) {
      if (drawparam.gllighting) {
        glMaterialfv(face, GL_AMBIENT, mat->ambient);
        glMaterialfv(face, GL_DIFFUSE, mat->diffuse);
        glMaterialfv(face, GL_EMISSION, mat->emissive);
        glMaterialfv(face, GL_SPECULAR, mat->specular);
        glMaterialf(face, GL_SHININESS, mat->shininess);

        if (drawparam.double_sided) {
          my_getmaterial(index_back, &mat);

          glMaterialfv(GL_BACK, GL_AMBIENT, mat->ambient);
          glMaterialfv(GL_BACK, GL_DIFFUSE, mat->diffuse);
          glMaterialfv(GL_BACK, GL_EMISSION, mat->emissive);
          glMaterialfv(GL_BACK, GL_SPECULAR, mat->specular);
          glMaterialf(GL_BACK, GL_SHININESS, mat->shininess);
        }
      } else {
        glColor3fv(mat->diffuse);
      }
    }
  } else
    glIndexi(index);
}

extern "C" void text_color(GLint index, GLint) {
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
