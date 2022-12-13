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



#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <image.h>
#include <iostream>

#include "control.h"
#include "glcanvas.h"
#include "drawparam.h"
#include "viewparam.h"
#include "animparam.h"
#include "interpret.h"
#include "platform.h"
#include "irisGL.h"
#include "test_malloc.h"
#include "blackbox.h"
#include "generate.h"
#include "SaveAs.h"
#include "utils.h"
#include <QGLFormat>
#include <QPrinter>
#include <QPainter>
#include <directorywatcher.h>

#include <qgl.h>
#include <QApplication>
#include <QFileDialog>
#include <QInputDialog>
#include <QWidget>
#include <QMenu>
#include <QCursor>
#include <QLabel>
#include <QString>
#include <QMessageBox>
#include <QTimer>
#include <QFocusEvent>
#include <QMouseEvent>
#include <sstream>

#include <iostream>

using namespace Qt;

// this must be included *after* the qt header files
#include "platformmenu.h"
#include "comlineparam.h"

#include "splinefunC.h"
#include "curveXYZc.h"

#include <iostream>
#include <fstream>

#ifdef __APPLE__
#include <errno.h>
#include <sys/sysctl.h>
#endif

namespace {
// timer for idle function
QTimer *idleTimer;
} // namespace

namespace {
// timer for idle function
QTimer *idleContinuousTimer;
} // namespace

extern char *currentString;
extern COMLINEPARAM clp;
extern VIEWPARAM viewparam;
extern DRAWPARAM drawparam;
extern ANIMPARAM animparam;
extern int double_buffering;
extern int is_menu;
extern int validLsystem;
extern LSYSDATA *LsystemList;
extern DRAWPARAM *dr;
extern char *currentString;
extern char *currentStringEnd;
extern unsigned long currentStringLength;
extern int animateFlag;
extern int animatesave;
extern int updateFlag;
extern int pixmaps_exist;
extern int gl_numlights; /* number of lights - from irisGL.c */
extern int pending_interpretation;
extern int interpret_new_model;

extern void InsertX(int, int);

#define SCALE_MIN 0.0005     /* minimal allowed scale */
#define SCALE_MAX 5000       /* maximal allowed scale */
#define UPDATE_SCALE_PAR 0.8 /* scale rate per 100-pixel mouse movement */
/********************************************************************************/

