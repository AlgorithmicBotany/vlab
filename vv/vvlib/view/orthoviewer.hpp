#ifndef __VIEW__ORTHOVIEWER_H__
#define __VIEW__ORTHOVIEWER_H__

#include "viewer.hpp"
#include "../util/point.hpp"

namespace view {
  /** @brief A Viewer subclass with an orthogonal view. */
  class OrthoViewer : public Viewer {
  public:
    OrthoViewer(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags = 0);
    virtual ~OrthoViewer();

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
