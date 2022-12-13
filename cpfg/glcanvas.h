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



#ifndef GLCANVAS_H
#define GLCANVAS_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_3_Core>

#include "control.h"
#include "platform.h"
#include "interpret.h"
#include "irisGL.h"

#include <QMenu>
#include <QCursor>
#include <QTimer>
#include <QFocusEvent>
#include <QMouseEvent>
// MC - Oct. 2015 - added Qt support for OpenGL shaders
#include "shaders.h"
#include <QMainWindow>
#include <QtOpenGL>
#include <chrono>
typedef std::chrono::high_resolution_clock::time_point Clock;

class DirectoryWatcher;

extern DRAWPARAM drawparam;
extern int cursType;
extern int is_menu;

QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)
QT_FORWARD_DECLARE_CLASS(QOpenGLShader)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)


// MC - Oct. 2015 - added QGLFunctions for creating shadow map
class glcanvas : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT
private:
  Clock clock_start,clock_end;

  int near, far;

  bool initialized, first_run;
  int new_model_pending;

  GLfloat xRot, yRot;
  GLfloat xScale, yScale;

  int menu_item;
  int hadFocus;
  int _outputFormat;
  int _pix_format;
  int _id;
  int _numberingImageName;
  std::string _imageBaseName;
  std::string _pathToSave;
  std::string _filename;
  QAction *_recordingId;
  bool _is_recording;
  bool _alphaChannel;
  bool _backGroundIsTransparent;

  bool _menuVisible;

  QAction *_saveAction;
  QAction *_clearAction;

  QAction *explicitMode;
  QAction *continuousMode;

  QAction *_stepAction;
  QAction *_stopAction;
  QAction *_runAction;
  QAction *_foreverAction;

  bool isRunning;
  int currentAnimateState;
  bool actionTriggered;
  bool isRunningForEver;
  bool _menuTriggered;
  QOpenGLWidget::UpdateBehavior _openGlBehavior;

  int waitOpenFile(const char *fname);

  // roate/scale flags
  bool rot, scl, pan, roll;

  // this flag is true if mainPop is displayed; else false
  bool mPopmenu;
  bool sPopmenu;

  // flag if a mouse event has come in for the popupmenu
  bool mevent, selected;

  // cursor
  QCursor *curs;

  // popup menu members
  QMenu *mainPop; // default popupmenu
  QMenu *secondaryPop;

  // sub (popup) menu members of mainPop
  QMenu *outputPop;
  QMenu *inputPop;

  // sub (popup) menu members of outputPop
  QMenu *imagePop;
  QMenu *rayshadePop;
  QMenu *pScriptPop;
  QMenu *stringOutPop;
  QMenu *glsFormPop;
  QMenu *viewVolPop;
  QMenu *objPop;

  // sub (popup) menu members of imagePop
  QMenu *rgbPop;
  QMenu *pngPop;
  QMenu *bmpPop;
  QMenu *gifPop;
  QMenu *jpgPop;
  QMenu *tiffPop;
  QMenu *pbmPop;
  QMenu *tgaPop;

  // sub (popup) menu members of stringOutPop
  QMenu *textOutPop;
  QMenu *binaryOutPop;

  // sub (popup) menu members of inputPop
  QMenu *stringInPop;

  // sub (popup) menu members of stringInPop
  QMenu *textInPop;
  QMenu *binaryInPop;

  // sub (popup) menus of secondaryPop
  QMenu *beginRecPop;
  QMenu *imageRecPop;
  QMenu *stringRecPop;

  DirectoryWatcher *directoryWatcher;

  virtual void initializeGL();
  virtual void paintGL();
  void Repaint();
  virtual void resizeGL(int w, int h);
  void rotate(QPoint);
  void scale(QPoint);
  void Pan(QPoint);
  void Roll(QPoint);


  // taken and modified (slightly) from control.c
  void OpenOutputFile(char *, int);
  void savewindow(char *, int);
  int InputString(const char *, char);
  void MakeEnoughRoom(unsigned long);

  // other functions
  bool OverwriteDialogBox(int, const char *);
  void getWindowOrigin(int *, int *);
  void SetIdle();

  // member functions
  /* mouse handling functions */
  void mousePressEvent(QMouseEvent *);
  void mouseReleaseEvent(QMouseEvent *);
  void mouseMoveEvent(QMouseEvent *);
  void focusInEvent(QFocusEvent *fevent);
  void focusOutEvent(QFocusEvent *fevent);
  void keyPressEvent(QKeyEvent *event);
  int _keyPressed;

  std::string getExtension();

