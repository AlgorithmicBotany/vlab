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


#define GL_SILENCE_DEPRECATION

#include "glwidget.h"
#include <QPainter>
#include <QPaintEngine>
#include <QOpenGLShaderProgram>
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QDesktopWidget>
#include <qmath.h>
#include <iostream>
#include "lpfg.h"

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include "projection.h"

#include "viewLnx.h"
#include "animparam.h"
#include "glutils.h"

#include "glenv.h"

GLWidget::GLWidget(View *mw, bool button, const QColor &background,
                   const int id, const QOpenGLWidget::UpdateBehavior behaviour)
    : _IdleTask(this), _RotateTask(this), _ZoomTask(this),
      _FrustumTask(this), _RollTask(this), _PanTask(this), m_mainWindow(mw), _view(mw)
{
  _format.setRedBufferSize(8);
  _format.setGreenBufferSize(8);
  _format.setBlueBufferSize(8);
  _format.setAlphaBufferSize(8);
  int samples = drawparams.antialiasing();
  if (samples > 0){
    _format.setSamples(samples);
  }
  setFormat(_format);
  
  _id = id;
  _pQ = 0;
  _pLpfg = mw->_pLpfg;
  _pTask = &_IdleTask;
  setUpdateBehavior(behaviour);
  shadowMapResolution = drawparams.ShadowMapSize();
  _initiated = false;
  _created = true;
}

GLWidget::~GLWidget() {

  // And now release all OpenGL resources.
  makeCurrent();
  if (_pQ)
    gl.DeleteQuadric(_pQ);
  doneCurrent();
}

void GLWidget::initializeGL() {

  initializeOpenGLFunctions(); // MC - Oct. 2015 - Qt setup for OpenGL
                               // extensions

  makeCurrent();
  int samples = drawparams.antialiasing();
  if (samples > 0){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
  }
  else{
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_LINE_SMOOTH);
  }  
  // MC - Oct. 2015 - support for shadow mapping using shaders from GLSL 1.2
  // check if shaders are supported
  if (!QOpenGLShaderProgram::hasOpenGLShaderPrograms() ||
      !hasOpenGLFeature(QOpenGLFunctions::Framebuffers)) {
    Utils::Message( "Warning! the 'render mode: shadows' view option will not work.\n");
    Utils::Message("OpenGL Shading Language or Framebuffers are not supported "
		   "and shadows will not be rendered.\n");
    drawparams.SetRenderMode(DParams::rmShaded);
  } else {
    // even if "render mode: shadows" is not specified when the OpenGL context
    // is first created, the shaders are loaded in case the render mode is
    // changed in the view file later, this is done in the View.
    initShadowMap();
  }

  _projection.Reset();

  // MC - Sept. 2016 - moved SetModifiers() before SetVolume(), so viewing
  // volume can be computed from view direction
  if (drawparams.ViewModifiersSet(_id))
    _projection.SetModifiers(drawparams.Modifiers(_id));

  if (drawparams.IsBoundingBoxSet(_id)) {
    _projection.SetVolume(drawparams.BoundingBox(_id), drawparams.Clip());
  } else {
    _projection.SetVolumeAndPos(_pLpfg->GetLEngine().CalculateVolume(_id),
                                drawparams.Clip());
  }

  if (0 == _pQ) {
    _pQ = gl.CreateQuadric();
  }

  // MC - June 2020 - gl.DoInit loads textures but it should do
  // this once only (and not for each view).
  // I changed DoInit to load textures if _id==0
  // (which is a quick solution but not very robust...)
  // gl.DoInit(_pQ, _id);
  // Call ResetOpenGL instead because light positions need to be
  // adjusted depending on the "stationary lights" command
  ResetOpenGL();

  // MC - June 2018 - In Qt 5, save the default frame buffer object
  // (not always == 0 like before)
  defaultFBO = defaultFramebufferObject();
}

QImage GLWidget::grabFrameBuffer(bool withAlpha) {

  makeCurrent();

  if (drawparams.antialiasing() > 0) {
    Utils::Message("WARNING: Image output with antialiasing is not supported.\n");
    Utils::Message("\tSet antialiasing: 0 in the view file.\n");
  }

  // ensure we will read the front buffer
  glReadBuffer(GL_FRONT);

  // get the size of the buffer (directly from OpenGL)
  // (we could compare to size of GLWidget window)
  GLint fbDims[4] = {0,0,0,0};
  glGetIntegerv(GL_VIEWPORT, fbDims);
  GLint fbWidth = fbDims[2];
  GLint fbHeight = fbDims[3];

  // Next, we test the platform's byte order for big endian or little endian,
  // which we assume corresponds to GL_RGBA or GL_BGRA buffer format.
  // But it is doubtful this will always work because we are reading a buffer
  // from the graphics card and the format probably depends on graphics hardware
  // and not the platform only.
  // In OpenGL 4.3 or later, you can query the graphics card for the buffer format
  // (using the function glGetInternalformativ(...)), so that's nice...

  // make a QImage and read the pixels from the buffer into it
  QImage img(fbWidth, fbHeight, QImage::Format_ARGB32);
  if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
    // OpenGL buffer is in RGBA format
    glReadPixels(0, 0, fbWidth, fbHeight, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
  } else {
    // OpenGL buffer is in BGRA format
    glReadPixels(0, 0, fbWidth, fbHeight, GL_BGRA, GL_UNSIGNED_BYTE, img.bits());
  }
 
  // if alpha channel was not requested, convert the image to RGB
  // (this assumes the default internal format of the frame buffer has alpha)
  if (!withAlpha) {
      img = img.convertToFormat(QImage::Format_RGB32);
  }
  // Qt wants the frame buffer mirrored...
  return img.mirrored();
}

void GLWidget::paintGL() {
  // Start painting OpenGL
  makeCurrent();
  int samples = drawparams.antialiasing();
  if (samples > 0){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
  }
  else{
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_LINE_SMOOTH);
  }  

  unsigned int clear =
      (animparam.ClearBetweenFrames() || clear_glwidget || !_initiated);
  if (clear) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    clear_glwidget = false;
    _initiated = true;
  }
  Repaint();
  // not used anymore: _initializeGL = true;
}

void GLWidget::resizeGL(int width, int height) {
  if (height > 0 && width > 0) {
    const int retinaScale = devicePixelRatio();
    _projection.Resize(width * retinaScale, height * retinaScale);
    makeCurrent();
    _projection.Apply(drawparams.ProjectionMode());
  }
  resize(width, height);
}

