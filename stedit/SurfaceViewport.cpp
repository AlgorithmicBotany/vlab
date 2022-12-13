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

#include "SurfaceViewport.h"

SurfaceViewport::SurfaceViewport(Surface *surface, Trackball *trackball,
                                 QWidget *parent, Colour borderColour)
    : QGLWidget(parent) {

  // Set the default camera positioning
  camPos = Point(0, 0, 5);
  camLook = Point();
  camUp = Vector3(0, 1, 0);

  ratio = 1.0;
  editorWidth = 125;
  editorHeight = 125;
  bgColour = Colour(parent->palette().color(
      QPalette::Window)); // Match the window's background colour
  subdivisionSamples = 3;
  currentPatch = -1;

  this->surface = surface;
  this->borderColour = borderColour;

  this->trackball = trackball;
}

SurfaceViewport::~SurfaceViewport() {}

QSize SurfaceViewport::minimumSizeHint() const { return QSize(125, 125); }

QSize SurfaceViewport::sizeHint() const { return QSize(125, 125); }

void SurfaceViewport::paintGL() {
  QGLWidget::makeCurrent();
  glMatrixMode(GL_MODELVIEW);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  gluLookAt(camPos.X(), camPos.Y(), camPos.Z(), camLook.X(), camLook.Y(), camLook.Z(),
            camUp.X(), camUp.Y(), camUp.Z());
  glMultMatrixf(trackball->getMatrix()); // Rotate the surface by the
                                         // trackball's rotation matrix
  glTranslated(
      -centerPoint.X(), -centerPoint.Y(),
      -centerPoint.Z()); // Translate the center of the surface to the origin

  glColor3f(0.5, 0.5, 0.5);
  surface->drawPatch(currentPatch); // Draws one patch out of the surface

  glDisable(GL_LIGHTING); // No lighting on the lines
  glPushMatrix();
  glLoadIdentity();
  // Translate slightly towards the camera to ensure that the lines are shown in
  // front of the surface, not overlapping with it
  Vector3 trans = (camPos - camLook).normalize() * 0.001;
  glTranslatef(trans.X(), trans.Y(), trans.Z());

  gluLookAt(camPos.X(), camPos.Y(), camPos.Z(), camLook.X(), camLook.Y(), camLook.Z(),
            camUp.X(), camUp.Y(), camUp.Z());
  glMultMatrixf(trackball->getMatrix()); // Rotate the surface by the
                                         // trackball's rotation matrix
  glTranslated(
      -centerPoint.X(), -centerPoint.Y(),
      -centerPoint.Z()); // Translate the center of the surface to the origin

  surface->drawPatchAdjacencyEdges(currentPatch);

  glPopMatrix();

  glPushMatrix();
  glLoadIdentity();
  drawBorder(); // Draw the coloured border around the edge of the viewport
  glPopMatrix();
  resizeGL(editorWidth,
           editorHeight); // Restore the viewport to its proper projection

  glEnable(GL_LIGHTING); // Reenable stuff

  glFlush();
}

// Draws a coloured border around the edge of the viewport
void SurfaceViewport::drawBorder() {
  QGLWidget::makeCurrent();

  glDisable(GL_DEPTH_TEST); // Draw this in front of everything
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, editorWidth, editorHeight);
  glOrtho(-1, 1, -1, 1, -1, 1); // Ortho projectionso that the border can be
                                // drawn in 2D around the edge

  glMatrixMode(GL_MODELVIEW);

  glLineWidth(4);
  glColor3f(borderColour.r, borderColour.g, borderColour.b);
  glBegin(GL_LINE_LOOP); // Draw the border around the edge of the viewport
  glVertex2f(-1, -1);
  glVertex2f(1, -1);
  glVertex2f(1, 1);
  glVertex2f(-1, 1);
  glEnd();
  glEnable(GL_DEPTH_TEST);
}

void SurfaceViewport::initializeGL() {
  QGLWidget::makeCurrent();

  glClearColor(bgColour.r, bgColour.g, bgColour.b,
               0); // Clear to the background colour

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // GL the functable
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_ALPHA);
  glDisable(GL_BLEND);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_SMOOTH);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  trackball->reset();
}

void SurfaceViewport::resizeGL(int w, int h) {
  QGLWidget::makeCurrent();
  editorWidth = w;
  editorHeight = h;
  ratio = (double)editorWidth / (double)editorHeight;

  glViewport(0, 0, editorWidth, editorHeight);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(50.0f, ratio, 0.01f, 10000.0f);
  glMatrixMode(GL_MODELVIEW);
}

void SurfaceViewport::mousePressEvent(QMouseEvent *event) {
  QGLWidget::makeCurrent();
  updateMousePosition(event->x(), event->y());
  if (event->button() == Qt::LeftButton) {
    trackball->press(
        getScreenPosition(mouseX, mouseY)); // Press the trackball at this point
  }
  updateGL();
}

