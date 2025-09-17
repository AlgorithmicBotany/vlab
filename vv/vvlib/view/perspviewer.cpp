#include "perspviewer.hpp"

using namespace view;
using namespace util;

/** @brief Constructor.
    @param parent The Qt parent widget.
    @param name   The Qt widget name.
    @param f      The Qt widget flags.

    The default viewing space is the cube from (-1, -1, -1) to (1, 1, 1).
*/
PerspViewer::PerspViewer(QWidget* parent, const char* name, Qt::WindowFlags f) :
  Viewer(parent, name, f),
  min(-1.0, -1.0, -1.0),
  max(1.0, 1.0, 1.0),
  upp(1),
  rotx(),
  roty(),
  panx(),
  pany(),
  scalefactor(0.0)
{}

/** @brief Destructor. */
PerspViewer::~PerspViewer() {}

/** @brief Set the minimum coordinate for the viewing volume. */
void PerspViewer::setMin(const Point<GLfloat> p) {
  min = p;
}

/** @brief Set the maximum coordinate for the viewing volume. */
void PerspViewer::setMax(const Point<GLfloat> p) {
  max = p;
}

/** @brief Initialise the OpenGL canvas. */
void PerspViewer::initializeGL() {
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glEnable(GL_DEPTH_TEST);
}

/** @brief Set the rendering for the new window size.
    @param w The width of the window.
    @param h The height of the window.
*/
void PerspViewer::resizeGL(int w, int h) {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  const GLfloat ratio = GLfloat(w) / GLfloat(h);
  const GLfloat scale = pow(1.003f, scalefactor);
  const GLfloat size  = 0.5 * min.distance(max) * scale;
  const GLfloat n_dist = -min.z();
  const GLfloat f_dist = n_dist + 2.0 * size / scale;

  if (ratio > 1.0) {
    glFrustum(
      -size * ratio, size * ratio,
      -size, size,
      n_dist, f_dist
    );
    upp = 2.0 * size / GLfloat(h);
  }
  else {
    glFrustum(
      -size, size,
      -size / ratio, size / ratio,
      n_dist, f_dist
    );
    upp = 2.0 * size / GLfloat(w);
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, w, h);

  glTranslatef(panx, pany, -0.5 * (n_dist + f_dist));
  glRotatef(rotx, 1.0, 0.0, 0.0);
  glRotatef(roty, 0.0, 1.0, 0.0);
}

/** @brief Render an empty scene. */
void PerspViewer::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/** @brief Increase the scaling corfficient of the scene.
    @brief dy The amount to increase by, in screen space units.
*/
void PerspViewer::zoom(int dy) {
  scalefactor += GLfloat(dy);
  update();
}

/** @brief Rotate the scene on the x and y-axes.
    @param dx the amount to rotate about y, in screen space units.
    @param dy the amount to rotate about x, in screen space units.
*/
void PerspViewer::rotate(int dx, int dy) {
  rotx += GLfloat(dy);
  if (rotx >= 360.0) rotx -= 360.0;
  else if (rotx < 0.0) rotx += 360.0;

  roty += GLfloat(dx);
  if (roty >= 360.0) roty -= 360.0;
  else if (roty < 0.0) roty += 360.0;

  update();
}

/** @brief Trtanslate the viewing window.
    @brief dx The amount to move in the x direction, in screen space units.
    @brief dy The amount to move in the y direction, in screen space units.
*/
void PerspViewer::pan(int dx, int dy) {
  panx += 2.0 * upp * GLfloat(dx);
  pany -= 2.0 * upp * GLfloat(dy);

  update();
}