// Not used:
/*
void GLWidget::setupViewport(int width, int height) {
  int side = qMin(width, height);
  glViewport((width - side) / 2, (height - side) / 2, side, side);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
#ifdef QT_OPENGL_ES
  glOrthof(-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#else
  glOrtho(-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#endif
  glMatrixMode(GL_MODELVIEW);
}
*/
void GLWidget::handleButtonPress() {}

void GLWidget::mousePressEvent(QMouseEvent *pEv) {
  // must be LPFG actions
  // emit mouseClicked(pEv->pos());

  switch (pEv->button()) {
  case Qt::LeftButton:
    // emulate midlle button
    if (_keyPressed == Qt::Key_Z)
      MBDown(pEv);
    else
      LBDown(pEv);
    break;
  case Qt::MidButton:
    MBDown(pEv);
    break;
  case Qt::RightButton:
    RBDown(pEv);
    break;
  default:
    break;
  }
}

void GLWidget::MBDown(QMouseEvent *pEv) {
  m_mainWindow->setGLWidgetClicked(_id);
  if (pEv->modifiers() & Qt::ControlModifier)
    StartFrustum(pEv->x(), pEv->y());
  else if (pEv->modifiers() & Qt::ShiftModifier)
    StartRoll(pEv->x(), pEv->y());
  else
    StartZooming(pEv->x(), pEv->y());
}

void GLWidget::RBDown(QMouseEvent *pEv) {
  m_mainWindow->setGLWidgetClicked(_id);

  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
  if (topWidget != nullptr)
    topWidget->raise();
  if (!((pEv->modifiers() & Qt::ControlModifier) &&
        (pEv->modifiers() & Qt::ShiftModifier) &&
        (pEv->modifiers() & Qt::AltModifier)) &&
      (_keyPressed != Qt::Key_4)) {

  } else if (_keyPressed == Qt::Key_4) {
    _view->UserContextMenu(pEv->x(), pEv->y());
    UserMenu::Map entries = _view->_pLpfg->_userMenu.entries;
    QMenu userMenu(this);
    for (UserMenu::Map::const_iterator iter = entries.begin();
         iter != entries.end(); iter++) {
      string name = (*iter).second;
      userMenu.addAction(name.c_str());
    }

    QAction *selectedItem = userMenu.exec(QCursor::pos());
    if (selectedItem) {
      QList<QAction *> actions = userMenu.actions();
      for (int i = 0; i < actions.size(); ++i) {
        if (actions.at(i) == selectedItem) {
          _view->_pLpfg->_userMenuChoice = i;
        }
      }
    } else {
      // nothing was chosen
    }
  }
}

void GLWidget::LBDown(QMouseEvent *pEv) {
  m_mainWindow->setGLWidgetClicked(_id);

  if (_keyPressed == Qt::Key_1) {
    if (!m_mainWindow->isRunning() && !m_mainWindow->isRunningForever())
      Utils::Message( "WARNING: the model is not running, insertion is not active\n");
    else {
      InsertX(pEv->x(), pEv->y());
      m_mainWindow->setLBMouseStatus(true);
      _mouseStatus.lbDown = true;
    }
  } else if (_keyPressed == Qt::Key_2) {
    if (!m_mainWindow->isRunning() && !m_mainWindow->isRunningForever())
      Utils::Message("WARNING: the model is not running, positionning is not active\n");
    else {
      InsertPosX(pEv->x(), pEv->y());
      m_mainWindow->setLBMouseStatus(true);
      _mouseStatus.lbDown = true;
    }
  } else if (_keyPressed == Qt::Key_3) {
    m_mainWindow->setLBMouseStatus(true);
    _mouseStatus.lbDown = true;
  } else if (_keyPressed == Qt::Key_4) {
    _keyPressed = Qt::Key_unknown;

    if (!m_mainWindow->isRunning() && !m_mainWindow->isRunningForever())
      Utils::Message("WARNING: the model is not running, menu are not available\n");
    UserMenu::Map entries = m_mainWindow->_pLpfg->_userMenu.entries;
    QMenu userMenu(this);
    for (UserMenu::Map::const_iterator iter = entries.begin();
         iter != entries.end(); iter++) {
      string name = (*iter).second;
      userMenu.addAction(name.c_str());
    }

    QAction *selectedItem = userMenu.exec(QCursor::pos());
    if (selectedItem) {
      QList<QAction *> actions = userMenu.actions();
      for (int i = 0; i < actions.size(); ++i) {
        if (actions.at(i) == selectedItem) {
          m_mainWindow->_pLpfg->_userMenuChoice = i;
        }
      }
    } else {
      // nothing was chosen
    }
  } else if ((pEv->modifiers() & Qt::ControlModifier) &&
             (pEv->modifiers() & Qt::ShiftModifier)) {
    if (!m_mainWindow->isRunning() && !m_mainWindow->isRunningForever())
      Utils::Message( "WARNING: the model is not running, insertion is not active\n");
    else {
      InsertX(pEv->x(), pEv->y());
      m_mainWindow->setLBMouseStatus(true);
      _mouseStatus.lbDown = true;
    }
  } else if ((pEv->modifiers() & Qt::ShiftModifier) &&
             (pEv->modifiers() & Qt::AltModifier)) {
    m_mainWindow->setLBMouseStatus(true);
    _mouseStatus.lbDown = true;

  } else if ((pEv->modifiers() & Qt::ControlModifier) &&
             (pEv->modifiers() & Qt::AltModifier)) {
    if (!m_mainWindow->isRunning() && !m_mainWindow->isRunningForever())
      Utils::Message("WARNING: the model is not running, positionning is not active\n");
    else {
      InsertPosX(pEv->x(), pEv->y());
      m_mainWindow->setLBMouseStatus(true);
      _mouseStatus.lbDown = true;
    }
  } else if (pEv->modifiers() & Qt::ShiftModifier)
    StartPanning(pEv->x(), pEv->y());
  else if (pEv->modifiers() & Qt::ControlModifier) {
    StartZooming(pEv->x(), pEv->y());
  } else {
    StartRotating(pEv->x(), pEv->y());
  }
}

void GLWidget::mouseMoveEvent(QMouseEvent *pEv) {
  QPoint pos = pEv->pos();
  _pTask->MouseMove(pos.x(), pos.y());
}

void GLWidget::mouseReleaseEvent(QMouseEvent *) {
  _keyPressed = Qt::Key_unknown;
  m_mainWindow->setLBMouseStatus(false);
  _mouseStatus.lbDown = false;
  StartIdle();
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
  if (event->isAutoRepeat())
    event->ignore();

  _keyPressed = event->key();
}

