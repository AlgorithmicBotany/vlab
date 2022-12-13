#ifndef GLDISPLAY_H
#define GLDISPLAY_H

#define GL_SILENCE_DEPRECATION

#include <QGLWidget>
#include <QMenu>

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

class GLDisplay : public QGLWidget {
  Q_OBJECT

public:
  GLDisplay(QWidget *parent);
  ~GLDisplay();

public slots:
  void updateGLWindow(void);
  void updateGLVisualization(void);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void showBoundingSphere(void);
  void showBoundingBox(void);
  void showWireFrame(void);
  void setBackgroundColour(void);
  void outputVertices(void);
  void outputImage(void);

protected:
  void initializeGL(void);
  void paintGL(void);
  void resizeGL(int w, int h);

private:
  float xpos, ypos, zpos;
  float xrot, yrot, zrot;
  int mouse_x, mouse_y;
  GLUquadricObj *disk;
  int showbox, showsphere, showwireframe, quasimc_update;
  QAction *showsphereAct, *showboxAct, *showwireframeAct;
  float max_intensity;
  QMenu *popup_menu;
};

#endif