glcanvas::glcanvas(QWidget *parent)
    : QOpenGLWidget(parent), new_model_pending(false), isRunning(false),
      isRunningForEver(false), directoryWatcher(NULL) {
  interpret_new_model = 1;
  QSurfaceFormat format;
  format.setRedBufferSize(8);
  format.setGreenBufferSize(8);
  format.setBlueBufferSize(8);
  format.setAlphaBufferSize(8);
  setFormat(format);
  _backGroundIsTransparent = false;
#ifdef __APPLE__
  char str[256];
  size_t size = sizeof(str);
  sysctlbyname("kern.osrelease", str, &size, NULL, 0);
  int version, x1, x2;
  sscanf(str, "%d.%d.%d", &version, &x1, &x2);
  if (version > 12) {
    // fix Mac OS X 10.9 (mavericks) font issue
    // https://bugreports.qt-project.org/browse/QTBUG-32789
    QFont::insertSubstitution(".Helvetica Neue DeskInterface", "Lucida Grande");
    QFont::insertSubstitution(".Lucida Grande UI", "Lucida Grande");
  }
#endif

  rot = scl = pan = roll = false;
  hadFocus = 2;
  initialized = first_run = false;
  _outputFormat = 0;
  _pix_format = 3;
  _numberingImageName = 0;
  _pathToSave = (QDir::currentPath()).toStdString();

  QString baseName = QFileInfo(clp.savefilename[SAVE_PNG]).baseName();
  _imageBaseName = baseName.toStdString();
  _filename = _pathToSave + "/" + _imageBaseName;
  _is_recording = false;
  _alphaChannel = false;
  _id = 0;

  /******************************/
  /*** create the popup menus ***/
  /******************************/

  mainPop = new QMenu("mainPop", this);
  secondaryPop = new QMenu("secondaryPop", this);

  outputPop = new QMenu("outputPop", this);
  inputPop = new QMenu("inputPop", this);

  imagePop = new QMenu("imagePop", this);
  rayshadePop = new QMenu("rayshadePop", this);
  pScriptPop = new QMenu("pScriptPop", this);
  stringOutPop = new QMenu("stringOutPop", this);
  glsFormPop = new QMenu("glsFormPop", this);
  viewVolPop = new QMenu("viewVolPop", this);
  objPop = new QMenu("objPop", this);

  stringInPop = new QMenu("stringInPop ", this);

  rgbPop = new QMenu("rgbPop", this);
  pngPop = new QMenu("pngPop", this);
  bmpPop = new QMenu("bmpPop", this);
  gifPop = new QMenu("gifPop", this);
  jpgPop = new QMenu("jpgPop", this);
  tiffPop = new QMenu("tiffPop", this);
  pbmPop = new QMenu("pbmPop", this);
  tgaPop = new QMenu("tgaPop", this);

  textOutPop = new QMenu("textOutPop", this);
  binaryOutPop = new QMenu("binaryOutPop", this);

  textInPop = new QMenu("textInPop", this);
  binaryInPop = new QMenu("binaryInPop", this);

  // secondaryPop menu's submenus
  beginRecPop = new QMenu("beginRecPop", this);
  imageRecPop = new QMenu("imageRecPop", this);
  stringRecPop = new QMenu("sringRecPop", this);

  // FIRST WE MUST SET UP THE SUB-MENUS AND THEN ADD THEM
  // TO THE MAIN MENUS

  /****************************************/
  /****** set up the sub Popup menus ******/
  /****************************************/

  QString save;
  QString saveAs = "Save as ...";
  QString input;
  QString inputFrom = "Input from ...";
  QString filename = clp.lfilename;

  // set the filename
  int dotIndex = filename.indexOf(".", 0);
  filename.truncate(dotIndex); // remove the current extension

  input = "Text file";
  textInPop->addAction(input, this, SLOT(InputText()));
  input = "Binary file";
  textInPop->addAction(input, this, SLOT(InputBinary()));
  save = "Text file";
  textOutPop->addAction(save, this, SLOT(SaveSTR()));
  save = "Binary file  ";
  textOutPop->addAction(save, this, SLOT(SaveSTRB()));
  save = "Save as ";
  save += filename;
  save += ".rgb";
  rgbPop->addAction(save, this, SLOT(SaveRGB()));
  rgbPop->addAction(saveAs, this, SLOT(SaveAsRGB()));

  save = "Save as ";
  save += filename;
  save += ".png";
  pngPop->addAction(save, this, SLOT(SavePNG()));
  pngPop->addAction(saveAs, this, SLOT(SaveAsPNG()));

  save = "Save as ";
  save += filename;
  save += ".bmp";
  bmpPop->addAction(save, this, SLOT(SaveBMP()));
  bmpPop->addAction(saveAs, this, SLOT(SaveAsBMP()));

  save = "Save as ";
  save += filename;
  save += ".jpg";
  jpgPop->addAction(save, this, SLOT(SaveJPG()));
  jpgPop->addAction(saveAs, this, SLOT(SaveAsJPG()));

  save = "Save as ";
  save += filename;
  save += ".gif";
  gifPop->addAction(save, this, SLOT(SaveGIF()));
  gifPop->addAction(saveAs, this, SLOT(SaveAsGIF()));

  save = "Save as ";
  save += filename;
  save += ".tiff";
  tiffPop->addAction(save, this, SLOT(SaveTIFF()));
  tiffPop->addAction(saveAs, this, SLOT(SaveAsTIFF()));

  save = "Save as ";
  save += filename;
  save += ".pbm";
  pbmPop->addAction(save, this, SLOT(SavePBM()));
  pbmPop->addAction(saveAs, this, SLOT(SaveAsPBM()));

  save = "Save as ";
  save += filename;
  save += ".tga";
  tgaPop->addAction(save, this, SLOT(SaveTGA()));
  tgaPop->addAction(saveAs, this, SLOT(SaveAsTGA()));

  stringInPop->addMenu(textInPop);
  textInPop->setTitle("text");
  stringOutPop->addMenu(textOutPop);

  imagePop->addMenu(rgbPop);
  rgbPop->setTitle("RGB");
  imagePop->addMenu(pngPop);
  pngPop->setTitle("PNG");
  imagePop->addMenu(bmpPop);
  bmpPop->setTitle("BMP");
  imagePop->addMenu(jpgPop);
  jpgPop->setTitle("JPG");
  imagePop->addMenu(gifPop);
  gifPop->setTitle("GIF");
  imagePop->addMenu(tiffPop);
  tiffPop->setTitle("TIFF");
  imagePop->addMenu(pbmPop);
  pbmPop->setTitle("PBM");
  imagePop->addMenu(tgaPop);
  tgaPop->setTitle("TGA");

  save = "Save as ";
  save += filename;
  save += ".ray";
  rayshadePop->addAction(save, this, SLOT(SaveRAY()));
  rayshadePop->addAction(saveAs, this, SLOT(SaveAsRAY()));

  save = "Save as ";
  save += filename;
  save += ".ps";
  pScriptPop->addAction(save, this, SLOT(SavePS()));
  pScriptPop->addAction(saveAs, this, SLOT(SaveAsPS()));

  save = "Save as ";
  save += filename;
  save += ".gls";
  glsFormPop->addAction(save, this, SLOT(SaveGLS()));
  glsFormPop->addAction(saveAs, this, SLOT(SaveAsGLS()));

  save = "Save as ";
  save += filename;
  save += ".vv";
  viewVolPop->addAction(save, this, SLOT(SaveVV()));
  viewVolPop->addAction(saveAs, this, SLOT(SaveAsVV()));

  save = "Save as ";
  save += filename;
  save += ".obj";
  objPop->addAction(save, this, SLOT(SaveOBJ()));
  objPop->addAction(saveAs, this, SLOT(SaveAsOBJ()));

  outputPop->addMenu(textOutPop);
  textOutPop->setTitle("Output");
  inputPop->addMenu(textInPop);
  textInPop->setTitle("Input");

  // secondaryPop's submenus
  imageRecPop->addAction("RGB", this, SLOT(RecordImageRGB()));
  imageRecPop->addAction("PNG", this, SLOT(RecordImagePNG()));
  imageRecPop->addAction("BMP", this, SLOT(RecordImageBMP()));
  imageRecPop->addAction("JPG", this, SLOT(RecordImageJPG()));
  imageRecPop->addAction("GIF", this, SLOT(RecordImageGIF()));
  imageRecPop->addAction("TIFF", this, SLOT(RecordImageTIFF()));
  imageRecPop->addAction("PBM", this, SLOT(RecordImagePBM()));
  imageRecPop->addAction("TGA", this, SLOT(RecordImageTGA()));

  stringRecPop->addAction("text", this, SLOT(RecordStringText()));
  stringRecPop->addAction("binary", this, SLOT(RecordStringBinary()));

  beginRecPop->addMenu(imageRecPop);
  imageRecPop->setTitle("Image");
  beginRecPop->addAction("Rayshade", this, SLOT(RecordImageRAY()));
  beginRecPop->addAction("Postscript", this, SLOT(RecordImagePS()));
  beginRecPop->addMenu(stringRecPop);
  stringRecPop->setTitle("String");
  beginRecPop->addAction("Gls format", this, SLOT(RecordImageGLS()));
  beginRecPop->addAction("View volume", this, SLOT(RecordImageVV()));

  beginRecPop->addMenu(imageRecPop);
  imageRecPop->setTitle("Image");
  beginRecPop->addAction("Rayshade", this, SLOT(RecordImageRAY()));
  beginRecPop->addAction("Postscript", this, SLOT(RecordImagePS()));
  beginRecPop->addMenu(stringRecPop);
  stringRecPop->setTitle("String");
  beginRecPop->addAction("Gls format", this, SLOT(RecordImageGLS()));
  beginRecPop->addAction("View volume", this, SLOT(RecordImageVV()));

  /**************************************/
  /***** set up the main popup menu *****/
  /**************************************/

  int slashIndex = 0;
  QString objectName = clp.lfilename;
  slashIndex = filename.lastIndexOf("/") + 1;
  objectName.remove(0, slashIndex);
  dotIndex = objectName.indexOf(".", 0);
  objectName.truncate(dotIndex); // remove the current extension
  objectName.insert(0, ' ');

  mainPop->addAction("New model", this, SLOT(NewModel()));
  mainPop->addAction("New L-system", this, SLOT(NewLSystem()));
  mainPop->addSeparator();

  mainPop->addAction("New view", this, SLOT(NewView()));
  mainPop->addAction("New rendering", this, SLOT(RereadDrawParams()));

  mainPop->addSeparator();
  std::string saveMenuName = "Save " + _imageBaseName;
  saveMenuName += getExtension();

  _saveAction = new QAction(saveMenuName.c_str(), this);
  connect(_saveAction, SIGNAL(triggered()), this, SLOT(save()));

  mainPop->addAction(_saveAction);
  mainPop->addAction("Save as ...", this, SLOT(saveAs()));
  QMenu *stringPopup = new QMenu(this);
  stringPopup->addMenu(textInPop);
  textInPop->setTitle("Input");
  stringPopup->addMenu(textOutPop);
  textOutPop->setTitle("Output");

  mainPop->addMenu(stringPopup);
  stringPopup->setTitle("String");

  // AnimateMode();
  mainPop->addSeparator();
  mainPop->addAction("Animate", this, SLOT(AnimateMode()));

  mainPop->addSeparator();
  QMenu *continuousModeMenu = new QMenu(this);
  continuousModeMenu->setTitle("Refresh mode");
  explicitMode = continuousModeMenu->addAction("Explicit", this,
                                               SLOT(SetExplicitMode(bool)));
  continuousMode = continuousModeMenu->addAction("Triggered/Continuous", this,
                                                 SLOT(SetContinuousMode(bool)));

  continuousMode->setCheckable(true);
  explicitMode->setCheckable(true);
  mainPop->addMenu(continuousModeMenu);

  mainPop->addSeparator();
  // quit the application entirely
  mainPop->addAction("Exit", this, SLOT(exit()));

  /*******************************************/
  /***** set up the secondary popup menu *****/
  /*******************************************/
  secondaryPop->addSeparator();
  _stepAction = secondaryPop->addAction("Step", this, SLOT(Step()),
                                        QKeySequence(Qt::CTRL + Qt::Key_F));
  this->addAction(_stepAction);
  _runAction =
      secondaryPop->addAction("Run", this, SLOT(Run()), Qt::CTRL + Qt::Key_R);
  this->addAction(_runAction);
  _foreverAction = secondaryPop->addAction("Forever", this, SLOT(Forever()),
                                           Qt::CTRL + Qt::Key_V);
  this->addAction(_foreverAction);
  _stopAction =
      secondaryPop->addAction("Stop", this, SLOT(Stop()), Qt::CTRL + Qt::Key_S);
  this->addAction(_stopAction);

  this->addAction(secondaryPop->addAction("Rewind", this, SLOT(Rewind()),
                                          Qt::CTRL + Qt::Key_W));

  _clearAction = new QAction("Clear", this);
  connect(_clearAction, SIGNAL(triggered()), this, SLOT(Clear()));
  unsigned int clear = getClearBetweenFrameFlag();
  if (clear)
    _clearAction->setEnabled(false);
  else
    _clearAction->setEnabled(true);

  secondaryPop->addAction(_clearAction);

  secondaryPop->addSeparator();
  secondaryPop->addAction("New model", this, SLOT(NewModel()));
  secondaryPop->addAction("New L-system", this, SLOT(NewLSystem()));
  secondaryPop->addSeparator();
  secondaryPop->addAction("New view", this, SLOT(NewView()));
  secondaryPop->addAction("New rendering", this, SLOT(RereadDrawParams()));
  secondaryPop->addAction("New animate", this, SLOT(RereadAnimParams()));

  connect(secondaryPop, SIGNAL(aboutToShow()), this, SLOT(Pause()));
  connect(secondaryPop, SIGNAL(triggered(QAction *)), this,
          SLOT(setMenuTriggered(QAction *)));
  _menuTriggered = false;

  secondaryPop->addSeparator();
  secondaryPop->addAction(_saveAction);
  secondaryPop->addAction("Save as ...", this, SLOT(saveAs()));
  _recordingId =
      secondaryPop->addAction("Start recording", this, SLOT(StartRecording()));

  secondaryPop->addMenu(stringPopup);
  stringPopup->setTitle("String");

  secondaryPop->addSeparator();
  secondaryPop->addAction("Don't animate", this, SLOT(DontAnimate()));

  secondaryPop->addSeparator();

  secondaryPop->addMenu(continuousModeMenu);

  secondaryPop->addSeparator();
  // to quit the application entirely
  secondaryPop->addAction("Exit", this, SLOT(exit()));

  connect(secondaryPop, SIGNAL(aboutToHide()), this, SLOT(setMenuHidden()));
  connect(secondaryPop, SIGNAL(aboutToShow()), this, SLOT(setMenuVisible()));

  /*******************************************/
  /******** set up the default values ********/
  /*******************************************/

  // set the default popup menu to be the mainPop

  if (animateFlag == 0) {
    mPopmenu = true;
    sPopmenu = false;
  } else {
    mPopmenu = false;
    sPopmenu = true;
  }

  // set default flags
  mevent = false;
  selected = false;

  // set the default cursor
  QWidget::setCursor(ArrowCursor);

  QWidget::setFocusPolicy(Qt::StrongFocus);
  mainPop->setFocusPolicy(Qt::StrongFocus);
  secondaryPop->setFocusPolicy(Qt::StrongFocus);

  // let cpfg handle its own double-buffering
  //[PASCAL] look at double buffer
  //    setAutoBufferSwap(false);

  /******** timer to run idle function ********/

  // create and set up the timer for the IdleFunction
  idleTimer = new QTimer(0);
  idleTimer->setTimerType(Qt::PreciseTimer);
  connect(idleTimer, SIGNAL(timeout()), SLOT(Idle()));

  // create and set up the timer for the IdleFunction
  idleContinuousTimer = new QTimer(this);
  connect(idleContinuousTimer, SIGNAL(timeout()), SLOT(IdleContinuousMode()));

    
  if ((clp.savingMode == CONTINUOUS) || (clp.savingMode == TRIGGERED)) {
    continuousMode->setChecked(true);
    explicitMode->setChecked(false);
    SetContinuousMode(true);
  } else {
    explicitMode->setChecked(true);
    continuousMode->setChecked(false);
    SetExplicitMode(true);
  }

  if (double_buffering)
    _openGlBehavior = QOpenGLWidget::NoPartialUpdate;
  else {
    _openGlBehavior = QOpenGLWidget::PartialUpdate;
  }

  setUpdateBehavior(_openGlBehavior);
}