void GLWidget::keyReleaseEvent(QKeyEvent *event) {
  Q_UNUSED(event);
  _keyPressed = Qt::Key_unknown;
  m_mainWindow->setLBMouseStatus(false);
  _mouseStatus.lbDown = false;
  StartIdle();
}

void GLWidget::StartIdle() {
  // Only regenerate if something has actually moved
  if (_hasMoved && DParams::rovTriggered == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  _hasMoved = false;
  _pTask = &_IdleTask;
}

/// LPFG Actions

void GLWidget::StartRotating(int x, int y) {
  _pTask = &_RotateTask;
  _pTask->ButtonDown(x, y);
}

void GLWidget::StartPanning(int x, int y) {
  _pTask = &_PanTask;
  _pTask->ButtonDown(x, y);
}

void GLWidget::StartZooming(int x, int y) {
  _pTask = &_ZoomTask;
  _pTask->ButtonDown(x, y);
}

void GLWidget::StartFrustum(int x, int y) {
  _pTask = &_FrustumTask;
  _pTask->ButtonDown(x, y);
}

void GLWidget::StartRoll(int x, int y) {
  _pTask = &_RollTask;
  _pTask->ButtonDown(x, y);
}

void GLWidget::OutputRGB() { m_mainWindow->OutputRGB(); }

void GLWidget::OutputPovray(int frameNumber) const {
  m_mainWindow->OutputPovray(frameNumber);
}

void GLWidget::OutputPovray(const char *FolderName) const {
  m_mainWindow->OutputPovray(FolderName);
}

void GLWidget::OutputPovray() { m_mainWindow->OutputPovray(); }

void GLWidget::OutputRayshade() { m_mainWindow->OutputRayshade(); }

void GLWidget::OutputView() { m_mainWindow->OutputView(); }

void GLWidget::OutputObj() { m_mainWindow->OutputObj(); }

void GLWidget::OutputPostscript() { m_mainWindow->OutputPostscript(); }

float GLWidget::vvXmin() const { return _projection.MinX(); }

float GLWidget::vvYmin() const { return _projection.MinY(); }

float GLWidget::vvZmin() const { return _projection.MinZ(); }

float GLWidget::vvXmax() const { return _projection.MaxX(); }

float GLWidget::vvYmax() const { return _projection.MaxY(); }

float GLWidget::vvZmax() const { return _projection.MaxZ(); }

float GLWidget::vvScale() const { return 1.0f / _projection.Scale(); }

void GLWidget::ResetRotation() {
  makeCurrent();
  _projection.ResetRotation();
  if (drawparams.ViewModifiersSet(_id)) {
    const WindowParams &wp = drawparams.GetView(_id);
    Vector3d dir = wp.GetDir(), up = wp.GetUp();
    _projection.SetRotation(dir, up);
  }
  _projection.Apply(drawparams.ProjectionMode());
  update();
}

void GLWidget::ResetZoom() {

  makeCurrent();
  _projection.ResetZoom();
  if (drawparams.ViewModifiersSet(_id)) {
    const WindowParams &wp = drawparams.GetView(_id);
    _projection.SetZoom(wp.Scale());
  }
  _projection.Apply(drawparams.ProjectionMode());
  update();
}

void GLWidget::ResetPan() {
  makeCurrent();
  _projection.ResetPan();
  if (drawparams.ViewModifiersSet(_id)) {
    const WindowParams &wp = drawparams.GetView(_id);
    _projection.SetPan(wp.GetPan());
  }
  _projection.Apply(drawparams.ProjectionMode());
  update();
}

void GLWidget::ResetRoll() {

  makeCurrent();
  _projection.ResetRoll();
  _projection.Apply(drawparams.ProjectionMode());
  update();
}

void GLWidget::ResetView() {
  Showing(true);
  m_mainWindow->SetTitle();
  update();
}

void GLWidget::ResetOpenGL() {
  if (drawparams.StationaryLights()) {
    // MC - Nov. 2015 - fix for stationary lights in lpfg for "new render"
    // selection: lpfg should not apply camera transformation to lights. Recall
    // OpenGL applies the modelview transformation to the light position (when
    // glLightf() is called in LightSource.cpp's Apply function), but, if
    // stationary lights are 'on', lpfg assumes the position is fixed. in the
    // case of positional or spot lights, we want to apply a translation...
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // get the original view direction from the camera translation
    // default view direction is set in ResetRotation() in projection.cpp
    Vector3d viewDir = Vector3d(0, 0, -1);
    if (drawparams.ViewModifiersSet(_id)) {
      const WindowParams::ViewModifiers vm = drawparams.Modifiers(_id);
      viewDir = vm.viewDir;
    }
    // apply the translation
    Vector3d viewShift = -_projection.GetLookAt() + _projection.GetPan() +
                         _projection.ZShift() * viewDir;
    glTranslatef(viewShift.X(), viewShift.Y(), viewShift.Z());

    // reset OpenGL settings
    gl.DoInit(_pQ, _id);

    // put the modelview matrix back so the original camera transformation is
    // applied to the model
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  } else {
    // if stationary lights are 'off', then lpfg should apply the
    // view transformation to the position of the light sources
    gl.DoInit(_pQ, _id);
  }
}

void GLWidget::PanBy(int x, int y) {
  _hasMoved = true;
  _projection.PanBy(x, y);
  makeCurrent();
  gl.ClearColor();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  _projection.Apply(drawparams.ProjectionMode());
  if (DParams::rovOn == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  else
    update();
}

void GLWidget::ZoomBy(int y) {

  _hasMoved = true;
  const float mlt = powf(1.01f, static_cast<float>(y));
  _projection.ZoomBy(mlt);
  makeCurrent();
  gl.ClearColor();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  _projection.Apply(drawparams.ProjectionMode());

  if (DParams::rovOn == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  else
    update();
}

void GLWidget::FrustumBy(int y) {
  _hasMoved = true;
  ASSERT(DParams::pmPerspective == drawparams.ProjectionMode());
  const float mlt = powf(1.01f, static_cast<float>(y));
  _projection.FrustumBy(mlt);
  makeCurrent();
  gl.ClearColor();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  _projection.Apply(drawparams.ProjectionMode());
  if (DParams::rovOn == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  else
    update();
}

void GLWidget::RollBy(int x) {
  _hasMoved = true;
  _projection.RollBy(0.5f * x);
  makeCurrent();
  gl.ClearColor();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  _projection.Apply(drawparams.ProjectionMode());
  if (DParams::rovOn == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  else
    update();
}

void GLWidget::DrawParamsChanged() {
  makeCurrent();
  ResetOpenGL();
}

void GLWidget::RotateBy(int x, int y) {
  _hasMoved = true;
  _projection.RotateBy(0.1f * x, -0.1f * y);

  makeCurrent();
  
  gl.ClearColor();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  _projection.Apply(drawparams.ProjectionMode());
  if (DParams::rovOn == drawparams.RedrawOnViewChange()){
    _pLpfg->GenerateStatic();
    return;
  }
  update();
}

void GLWidget::DrawStep() {

  if (m_mainWindow->isRecordingOn()) {
    //[PASCAL] Disable for now
    //_SaveFrame(_pLpfg->GetLEngine().StepNo());
  }
  update();
}

void GLWidget::Repaint() {
  if (m_mainWindow->dontPaint())
    return;
  makeCurrent();
  
  // MC - Oct. 2015 - generate shadow map.
  // this could be done only in DrawStep but this way there is a single shadow
  // map for all view windows
  if (drawparams.RenderMode() == DParams::rmShadows) {
    beginShadowMap();
    _pLpfg->GetLEngine().DrawGL(_projection.ViewNormal(), _id, _flist, _pQ);
    endShadowMap();
  }

  if (animparam.ClearBetweenFrames()) {
    gl.ClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  if (!_hasMoved){
    if (animparam.NewViewBetweenFrames())
      ResetView();
    else if (animparam.HCenterBetweenFrames())
      HCenter();
    else if (animparam.ScaleBetweenFrames())
      Scale();
  }

  gl.SetColor(1);

  // Blend has not been set yet shouldn't be restore
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Cull has not been set yet shouldn't be restore
  if (drawparams.BackfaceCulling())
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);

  // MC - Oct. 2015 - use shader if shadow mapping selected
  if (drawparams.RenderMode() == DParams::rmShadows)
    beginMainShader();

  _pLpfg->GetLEngine().DrawGL(_projection.ViewNormal(), _id, _flist, _pQ);
  _DrawAxis();

  if (drawparams.RenderMode() == DParams::rmShadows)
    endMainShader();

  glFlush();
}

void GLWidget::Resize(int w, int h) {
  // the code here was the same as in resizeGL(w,h)
  // so, why not make the call...
  resizeGL(w,h);
}

void GLWidget::UserContextMenu(int, int) {
  std::cerr << "UserContextMenu Not yet implemented" << std::endl;
}

void GLWidget::ContextMenu(int, int) {
  std::cerr << "ContextMenu Not yet implemented" << std::endl;
}

void GLWidget::Command(int) {
  std::cerr << "Command Not yet implemented" << std::endl;
}

void GLWidget::ShowAxis(bool) {
  std::cerr << "ShowAxis Not yet implemented" << std::endl;
}

bool GLWidget::IsVisible() const { return isVisible(); }

void GLWidget::Show() {
  show();
  Showing(true);
  update();
}

void GLWidget::Showing(bool show) {
  if (show) {
    makeCurrent();
    _projection.Reset();

    // MC - Sept. 2016 - moved SetModifiers() before SetVolume(), so viewing
    // volume can be computed from view direction
    if (drawparams.ViewModifiersSet(_id))
      _projection.SetModifiers(drawparams.Modifiers(_id));

    if (drawparams.IsBoundingBoxSet(_id))
      _projection.SetVolume(drawparams.BoundingBox(_id), drawparams.Clip());
    else
      _projection.SetVolumeAndPos(_pLpfg->GetLEngine().CalculateVolume(_id),
                                  drawparams.Clip());

    _projection.Apply(drawparams.ProjectionMode());
  }
  // why does _pQ have to be created here?
  // some other part of the code is destroying it?!
  // anyway, _pQ should be created regardless of the value of "show"
  if (_pQ == 0)
    _pQ = gl.CreateQuadric();
}

void GLWidget::Show(const WindowParams &) {
  std::cerr << "Show Not yet implemented" << std::endl;
}

void GLWidget::ShowMaximized() {
  std::cerr << "ShowMaximized Not yet implemented" << std::endl;
}

void GLWidget::HCenter() {
  _projection.HCenter(_pLpfg->GetLEngine().CalculateVolume(_id).first,
                      drawparams.ProjectionMode());
}

void GLWidget::Scale() {
  _projection.Scale(_pLpfg->GetLEngine().CalculateVolume(_id).first,
                    drawparams.Clip(), drawparams.ProjectionMode());
}

void GLWidget::_DrawAxis() {
  if (m_mainWindow->isAxisOn()) {
    View::GLRestore depth(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_TEST);
    View::GLRestore light(GL_LIGHTING);
    glDisable(GL_LIGHTING);
    _projection.DrawAxis();
  }
}

void GLWidget::Clear() { clear_glwidget = true; }

void GLWidget::updateFilename() {
  std::cerr << "updateFilename Not yet implemented" << std::endl;
}

bool GLWidget::OverwriteDialogBox(const char *) {
  std::cerr << "OverwriteDialogBox Not yet implemented" << std::endl;
  return false;
}

void GLWidget::Step() { std::cerr << "Step Not yet implemented" << std::endl; }

void GLWidget::Run() { std::cerr << "Run Not yet implemented" << std::endl; }

void GLWidget::Pause() {
  std::cerr << "Pause Not yet implemented" << std::endl;
}

void GLWidget::save() {
  makeCurrent();
  m_mainWindow->save();
}

void GLWidget::saveAs() {
  std::cerr << "saveAs Not yet implemented" << std::endl;
}

void GLWidget::Animate() {
  std::cerr << "Animate Not yet implemented" << std::endl;
}

void GLWidget::Destroy() {
  std::cerr << "Destroy Not yet implemented" << std::endl;
}

void GLWidget::GetRelSize(const Rect &, float *) const {
  std::cerr << "GetRelSize Not yet implemented" << std::endl;
}

void GLWidget::renderText(double x, double y, double z, const QString &str,
                          const QColor color, const QFont &font) {

  // save GL attributes before calling QPainter
  glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
  glPushAttrib(GL_ALL_ATTRIB_BITS);
    
  // use the Projection class to project x,y,z on the screen
  Vector3d textPos = _projection.Project(x,y,z);
  textPos[1] = float(_projection.getWindowHeight()) - float(textPos[1]);

  // on Retina displays the window coordinates must be scaled
  // devicePixelRatioF returns qreal which should be double
  const double retinaScale = (double)devicePixelRatioF();
  textPos[0] /= float(retinaScale);
  textPos[1] /= float(retinaScale);

  // create a QPixmap to draw the text to 
  int fontSize = font.pointSize();
  QFontMetrics metrics(font);
  int text_width = (metrics.width(QString(str)) + 10);
  //int text_width = (metrics.width(QString(str)) + 10) * int(retinaScale);

  int text_height = fontSize;
  QPixmap textimg(text_width, fontSize + fontSize / 3 + 1);
  //int text_height = fontSize * int(retinaScale);
  //QPixmap textimg(text_width, (fontSize + fontSize / 3 + 1) * retinaScale);
  //textimg.setDevicePixelRatio(retinaScale);
  textimg.fill(Qt::transparent);

  // paint the text to the QPixmap 
  QPainter painter(&textimg);
  painter.setRenderHints(QPainter::HighQualityAntialiasing |
                         QPainter::TextAntialiasing |
                         QPainter::NonCosmeticDefaultPen);
  painter.setBrush(color);
  painter.setPen(color);
  painter.setFont(font);
  painter.drawText(5, text_height, str);
  painter.end();

  // Paint the Pixmap to the OpenGL QtWidget...
  // This causes problems when shadows are enabled, because
  // QPainter's constructor (or QPainter::begin())
  // automatically calls glClear().
  // But why does the call to QPainter p1(this) clear the shadow map?
  // - trying setAutoFillBackground(false) on the widget has no effect
  // - calling glDisable(GL_DEPTH_TEST) has no effect
  // - skipping the renderText when in shadows mode also has no effect
  //GLint fbId;
  //glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbId);
  //if (fbId == ((GLint)shadowMapFBO)) 
  //    return;
  // - Maybe the solution is to use a QOpenGLPaintDevice instead
  QPainter p1(this);
  p1.drawPixmap(static_cast<int>(std::round(textPos[0])) - 5,
                static_cast<int>(std::round(textPos[1])) - text_height + 2,
                textimg);
  //QRectF target(textPos[0] - 5.f, textPos[1] - text_height + 2.f, text_width + 5.f, text_height + 2.f);
  //QRectF source(0,0,text_width,text_height);
  //p1.drawPixmap(target, textimg, source);
  p1.end();

  glPopAttrib();
  glPopClientAttrib();
}

CameraPosition GLWidget::GetCameraPosition(void) const {
  CameraPosition pos;
  // Sometimes, having two separate Vector classes sucks.
  Vector3d vec = _projection.GetLookFrom();
  pos.position = V3f(vec.X(), vec.Y(), vec.Z());

  vec = _projection.GetLookAt();
  pos.lookat = V3f(vec.X(), vec.Y(), vec.Z());

  vec = _projection.ViewNormal();
  pos.head = V3f(vec.X(), vec.Y(), vec.Z());

  vec = _projection.ViewLeft();
  pos.left = V3f(vec.X(), vec.Y(), vec.Z());

  vec = _projection.ViewUp();
  pos.up = V3f(vec.X(), vec.Y(), vec.Z());

  pos.scale = vvScale();
  return pos;
}

// MC - July 2016 - added shadow mapping via GLSL shaders
void GLWidget::initShadowMap(void) {
  // Change default OpenGL texture to white texel instead of black
  // that way whenever a fragment is not textured, it uses this default white
  // texture. It may be faster to use a uniform variable in the shader, but this
  // requires changing lpfg's texturing. Because there is no way to check in the
  // shaders if GL_TEXTURE_2D is enabled, everytime glEnable(GL_TEXTURE_2D) is
  // called a uniform variable would need to set in the shaders to TRUE, and
  // when glDisable(GL_TEXTURE_2D) is called the uniform variable would need to
  // be set to FALSE. Using a default white texture that is applied in the
  // shaders requires almost no changes to lpfg's texturing. Just make sure that
  // when glDisable(GL_TEXTURE_2D) is called, glBindTexture(GL_TEXTURE_2D,0) is
  // called!
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  static GLubyte whiteImage[4] = {255, 255, 255, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               whiteImage);

  // create a shadow-map texture
  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1, &shadowMapTexture);
  glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapResolution,
               shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_R_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  GLfloat ones[4] = {1.f, 1.f, 1.f, 1.f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, ones);

  // create a framebuffer for the shadow map
  glGenFramebuffers(1, &shadowMapFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         shadowMapTexture, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    Utils::Message("Warning! the 'render mode: shadows' view option will not work.\n");
    Utils::Message("Failed to make complete frame buffer object. Error(%d)\n",error);
  }
  // bind render to back buffer
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  glActiveTexture(GL_TEXTURE0);

  // setup shaders - set path to directory in L-studio
#ifdef _WINDOWS
  static char path[_MAX_PATH + 1], vfile[_MAX_PATH + 1], ffile[_MAX_PATH + 1];
  GetModuleFileName(NULL, path, _MAX_PATH); // get path to lpfg.exe
  size_t l = strlen(path);
  // remove 'lpfg.exe' from the path
  while (path[l - 1] != '\\' && l > 0)
    --l;
  path[l - 1] = '\0';
  // add '\'
  strcat(path, "\\");
  ++l;
  strcat(path, "..\\shaders\\");
#else // set path to directory in VLAB
  static char path[PATH_MAX], vfile[PATH_MAX], ffile[PATH_MAX];
#ifdef VLAB_MACX
  QString shadersPath(qgetenv("LPFGRESOURCES"));
  shadersPath += "/shaders/";
#else
  QString shadersPath(qgetenv("VLABROOT"));
  shadersPath += "/share/shaders/";
#endif
  QByteArray ba = shadersPath.toLatin1();
  const char *c_str = ba.data();
  strcpy(path, c_str);
#endif

  // open main shaders
  strcpy(vfile, path);
  strcpy(ffile, path);
  strcat(vfile, "main_vshader.glsl");
  strcat(ffile, "main_fshader.glsl");
  mainShaderProgramID = initShaders(vfile, ffile);

  // open shadow mapping shaders
  strcpy(vfile, path);
  strcpy(ffile, path);
  strcat(vfile, "shadow_vshader.glsl");
  strcat(ffile, "shadow_fshader.glsl");
  shadowShaderProgramID = initShaders(vfile, ffile);

  // set location of uniform variables
  // for the main shaders
  lightMatrixLocation =
      glGetUniformLocation(mainShaderProgramID, "lightMatrix");
  shadowMapLocation = glGetUniformLocation(mainShaderProgramID, "shadowMapTex");
  shadowColorLocation =
      glGetUniformLocation(mainShaderProgramID, "shadowMapColor");
  textureLocation = glGetUniformLocation(mainShaderProgramID, "texture");
  numLightsLocation = glGetUniformLocation(mainShaderProgramID, "numLights");

  // for the shadow mapping shaders
  textureShadowLocation =
      glGetUniformLocation(shadowShaderProgramID, "texture");
}

void GLWidget::freeShadowMap(void) {
  if (mainShaderProgramID)
    glDeleteProgram(mainShaderProgramID);
  if (shadowShaderProgramID)
    glDeleteProgram(shadowShaderProgramID);
  if (shadowMapFBO_supported) {
    glDeleteFramebuffers(1, &shadowMapFBO);
  }
  if (shadowMapTexture)
    glDeleteTextures(1, &shadowMapTexture);
}

GLuint GLWidget::initShaders(const char *vertex_file_path,
                             const char *fragment_file_path) {
  // Create the shader ids
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if (VertexShaderStream.is_open()) {
    std::string Line = "";
    while (getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  } else {
    Utils::Message( "Cannot open vertex shader %s \n 'render mode: shadows' will not work.\n",vertex_file_path);
    return (0);
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if (FragmentShaderStream.is_open()) {
    std::string Line = "";
    while (getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  } else {
    Utils::Message( "Cannot open fragment shader %s \n 'render mode: shadows' will not work.\n",vertex_file_path);
    return (0);
  }

  // Compile the shaders
  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  char const *VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  if (Result == GL_FALSE) {
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    char *InfoLogErrorMessage = (char *)malloc(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL,
                       &InfoLogErrorMessage[0]);
    Utils::Message("%s\n", InfoLogErrorMessage);
    return (0);
  }

  // Compile Fragment Shader
  char const *FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  if (Result == GL_FALSE) {
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    char *InfoLogErrorMessage = (char *)malloc(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL,
                       &InfoLogErrorMessage[0]);
    Utils::Message("%s\n", InfoLogErrorMessage);
    return (0);
  }

  // Link the program
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  if (Result == GL_FALSE) {
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    char *InfoLogErrorMessage = (char *)malloc(InfoLogLength);
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL,
                        &InfoLogErrorMessage[0]);
    Utils::Message("%s\n", InfoLogErrorMessage);
    return (0);
  }

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

void GLWidget::resizeShadowMap(void) {

  if (shadowMapResolution == (unsigned int) drawparams.ShadowMapSize())
    return;

  shadowMapResolution = drawparams.ShadowMapSize();

  // create a shadow-map texture
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapResolution,
               shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_R_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  GLfloat ones[4] = {1.f, 1.f, 1.f, 1.f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, ones);

  // bind default texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void GLWidget::beginShadowMap(void) {
  if (shadowShaderProgramID == 0)
    return;

  // set OpenGL depth testing for generating the shadow map
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClearDepth(1.0);

  // render to shadow buffer
  // set to 'defaultFBO' if rendering shadow map, also comment out mainshader in
  // Repaint
  glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
  // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // use this if
  // rendering shadow map, and comment out glClear in Repaint
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, shadowMapResolution, shadowMapResolution);

  // to address shadow acne, use polygon offset in generating depth map
  glPolygonOffset(drawparams.ShadowMapOffsetFactor(),
                  drawparams.ShadowMapOffsetUnits());
  glEnable(GL_POLYGON_OFFSET_FILL);

  // get the current view matrix and transform the light "view"
  GLfloat gl_modelview[16];
  OpenGLMatrix mvMatrix;
  if (drawparams.StationaryLights()) {
    glGetFloatv(GL_MODELVIEW_MATRIX, gl_modelview);
    mvMatrix = OpenGLMatrix(gl_modelview);
    mvMatrix.Transpose(); // transpose because opengl is column-major order?
  }

  // save and reset the projection and modelview matrix before rendering with
  // the shadow shader because they are used to render from the lights point of
  // view
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // get the light source
  LightSource ls = gl.GetLight(0);
  float *ls_pos = ls.GetPosition();

  // set light projection and view matrix
  if (ls_pos[3] == 0.f) { // directional light source
    // compute light ModelView matrix
    Vector3d lightDir(-ls_pos[0], -ls_pos[1], -ls_pos[2]);
    lightDir.Normalize();
    Vector3d lightUp(0.f, 1.f, 0.f);
    // if light directly above, we need a different 'up' vector
    if (ls_pos[0] == 0.f && ls_pos[2] == 0.f) {
      lightUp = Vector3d(0.f, 0.f, 1.f);
    }

    // apply view matrix
    if (drawparams.StationaryLights()) {
      lightDir.Transform(
          mvMatrix); // thanks to the tranpose, this transformation excludes
                     // translation (rotations only)
      lightUp.Transform(mvMatrix);
    }

    // compute the view volume and its half depth given the light source
    // direction
    Volume lightVV;
    _projection.ComputeViewVolume(lightDir, lightVV);
    float lightVVHalfDepth = 0.5f * Distance(lightVV.Max(), lightVV.Min());
    if (0.f == lightVVHalfDepth)
      lightVVHalfDepth = 0.5f;

    // compute light's position and "lookat" vector (remove panning by mouse)
    Vector3d lightLookAt = _projection.GetLookAt() - _projection.GetPan();
    Vector3d lightPos = lightLookAt - lightVVHalfDepth * lightDir;

    // compute light's view matrix, and set gl_ModelViewMatrix in shader
    gluLookAt(lightPos.X(), lightPos.Y(), lightPos.Z(), lightLookAt.X(),
              lightLookAt.Y(), lightLookAt.Z(), lightUp.X(), lightUp.Y(),
              lightUp.Z());

    // set the gl_projection_matrix in the shadow shader
    glMatrixMode(GL_PROJECTION);
    glOrtho(lightVV.MinX(), lightVV.MaxX(), lightVV.MinY(), lightVV.MaxY(),
            lightVV.MinZ(), lightVV.MaxZ());
  } else {
    float ls_spot[5];
    ls.GetSpotlight(ls_spot);

    if (ls_spot[4] == 180.f) {
      Utils::Message("Warning: Lpfg does not support rendering shadows from "
                     "omnidirectional light sources.\n");
    }

    // compute light ModelView matrix
    Vector3d lightDir(ls_spot[0], ls_spot[1], ls_spot[2]);
    lightDir.Normalize();
    Vector3d lightUp(0.f, 1.f, 0.f);
    // if light directly above, we need a different 'up' vector
    if (lightDir.X() == 0.f && lightDir.Z() == 0.f) {
      lightUp = Vector3d(0.f, 0.f, 1.f);
    }

    Vector3d lightPos =
        Vector3d(ls_pos[0], ls_pos[1], ls_pos[2]) + _projection.GetPan();
    Vector3d lightLookAt = lightPos + lightDir;

    // apply view matrix
    if (drawparams.StationaryLights()) {
      lightPos.Transform(mvMatrix);
      lightLookAt.Transform(mvMatrix);
      lightUp.Transform(mvMatrix);
    }

    // compute light's view matrix, and set gl_ModelViewMatrix in shader
    gluLookAt(lightPos.X(), lightPos.Y(), lightPos.Z(), lightLookAt.X(),
              lightLookAt.Y(), lightLookAt.Z(), lightUp.X(), lightUp.Y(),
              lightUp.Z());

    // compute projection matrix, which sets the gl_projection_matrix in the
    // shadow shader
    glMatrixMode(GL_PROJECTION);
    Vector3d d = lightPos - _projection.GetLookAt();
    // using ls_spot[4] is not correct; it should be the spotCosCutoff (like in
    // the main fragment shader) as this requires the vector from surface to
    // light position, use the maximum spotCutoff of 90
    gluPerspective(90., 1., 1., d.Length() + _projection.Depth());
  }

  // bind the shadow shader for rendering
  glUseProgram(shadowShaderProgramID);

  // set location of default texture (a white pixel) to match
  // glActiveTexture(GL_TEXTURE0)
  glUniform1i(textureShadowLocation, 0);

  // save the light's projection * modelview matrix
  glGetFloatv(GL_PROJECTION_MATRIX, gl_modelview);
  lightMatrix = OpenGLMatrix(gl_modelview);
  glGetFloatv(GL_MODELVIEW_MATRIX, gl_modelview);
  lightMatrix.Multiply(gl_modelview);

  // ensure OpenGL in ModelView matrix state before rendering
  glMatrixMode(GL_MODELVIEW);
}

void GLWidget::endShadowMap(void) {
  if (shadowShaderProgramID == 0)
    return;

  // release the shadow shader after rendering
  glUseProgram(0);

  // render to main frame buffer
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
  glViewport(0, 0, _projection.getWindowWidth(), _projection.getWindowHeight());
  glDisable(GL_POLYGON_OFFSET_FILL);

  // put back the projection and modelview matrix before rendering with main
  // shader
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void GLWidget::beginMainShader(void) {
  if (mainShaderProgramID == 0)
    return;

  glUseProgram(mainShaderProgramID);

  // get the inverse of the view matrix before rendering
  // this undoes view transformations so the correct shadow map coordinate is
  // computed the inverse must be calucated here before the turtle changes the
  // modelview matrix in particular, using the builtin inverse matrices
  // (gl_ModelViewInverse or gl_NormalMatrix) in the shader will not work
  GLfloat modelview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
  OpenGLMatrix invViewMatrix(modelview);
  invViewMatrix.Inverse();

  // pass light projection-model-view matrix to main shader program
  const float offset[16] = {0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0,
                            0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0};
  OpenGLMatrix lightModelView(offset);
  lightModelView.Multiply(
      lightMatrix.Buf()); // multiply by saved projection-modelview matrix
  lightModelView.Multiply(
      invViewMatrix.Buf()); // multiply by inverse of current view matrix
                            // (before turtle interpretation)
  glUniformMatrix4fv(lightMatrixLocation, 1, GL_FALSE, lightModelView.Buf());

  // pass texture to main shader - value used in glActiveTexture()
  glUniform1i(textureLocation, 0);

  // bind shadow-map texture for reading - value used in glActiveTexture()
  glUniform1i(shadowMapLocation, 1);

  // pass shadow color to main shader program
  glUniform4f(shadowColorLocation, drawparams.ShadowMapRed(),
              drawparams.ShadowMapGreen(), drawparams.ShadowMapBlue(), 1.0);

  // pass the number of enabled lights (ignore the default light)
  int numlights = gl.GetLightCount() > 1 ? gl.GetLightCount() - 1 : 1;
  glUniform1i(numLightsLocation, numlights);
}

void GLWidget::endMainShader(void) {
  if (mainShaderProgramID == 0)
    return;
  glUseProgram(0); // release shader from pipeline
}

void GLWidget::InsertX(int x, int y) {
  _DoInsertX(x, y);
  if (comlineparam.DebugMode())
    _pLpfg->GetLEngine().DumpString();
  // Not needed: update();
}

void GLWidget::InsertPosX(int x, int y) { _DoInsertPosX(x, y); }

const float PickWidth = 3.0f, HalfPickWidth = 1;
const float PickHeight = 3.0f, HalfPickHeight = 1;
const int PickRegionSize = 9;

void GLWidget::_DoInsertX(int x, int y) {
  // scale the window coordinates for Retina displays
  const int retinaScale = devicePixelRatio();
  x *= retinaScale;
  y *= retinaScale;

  makeCurrent();
  const int SelectionBufferSize = 1024;
  static GLuint buffer[SelectionBufferSize];
  int hitcount = 0;
  {
    glSelectBuffer(SelectionBufferSize, buffer);
    glSelectionMode rm(hitcount);
    glInitNames();
    float projmtrx[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projmtrx);
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      _projection.PickRegion(static_cast<float>(x), static_cast<float>(y),
                             PickWidth, PickHeight,
                             drawparams.ProjectionMode());
      glMatrixMode(GL_MODELVIEW);
      _pLpfg->GetLEngine().DrawGL(_projection.ViewNormal(), _id, _flist, _pQ);
      glFlush();
    }
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projmtrx);
    glMatrixMode(GL_MODELVIEW);
  }
  if (0 == hitcount) {
    Debug("No hits\n");
    return;
  }
  Debug("Hits: %d\n", hitcount);
  int ix = 0;
  unsigned int minx = UINT_MAX;
  unsigned int minxidx = 0;
  for (int i = 0; i < hitcount; ++i) {
    Debug("Hit # %d\n", i);
    unsigned int n = buffer[ix];
    Debug("Name is %u long\n", n);
    ix++;
    Debug("Zmin = %u\tZmax = %u\n", buffer[ix], buffer[ix + 1]);
    if (buffer[ix] < minx) {
      minx = buffer[ix];
      minxidx = ix - 1;
    }
    ix += 2;
    Debug("Name is: ");
    for (unsigned int j = 0; j < n; ++j) {
      Debug("%u ", buffer[ix]);
      ix++;
    }
    Debug("\n");
  }
  Debug("The closest hit is: %u at %u\n", minx, minxidx);
  int insertpos = buffer[minxidx + 3];
  try {
    __MouseIns_ParamsStructArr_ moduleArr;
    moduleArr.arr[0] = MouseIns_id;
    _pLpfg->GetLEngine().Insert<__MouseIns_ParamsStructArr_>(moduleArr,
                                                             insertpos);
  } catch (char *msg) {
    Debug("Cannot insert X: %s\n", msg);
  }
  // Not needed: update();
}

void GLWidget::_DoInsertPosX(int x, int y) {
  // scale the window coordinates for Retina displays
  const int retinaScale = devicePixelRatio();
  x *= retinaScale;
  y *= retinaScale;

  makeCurrent();
  const int SelectionBufferSize = 1024;
  static GLuint buffer[SelectionBufferSize];
  int hitcount = 0;
  {
    glSelectBuffer(SelectionBufferSize, buffer);
    glSelectionMode rm(hitcount);
    glInitNames();
    float projmtrx[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projmtrx);
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      _projection.PickRegion(static_cast<float>(x), static_cast<float>(y),
                             PickWidth, PickHeight,
                             drawparams.ProjectionMode());
      glMatrixMode(GL_MODELVIEW);
      _pLpfg->GetLEngine().DrawGL(_projection.ViewNormal(), _id, _flist, _pQ);
      glFlush();
    }
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projmtrx);
    glMatrixMode(GL_MODELVIEW);
  }
  if (0 == hitcount) {
    Debug("No hits\n");
    return;
  }
  Debug("Hits: %d\n", hitcount);
  int ix = 0;
  unsigned int minx = UINT_MAX;
  unsigned int minxidx = 0;
  for (int i = 0; i < hitcount; ++i) {
    Debug("Hit # %d\n", i);
    unsigned int n = buffer[ix];
    Debug("Name is %u long\n", n);
    ix++;
    Debug("Zmin = %u\tZmax = %u\n", buffer[ix], buffer[ix + 1]);
    if (buffer[ix] < minx) {
      minx = buffer[ix];
      minxidx = ix - 1;
    }
    ix += 2;
    Debug("Name is: ");
    for (unsigned int j = 0; j < n; ++j) {
      Debug("%u ", buffer[ix]);
      ix++;
    }
    Debug("\n");
  }
  Debug("The closest hit is: %u at %u\n", minx, minxidx);
  int insertpos = buffer[minxidx + 3];
  try {
    __MouseInsPos_ParamsStruct_ module;
    MouseStatus &ms = module.data.Param0;
    ms.viewNum = -1;
    ms.viewX = x;
    ms.viewY = y;
    FillMouseStatus(ms);
    ms.lbPushed = true;
    ms.selectPoint =
        (ms.selectDepth * ms.atRear) + ((1. - ms.selectDepth) * ms.atFront);
    __MouseInsPos_ParamsStructArr_ *pArr =
        reinterpret_cast<__MouseInsPos_ParamsStructArr_ *>(&module);
    pArr->arr[0] = MouseInsPos_id;
    _pLpfg->GetLEngine().Insert<__MouseInsPos_ParamsStructArr_>(*pArr,
                                                                insertpos);
  } catch (char *msg) {
    Debug("Cannot insert X: %s\n", msg);
  }
  // Not needed: update();
}

void GLWidget::FillMouseStatus(MouseStatus &ms) {
  // This assumes that ms.view[XY] are the pixel coordinates of the
  // mouse in the viewspace, and the mouse is in this view.
  makeCurrent();
  double depth;
  GLint viewport[4];

  // save current viewport, projection and modelview
  glGetIntegerv(GL_VIEWPORT, viewport);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  // set the transformations to the correct ones for this
  // view before calling Unproject
  _projection.Apply(drawparams.ProjectionMode());

  // get Retina display scaling and apply to ms.view[XY]
  const int retinaScale = devicePixelRatio();

  // Read depth under pointer from the z-buffer
  float winZ[PickRegionSize];
  glReadPixels(static_cast<int>(ms.viewX - HalfPickWidth) * retinaScale,
               static_cast<int>(_projection.getWindowHeight() - (ms.viewY -
                                HalfPickHeight * retinaScale)),
               static_cast<int>(PickWidth), static_cast<int>(PickHeight),
               GL_DEPTH_COMPONENT, GL_FLOAT, winZ);

  depth = winZ[0];
  for (int i = 1; i < PickRegionSize; i++)
    if (depth > winZ[i])
      depth = winZ[i];

  // do the unproject (map window coordinates into object coordinates)
  Vector3d prFront = _projection.Unproject(ms.viewX * retinaScale, ms.viewY * retinaScale, 0);
  ms.atFront = V3d(prFront.X(), prFront.Y(), prFront.Z());
  Vector3d prRear = _projection.Unproject(ms.viewX * retinaScale, ms.viewY * retinaScale, 1);
  ms.atRear = V3d(prRear.X(), prRear.Y(), prRear.Z());
  ms.atMiddle = 0.5 * (ms.atFront + ms.atRear);
  Vector3d prSelect = _projection.Unproject(ms.viewX * retinaScale, ms.viewY * retinaScale, depth);
  ms.selectPoint = V3d(prSelect.X(), prSelect.Y(), prSelect.Z());
  {
    V3d sf = ms.selectPoint - ms.atFront, rf = ms.atRear - ms.atFront;
    ms.selectDepth = (sf * rf) / (rf * rf);
  }
  ms.lbDown = _mouseStatus.lbDown;

  // put the projection, modelview and viewport back
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void GLWidget::OutputView(const std::string &fnm) const {

  std::ofstream trg(fnm.c_str());
  trg << "view: " << _id << ' ' << "dir: " << _projection.ViewNormal().X()
      << ' ' << _projection.ViewNormal().Y() << ' '
      << _projection.ViewNormal().Z() << ' '
      << "up: " << _projection.ViewUp().X() << ' ' << _projection.ViewUp().Y()
      << ' ' << _projection.ViewUp().Z() << ' '
      << "pan: " << _projection.GetPan().X() << ' ' << _projection.GetPan().Y()
      << ' ' << _projection.GetPan().Z() << ' '
      << "fov: " << _projection.GetFOV() << ' '
      // To match "scale" commmand, output 1 / scale 
      << "scale: " << 1.0f / _projection.Scale() << ' '
      << "shift: " << _projection.ZShift() << std::endl;

  trg << "box: " << _id << ' ';
  _projection.OutputBoundingBox(trg);

  trg << std::endl;
}
