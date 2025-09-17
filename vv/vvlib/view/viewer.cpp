#include <QCursor>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include "viewer.hpp"

using namespace view;

QGLWidget* Viewer::pShareWidget = 0;

/** @brief Constructor
    @param parent The parent Qt widget.
    @param name   The Qt widget name.
    @param f      The Qt widget flags.
*/
Viewer::Viewer(QWidget* parent, const char* name, Qt::WindowFlags f) :
  QGLWidget(parent, pShareWidget, f),
  pContextMenu(0),
  prevMouseX(),
  prevMouseY()
{
  Q_ASSERT(name == NULL); // why does this parameter even exist?
  if (!pShareWidget) pShareWidget = this;
}

/** @brief Destructor. */
Viewer::~Viewer() {}

/** @brief Force a repaint. */
void Viewer::update() {
  makeCurrent();
  resizeGL(width(), height());
  updateGL();
}

/** @brief Handle mouse clicks. */
void Viewer::mousePressEvent(QMouseEvent* pEv) {
  switch (pEv->button()) {
    case Qt::LeftButton:
      switch (pEv->modifiers()) {
        case Qt::ShiftModifier:
          editMode = PAN;
          break;
        case Qt::ControlModifier:
          editMode = ZOOM;
          break;
        default: 
          editMode = ROTATE;
          break;
      }
      break;
    case Qt::MidButton:
      editMode = ZOOM;
      break;
    case Qt::RightButton:
      editMode = NONE;
      if (pContextMenu) 
        pContextMenu->popup(QCursor::pos());
      break;
    default: break;
  }
  prevMouseX = pEv->x();
  prevMouseY = pEv->y();
}

/** @brief Reset the mode one button release. */
void Viewer::mouseReleaseEvent(QMouseEvent* pEv) {
  editMode = NONE;
  pEv->accept();
}

/** @brief Double mouse clicks do nothing. */
void Viewer::mouseDoubleClickEvent(QMouseEvent* pEv) {
  pEv->accept();
}

/** @brief Handle mouse motions when a button is down. */
void Viewer::mouseMoveEvent(QMouseEvent* pEv) {
  switch (editMode) {
  case ROTATE:
    rotate(pEv->x() - prevMouseX, pEv->y() - prevMouseY);
    break;
  case ZOOM:
    zoom(pEv->y() - prevMouseY);
    break;
  case PAN:
    pan(pEv->x() - prevMouseX, pEv->y() - prevMouseY);
    break;
  case NONE:
  default: break;
  }

  prevMouseX = pEv->x();
  prevMouseY = pEv->y();
}

/** @brief Do nothing with keyboard presses. */
void Viewer::keyPressEvent(QKeyEvent* pEv) {
  pEv->ignore();
}

/** @brief Do nothing with keyboard releases. */
void Viewer::keyReleaseEvent(QKeyEvent* pEv) {
  pEv->ignore();
}