/********************************************************************************/

glcanvas::~glcanvas() {
  freeShadowMap(); // MC - July 2016 - free shadow map from memory
}

// openGL rendering
/********************************************************************************/

void glcanvas::initializeGL() {
  makeCurrent();
  //  resize(clp.xsize,clp.ysize);
  //clp.xsize = width();
  //clp.ysize = height(); /* do not remove. Used in SetView() */

  initializeOpenGLFunctions(); // MC - Oct. 2015 - set OpenGL extensions using
                               // Qt
  makeCurrent();
  InitializeOpenGLGraphics();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_MULTISAMPLE);
  static GLfloat lightPosition[4] = {0.5, 5.0, 7.0, 1.0};
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

#include "utility.h"
/********************************************************************************/

void glcanvas::Repaint() {
  int setview = 0;
  makeCurrent();

  if (initialized == false) {
    interpret_new_model = 1;

    // InitializeOpenGLGraphics();

    if (!first_run) {
      // first cpfg run
      FirstRun();
      first_run = true;
      setview = rfScale | rfViewVol;
    }

    InitializeDispatcher(); // should be after FirstRun()

    // MC - Oct. 2015 - support for shadow mapping using shaders from GLSL 1.2
    // check if shaders and framebuffers are supported
    if (!QGLShaderProgram::hasOpenGLShaderPrograms() ||
        !hasOpenGLFeature(QOpenGLFunctions::Framebuffers)) {
      Utils::Warning("Warning! the 'render mode: shadows' view option will not work.\n");
      Utils::Warning("OpenGL Shading Language or Framebuffers are not supported ");
      Utils::Warning("and shadows cannot be rendered.\n");
      drawparam.render_mode = RM_SHADED;
    } else {
      // even if "render mode: shadows" is not specified when the OpenGL context
      // is first created, the shaders are loaded in case the render mode is
      // changed in the view file later
      QString shadersPath = QCoreApplication::applicationDirPath();
      QByteArray ba = shadersPath.toLatin1();
      const char *c_str = ba.data();
      initShadowMap(c_str);
    }
  }

  // if any scaling/rotating needs to be done whether in either
  // animate mode or view-only mode
  unsigned int clear = getClearBetweenFrameFlag();
  if (scl || rot || pan || roll) {
    //for any operation with the mouse we want to clear anyway
    //this why we comment the following
    //    if (clear || !initialized) {
      setview |= rfClear;
      //}
    // MC - July 2016 - on initialize render the shadow map
    if (!initialized) {
      setview |= rfShadows;
    }

    StartRedraw(setview);
    ContinueRedraw();
  }
  // if no rotating nor scaling needs to be done, check the mode
  // we set an idle function only when we are in animate mode
  //(so that we may be in mode run, forever, etc.)
  else {
    if (clear || !initialized) {
      //[PASCAL] new QT5
      setview |= rfClear;
    }
    // MC - July 2016 - on initialize render the shadow map
    if (!initialized) {
      setview |= rfShadows;
    }
    StartRedraw(setview);

    if (animateFlag) {
      //SetIdle(); 
      ContinueRedraw();
    } else {
      ContinueRedraw();
    }
  }
  interpret_new_model = 0;
  initialized = true;

  glFlush();
}

void glcanvas::paintGL() {
  if (!validLsystem) {
    Utils::Warning("Warning: LSystem not valid\n");
    return;
  }
  Repaint();

}

/********************************************************************************/

void glcanvas::resizeGL(int w, int h) {
  int setview = 0;

  clp.xsize = w; // idth();
  clp.ysize = h; // eight();
  makeCurrent();

  if (!getClearBetweenFrameFlag()) {
  }

  if (!initialized) {
    // initialized = true;
    // we clear the window the first time the window is resized

    InitializeOpenGLGraphics();

    if (!first_run) {
      /* first cpfg run */
      FirstRun();
      first_run = true;
      setview = rfScale | rfViewVol;
    }

    InitializeDispatcher(); /* should be after FirstRun() */
    if (validLsystem)
        CalculateViewVolume(currentString, &drawparam, &viewparam);
  }
  SetView(&viewparam); /* view volume is the same */
}

/************* Mouse HANDLING STUFF ****************/

// we display a popup menu only if the right button has been pressed
// otherwise, we do nothing

void glcanvas::keyPressEvent(QKeyEvent *event) {
  if (event->isAutoRepeat())
    event->ignore();

  _keyPressed = event->key();
}

void glcanvas::mousePressEvent(QMouseEvent *mouse) {
  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
  if (topWidget != nullptr)
    topWidget->raise();

  initialized = true;
  int pos_x = mouse->pos().x();
  int pos_y = mouse->pos().y();
  // bring up popup menus
  if (RightButton == mouse->button()) {
    mevent = true;

    // display the main popup menu wherever the mouse
    // position is
    if (mPopmenu)
      mainPop->exec(QCursor::pos());
    else if (sPopmenu) {
      unsigned int clear = getClearBetweenFrameFlag();
      if (clear)
        _clearAction->setEnabled(false);
      else
        _clearAction->setEnabled(true);
      secondaryPop->exec(QCursor::pos());
    }
  }

  // scale the image
  else if ((MidButton == mouse->button()) || ((LeftButton == mouse->button() && (_keyPressed == Qt::Key_Z))))
      
 {
   //roll 
   if (Qt::ShiftModifier & mouse->modifiers()){
       roll = true;
       viewparam.xStart = pos_x;
      viewparam.xLast = (short)viewparam.xRotation;

     }
   //zoom
     else{
       scl = true;
       viewparam.yStart = pos_y;
     }
  }

  // rotate the image
  else if (LeftButton == mouse->button()) {
    if ((Qt::ShiftModifier & mouse->modifiers()) &&
        (Qt::ControlModifier & mouse->modifiers())) {
      // InsertX //
      makeCurrent();
      InsertX(pos_x, pos_y);
    } else if (Qt::ShiftModifier & mouse->modifiers()) {
      pan = true;
      viewparam.xStart = pos_x;
      viewparam.yStart = pos_y;

    } else if (Qt::ControlModifier & mouse->modifiers()) {
      scl = true;
      viewparam.yStart = pos_y;

    } else {
      rot = true;
      viewparam.xStart = pos_x;
      viewparam.yStart = pos_y;

      viewparam.xLast = (short)viewparam.xRotation;
      viewparam.yLast = (short)viewparam.yRotation;
    }
  }
}

/********************************************************************************/

void glcanvas::mouseReleaseEvent(QMouseEvent *) {

  rot = scl = pan = roll = false;
  updateFlag = NO_UPDATE;
  mevent = false;
}

/********************************************************************************/

void glcanvas::mouseMoveEvent(QMouseEvent *mouse) {
  initialized = true;
  QPoint newPos = mouse->pos();
  hadFocus++;
  //[PASCAL] hack : counting the number of move before getting the release
  // event, if not enough move
  // we don't do anything, to make sure there is no rotation when cpfg get into
  // focus
  if (hadFocus >= 3) {
    hadFocus = 3;

    if (rot)
      rotate(newPos);

    else if (scl)
      scale(newPos);
    else if (roll)
      Roll(newPos);

    else if (pan)
      Pan(newPos);
  }
}

void glcanvas::focusInEvent(QFocusEvent *) {
  initialized = true;
  mevent = false;
  if (selected)
    SetIdle();
}

void glcanvas::focusOutEvent(QFocusEvent *) {
  initialized = true;
  hadFocus = 0;
}

/********************************************************************************/

void glcanvas::rotate(QPoint newPos) {
  // if moved
  makeCurrent();
  if ((newPos.y() != viewparam.yStart) || (newPos.x() != viewparam.xStart)) {

    // update rotation parameters
    viewparam.xRotation += 2 * (newPos.y() - viewparam.yStart);
    viewparam.yRotation += 2 * (newPos.x() - viewparam.xStart);

    viewparam.xStart = newPos.x();
    viewparam.yStart = newPos.y();
    update();
  }
}

void glcanvas::Pan(QPoint newPos) {
  makeCurrent();
  QPoint oldPos(viewparam.xStart, viewparam.yStart);
  QPoint difPos = newPos - oldPos;
  if (!difPos.isNull()) {
    viewparam.xPan -= difPos.x() / viewparam.real_to_pixel;
    viewparam.yPan += difPos.y() / viewparam.real_to_pixel;
    viewparam.xStart = newPos.x();
    viewparam.yStart = newPos.y();
    update();
  }
}

