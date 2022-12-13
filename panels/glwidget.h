
#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QPushButton>
#include "panel.h"
class Panel;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT
public:
  GLWidget(Panel *mw, const QColor &background);
  ~GLWidget();
  void repaint() {
    std::cerr << "Repainting" << std::endl;
    makeCurrent();
    update();
  }

  void renderText(double x, double y, double z, const QString &str,
                  const QColor color, const QFont &font);

public slots:

private slots:

protected:
  void resizeGL(int w, int h) override;
  void paintGL() override;
  void initializeGL() override;
  void mousePressEvent(QMouseEvent *) override ;
  void mouseDoubleClickEvent(QMouseEvent *) override ;
  void mouseMoveEvent(QMouseEvent *) override ;
  void mouseReleaseEvent(QMouseEvent *) override ;
  void keyPressEvent(QKeyEvent *) override ;

private:
  Panel *m_mainWindow;
  QColor m_background;
  bool _init;

  // following is used to render text
  inline GLint project(GLdouble objx, GLdouble objy, GLdouble objz,
                       const GLdouble model[16], const GLdouble proj[16],
                       const GLint viewport[4], GLdouble *winx, GLdouble *winy,
                       GLdouble *winz);
  inline void transformPoint(GLdouble out[4], const GLdouble m[16],
                             const GLdouble in[4]);
};

#endif
