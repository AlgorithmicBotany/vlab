#ifndef __VIEW__VIEWER_H__
#define __VIEW__VIEWER_H__

#include <string>

#include <QGLWidget>
class QMenu;
class QMouseEvent;
class QKeyEvent;

namespace view {
  /** @brief An encapsulation for a GL canvas with windowing facilities. */
  class Viewer : public QGLWidget {
  public:
    Viewer(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags f = 0);
    virtual ~Viewer();

    void update();

  protected:
    enum {
      NONE,
      ROTATE,
      ZOOM,
      PAN
    } editMode;

    virtual void mousePressEvent(QMouseEvent* pEv);
    virtual void mouseReleaseEvent(QMouseEvent* pEv);
    virtual void mouseDoubleClickEvent(QMouseEvent* pEv);
    virtual void mouseMoveEvent(QMouseEvent* pEv);

    virtual void keyPressEvent(QKeyEvent* pEv);
    virtual void keyReleaseEvent(QKeyEvent* pEv);

    virtual void initializeGL() = 0;
    virtual void resizeGL(int w, int h) = 0;
    virtual void paintGL() = 0;

    virtual void zoom(int dy) = 0;
    virtual void rotate(int dx, int dy) = 0;
    virtual void pan(int dx, int dy) = 0;

    QMenu* pContextMenu;

  private:
    static QGLWidget* pShareWidget;

    int prevMouseX;
    int prevMouseY;
  };
}

#endif