/********************************************************************************/

void glcanvas::scale(QPoint newPos) {
  /* if moved */
  makeCurrent();
  if (newPos.y() != viewparam.yStart) {

    // update scaling parameters
    if (viewparam.parallel_projection_on) {
      viewparam.scale =
          viewparam.scale *
          (float)pow(UPDATE_SCALE_PAR,
                     (double)(newPos.y() - viewparam.yStart) * 0.01);
    } else
      viewparam.scale =
          viewparam.scale *
          (float)pow(UPDATE_SCALE_PAR,
                     (double)(newPos.y() - viewparam.yStart) * 0.01);

    viewparam.yStart = newPos.y();

    if (viewparam.scale < SCALE_MIN)
      viewparam.scale = SCALE_MIN;
    if (viewparam.scale > SCALE_MAX)
      viewparam.scale = SCALE_MAX;
    SetView(&viewparam);

    update();
  }
}

/********************************************************************************/

void glcanvas::Roll(QPoint newPos) {
  /* if moved */
  makeCurrent();
  if ( (newPos.x() != viewparam.xStart)) {

    // update rotation around z axis
    viewparam.zRotation += 2 * (newPos.x() - viewparam.xStart);
    viewparam.xStart = newPos.x();
    update();
  }
}



/********************** SLOTS **********************/

void glcanvas::setMenuVisible() { _menuVisible = true; }
void glcanvas::setMenuHidden() { _menuVisible = false; }

void glcanvas::SetContinuousMode(bool enable) {
  if (enable && !directoryWatcher) {
    QStringList ignoredPrefixes;
    QStringList ignoredSuffixes;
    directoryWatcher =
        new DirectoryWatcher(QString::fromStdString(_pathToSave),
                             ignoredPrefixes, ignoredSuffixes, this);
    connect(directoryWatcher, SIGNAL(fileChanged(QString)), this,
            SLOT(RequestNewModel(QString)));
  }
  continuousMode->setChecked(true);
  explicitMode->setChecked(false);
}

void glcanvas::SetExplicitMode(bool enable) {
  if (enable && directoryWatcher) {
    delete directoryWatcher;
    directoryWatcher = NULL;
  }
  continuousMode->setChecked(false);
  explicitMode->setChecked(true);
}

// [Pascal] keep opening the file until the size is stable
int glcanvas::waitOpenFile(const char *fname) {
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  FILE *fp = fopen(fname, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
  while ((fp == NULL) && (counter < 10000)) {
    fp = fopen(fname, "r");
    counter++;
  }
  if (counter == 10000) {
    fprintf(stderr, "WARNING: Can't open file %s.\n", fname);
    return 0;
  } else {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size = ftell(fp);       // get current file pointer
    counter = 0;
    while (((size == 0) || (current_size != size)) && (counter < 10000)) {
      current_size = size;
      fclose(fp);

      fp = NULL;
      while (fp == NULL) {
        fp = fopen(fname, "r");
        counter++;
      }
      fseek(fp, 0, SEEK_END); // seek to end of file
      size = ftell(fp);       // get current file pointer
    }
  }
  fseek(fp, 0L, SEEK_SET);
  fclose(fp);
  if (counter >= 1000) {
    Utils::Warning("we can't reload the file : %d\n",counter);
    return 0;
  }

  return 1;
}

int glcanvas::ActionToBePerformedInContinuousMode(QString filename) {
  // action to be performed according to the modified file
  // LSystem has been modified
  QFileInfo fileInfo(filename);
  QString f(fileInfo.fileName());
  std::string file = f.toStdString();
  QString lSystemFile = QString(clp.lfilename);
  if (f.compare(lSystemFile) == 0) {
    if (waitOpenFile(file.c_str()) == 0)
      return 0;
    return 1;
  }
  for (int i = 0; i < MAXCOLORMAPS; ++i) {
    QString colormapfile = QString(clp.colormapname[i]);
    if (f.compare(colormapfile) == 0)
      return 3;
  }
  for (int i = 0; i < MAXCOLORMAPS; ++i) {
    QString materialfile = QString(clp.materialname[i]);
    if (f.compare(materialfile) == 0)
      return 3;
  }

  for (int i = 0; i < SplineFuncCount(); ++i) {
    QString functionfile = QString(SplineFuncName(i));
    if (f.compare(functionfile) == 0) {
      return 4;
    }
  }

  if (FunctionsFromGallery()) {
    QString galleryName = QString(GetGalleryName());
    if (f.compare(galleryName) == 0) {
      return 4;
    }
  }

  // [PASCAL] should call\ CURVEXYZ ?
  for (int i = 0; i < CurveXYZCount(); ++i) {
    QString curvefile = QString(CurveXYZName(i));

    if (f.compare(curvefile) == 0) {
      return 9;
    }
  }
  for (int i = 0; i < clp.con_stored; ++i) {
    QString contourFile = QString(clp.contourFileName[i]);

    if (f.compare(contourFile) == 0) {
      return 5;
    }
  }

  QString animFile = QString(clp.afilename);
  if (f.compare(animFile) == 0)
    return 6;

  QString viewFile = QString(clp.vfilename);
  if (f.compare(viewFile) == 0)
    return 7;

  QString fileExtension = QFileInfo(f).suffix();
  if (fileExtension.compare("s") == 0)
    return 10;

  for (int i = 0; i < NUMTEXTURES; i++) {
    QString texturefile = QString(clp.textureFiles[i]);
    if (f.compare(texturefile) == 0) {
      return 8;
    }
  }

  return 0;
}

void glcanvas::ExecuteAction(int code) {
  QString newWindowTitle = "_title";

  QString text = QString("Executing: ");
  switch (code) {
  case 1:
    text.append(QString("New Model"));
    newWindowTitle.append(QString(" (")).append(text).append(")");
    NewLSystem();
    break;
  case 2:
    text.append(QString("New run"));
    newWindowTitle.append(QString(" (")).append(text).append(")");
    Rerun();
    break;
  case 3:
    RereadColors();
    break;
  case 4:
    RereadFunctions();
    break;
  case 5:
    RereadContours();
    break;
  case 6:
    RereadAnimParams();
    break;
  case 7:
    NewView();
    break;
  case 8:
    RereadDrawParams();
    break;
  case 9:
    RereadCurveXYZ();
    break;
  case 10:
    RereadSurfaces();
    break;
  default:
    break;
  }
  update();
}

void glcanvas::Idle() {
  //  QWidget::setCursor(WaitCursor);
  int idle = 0;
  makeCurrent();

  // [Pascal] but before that if files such as material or palette
  // in continuous mode are not read correctly, don't do anything ....
  if (clp.filesFullyLoaded == 1) {

    auto idle_clock_start = std::chrono::high_resolution_clock::now();
    idle = IdleFunction();
    clock_end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds> (clock_end-clock_start);
    auto elapsedIdle = std::chrono::duration_cast<std::chrono::microseconds> (clock_end-idle_clock_start);

    clock_start = clock_end;
  }
  clp.filesFullyLoaded = 1;

  if ((_is_recording) && (!_menuVisible))
    save();

  if (idle == 1) {
    idleTimer->stop();
    QWidget::setCursor(ArrowCursor);
  }

  ////////////////////////////////////////////////
  //[Pascal] in animate mode (and running) idleFunction return 0, why is it so ?
  // to prevent, we force to redraw if we are in animate mode
  /*
  if (new_model_pending != 0) {
    //std::cerr<<"NEW MODEL PENDING"<<std::endl;
    int code = new_model_pending;
    new_model_pending = 0;
    ExecuteAction(code);
    // change opacity of the window if code = 1 or 2?
  }
  */
  // update is done by a QTimer: if in animate mode
  update();
  
}

void glcanvas::IdleContinuousMode() {
  QWidget::setCursor(ArrowCursor);
  if (new_model_pending != 0) {
    int code = new_model_pending;
    new_model_pending = 0;
    ExecuteAction(code);
   }
}


/********************************************************************************/

void glcanvas::SetIdle() {
  idleTimer->setSingleShot(false);
  int swapinterval = 10 * (int)animparam.swap_interval;

  idleTimer->start(swapinterval); // start timer connected to Idle()
}

/********************************************************************************/

void glcanvas::RequestNewModel(QString f) {
  int code = this->ActionToBePerformedInContinuousMode(f);
  if (!idleContinuousTimer->isActive()) {
    idleContinuousTimer->setSingleShot(true);
    idleContinuousTimer->start(0);
    ExecuteAction(code);
  } else {
    new_model_pending = code;
  }
}

/********************************************************************************/

void glcanvas::NewModel() {
  makeCurrent();
  menu_item = 11;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);
  _id = 0; // resetting numbering every new model
  mevent = false;
  SetIdle();
 
  update();
}

/********************************************************************************/

void glcanvas::NewLSystem() {
  makeCurrent();
  menu_item = 12;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();

  isRunning = false;
  isRunningForEver = false;

  makeCurrent();
  SetGraphics();

  update();
}

/********************************************************************************/

void glcanvas::NewHomomorph() {
  makeCurrent();
  menu_item = 16;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
  update();
}

/********************************************************************************/