private slots:

#ifdef LINUX
  void RestoreState();
  void Pause();
#endif
  void NewModel();
  void NewLSystem();
  void NewHomomorph();
  void NewView();
  void NewAnimate();
  void AnimateMode();
  void Step();
  void Run();
  void Forever();
  void Stop();
  void Rewind();
  void Clear();
  void RereadColors();
  void RereadAnimParams();
  void RereadSurfaces();
  void RereadContours();
  void RereadCurveXYZ();
  void RereadFunctions();
  void RereadDrawParams();
  void SetContinuousMode(bool enabled);
  void SetExplicitMode(bool enabled);

  void StopRecording();
  void StartRecording();
  void DontAnimate();
  void RecordImageRGB();
  void RecordImagePNG();
  void RecordImageGIF();
  void RecordImageJPG();
  void RecordImageBMP();
  void RecordImageTIFF();
  void RecordImagePBM();
  void RecordImageTGA();
  void RecordImageRAY();
  void RecordImagePS();
  void RecordImageGLS();
  void RecordImageVV();
  void RecordStringText();
  void RecordStringBinary();

  void SaveAsRGB();
  void SaveAsPNG();
  void SaveAsGIF();
  void SaveAsJPG();
  void SaveAsBMP();
  void SaveAsTIFF();
  void SaveAsPBM();
  void SaveAsTGA();
  void SaveAsRAY();
  void SaveAsPS();
  void SaveAsSTR();
  void SaveAsSTRB();
  void SaveAsGLS();
  void SaveAsVV();
  void SaveAsOBJ();
  void SavePS();
  void SaveRGB();
  void SavePNG();
  void SavePDF();
  void SaveGIF();
  void SaveJPG();
  void SaveBMP();
  void SaveTIFF();
  void SavePBM();
  void SaveTGA();
  void SaveRAY();
  void SaveSTR();
  void SaveSTRB();
  void SaveGLS();
  void SaveVV();
  void SaveOBJ();

  void save();
  void saveAs();

  // input slots go here later
  void InputText();
  void InputTextFrom();
  void InputBinary();
  void InputBinaryFrom();

  // timer slot
  void Idle();
  void IdleContinuousMode();

  void setMenuVisible();
  void setMenuHidden();

  void RequestNewModel(QString);
  int ActionToBePerformedInContinuousMode(QString f);
  void ExecuteAction(int code);
  void setMenuTriggered(QAction *);

signals:
  void signalSave(char *, int);

protected:
  // this gets rid of some unnecessary redraws
  virtual void windowActivationChange(bool) { ; }

  QImage qt_gl_read_framebuffer(const QSize &size, bool alpha_format,
                                bool include_alpha);

public slots:
  void exit();


public:
  glcanvas(QWidget *parent);

  ~glcanvas();

  // following is used to render text
  inline GLint project(GLdouble objx, GLdouble objy, GLdouble objz,
                       const GLdouble model[16], const GLdouble proj[16],
                       const GLint viewport[4], GLdouble *winx, GLdouble *winy,
                       GLdouble *winz);
  inline void transformPoint(GLdouble out[4], const GLdouble m[16],
                             const GLdouble in[4]);

  void renderText(double x, double y, double z, const QString &str,
                  const QColor color, const QFont &font = QFont());

  bool isBackgroundTransparent() { return _backGroundIsTransparent; }

  QImage grabFrameBuffer(bool);
};

#endif