void SurfaceViewport::mouseReleaseEvent(QMouseEvent *event) {
  QGLWidget::makeCurrent();
  if (event->button() == Qt::LeftButton) {
    trackball->release(); // Release the trackball on left mouse button release
    updateGL();
  }
}

void SurfaceViewport::mouseMoveEvent(QMouseEvent *event) {
  QGLWidget::makeCurrent();

  if (event->buttons() &
      Qt::MidButton) {      // Pan the camera on right mouse button drag
    int oldMouseX = mouseX; // Remember where the mouse was
    int oldMouseY = mouseY;
    updateMousePosition(event->x(), event->y());
    int diffX = oldMouseX - mouseX; // Find the difference in the mouse position
    int diffY = mouseY - oldMouseY;
    camPos = camPos + (Point(diffX, diffY) * camPos.Z()) /
                          (double)min(editorWidth,
                                      editorHeight); // Move the camera position
                                                     // and look point to pan it
    camLook = camLook + (Point(diffX, diffY) * camPos.Z()) /
                            (double)min(editorWidth, editorHeight);
    updateGL();
  } else if (event->buttons() & Qt::LeftButton) {
    updateMousePosition(event->x(), event->y());
    trackball->move(getScreenPosition(
        mouseX, mouseY)); // Rotate the trackball on left mouse button drag
    emit(trackballMoved());
  }
}

void SurfaceViewport::wheelEvent(QWheelEvent *event) {
  QGLWidget::makeCurrent();
  if (event->orientation() ==
      Qt::Vertical) { // Zoom in and out with the mouse wheel
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    camPos.setZ(camPos.Z() - ((camPos.Z()) / 10.0) * numSteps);
    updateGL();
  }
}

// Updates the mouse position in screen coordinates as well as recalculating the
// relative coordinates
void SurfaceViewport::updateMousePosition(int x, int y) {
  mouseX = x;
  mouseY = y;
}

// Gets the mouse position with respect to the screen coordinates of this widget
Point SurfaceViewport::getScreenPosition(int x, int y) {
  Point result;
  result.setX(((double)x / (double)editorWidth * 2.0 - 1.0));
  result.setY( -((double)y / (double)editorHeight * 2.0 - 1.0));
  return result;
}

// Centers the camera and zooms to fit the surface's current position
void SurfaceViewport::centerCamera() {
  double xmax = -10001;
  double xmin = 10001;
  double ymax = -10001;
  double ymin = 10001;
  double zmax = -10001;
  double zmin = 10001;

  Patch selectedPatch;
  if (currentPatch >= 0 && currentPatch < surface->numPatches()) {
    selectedPatch = *surface->getPatch(currentPatch);
  } else
    return;

  for (int i = 0; i < surface->numRows();
       i++) { // Look at the control points to find the current maximum
              // coordinates
    for (int j = 0; j < surface->numColumns(); j++) {
      if (selectedPatch.getPoint(i, j)->X() > xmax)
        xmax = selectedPatch.getPoint(i, j)->X();
      if (selectedPatch.getPoint(i, j)->X() < xmin)
        xmin = selectedPatch.getPoint(i, j)->X();
      if (selectedPatch.getPoint(i, j)->Y() > ymax)
        ymax = selectedPatch.getPoint(i, j)->Y();
      if (selectedPatch.getPoint(i, j)->Y() < ymin)
        ymin = selectedPatch.getPoint(i, j)->Y();
      if (selectedPatch.getPoint(i, j)->Z() > zmax)
        zmax = selectedPatch.getPoint(i, j)->Z();
      if (selectedPatch.getPoint(i, j)->Z() < zmin)
        zmin = selectedPatch.getPoint(i, j)->Z();
    }
  }
  centerPoint = selectedPatch.getCenter();
  camPos.setZ(
      max(max(max(max(max(fabs(xmin), fabs(xmax)), fabs(ymin)), fabs(ymax)),
              fabs(zmin)),
          fabs(zmax)) *
      2);
  camPos.setX(0); // Set the camera position back to the z axis
  camPos.setY(0);
  camLook = Point(); // Look at the origin
  updateGL();
}

// Resets the viewer to its initial state
void SurfaceViewport::resetView() {
  surface->subdivide(subdivisionSamples);
  trackball->reset(); // Reset the trackball's rotation matrix to identity
  centerCamera();
  updateGL();
}

void SurfaceViewport::setCurrentPatch(int index) {
  currentPatch = index - 1;
  centerCamera();
  updateGL();
}

// Re-subdivides and displays the patch
void SurfaceViewport::update() {
  QGLWidget::makeCurrent();
  surface->subdivide(subdivisionSamples);
  updateGL();
}