void glcanvas::NewView() {
  makeCurrent();
  menu_item = 13;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();

  update();
}

/********************************************************************************/

void glcanvas::NewAnimate() {
  makeCurrent();
  menu_item = 14;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
  update();
}

/********************************************************************************/

void glcanvas::AnimateMode() {
  makeCurrent();
  mPopmenu = false;
  sPopmenu = true;

  menu_item = 20;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
  SetGraphics();

  isRunning = false;
  isRunningForEver = false;
  update();
}

/********************************************************************************/

void glcanvas::Step() {
  makeCurrent();
  if (mPopmenu) {
    mPopmenu = false;
    sPopmenu = true;
  }

  menu_item = 21;
  QWidget::setCursor(WaitCursor);

  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
  isRunning = false;
  isRunningForEver = false;
  update();
}

/********************************************************************************/

void glcanvas::Run() {
  clock_start = std::chrono::high_resolution_clock::now();

  makeCurrent();
  if (mPopmenu) {
    mPopmenu = false;
    sPopmenu = true;
  }

  menu_item = 22;
  //  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
  isRunning = true;
  isRunningForEver = false;
  update();
}

/********************************************************************************/

void glcanvas::Forever() {
  clock_start = std::chrono::high_resolution_clock::now();

  makeCurrent();
  if (mPopmenu) {
    mPopmenu = false;
    sPopmenu = true;
  }

  menu_item = 23;
  //  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
  isRunningForEver = true;
  isRunning = false;
  update();
}

/********************************************************************************/

void glcanvas::Stop() {

  if (mPopmenu) {
    mPopmenu = false;
    sPopmenu = true;
  }
  isRunning = false;
  isRunningForEver = false;

  menu_item = 24;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  return;
  SetIdle();
}

void glcanvas::Pause() {
  if (mPopmenu) {
    mPopmenu = false;
    sPopmenu = true;
  }
  currentAnimateState = animateFlag;
  bool wasRunning = isRunning;
  bool wasRunningForEver = isRunningForEver;
  Stop();
  isRunning = wasRunning;
  isRunningForEver = wasRunningForEver;
}

void glcanvas::setMenuTriggered(QAction *action) {
  _menuTriggered = true;

  if (action == _stopAction) {
    currentAnimateState = false;
  }
}

void glcanvas::RestoreState() {
  if ((isRunning) && (currentAnimateState))
    Run();
  else if ((isRunningForEver) && (currentAnimateState))
    Forever();
  return;
}

/********************************************************************************/

void glcanvas::Rewind() {
  if (mPopmenu) {
    mPopmenu = false;
    sPopmenu = true;
  }

  menu_item = 25;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
  isRunning = false;
  isRunningForEver = false;

  makeCurrent();
  SetGraphics();

  update();
}

/********************************************************************************/
// doesn't work properly
void glcanvas::Clear() {
  makeCurrent();
  menu_item = 26;
  QWidget::setCursor(WaitCursor);

  if (double_buffering && clp.doublebuffer) {
  }
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  SetGraphics();

  update();
  QWidget::setCursor(ArrowCursor);

  mevent = false;
}

void glcanvas::RereadDrawParams() {
  makeCurrent();
  menu_item = 141;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);
  mevent = false;
  SetIdle();
}

void glcanvas::RereadColors() {
  makeCurrent();
  menu_item = 100;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);
  mevent = false;

  SetIdle();
}

void glcanvas::RereadAnimParams() {
  makeCurrent();
  menu_item = 14;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);
  mevent = false;

  SetIdle();

  if (double_buffering)
    _openGlBehavior = QOpenGLWidget::NoPartialUpdate;
  else
    _openGlBehavior = QOpenGLWidget::PartialUpdate;

  setUpdateBehavior(_openGlBehavior);

  update();
}

void glcanvas::RereadSurfaces() {
  makeCurrent();
  menu_item = 102;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);
  mevent = false;
  SetIdle();
}

void glcanvas::RereadContours() {
  makeCurrent();
  menu_item = 101;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);
  mevent = false;
  SetIdle();
}

void glcanvas::RereadCurveXYZ() {
  makeCurrent();
  menu_item = 104;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);
  mevent = false;
  SetIdle();
}

void glcanvas::RereadFunctions() {
  makeCurrent();
  menu_item = 103;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);
  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::StartRecording() {
  if (!_is_recording) {
    _recordingId->setText("Stop Recording");
    _is_recording = true;

    if (_numberingImageName == 0) {
      QString message = QString((_imageBaseName + getExtension()).c_str());
      QMessageBox box;
      box.setWindowTitle("Numbering is off");
      box.setText("File " + message + " exists.");
      box.setInformativeText(
          "Numbering file name (otherwise file will be overwritten)?");
      box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
      box.setDefaultButton(QMessageBox::Cancel);
      box.setEscapeButton(QMessageBox::Cancel);
      box.setIcon(QMessageBox::Warning);
      int ret = box.exec();

      switch (ret) {
      case QMessageBox::Ok:
        _numberingImageName = true;
        break;
      case QMessageBox::Cancel:
        _numberingImageName = false;
        break;
      default:
        break;
      }
      QWidget::setCursor(ArrowCursor); // change to arrow cursor */
    }
  } else {
    _recordingId->setText("Start Recording");
    _is_recording = false;
  }
}

/********************************************************************************/

void glcanvas::StopRecording() {
  _recordingId->setText("Start Recording");
  _is_recording = false;
}
/********************************************************************************/

void glcanvas::DontAnimate() {
  mPopmenu = true;
  sPopmenu = false;

  menu_item = 27;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  double_buffering = 1;

  SetIdle();
}

/********************************************************************************/

