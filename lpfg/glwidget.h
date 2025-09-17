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



#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTime>
#include <QPushButton>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMessageBox>
#include "projection.h"
#include "rect.h"
#include "viewtask.h"

// we need to include lintrfc for MouseStatus,
// and lparams for lintrfc
#include "include/lparams.h"
#include "include/lintrfc.h"

class View;
class LPFG;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT
public:
  GLWidget(View *mw, bool button, const QColor &background, const int id,
           const QOpenGLWidget::UpdateBehavior behaviour);
  ~GLWidget();

  void setBackgroundTransparent(bool t) { _transparent = t; }
  void StartRotating(int x, int y);
  void StartPanning(int x, int y);
  void StartZooming(int x, int y);

  void ResetView();
  void ResetOpenGL();
  void PanBy(int, int);
  void ZoomBy(int);
  void FrustumBy(int);
  void RollBy(int);
  void RotateBy(int, int);
  void DrawStep();

  void Repaint();
  void Resize(int, int);
  void UserContextMenu(int, int);
  void ContextMenu(int, int);
  void Command(int);

  void ShowAxis(bool);
  void ResetRotation();
  void ResetZoom();
  void ResetPan();
  void ResetRoll();

  bool IsVisible() const;
  void setCreated(bool b) {_created = b;}
  bool isCreated() {return _created;}
  void Show();
  void Showing(bool);
  void Show(const WindowParams &);
  void ShowMaximized();
  Projection *GetProjection() { return &_projection; }
  void resetProjection(int width, int height);
  int getId() { return _id; }
  void HCenter();
  void Scale();
  void _DrawAxis();
  void Clear();
  void DrawParamsChanged();

  void InsertX(int, int);
  void InsertPosX(int, int);
  void StartFrustum(int, int);
  void StartRoll(int, int);
  void StartIdle();

  void MouseMove(int, int);
  void FillMouseStatus(MouseStatus &ms);

  CameraPosition GetCameraPosition(void) const;
  void renderText(double x, double y, double z, const QString &str,
                  const QColor color, const QFont &font);
  void Destroy();

  void Resize(int, int, int, int);
  void GetRelSize(const Rect &, float *) const;

  float vvXmin() const;
  float vvYmin() const;
  float vvZmin() const;
  float vvXmax() const;
  float vvYmax() const;
  float vvZmax() const;
  float vvScale() const;

  void setAlphaFormat(bool alpha) {
    if (alpha)
      setFormat(_format);
    else {
      QSurfaceFormat format = QSurfaceFormat::defaultFormat();
      setFormat(format);
    }
  }

  QImage grabFrameBuffer(bool);

signals:
  void mouseClicked(const QPoint &);
  void mouseMoved(const QPoint &);
  void mouseReleased(const QPoint &);

public slots:

  void save();
  void updateFilename();
  bool OverwriteDialogBox(const char *sfilename);

  void saveAs();
  void Animate();

  void OutputRGB();
  void OutputPovray();
  void OutputPovray(int frameNumber) const; // Used when recording
  void OutputPovray(const char *FolderName) const;

  void OutputRayshade();
  void OutputView();
  void OutputObj();
  void OutputPostscript();
  void OutputView(const std::string &) const;
  void Step();
  void Run();
  void Pause();
  bool getAlphaChannel() { return _alphaChannel; }
  void *getPQ() { return _pQ; }

  void keyPressEvent(QKeyEvent *) override;
  void keyReleaseEvent(QKeyEvent *) override;

private slots:
  void handleButtonPress();

protected:
  void resizeGL(int w, int h) override;
  void paintGL() override;
  void initializeGL() override;
  // Not used: void setupViewport(int width, int height);

  void mouseMoveEvent(QMouseEvent *pEv) override;

  void mousePressEvent(QMouseEvent *pEv) override;
  void mouseReleaseEvent(QMouseEvent *pEv) override;

  void LBDown(QMouseEvent *pEv);
  void MBDown(QMouseEvent *pEv);
  void RBDown(QMouseEvent *pEv);

private:
//  QImage qt_gl_read_framebuffer(const QSize &size, bool alpha_format,
//                                bool include_alpha);

  bool _transparent;
  bool _created;
  ViewTask *_pTask;
  ViewIdleTask _IdleTask;
  ViewRotateTask _RotateTask;
  ViewZoomTask _ZoomTask;
  ViewFrustumTask _FrustumTask;
  ViewRollTask _RollTask;
  ViewPanTask _PanTask;
  bool _hasMoved;
  bool clear_glwidget;
  bool _initiated;

  int _keyPressed;

  View *m_mainWindow;
  View *_view;
  QPoint _mouse_init_position;
  // this variable is not used: bool _initializeGL = false;
  int _id = 0;
  unsigned int _flist;
  unsigned int _oflist;

  void *_pQ;
  LPFG *_pLpfg;
  bool _alphaChannel;

  QMenu *testMenu;
  Projection _projection;
  MouseStatus _mouseStatus;

  QSurfaceFormat _format;

  static int _counter;

  // MC - July 2016 - added private members for GLSL shaders and methods for
  // generating shadow map
  bool shadowMapFBO_supported;
  GLuint shadowMapTexture, shadowMapResolution, shadowMapFBO, defaultFBO;
  GLuint lightMatrixLocation, shadowMapLocation, shadowColorLocation;
  GLuint textureLocation, textureShadowLocation, numLightsLocation;

  void initShadowMap(void);
  void resizeShadowMap(void);
  void freeShadowMap(void);
  void beginShadowMap(void);
  void endShadowMap(void);
  void beginMainShader(void);
  void endMainShader(void);
  void _DoInsertX(int, int);
  void _DoInsertPosX(int, int);

  GLuint mainShaderProgramID, shadowShaderProgramID;
  GLuint initShaders(const char *vertex_file_path,
                     const char *fragment_file_path);
  OpenGLMatrix
      lightMatrix; // the projection-model-view matrix for rendering from the
                   // light source following is used to render text
};

#endif
