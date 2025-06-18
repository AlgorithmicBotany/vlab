#ifndef __VIEW__PERSPVIEWER_H__
#define __VIEW__PERSPVIEWER_H__

#include "viewer.hpp"
#include "../util/point.hpp"

namespace view {
  /** @brief A Viewer subclass with a perspective view. */
  class PerspViewer : public Viewer {
  public:
    PerspViewer(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags = 0);
    virtual ~PerspViewer();

    void setMin(const util::Point<GLfloat> p);
    void setMax(const util::Point<GLfloat> p);

  protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();

    virtual void zoom(int dy);
    virtual void rotate(int dx, int dy);
    virtual void pan(int dx, int dy);

    util::Point<GLfloat> min;
    util::Point<GLfloat> max;
    GLfloat              upp;
    GLfloat              rotx;
    GLfloat              roty;
    GLfloat              panx;
    GLfloat              pany;
    GLfloat              scalefactor;
  };
}

#endif