void glcanvas::RecordImageRGB() {
  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_RGB;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::RecordImagePNG() {
  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_PNG;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::RecordImageBMP() {
  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_BMP;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}
/********************************************************************************/

void glcanvas::RecordImageGIF() {
  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_GIF;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}
/********************************************************************************/

void glcanvas::RecordImageJPG() {
  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_JPG;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}
/********************************************************************************/

void glcanvas::RecordImageTIFF() {
  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_TIFF;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}
/********************************************************************************/

void glcanvas::RecordImagePBM() {
  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_PBM;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::RecordImageTGA() {
  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_TGA;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::RecordImageRAY() {

  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_RAYSHADE;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::RecordImagePS() {
  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_POSTSCRIPT;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::RecordImageGLS() {

  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_GLS;

  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::RecordImageVV() {

  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_VIEWVOLUME;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::RecordStringText() {

  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_STRING;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::RecordStringBinary() {

  menu_item = OUTPUT_ANIMATE_OFFSET + SAVE_STRINGDUMP;
  QWidget::setCursor(WaitCursor);
  SelectInMenu(menu_item);
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::InputText() {
  std::string filename = _filename + ".str";

  // text string
  QWidget::setCursor(WaitCursor);
  InputString(filename.c_str(), 't');
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::InputTextFrom() {

  // open dialog box to the input filename from user
  QString inputname = QFileDialog::getOpenFileName(
      this, "", clp.savefilename[SAVE_STRING], "*.str");

  if (!inputname.isNull()) {
    char *name = const_cast<char *>(inputname.toStdString().c_str());

    QWidget::setCursor(WaitCursor);
    InputString(name, 't');
    QWidget::setCursor(ArrowCursor);
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::InputBinary() {
  std::string filename = _filename + ".str";

  // binary string
  QWidget::setCursor(WaitCursor);
  InputString(filename.c_str(), 'b');
  QWidget::setCursor(ArrowCursor);

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::InputBinaryFrom() {

  // open dialog box to the input filename from user
  QString inputname = QFileDialog::getOpenFileName(
      this, "", clp.savefilename[SAVE_STRINGDUMP], "*.strb");

  if (!inputname.isNull()) {
    char *name = const_cast<char *>(inputname.toStdString().c_str());

    QWidget::setCursor(WaitCursor);
    InputString(name, 'b');
    QWidget::setCursor(ArrowCursor);
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/
void glcanvas::SaveAsRGB() {
  // open dialog box to the savename for file from user

  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_RGB], "*.rgb");

  // if the user did not hit cancel
  if (!savefname.isNull()) {

    // if .rgb file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".rgb");

    char *name = const_cast<char *>(savefname.toStdString().c_str());

    OpenOutputFile(name, SAVE_RGB);
  }

  mevent = false;
  SetIdle();
}

///// Code From QT4 used to get transparency while saving images

static void convertFromGLImage(QImage &img, int w, int h, bool alpha_format,
                               bool include_alpha) {
  if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
    // OpenGL gives RGBA; Qt wants ARGB
    uint *p = (uint *)img.bits();
    uint *end = p + w * h;
    if (alpha_format && include_alpha) {
      while (p < end) {
        uint a = *p << 24;
        *p = (*p >> 8) | a;
        p++;
      }
    } else {
      // This is an old legacy fix for PowerPC based Macs, which
      // we shouldn't remove
      while (p < end) {
        *p = 0xff000000 | (*p >> 8);
        ++p;
      }
    }
  } else {
    // OpenGL gives ABGR (i.e. RGBA backwards); Qt wants ARGB
    for (int y = 0; y < h; y++) {
      uint *q = (uint *)img.scanLine(y);
      for (int x = 0; x < w; ++x) {
        const uint pixel = *q;
        if (alpha_format && include_alpha) {
          *q = ((pixel << 16) & 0xff0000) | ((pixel >> 16) & 0xff) |
               (pixel & 0xff00ff00);
        } else {
          *q = 0xff000000 | ((pixel << 16) & 0xff0000) |
               ((pixel >> 16) & 0xff) | (pixel & 0x00ff00);
        }

        q++;
      }
    }
  }
  img = img.mirrored();
}

QImage glcanvas::qt_gl_read_framebuffer(const QSize &size, bool alpha_format,
                                        bool include_alpha) {
  QImage img(size, (alpha_format && include_alpha)
                       ? QImage::Format_ARGB32_Premultiplied
                       : QImage::Format_RGB32);
  int w = size.width();
  int h = size.height();
  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
  convertFromGLImage(img, w, h, alpha_format, include_alpha);
  return img;
}

QImage glcanvas::grabFrameBuffer(bool withAlpha) {
  makeCurrent();
  QImage res;
  int w = width();
  int h = height();
  if (true) {
    res = qt_gl_read_framebuffer(QSize(w, h), format().hasAlpha(), withAlpha);
  } else {
  }

  return res;
}

///////////////////////////////////////

/********************************************************************************/

void glcanvas::SaveAsPNG() {
  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_PNG], "*.png");
  // if the user did not hit cancel
  if (!savefname.isNull()) {

    // if .png file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".png");

    char *name = const_cast<char *>(savefname.toStdString().c_str());

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    image.save(QString(name), "PNG");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle(); // [PASCAL] is this SetIdle necessary?
}

/********************************************************************************/

void glcanvas::SaveAsBMP() {
  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_BMP], "*.bmp");

  // if the user did not hit cancel
  if (!savefname.isNull()) {

    // if .bmp file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".bmp");

    char *name = const_cast<char *>(savefname.toStdString().c_str());

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    image.save(QString(name), "BMP");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}
/********************************************************************************/

void glcanvas::SaveAsJPG() {
  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_JPG], "*.jpg");

  // if the user did not hit cancel
  if (!savefname.isNull()) {

    // if .jpg file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".jpg");

    char *name = const_cast<char *>(savefname.toStdString().c_str());

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    image.save(QString(name), "JPG");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}
/********************************************************************************/

void glcanvas::SaveAsGIF() {
  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_GIF], "*.gif");

  // if the user did not hit cancel
  if (!savefname.isNull()) {

    // if .gif file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".gif");

    char *name = const_cast<char *>(savefname.toStdString().c_str());

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    image.save(QString(name), "GIF");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}
/********************************************************************************/

void glcanvas::SaveAsTIFF() {
  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_TIFF], "*.tiff");

  // if the user did not hit cancel
  if (!savefname.isNull()) {

    // if .tiff file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".tiff");

    char *name = const_cast<char *>(savefname.toStdString().c_str());

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    image.save(QString(name), "TIFF");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}
/********************************************************************************/

void glcanvas::SaveAsPBM() {
  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_PBM], "*.pbm");

  // if the user did not hit cancel
  if (!savefname.isNull()) {

    // if .pbm file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".pbm");

    char *name = const_cast<char *>(savefname.toStdString().c_str());

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    image.save(QString(name), "PBM");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveAsTGA() {

  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_TGA], "*.tga");

  // if the user did not hit cancel
  if (!savefname.isNull()) {
    // if .tga file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".tga");

    char *name = const_cast<char *>(savefname.toStdString().c_str());
    OpenOutputFile(name, SAVE_TGA);
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveAsRAY() {

  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_RAYSHADE], "*.ray");

  // if the user did not hit cancel
  if (!savefname.isNull()) {
    // if .ray file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".ray");

    char *name = const_cast<char *>(savefname.toStdString().c_str());
    OpenOutputFile(name, SAVE_RAYSHADE);
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveAsPS() {

  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_POSTSCRIPT], "*.ps");

  // if the user did not hit cancel
  if (!savefname.isNull()) {
    // if .ps file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".ps");

    char *name = const_cast<char *>(savefname.toStdString().c_str());
    OpenOutputFile(name, SAVE_POSTSCRIPT);
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveAsSTR() {

  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_STRING], "*.str");

  // if the user did not hit cancel
  if (!savefname.isNull()) {
    // if .tga file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".str");

    char *name = const_cast<char *>(savefname.toStdString().c_str());
    OpenOutputFile(name, SAVE_STRING);
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveAsSTRB() {

  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_STRINGDUMP], "*.strb");

  // if the user did not hit cancel
  if (!savefname.isNull()) {
    // if .strb file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".strb");

    char *name = const_cast<char *>(savefname.toStdString().c_str());
    OpenOutputFile(name, SAVE_STRINGDUMP);
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveAsGLS() {

  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_GLS], "*.gls");

  // if the user did not hit cancel
  if (!savefname.isNull()) {
    // if .gls file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".gls");

    char *name = const_cast<char *>(savefname.toStdString().c_str());
    OpenOutputFile(name, SAVE_GLS);
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveAsVV() {

  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_VIEWVOLUME], "*.vv");

  // if the user did not hit cancel
  if (!savefname.isNull()) {
    // if .vv file extension not found in savefname we must add it
    if (savefname.indexOf('.') == -1)
      savefname.append(".vv");

    char *name = const_cast<char *>(savefname.toStdString().c_str());
    OpenOutputFile(name, SAVE_VIEWVOLUME);
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveAsOBJ() {
  QString savefname = QFileDialog::getSaveFileName(
      this, "", clp.savefilename[SAVE_OBJ], "*.obj");

  if (savefname.isNull()) {
    if (savefname.indexOf('.') == -1)
      savefname.append(".obj");
    Free(clp.savefilename[SAVE_OBJ]);
    clp.savefilename[SAVE_OBJ] = Strdup(savefname.toStdString().c_str());
    OutputObj();
    fclose(clp.savefp[SAVE_OBJ]);
  }
  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SavePNG() {
  if (validLsystem) {
    makeCurrent();
    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    std::string filename = _filename + ".png";
    if (access(filename.c_str(), F_OK) == 0) {
      if (!OverwriteDialogBox(SAVE_PNG, filename.c_str()))
        return;
    }

    image.save(QString::fromStdString(_filename + ".png"), "PNG");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle(); //[PASCAL] is this necessary?
}

/********************************************************************************/

void glcanvas::SavePDF() {
  if (validLsystem) {
    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    std::string filename = _filename + ".pdf";
    if (access(filename.c_str(), F_OK) == 0) {
      if (!OverwriteDialogBox(SAVE_PNG, filename.c_str()))
        return;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::NativeFormat);
    printer.setColorMode(QPrinter::Color);
    printer.setOutputFileName(QString::fromStdString(_filename + ".pdf"));
    printer.setPaperSize(QSizeF(image.size()), QPrinter::DevicePixel);
    printer.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
    printer.setFullPage(true);
    printer.setOrientation(QPrinter::Portrait);

    QPainter painter(&printer);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform);
    painter.drawImage(QPoint(0, 0), image);
    painter.end();
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveTIFF() {
  if (validLsystem) {
    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    std::string filename = _filename + ".tiff";
    if (access(filename.c_str(), F_OK) == 0) {
      if (!OverwriteDialogBox(SAVE_TIFF, filename.c_str()))
        return;
    }

    image.save(QString::fromStdString(_filename + ".tiff"), "TIFF");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveBMP() {
  if (validLsystem) {
    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    std::string filename = _filename + ".bmp";
    if (access(filename.c_str(), F_OK) == 0) {
      if (!OverwriteDialogBox(SAVE_BMP, filename.c_str()))
        return;
    }

    image.save(QString::fromStdString(_filename + ".bmp"), "BMP");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveJPG() {
  if (validLsystem) {
    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    std::string filename = _filename + ".jpg";
    if (access(filename.c_str(), F_OK) == 0) {
      if (!OverwriteDialogBox(SAVE_JPG, filename.c_str()))
        return;
    }

    image.save(QString::fromStdString(_filename + ".jpg"), "JPG");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveGIF() {
  if (validLsystem) {
    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    std::string filename = _filename + ".gif";
    if (access(filename.c_str(), F_OK) == 0) {
      if (!OverwriteDialogBox(SAVE_GIF, filename.c_str()))
        return;
    }

    image.save(QString::fromStdString(_filename + ".gif"), "GIF");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SavePBM() {
  if (validLsystem) {
    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    QImage image = this->grabFrameBuffer(_alphaChannel);
    std::string filename = _filename + ".pbm";
    if (access(filename.c_str(), F_OK) == 0) {
      if (!OverwriteDialogBox(SAVE_PBM, filename.c_str()))
        return;
    }

    image.save(QString::fromStdString(_filename + ".pbm"), "PBM");
    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveRGB() {
  if (validLsystem) {
    char *name = clp.savefilename[SAVE_RGB];
    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */

    OpenOutputFile(name, SAVE_RGB);

    QWidget::setCursor(ArrowCursor); // change to arrow cursor */
  }
  mevent = false;
  SetIdle();
}

/*******************************************************************************/

void glcanvas::SaveTGA() {

  if (validLsystem) {
    char *name = clp.savefilename[SAVE_TGA];

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    OpenOutputFile(name, SAVE_TGA);
    QWidget::setCursor(ArrowCursor); /* change to arrow cursor */
  }

  mevent = false;
  SetIdle();
}

void glcanvas::SaveRAY() {
  if (validLsystem) {

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    char *name = const_cast<char *>((_filename + ".ray").c_str());
    OpenOutputFile(name, SAVE_RAYSHADE);
    QWidget::setCursor(ArrowCursor); /* change to arrow cursor */
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SavePS() {
  if (validLsystem) {

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    char *name = const_cast<char *>((_filename + ".ps").c_str());
    OpenOutputFile(name, SAVE_POSTSCRIPT);
    QWidget::setCursor(ArrowCursor); /* change to arrow cursor */
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveSTR() {
  if (validLsystem) {

    QWidget::setCursor(WaitCursor); // change to hourglass cursor
    char *name = const_cast<char *>((_filename + ".str").c_str());
    OpenOutputFile(name, SAVE_STRING);
    QWidget::setCursor(ArrowCursor); // change to arrow cursor
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveSTRB() {
  if (validLsystem) {

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor*/
    char *name = const_cast<char *>((_filename + ".strb").c_str());
    OpenOutputFile(name, SAVE_STRINGDUMP);
    QWidget::setCursor(ArrowCursor); /* change to arrow cursor */
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveGLS() {
  if (validLsystem) {

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    std::string filename = _filename + ".gls";
    if (access(filename.c_str(), F_OK) == 0) {
      if (!OverwriteDialogBox(SAVE_GLS, filename.c_str()))
        return;
    }

    QString savefname = QString::fromStdString(filename);

    Free(clp.savefilename[SAVE_GLS]);
    clp.savefilename[SAVE_GLS] = Strdup(savefname.toStdString().c_str());
    char *name = const_cast<char *>(filename.c_str());
    SaveFile(name, SAVE_GLS);

    QWidget::setCursor(ArrowCursor); /* change to arrow cursor */
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveVV() {
  if (validLsystem) {

    QWidget::setCursor(WaitCursor); /* change to hourglass cursor */
    std::string filename = _filename + ".vv";
    if (access(filename.c_str(), F_OK) == 0) {
      if (!OverwriteDialogBox(SAVE_VIEWVOLUME, filename.c_str()))
        return;
    }

    QString savefname = QString::fromStdString(filename);

    Free(clp.savefilename[SAVE_VIEWVOLUME]);
    clp.savefilename[SAVE_VIEWVOLUME] = Strdup(savefname.toStdString().c_str());
    char *name = const_cast<char *>(filename.c_str());
    SaveFile(name, SAVE_VIEWVOLUME);
    QWidget::setCursor(ArrowCursor); /* change to arrow cursor */
  }

  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::SaveOBJ() {
  if (validLsystem) {
    QWidget::setCursor(WaitCursor);
    std::string filename = _filename + ".obj";
    if (access(filename.c_str(), F_OK) == 0) {
      if (!OverwriteDialogBox(SAVE_OBJ, filename.c_str()))
        return;
    }

    QString savefname = QString::fromStdString(filename);

    Free(clp.savefilename[SAVE_OBJ]);
    clp.savefilename[SAVE_OBJ] = Strdup(savefname.toStdString().c_str());
    OutputObj();
    fclose(clp.savefp[SAVE_OBJ]);
    QWidget::setCursor(ArrowCursor);
  }
  mevent = false;
  SetIdle();
}

/********************************************************************************/

void glcanvas::exit() {
  menu_item = 99;
  SelectInMenu(menu_item);
}

/********************************************************************************/

void glcanvas::OpenOutputFile(char *filename, int format) {
  // if file exists, give user option of overwriting or canceling the
  // save
  std::string saveFilename = std::string(filename);
  if (access(filename, F_OK) == 0) {
    if (!OverwriteDialogBox(format, (saveFilename.c_str())))
      return;
  }

  // save the file
  makeCurrent();
  SaveFile(const_cast<char *>(saveFilename.c_str()), format);
}

/********************************************************************************/

// returns true if the user wants to overwrite the old file
// false otherwise
bool glcanvas::OverwriteDialogBox(int, const char *sfilename) {

  //  if (animateFlag) // if we animate, we don't overwrite anyway (question has
  //  been asked earlier return true;
  bool overwrite = false;

  QString message = " ";
  message.append(sfilename);
  QMessageBox box;
  box.setWindowTitle("Overwrite File");
  box.setText("File " + message + " exists.");
  box.setInformativeText("Overwrite?");
  box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  box.setDefaultButton(QMessageBox::Cancel);
  box.setEscapeButton(QMessageBox::Cancel);
  box.setIcon(QMessageBox::Warning);
  int ret = box.exec();

  switch (ret) {
  case QMessageBox::Ok:
    overwrite = true;
    break;
  case QMessageBox::Cancel:
    overwrite = false;
    break;
  default:
    break;
  }

  QWidget::setCursor(ArrowCursor); // change to arrow cursor */

  return overwrite;
}

/********************************************************************************/

void glcanvas::savewindow(char *name, int rgb) {

  char cmd[256];
  int xorg = 0, yorg = 0;
  GLint xsize, ysize;
  int y, x;
  GLboolean index_mode;
  unsigned short *rows;
  unsigned char *rowb;
  IMAGE *oimage;
  GLint tmp[4];

  if (clp.graphics_output) {
    glGetIntegerv(GL_VIEWPORT, tmp);
    xsize = tmp[2];
    ysize = tmp[3];
  } else {
    xsize = clp.xsize;
    ysize = clp.ysize;
    rgb = 0;
  }

  if (rgb) {
    sprintf(cmd, "scrsave %s %d %d %d %d\n", name, xorg + 1, xorg + xsize,
            yorg - ysize - 1, yorg - 2);
    system(cmd);
  }

  // this code doesn't implement .ras files!!!!!
  else {
    if ((rows = (unsigned short *)Malloc(
             (unsigned int)(xsize * sizeof(short)))) == NULL)
      return;
    if ((rowb = (unsigned char *)Malloc(
             (unsigned int)(xsize * sizeof(char)))) == NULL)
      return;

    // Sets a temporary orthographics projection corresponding with the size
    // the saved image

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glOrtho(-0.5, xsize - 0.5, -0.5, ysize - 0.5, 0, 1);

    // actual read buffer is the front one
    glReadBuffer(GL_FRONT);

    if (clp.graphics_output)
      // query the actual mode
      glGetBooleanv(GL_INDEX_MODE, &index_mode);
    else
      index_mode = 0; // always rgb mode for off-screen rendering

    if (!index_mode) {
      // RGBA mode
      if (clp.verbose)
        Message("Outputing truecolor image.\n");

      // iopen *always* returns a NULL if we pass it "w" mode
      // i have no idea why this line of code was here, so i'm taking it
      // out--sam
      // oimage = iopen(name, "w", RLE(1), 3, xsize, ysize,3);

      oimage = iopen(name, "w", RLE(1), 3, xsize, ysize, 3);
      if (oimage == NULL) {
        perror(name);
        goto ret;
      }

      // this function is empty so i'm commenting it out -- Sam
      // isetname(oimage,name);

      for (y = 0; y < ysize; y++) {
        glReadPixels(0, y, (int)xsize, 1, GL_RED, GL_UNSIGNED_BYTE, rowb);
        for (x = 0; x < xsize; x++)
          rows[x] = rowb[x];
        putrow(oimage, rows, y, 0);

        glReadPixels(0, y, (int)xsize, 1, GL_GREEN, GL_UNSIGNED_BYTE, rowb);
        for (x = 0; x < xsize; x++)
          rows[x] = rowb[x];
        putrow(oimage, rows, y, 1);

        glReadPixels(0, y, (int)xsize, 1, GL_BLUE, GL_UNSIGNED_BYTE, rowb);
        for (x = 0; x < xsize; x++)
          rows[x] = rowb[x];
        putrow(oimage, rows, y, 2);
      }
    } else {
      // index mode
      if (clp.verbose)
        Message("Outputing colormap image.\n");

      oimage = iopen(name, "w", RLE(1), 2, xsize, ysize, 1);
      if (oimage == NULL) {
        perror(name);
        goto ret;
      }
      oimage->colormap = CM_SCREEN; // see image.h
      isetname(oimage, name);

      for (y = 0; y < ysize; y++) {
        glReadPixels(0, y, (int)xsize, 1, GL_COLOR_INDEX, GL_UNSIGNED_SHORT,
                     rows);
        putrow(oimage, rows, y, 0);
      }
    }

    Free(rows);
    Free(rowb);
    iclose(oimage);
  ret:
    // restores the previous projection
    glPopMatrix();
    // modelview matrix is kept as default
    glMatrixMode(GL_MODELVIEW);
  }
}

/********************************************************************************/

int glcanvas::InputString(const char *filename, char type) {
  FILE *fp;
  char str[30];
  unsigned long len;
  char unique_strb_header[] = "L-system string:";

  if (strcmp(filename, "stdin") == 0)
    fp = stdin;
  else if ((fp = fopen(filename, "r")) == NULL) {
    Utils::Warning("Cannot open string file %s!\n", filename);
    return 0;
  }

  if (type == 'b') {
    // binary
    if (fread(str, strlen(unique_strb_header), 1, fp) < 1) {
      Utils::Warning("Cannot read header of file %s!\n", filename);
      return 0;
    }
    str[strlen(unique_strb_header)] = 0;

    // safety check
    if (strcmp(unique_strb_header, str) != 0) {
      Utils::Warning("File doesn't start with correct header '%s'! '%s' found.\n",
              unique_strb_header, str);
      return 0;
    }

    // get the length
    if (fscanf(fp, "%lu %d", &len, &LsystemList->current) < 1) {
      Utils::Warning("Cannot read string length and/or current step!\n");
      return 0;
    }

    fgetc(fp);

    MakeEnoughRoom(len);

    if (fread(currentString, 1, len, fp) < len)
      Utils::Warning("Only part of the string read in !!!\n");
  } else {
    char mod;
    float val;

    // no header
    len = 0;

    while (!feof(fp)) {
      MakeEnoughRoom(len + MAXPARMS * 5 + 2);
      mod = fgetc(fp);
      // skip white spaces
      if (isspace(mod))
        continue;

      currentString[len++] = mod;

      if (mod == '(') {
        mod = ',';

        while (mod == ',') {
          fscanf(fp, "%f", &val);
          memcpy(currentString + len, &val, sizeof(val));
          len += sizeof(val);

          mod = fgetc(fp);
          currentString[len++] = mod;
        }

        if (mod != ')')
          Utils::Warning("Error! ')' expected!\n");
      }
    }
    // the last character read is the eof, it messes up with the homorphism
    // so just remove this last character so we don't have any issue
    *(currentString + len-1) = 0;
  }
  if (fp != stdin)
    fclose(fp);


  // reset initial rotation
  viewparam.xRotation = 0;
  viewparam.yRotation = 0;

  init_blackbox(); // initialize blackbox functions

  Redraw(rfScale | rfViewVol | rfClear);

  return 1;
}

/********************************************************************************/

void glcanvas::MakeEnoughRoom(unsigned long len) {

  unsigned long step = (unsigned long)floor(log((double)len) / log(2.0)) / 2;

  if (step <= 100000)
    step = 100000;

  // check whether enough room
  if (currentString + len + 1 > currentStringEnd) {
    currentStringLength = (1 + (len + 1000) / step) * step;
    Utils::Warning("Inserted string is too long; reallocating.\n");
    // reallocate should start one character to the left

    if ((currentString =
             (char *)Realloc(currentString - 1, currentStringLength)) == NULL) {
      Utils::Warning("Error: can't reallocate L-system string.\n");
      MyExit(1);
    }

    currentStringEnd = currentString + currentStringLength - 1;
    currentString++;
  }
}

void glcanvas::saveAs() {
  SaveAs *window = new SaveAs(this, QString::fromStdString(_imageBaseName),
                              QString::fromStdString(_pathToSave),
                              QString(QDir::currentPath()), _numberingImageName,
                              _id, _outputFormat, _pix_format);
  QString savePath;
  window->setAttribute(Qt::WA_DeleteOnClose, false);
  window->setAlphaChannel(_alphaChannel);
  int result = window->exec();
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  window->close();

  if (result) {
    _id = window->getId();
    _outputFormat = window->getFormat();
    _pathToSave = window->getPath().toStdString();
    int imageType = window->getImageType();

    _imageBaseName = window->getImageBaseName().toStdString();
    _numberingImageName = window->getNumbering();
    _alphaChannel = window->getAlphaChannel();
    _pix_format = imageType;
    if (_alphaChannel) {
      _backGroundIsTransparent = true;
    }
    save();
  }
}

std::string glcanvas::getExtension() {
  std::string ext = "";
  if (_numberingImageName) {
    std::stringstream st1;
    st1.fill('0');
    st1.width(4);
    st1 << _id;
    ext = st1.str();
  }

  switch (_outputFormat) {
  case 0: {
    switch (_pix_format) {
    case 0:
      ext += ".bmp";
      break;
    case 1:
      ext += ".jpg";
      break;
    case 2:
      ext += ".pdf";
      break;
    case 3:
      ext += ".png";
      break;
    case 4:
      ext += ".tiff";
      break;
    default:
      ext += ".png";
      break;
    }
    break;
  }
  case 1: {
    ext += ".ps";
    break;
  }
  case 2:
    ext += ".obj";
    break;
  case 3:
    ext += ".ray";
    break;
  case 4:
    ext += ".gls";
    break;
  case 5:
    ext += ".vv";
    break;
  default:
    ext += ".png";
    break;
  }
  return ext;
}

void glcanvas::save() {
  _filename = _pathToSave + "/" + _imageBaseName;
  _saveAction->setText(
      QString(("Save " + _imageBaseName + getExtension()).c_str()));
  if (_numberingImageName) {
    std::stringstream st1;
    st1.fill('0');
    st1.width(4);
    st1 << _id;
    _filename = _filename + st1.str();
  }

  switch (_outputFormat) {
  case 0: {
    switch (_pix_format) {
    case 0:
      SaveBMP();
      break;
    case 1:
      SaveJPG();
      break;
    case 2: {
      SavePDF();
      break;
    }
    case 3:
      SavePNG();
      break;
    case 4:
      SaveTIFF();
      break;
    default:
      SavePNG();
      break;
    }
    break;
  }
  case 1: {
    SavePS();
    break;
  }
  case 2:
    SaveOBJ();
    break;
  case 3:
    SaveRAY();
    break;
  case 4:
    SaveGLS();
    break;
  case 5:
    SaveVV();
    break;
  default:
    break;
  }
  if (_numberingImageName) {
    _id++;
    std::stringstream st1;
    st1.fill('0');
    st1.width(4);
    st1 << _id;
    _filename = _filename + st1.str();
  }
  _saveAction->setText(
      QString(("Save " + _imageBaseName + getExtension()).c_str()));
}

// Routine to render text
inline GLint glcanvas::project(GLdouble objx, GLdouble objy, GLdouble objz,
                               const GLdouble model[16],
                               const GLdouble proj[16], const GLint viewport[4],
                               GLdouble *winx, GLdouble *winy, GLdouble *winz) {
  GLdouble in[4], out[4];
  in[0] = objx;
  in[1] = objy;
  in[2] = objz;
  in[3] = 1.0;
  transformPoint(out, model, in);
  transformPoint(in, proj, out);

  if (in[3] == 0.0)
    return GL_FALSE;

  in[0] /= in[3];
  in[1] /= in[3];
  in[2] /= in[3];

  *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
  *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;

  *winz = (1 + in[2]) / 2;
  return GL_TRUE;
}

inline void glcanvas::transformPoint(GLdouble out[4], const GLdouble m[16],
                                     const GLdouble in[4]) {
#define M(row, col) m[col * 4 + row]
  out[0] =
      M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
  out[1] =
      M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
  out[2] =
      M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
  out[3] =
      M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

void glcanvas::renderText(double x, double y, double z, const QString &str,
                          const QColor color, const QFont &font) {
  if (viewparam.z_buffer_on)
    glDisable(GL_DEPTH_TEST);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  int height = this->height();

  GLdouble model[4][4], proj[4][4];
  GLint view[4];
  glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
  glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
  glGetIntegerv(GL_VIEWPORT, &view[0]);
  GLdouble textPosX = 0, textPosY = 0, textPosZ = 0;

  project(x, y, z, &model[0][0], &proj[0][0], &view[0], &textPosX, &textPosY,
          &textPosZ);

  textPosY = height - textPosY; // y is inverted

  int fontSize = font.pointSize();

  QFontMetrics metrics(font);
  int text_width = metrics.width(QString(str)) + 10;
  int text_height = fontSize + 5;
  QPixmap textimg(text_width, text_height);
  textimg.fill(Qt::transparent);

  QPainter painter(&textimg);
  QColor backgroundColor;
  if (clp.iscolormapfile) { /* RGBA mode */
    short red, green, blue;
    my_getmcolor((short)clp.colormap, &red, &green, &blue);
    backgroundColor = QColor(red, green, blue);
  } else if (clp.ismaterialfile) {
    /* background is the emissive color of material 0 (wrt reference */
    /* colormap) */
    material_type *mat;
    my_getmaterial(clp.colormap, &mat);
    backgroundColor =
        QColor((int)(255 * mat->emissive[0]), (int)(255 * mat->emissive[1]),
               (int)(255 * mat->emissive[2]));
  }
  painter.setRenderHints(QPainter::HighQualityAntialiasing |
                         QPainter::TextAntialiasing |
                         QPainter::NonCosmeticDefaultPen);

  painter.setBrush(color);
  painter.setPen(color);
  painter.setFont(font);
  painter.drawText(5, text_height - 2, str);
  painter.end();
  QPainter p1(this);
  p1.drawPixmap(textPosX - 5, textPosY - text_height + 2, textimg);
  glPopAttrib();
  glPopClientAttrib();

  glEnable(GL_DEPTH_TEST);
  // Enable blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
