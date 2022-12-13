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


// to avoid GLu Warning due to deprecated function on MacOs
#define GL_SILENCE_DEPRECATION

#include "BezierEditor.h"
#include <QDebug>
BezierEditor::BezierEditor(QWidget *parent) : QGLWidget(parent) {
  //setFocusPolicy(Qt::StrongFocus);

  // Set the default camera positioning
  camPos = Point(0, 0, 3);
  camLook = Point();
  camUp = Vector3(0, 1, 0);

  ratio = 1.0;
  xrot = 0;
  yrot = 0;
  editorWidth = 600;
  editorHeight = 600;
  // Match the
  // window's background colour
  bgColour = Colour(0, 0, 0);
  defBgColour = bgColour;
  pointColour = Colour(1, 0, 0);
  selectedPointColour = Colour(0, 0, 1);
  lineColour = Colour(1, 0, 0);
  wireframeColour = Colour(0.4, 0.4, 0.6);
  vectorColour = Colour(0.6, 0, 0.8);
  patchColour = Colour(1, 1, 1);
  selectedPatchColour = Colour(1, 0, 0);
  pointSize = DEFAULT_POINT_SIZE;
  contactPointSize = DEFAULT_CONTACT_POINT_SIZE;
  lineWidth = DEFAULT_LINE_WIDTH;
  wireframeWidth = DEFAULT_WIREFRAME_WIDTH;
  subdivisionSamples = DEFAULT_SUBDIVISION_SAMPLES;
  textureRotation = 270;
  textureFlippedH = false;
  textureFlippedV = false;

  previousTranslation = Vector3();
  previousRotationAxis = Vector3();
  previousRotationAngle = 0;
  previousScale = Vector3(1, 1, 1);

  showPoints = true;
  showLines = true;
  showSurface = true;
  showVectors = true;
  wireframe = true;
  symmetrical14Y = false;
  symmetrical14X = false;
  symmetrical113X = false;
  symmetrical113Y = false;
  centerContact = false;
  unsavedChanges = false;
  hasCurrentTexture = false;
  enforceAdjacency = true;
  rotationLocked = false;
  dragPoint = false;
  dragContact = false;
  contactSelected = false;
  extendedFormat = false;
  linearInterpolation = true;

}

BezierEditor::~BezierEditor() { glDeleteTextures(1, &tex); }

QSize BezierEditor::minimumSizeHint() const { return QSize(100, 100); }

QSize BezierEditor::sizeHint() const { return QSize(10000, 10000); }

void BezierEditor::forceExtendedFormat() {
  extendedFormat = true;
  surface = Surface(extendedFormat);
}

// Provides an initial surface filename and loads it. This comes from the
// command line when starting the program
bool BezierEditor::initSurfaceFilename(string filename) {
  bool res = load(filename);
  surfaceFilename = filename;
  return res;
}

// Provides an initial texture filename and loads it. This comes from the
// command line when starting the program
bool BezierEditor::initTextureFilename(string filename) {
  bool success = loadImage(filename.c_str());
  textureName = filename;
  return success;
}

// Two pass rendering of the surface is used to overcome a GL bug related to the
// rendering order of polygons when alpha blending is used. Without this,
// polygons can be hidden behind tranparent polygons, drawing the background
// colour instead of the texture.
void BezierEditor::paintGL() {
  const int retinaScale = devicePixelRatio();

  QGLWidget::makeCurrent(); // This is the real GL!
  glMatrixMode(GL_MODELVIEW);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  gluLookAt(camPos.X(), camPos.Y(), camPos.Z(), camLook.X(), camLook.Y(), camLook.Z(),
            camUp.X(), camUp.Y(), camUp.Z());
  if (parallelProjection) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (editorWidth > editorHeight)
      glOrtho(-camPos.Z() * ratio, camPos.Z() * ratio, -camPos.Z(), camPos.Z(), -10000,
              10000);
    else
      glOrtho(-camPos.Z(), camPos.Z(), -camPos.Z() / ratio, camPos.Z() / ratio, -10000,
              10000);
    glMatrixMode(GL_MODELVIEW);
  }
  // Rotate the surface by the
  // trackball's rotation matrix
  glRotated(xrot, 1, 0, 0);
  glRotated(yrot, 0, 1, 0);
  glTranslated(
      -centerPoint.X(), -centerPoint.Y(),
      -centerPoint.Z()); // Translate the center of the surface to the origin

  if (showSurface) { // Draw the surface (First pass)
    glColor3f(1, 1, 1);
    if (wireframe)
      surface.draw(
          wireframeColour,
          wireframeWidth); // Draw the surface as a wireframe if applicable
    else {
      surface.draw(patchColour, selectedPatchColour);
      glAlphaFunc(
          GL_LESS,
          1.0); // Change the alpha function in preparation for the second pass
    }
  }

  glDisable(GL_ALPHA_TEST); // Disable the stuff we don't want to use when
                            // drawing lines and points
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glPushMatrix();
  glLoadIdentity();
  // Translate slightly towards the camera to ensure that the lines are shown in
  // front of the surface, not overlapping with it
  Vector3 trans = (camPos - camLook).normalize() * 0.001;
  glTranslatef(trans.X(), trans.Y(), trans.Z());

  gluLookAt(camPos.X(), camPos.Y(), camPos.Z(), camLook.X(), camLook.Y(), camLook.Z(),
            camUp.X(), camUp.Y(), camUp.Z());
  if (parallelProjection) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (editorWidth > editorHeight)
      glOrtho(-camPos.Z() * ratio, camPos.Z() * ratio, -camPos.Z(), camPos.Z(), -10000,
              10000);
    else
      glOrtho(-camPos.Z(), camPos.Z(), -camPos.Z() / ratio, camPos.Z() / ratio, -10000,
              10000);
    glMatrixMode(GL_MODELVIEW);
  }
  // Rotate the surface by the
  // trackball's rotation matrix
  glRotated(xrot, 1, 0, 0);
  glRotated(yrot, 0, 1, 0);
  glTranslated(
      -centerPoint.X(), -centerPoint.Y(),
      -centerPoint.Z()); // Translate the center of the surface to the origin

  if (showLines) { // Draw the grid lines
    surface.drawLines(lineWidth, lineColour);
  }

  if (showVectors &&
      surface.numPatches() > 0) { // Draw the heading and up vectors
    surface.drawVectors(lineWidth, vectorColour);
  }

  if (showPoints && surface.numPatches() >
                        0) { // Draw the points second so that they go on top
    surface.drawPoints(pointSize*retinaScale, contactPointSize*retinaScale, pointColour,
                       selectedPointColour, vectorColour,
                       symmetrical14Y || symmetrical14X,
                       symmetrical113X || symmetrical113Y);
  }
  glPopMatrix();
  glEnable(GL_LIGHTING); // Reenable stuff
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);

  if (showSurface && !wireframe) { // Only do the second pass if the surface is
                                   // not in wireframe mode
    glColor3f(1, 1, 1);
    surface.draw(patchColour, selectedPatchColour);
    glAlphaFunc(GL_EQUAL,
                1.0); // Reset the alpha function for the next paint event
  }

  if (showPoints && surface.numPatches() > 0) {
    numberPoints();
  }
  drawAxes();

  glFlush();
}

void BezierEditor::numberPoints() {
  const int retinaScale = devicePixelRatio();

  double model[16], proj[16];
  int view[4];
  glGetDoublev(GL_MODELVIEW_MATRIX, model);
  glGetDoublev(GL_PROJECTION_MATRIX, proj);
  glGetIntegerv(GL_VIEWPORT, view);

  glDisable(GL_ALPHA_TEST); // Disable the stuff we don't want to use when
                            // drawing lines and points
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  for (int k = 0; k < surface.numPatches(); k++) { // Number the points
    Patch *patch = surface.getPatch(k);
    if (!patch->isVisible())
      continue;
    for (int i = 0; i < surface.numRows(); i++) {
      for (int j = 0; j < surface.numColumns(); j++) {
        if (!contactSelected &&
            (surface.getPatch(surface.getSelectedPatch()) == patch) &&
            ((patch->getPoint(i, j) ==
              patch->getPoint(surface.selectedPointI,
                              surface.selectedPointJ)) ||
             ((symmetrical14Y || symmetrical14X) &&
              patch->getPoint(i, j) ==
                  patch->getPoint(surface.numRows() - surface.selectedPointI -
                                      1,
                                  surface.selectedPointJ)) ||
             ((symmetrical113X || symmetrical113Y) &&
              patch->getPoint(i, j) ==
                  patch->getPoint(surface.selectedPointI,
                                  surface.numColumns() -
                                      surface.selectedPointJ - 1)) ||
             ((symmetrical14Y || symmetrical14X) &&
              (symmetrical113X || symmetrical113Y) &&
              patch->getPoint(i, j) ==
                  patch->getPoint(
                      surface.numRows() - surface.selectedPointI - 1,
                      surface.numColumns() - surface.selectedPointJ - 1)))){
          glColor3f(selectedPointColour.r, selectedPointColour.g, selectedPointColour.b);
	}
        else{
          glColor3f(pointColour.r, pointColour.g, pointColour.b);
	}
        Point *p = patch->getPoint(i, j);
	double x,y,z;
        gluProject(p->X(), p->Y(), p->Z(), model, proj, view, &x,&y,&z);

	Point winP(x,y,x);
	// [PASCAL] I don't know why there is the following test
        //if (winP.Z() < 1) {
	renderText(winP.X()/retinaScale - 3, (view[3] - winP.Y())/retinaScale + 3,
                     "+"); // Mark the point's actual location with a plus if
                           // you can't actually see the point in front
	renderText((winP.X() + 8)/retinaScale, (view[3] - winP.Y() - 8)/retinaScale,
                     QString::number(i + j * surface.numRows() + 1));
	  //}
      }
    }
  }
  glEnable(GL_LIGHTING); // Reenable stuff
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_ALPHA_TEST);
}

void BezierEditor::drawAxes() {
  glDisable(GL_ALPHA_TEST); // Disable the stuff we don't want to use when
                            // drawing lines and points
  glDisable(GL_LIGHTING);
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glLineWidth(2.0);

  glPushMatrix();
  glLoadIdentity();
  gluLookAt(0, 0, 1, 0, 0, 0, 0, 1, 0);
  if (parallelProjection) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5 * ratio, 0.5 * ratio, -0.5, 0.5, -10000, 10000);
    glMatrixMode(GL_MODELVIEW);
  }
  glTranslatef(-0.4 * ratio, -0.4, 0);
  // Rotate the surface by the
  // trackball's rotation matrix
  glRotated(xrot, 1, 0, 0);
  glRotated(yrot, 0, 1, 0);
  glBegin(GL_LINES);
  glColor3f(0, 0, 1);  // Blue
  glVertex3f(0, 0, 0); // X axis
  glVertex3f(0.05, 0, 0);
  glEnd();
  renderText(0.06, 0, 0, "X");
  glBegin(GL_LINES);
  glColor3f(0, 1, 0);  // Green
  glVertex3f(0, 0, 0); // Y axis
  glVertex3f(0, 0.05, 0);
  glEnd();
  renderText(0, 0.06, 0, "Y");
  glBegin(GL_LINES);
  glColor3f(1, 0, 0);  // Red
  glVertex3f(0, 0, 0); // Z axis
  glVertex3f(0, 0, 0.05);
  glEnd();
  renderText(0, 0, 0.06, "Z");
  glPopMatrix();

  glEnable(GL_LIGHTING); // Reenable stuff
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_ALPHA_TEST);
}

void BezierEditor::initializeGL() {
  QGLWidget::makeCurrent();

  glClearColor(bgColour.r, bgColour.g, bgColour.b,
               0); // Clear to the background colour

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &tex);
  loadImage(textureName.c_str());

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // GL the functable
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  xrot = 0;
  yrot = 0;
}

// Currently only used in reset...
void BezierEditor::init() {
  if (surfaceFilename.empty()) {
    surface =
        Surface(extendedFormat); // Make a default surface if there is none
    emit(newSurface());
  }
  surface.subdivide(subdivisionSamples); // Subdivide the surface appropreatly
}

// Return the editor to its initial state
void BezierEditor::reset() {
  init();

  undoStack.clear();
  redoStack.clear();
  emit(canUndo(false));
  emit(canRedo(false));
  emit(canRevert(false));

  centerPoint = Point(); // Reset the center to the origin
  camPos.setZ(5); // Reset the zoom to an appropriate level for the default surface
  updateGL();
}

void BezierEditor::resizeGL(int w, int h) {
  QGLWidget::makeCurrent();
  editorWidth = w;
  editorHeight = h;
  ratio = (double)editorWidth / (double)editorHeight;
#ifndef __APPLE__
  glViewport(0, 0, editorWidth, editorHeight);
#endif
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (parallelProjection) {
    if (editorWidth > editorHeight)
      glOrtho(-camPos.Z() * ratio, camPos.Z() * ratio, -camPos.Z(), camPos.Z(), -10000,
              10000);
    else
      glOrtho(-camPos.Z(), camPos.Z(), -camPos.Z() / ratio, camPos.Z() / ratio, -10000,
              10000);
  } else
    gluPerspective(50.0f, ratio, 0.01f, 10000.0f);
  glMatrixMode(GL_MODELVIEW);
}

void BezierEditor::mousePressEvent(QMouseEvent *event) {
#ifndef __APPLE__
  QGLWidget::makeCurrent();
#endif
  updateMousePosition(event->x(), event->y());
  // Allow point selection if ctrl is held, or if rotation is locked
  if (event->buttons() & Qt::LeftButton &&
      (event->modifiers() & Qt::ControlModifier || rotationLocked)) {
    if (surface.getSelectedPoint() != 0) {
      saveUndoState(); // Save undo data before moving the point so it can be
                       // moved back
      // [PASCAL] don't update the view port it will call a wrong paintgl in
      // TextureEditor
#ifndef __APPLE__
      glViewport(0, 0, editorWidth, editorHeight);
#endif
      if (parallelProjection) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (editorWidth > editorHeight)
          glOrtho(-camPos.Z() * ratio, camPos.Z() * ratio, -camPos.Z(), camPos.Z(),
                  -10000, 10000);
        else
          glOrtho(-camPos.Z(), camPos.Z(), -camPos.Z() / ratio, camPos.Z() / ratio,
                  -10000, 10000);
        glMatrixMode(GL_MODELVIEW);
      }

      glGetDoublev(GL_MODELVIEW_MATRIX, model); // Get the GL modelview matrix
      glGetDoublev(GL_PROJECTION_MATRIX, proj); // Get the GL projection matrix
      glGetIntegerv(GL_VIEWPORT, view);         // Get the viewport information

      double minDistance = 1000;

      Point point = surface.getContactPoint();
      double x,y,z;
      gluProject(point.X(), point.Y(), point.Z(), model, proj, view, &x,&y,&z);
      Point winPoint = Point(x,y,z);
      double distance = winPoint.distanceTo(
          Point(mouseX, editorHeight - mouseY,
                winPoint.Z())); // Find the closest point to the mouse click
      if (distance < min(pointSize, minDistance)) {
        minDistance = distance;
        dragContact = true;
        dragPoint = false;
      }

      for (int i = 0; i < surface.numPatches(); i++) {
        Patch patch = *surface.getPatch(
            i); // Search through all patches for a possible point
        if (!patch.isVisible())
          continue;
        for (int x = 0; x < patch.numRows(); x++) {
          for (int y = 0; y < patch.numColumns(); y++) {
            Point point = *patch.getPoint(x, y);
	    double xw,yw,zw;
            gluProject(point.X(), point.Y(), point.Z(), model, proj, view,
                       &xw,&yw,&zw);
	    winPoint = Point(xw,yw,zw);
            distance = winPoint.distanceTo(
                Point(mouseX, editorHeight - mouseY, winPoint.Z()));
            // Use point size as a threshold to determine how close to a point
            // the user has to click to select it
            if (distance < min(pointSize, minDistance)) {
              minDistance = distance;
              dragPoint = true;
              dragContact = false;
              surface.setSelectedPatch(i);
              emit(updatePatchSelection(i));
              selectPoint(x, y);
            }
          }
        }
      }
      double xw,yw,zw;
      gluProject(surface.getSelectedPoint()->X(), surface.getSelectedPoint()->Y(),
                 surface.getSelectedPoint()->Z(), model, proj, view, &xw,&yw,&zw);
      winPoint = Point(xw,yw,zw);
      mouseZ = winPoint.Z(); // Remember the depth of the click
    }
  } else if (event->button() == Qt::LeftButton) {

    // Press the
    // trackball at this point
  }

  if (dragContact) {
    selectContactPoint();
  }
  updateGL();
}

void BezierEditor::mouseReleaseEvent(QMouseEvent *event) {
#ifndef __APPLE__
  QGLWidget::makeCurrent();
#endif
  if (event->button() == Qt::LeftButton) {
    // Release the trackball on left mouse button
    // release
    if (dragPoint || dragContact){
      if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
	emit continuousSave();
      
      updateGL();
    }
  }
  dragPoint = false;
  dragContact = false;

}

void BezierEditor::mouseMoveEvent(QMouseEvent *event) {
#ifndef __APPLE__
  QGLWidget::makeCurrent();
#endif
  if (event->buttons() & Qt::LeftButton &&
      event->modifiers() &
          Qt::AltModifier) {
    //zoom
#ifndef __APPLE__
    QGLWidget::makeCurrent();
#endif
    int oldMouseX = mouseX;    // Remember where the mouse was
    int oldMouseY = mouseY;
    updateMousePosition(event->x(), event->y());
    int deltaX = oldMouseX - mouseX; // Find the difference in the mouse position
    int deltaY = mouseY - oldMouseY;
    // Zoom in and out with the mouse wheel
    if (deltaY < 0) {
      camPos.setZ(camPos.Z() - ((camPos.Z()) / 10.0));
    } else if (deltaY > 0) {
      camPos.setZ(camPos.Z() + ((camPos.Z()) / 10.0));
    }
    updateGL();
    
  }
  else if (event->buttons() & Qt::LeftButton &&
      event->modifiers() &
          Qt::ShiftModifier) { // Pan the camera on right mouse button drag
    int oldMouseX = mouseX;    // Remember where the mouse was
    int oldMouseY = mouseY;
    updateMousePosition(event->x(), event->y());
    int diffX = oldMouseX - mouseX; // Find the difference in the mouse position
    int diffY = mouseY - oldMouseY;
    if (parallelProjection) {
      camPos =
          camPos + (Point(diffX, diffY) * camPos.Z() * 2) /
                       (double)min(editorWidth,
                                   editorHeight); // Move the camera position
                                                  // and look point to pan it
      camLook = camLook + (Point(diffX, diffY) * camPos.Z() * 2) /
                              (double)min(editorWidth, editorHeight);
    } else {
      camPos = camPos + (Point(diffX, diffY) * camPos.Z()) /
                            (double)min(editorWidth, editorHeight);
      camLook = camLook + (Point(diffX, diffY) * camPos.Z()) /
                              (double)min(editorWidth, editorHeight);
    }

    updateGL();
  } else if (dragPoint && event->buttons() & Qt::LeftButton &&
             (event->modifiers() & Qt::ControlModifier ||
              rotationLocked)) { // Allow point dragging if shift is held, or if
                                 // rotation is locked
    if (surface.getSelectedPoint() != 0) {
      updateMousePosition(event->x(), event->y());
      double x,y,z;
      gluUnProject(mouseX, editorHeight - mouseY, mouseZ, model, proj,
                   view,&x,&y,&z); // Unproject the mouse position into the editor
      //                   &surface.getSelectedPoint()->X(),
      //             &surface.getSelectedPoint()->Y(),
      //             &surface.getSelectedPoint()->Z());
      surface.getSelectedPoint()->setX(x);
      surface.getSelectedPoint()->setY(y);
      surface.getSelectedPoint()->setZ(z);
      
      emit(selectedPointX(surface.getSelectedPoint()
                              ->X())); // Tell the combo boxes what the
                                     // coordinates of the selected point are
      emit(selectedPointY(surface.getSelectedPoint()->Y()));
      emit(selectedPointZ(surface.getSelectedPoint()->Z()));
    }
    surface.subdivide(subdivisionSamples); // Re-subdivide the surface
    if (_savingMode == CONTINUOUS){
      emit continuousSave();
    }

    updateGL();
  } else if (dragContact && event->buttons() & Qt::LeftButton &&
             (event->modifiers() & Qt::ControlModifier ||
              rotationLocked)) { // Allow point dragging if shift is held, or if
                                 // rotation is locked
    updateMousePosition(event->x(), event->y());
    Point contactPoint = surface.getContactPoint();
    double xc = contactPoint.X();
    double yc = contactPoint.Y();
    double zc = contactPoint.Z();
    gluUnProject(mouseX, editorHeight - mouseY, mouseZ, model, proj,
                 view, // Unproject the mouse position into the editor
                 &xc, &yc, &zc);
    contactPoint = Point(xc,yc,zc);

    emit(selectedPointX(
        surface.getContactPoint().X())); // Tell the combo boxes what the
                                       // coordinates of the selected point are
    emit(selectedPointY(surface.getContactPoint().Y()));
    emit(selectedPointZ(surface.getContactPoint().Z()));

    emit(updateContactPoint(contactPoint));
    surface.setContactPoint(contactPoint);
    if (_savingMode == CONTINUOUS)
      emit continuousSave();

    updateGL();
  } else if (event->buttons() & Qt::LeftButton && !rotationLocked) {
    // Rotate the
    // trackball on left mouse button drag
    int oldMouseX = mouseX; // Remember where the mouse was
    int oldMouseY = mouseY;
    updateMousePosition(event->x(), event->y());
    int diffX = oldMouseX - mouseX; // Find the difference in the mouse position
    int diffY = mouseY - oldMouseY;
    yrot -= (double)diffX / 4.0;
    xrot += (double)diffY / 4.0; // Add to the rotation angles
    while (xrot >= 360)
      xrot -= 360;
    while (xrot < 0)
      xrot += 360;
    while (yrot >= 360)
      yrot -= 360; // Clamp to within 360 degrees
    while (yrot < 0)
      yrot += 360;
    updateGL();
  } else if (event->buttons() & Qt::MidButton) {
    int oldMouseY = mouseY;
    updateMousePosition(event->x(), event->y());
    int diffY = mouseY - oldMouseY;
    double camposz = camPos.Z() + ((camPos.Z()) / 50.0) * diffY;
    camPos.setZ(camposz);
    if (_savingMode == CONTINUOUS)
      emit continuousSave();

    updateGL();
  }
}

void BezierEditor::wheelEvent(QWheelEvent *event) {
#ifndef __APPLE__
  QGLWidget::makeCurrent();
#endif
  if (event->orientation() ==
      Qt::Vertical) { // Zoom in and out with the mouse wheel
    if (event->delta() > 0) {
      camPos.setZ(camPos.Z() - ((camPos.Z()) / 10.0));
    } else if (event->delta() < 0) {
      camPos.setZ(camPos.Z() + ((camPos.Z()) / 10.0));
    }
    updateGL();
  }
}

// Updates the mouse position in screen coordinates as well as recalculating the
// relative coordinates
void BezierEditor::updateMousePosition(int x, int y) {
  const int retinaScale = devicePixelRatio();

  mouseX = x*retinaScale;
  mouseY = y*retinaScale;
}

// Gets the mouse position with respect to the screen coordinates of this widget
Point BezierEditor::getScreenPosition(int x, int y) {
  Point result;
  result.setX(((double)x / (double)editorWidth * 2.0 - 1.0));
  result.setY(-((double)y / (double)editorHeight * 2.0 - 1.0));
  return result;
}

// This function is here so that the GL widget gets the keyboard focus when it
// is active This allows keyboard shortcuts to be passed to the window instead
// of being directed to and ignored by the combo boxes
void BezierEditor::keyPressEvent(QKeyEvent *event) { event->ignore(); }

// Saves the current triangle and point information into their respective undo
// stacks
void BezierEditor::saveUndoData() {
  emit(canUndo(true)); // Let the window know that there is something to undo

  if (undoStack.empty() ||
      surface !=
          undoStack
              .back()) { // Don't save the state if nothing was actually changed
    undoStack.push_back(surface);

    if (undoStack.size() > MAX_UNDOS) { // Throw away old states if their number
                                        // exceeds the maximum
      undoStack.erase(undoStack.begin());
    }
  }
}

// Saves undo data and clears the redo stack
void BezierEditor::saveUndoState() {
  saveUndoData();
  redoStack.clear();
  emit(canRedo(false));
}

// Saves the current triangle and point information into their respective redo
// stacks
void BezierEditor::saveRedoData() {
  emit(canRedo(true)); // Let the window know that there is something to redo

  redoStack.push_back(surface);
}

// Returns the editor to the state saved on the top of the undo stack
void BezierEditor::undo() {
  if (undoStack.size() > 0) {
    QGLWidget::makeCurrent();
    saveRedoData(); // Save the current state so that it can be redone
    surface = undoStack.back(); // Restore the state
    undoStack.pop_back(); // Pop the stack so that the next undo state is on top

    if (undoStack.size() == 0)
      emit(canUndo(false));

    changed();
    emit(newSurface());
    for (int i = 0; i < surface.numPatches(); i++) {
      emit(addPatchToList(surface.getPatchName(i)));
    }
    surface.subdivide(subdivisionSamples);
    if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
      emit continuousSave();

    updateGL();
  }
}

// Returns the editor to the state saved on the top of the redo stack
void BezierEditor::redo() {
  if (redoStack.size() > 0) {
    QGLWidget::makeCurrent();
    saveUndoData(); // Save the current state so that the redo can be undone
    surface = redoStack.back(); // Restore the state
    redoStack.pop_back(); // Pop the stack so that the next redo state is on top

    if (redoStack.size() == 0)
      emit(canRedo(false));

    changed();
    surface.subdivide(subdivisionSamples);
    emit(newSurface());
    for (int i = 0; i < surface.numPatches(); i++) {
      emit(addPatchToList(surface.getPatchName(i)));
    }
    if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
      emit continuousSave();
    updateGL();
  }
}

// Loads a texture with the given filename into the given GL texture
bool BezierEditor::loadImage(const char *filename) {
#ifndef __APPLE__
  QGLWidget::makeCurrent();
#endif
  QImage image;
  if (string(filename).empty())
    return false; // An empty string is passed in the case that no image should
                  // be loaded
  /*
  bool supported = QImageFormatStrings::isSupportedFormat(QString(filename));
  if (!supported) { // If the texture was not found send an error message...
    qDebug() << QString("File type of ")
                    .append(filename)
                    .append(" load image is not supported")
                    .toStdString()
                    .c_str();
    return false;
  }
  */
  bool success = image.load(filename);
  if (!success) { // If the texture was not found send an error message...
    qDebug() << QString("Cannot load texture file ")
                    .append(filename)
                    .toStdString()
                    .c_str();
    return false;
  }

  if (textureFlippedH)
    image = image.transformed(QMatrix().scale(-1, 1));
  if (textureFlippedV)
    image = image.transformed(QMatrix().scale(1, -1));
  //PASCAL: For some reason the image is flipped and rotated, so we flip adn rotate it back
  /*
  image = image.transformed(QMatrix().scale(1, -1));
  image = image.transformed(QMatrix().scale(-1, 1));
  image = image.transformed(QMatrix().rotate(textureRotation));
  */
  image = QGLWidget::convertToGLFormat(image);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0,
               GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
  if (linearInterpolation) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  //	updateGL();
  return true;
}

// Reloads the texture that was saved as the current texture
void BezierEditor::reloadCurrentTexture() {
  QGLWidget::makeCurrent();
  QImage image = currentTexture;
  if (textureFlippedH)
    image = image.transformed(QMatrix().scale(-1, 1));
  if (textureFlippedV)
    image = image.transformed(QMatrix().scale(1, -1));
  //PASCAL: For some reason the image is flipped and rotated, so we flip adn rotate it back
  /*
  image = image.transformed(QMatrix().scale(1, -1));
  image = image.transformed(QMatrix().scale(-1, 1));
  image = image.transformed(QMatrix().rotate(textureRotation));
  */
  image = QGLWidget::convertToGLFormat(image);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0,
               GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
  if (linearInterpolation) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
}

// Loads a new texture to show on the surface
bool BezierEditor::loadTexture(string filename) {
  QGLWidget::makeCurrent();
  hasCurrentTexture = false;
  bool success = loadImage(filename.c_str());
  if (!success)
    return false;
  textureName = filename; // Remember what texture is loaded so that it can be
                            // used in the texture editor
  updateGL();
  return true;
}

// Loads a .s file for editing
bool BezierEditor::load(string filename) {
  QGLWidget::makeCurrent();
  emit(newSurface());
  std::string extension = filename.substr(filename.find_last_of(".") + 1);
  bool supported = (extension.compare("s") == 0);
  if (!supported) { // If the surface file to be loaded is an invalid format,
                    // send an error message...
    qDebug() << QString("File type of ")
                    .append(filename.c_str())
                    .append(" is not supported")
                    .toStdString()
                    .c_str();
    
    return false;
  }

  bool success = surface.load(filename);
  if (!success) { // If the surface was not found send an error message...
    qDebug() << QString("Cannot load surface file ")
                    .append(filename.c_str())
                    .toStdString()
                    .c_str();
    return false;
  } else {
    surfaceFilename = filename; // Remember the name of the loaded file
    vector<string> names = surface.getPatchNames();
    for (unsigned int i = 0; i < names.size(); i++) {
      emit(addPatchToList(names.at(i)));
    }
  }
  surface.subdivide(subdivisionSamples); // Subdivide the surface appropreatly

  selectPoint(0, 0);
  contactSelected = false;
  resetView();
  unsavedChanges = false;
  undoStack.clear();
  emit(canUndo(false));
  emit(canRevert(false));

  emit(updatePatchSelection(0));
  updateGL();
  return true;
}

// Load the most recently loaded project again to revert it to its most recently
// saved state
bool BezierEditor::reload() {
  bool success = load(surfaceFilename);
  surface.subdivide(subdivisionSamples); // Subdivide the surface appropreatly
  return success;
}

// Saves a .s file
void BezierEditor::save(string filename) {
  surface.save(filename);
  surfaceFilename =
      filename; // Remember the name of the save file so it can be saved again
  unsavedChanges = false;
  emit(canRevert(false));
}

// Saves a .s file over the previously loaded .s file
void BezierEditor::save() { save(surfaceFilename); }

// Returns the background colour for this widget
Colour BezierEditor::getBgColour() { return bgColour; }

// Returns the point colour for this widget
Colour BezierEditor::getPointColour() { return pointColour; }

// Returns the selected point colour for this widget
Colour BezierEditor::getSelectedPointColour() { return selectedPointColour; }

// Returns the line colour for this widget
Colour BezierEditor::getLineColour() { return lineColour; }

// Returns the wireframe colour for this widget
Colour BezierEditor::getWireframeColour() { return wireframeColour; }

// Returns the vector colour for this widget
Colour BezierEditor::getVectorColour() { return vectorColour; }

// Returns the patch colour for this widget
Colour BezierEditor::getPatchColour() { return patchColour; }
// Returns the selected patch colour for this widget
Colour BezierEditor::getSelectedPatchColour() { return selectedPatchColour; }

// Resets the background colour to the window colour (saved as defBgColour)
void BezierEditor::resetBgColour() { changeBgColour(defBgColour); }

// Changes the background colour to the given colour
void BezierEditor::changeBgColour(Colour colour) {
  QGLWidget::makeCurrent();
  bgColour = colour;
  glClearColor(bgColour.r, bgColour.g, bgColour.b, 0);
  changed();
  updateGL();
}

// Changes the point colour to the given colour
void BezierEditor::changePointColour(Colour colour) {
  pointColour = colour;
  changed();
  updateGL();
}

// Changes the selected point colour to the given colour
void BezierEditor::changeSelectedPointColour(Colour colour) {
  selectedPointColour = colour;
  changed();
  updateGL();
}

// Changes the line colour to the given colour
void BezierEditor::changeLineColour(Colour colour) {
  lineColour = colour;
  changed();
  updateGL();
}

// Changes the wireframe colour to the given colour
void BezierEditor::changeWireframeColour(Colour colour) {
  wireframeColour = colour;
  changed();
  updateGL();
}

// Changes the vector colour to the given colour
void BezierEditor::changeVectorColour(Colour colour) {
  vectorColour = colour;
  changed();
  updateGL();
}

// Changes the patch to the given colour
void BezierEditor::changePatchColour(Colour colour) {
  patchColour = colour;
  changed();
  updateGL();
}

// Changes the selected patch to the given colour
void BezierEditor::changeSelectedPatchColour(Colour colour) {
  selectedPatchColour = colour;
  changed();
  updateGL();
}

// Sets the visibility of the points to the given boolean value
void BezierEditor::showHidePoints(bool value) {
  showPoints = value;
  updateGL();
}

// Sets the visibility of the lines to the given boolean value
void BezierEditor::showHideLines(bool value) {
  showLines = value;
  updateGL();
}

// Sets the visibility of the surface to the given boolean value
void BezierEditor::showHideSurface(bool value) {
  showSurface = value;
  updateGL();
}

// Sets the visibility of the vectors to the given boolean value
void BezierEditor::showHideVectors(bool value) {
  showVectors = value;
  updateGL();
}

// Centers the camera and zooms to fit the surface's current position
void BezierEditor::centerCamera() {
  double xmax = -10001;
  double xmin = 10001;
  double ymax = -10001;
  double ymin = 10001;
  double zmax = -10001;
  double zmin = 10001;
  for (int i = 0; i < surface.numRows();
       i++) { // Look at the control points to find the current maximum
              // coordinates
    for (int j = 0; j < surface.numColumns(); j++) {
      if (surface.getPoint(i, j)->X() > xmax)
        xmax = surface.getPoint(i, j)->X();
      if (surface.getPoint(i, j)->X() < xmin)
        xmin = surface.getPoint(i, j)->X();
      if (surface.getPoint(i, j)->Y() > ymax)
        ymax = surface.getPoint(i, j)->Y();
      if (surface.getPoint(i, j)->Y() < ymin)
        ymin = surface.getPoint(i, j)->Y();
      if (surface.getPoint(i, j)->Z() > zmax)
        zmax = surface.getPoint(i, j)->Z();
      if (surface.getPoint(i, j)->Z() < zmin)
        zmin = surface.getPoint(i, j)->Z();
    }
  }
  centerPoint = surface.getCenter();
  if (parallelProjection)
    camPos.setZ(
        max(max(max(max(max(fabs(xmin), fabs(xmax)), fabs(ymin)), fabs(ymax)),
                fabs(zmin)),
            fabs(zmax)) *
        1.35);
  else
    camPos.setZ(
        max(max(max(max(max(fabs(xmin), fabs(xmax)), fabs(ymin)), fabs(ymax)),
                fabs(zmin)),
            fabs(zmax)) *
        2.);
  camPos.setX(0); // Set the camera position back to the z axis
  camPos.setY(0);
  camLook = Point(); // Look at the origin
  centerContact = false;
  emit(contactCenteredChanged(false));
  updateGL();
}

// Centers the camera at the surface's contact point
void BezierEditor::centerAtContact(bool value) {
  if (value) {
    centerPoint = surface.getContactPoint();
    centerContact = true;
    updateGL();
  } else {
    centerContact = false;
  }
}

// Resets the viewer to its initial state
void BezierEditor::resetView() {
  // trackball.reset(); // Reset the trackball's rotation matrix to identity
  xrot = 0;
  yrot = 0;
  centerCamera();
  updateGL();
}

// Enables or disables wireframe mode
void BezierEditor::setWireframe(bool value) {
  wireframe = value;
  updateGL();
}

// Selects the point with the given coordinates in the array of control points
void BezierEditor::selectPoint(int index, int jndex) {
  surface.selectPoint(index, jndex); // Select it in the surface
  contactSelected = false;
  if (surface.getSelectedPoint() != 0) {
    emit(selectedPointX(surface.getSelectedPoint()
                            ->X())); // Tell the combo boxes what the coordinates
                                   // of the selected point are
    emit(selectedPointY(surface.getSelectedPoint()->Y()));
    emit(selectedPointZ(surface.getSelectedPoint()->Z()));
    emit(pointSelected(index, jndex));
  }
  updateGL();
}

// Marks the contact point as selected so that it can be manipulated
void BezierEditor::selectContactPoint() {
  contactSelected = true;
  emit(selectedPointX(
      surface.getContactPoint().X())); // Tell the combo boxes what the
                                     // coordinates of the selected point are
  emit(selectedPointY(surface.getContactPoint().Y()));
  emit(selectedPointZ(surface.getContactPoint().Z()));
  emit(contactPointSelected());
  updateGL();
}

void BezierEditor::saveXYZValue() {
     if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
      emit continuousSave();
}
// Sets the X value of the currently selected point, and updates the position of
// the opposite point if symmetrical editing is on
void BezierEditor::setXValue(double value) {
  if (contactSelected) {
    Point oldContact = surface.getContactPoint();
    surface.setContactPoint(Point(value, oldContact.Y(), oldContact.Z()));
    changed();
    if(_savingMode == CONTINUOUS)
      emit continuousSave();

    updateGL();
  } else if (surface.getSelectedPoint() != 0) {
    surface.getSelectedPoint()->setX(value);
    // Move the point symmetrically about the x line going through the surface's
    // contact point
    if (symmetrical14Y)
      surface.getSymmetryPoint14()->setX(
					 -value + (surface.getContactPoint().X() * 2));
    else if (symmetrical14X)
      surface.getSymmetryPoint14()->setX(value);
    if (symmetrical113X)
      surface.getSymmetryPoint113()->setX(
					  -value + (surface.getContactPoint().X() * 2));
    else if (symmetrical113Y)
      surface.getSymmetryPoint113()->setX(value);

    if (symmetrical14Y && symmetrical113Y)
      surface.getSymmetryPointOpposite()->setX(
					       -value + (surface.getContactPoint().X() * 2));
    else if (symmetrical14X && symmetrical113X)
      surface.getSymmetryPointOpposite()->setX(
					       -value + (surface.getContactPoint().X() * 2));

    updateAdjacencies();
    changed();
    surface.subdivide(subdivisionSamples); // Subdivide the surface appropreatly
    if (_savingMode == CONTINUOUS)
      emit continuousSave();

    updateGL();
  }
}

// Sets the Y value of the currently selected point, and updates the position of
// the opposite point if symmetrical editing is on
void BezierEditor::setYValue(double value) {

  if (contactSelected) {
    Point oldContact = surface.getContactPoint();
    surface.setContactPoint(Point(oldContact.X(), value, oldContact.Z()));
    changed();
    if  (_savingMode == CONTINUOUS)
      emit continuousSave();

    updateGL();
  } else if (surface.getSelectedPoint() != 0) {
    surface.getSelectedPoint()->setY(value);
    if (symmetrical14Y)
      surface.getSymmetryPoint14()->setY(value);
    else if (symmetrical14X)
      surface.getSymmetryPoint14()->setY(
					 -value + (surface.getContactPoint().Y() * 2));
    if (symmetrical113X)
      surface.getSymmetryPoint113()->setY(value);
    else if (symmetrical113Y)
      surface.getSymmetryPoint113()->setY(
					  -value + (surface.getContactPoint().Y() * 2));

    if (symmetrical14Y && symmetrical113Y)
      surface.getSymmetryPointOpposite()->setY(
					       -value + (surface.getContactPoint().Y() * 2));
    else if (symmetrical14X && symmetrical113X)
      surface.getSymmetryPointOpposite()->setY(
					       -value + (surface.getContactPoint().Y() * 2));

    updateAdjacencies();
    changed();
    surface.subdivide(subdivisionSamples); // Subdivide the surface appropreatly
    if (_savingMode == CONTINUOUS)
      emit continuousSave();

    updateGL();
  }
}

// Sets the Z value of the currently selected point, and updates the position of
// the opposite point if symmetrical editing is on
void BezierEditor::setZValue(double value) {

  if (contactSelected) {
    Point oldContact = surface.getContactPoint();
    surface.setContactPoint(Point(oldContact.X(), oldContact.Y(), value));
    changed();
    if  (_savingMode == CONTINUOUS)
      emit continuousSave();

    updateGL();
  } else if (surface.getSelectedPoint() != 0) {
    surface.getSelectedPoint()->setZ(value);
    if (symmetrical14Y || symmetrical14X)
      surface.getSymmetryPoint14()->setZ(value);
    if (symmetrical113X || symmetrical113Y)
      surface.getSymmetryPoint113()->setZ(value);

    if (symmetrical14Y && symmetrical113Y)
      surface.getSymmetryPointOpposite()->setZ(value);
    else if (symmetrical14X && symmetrical113X)
      surface.getSymmetryPointOpposite()->setZ(value);

    updateAdjacencies();
    changed();
    surface.subdivide(subdivisionSamples); // Subdivide the surface appropreatly
    if (_savingMode == CONTINUOUS)
      emit continuousSave();
    updateGL();
  }
}

// Returns the current point size
double BezierEditor::getPointSize() { return pointSize; }

// Returns the contact current point size
double BezierEditor::getContactPointSize() { return contactPointSize; }

// Returns the current line width
double BezierEditor::getLineWidth() { return lineWidth; }

// Returns the current width of the wireframe lines
double BezierEditor::getWireframeWidth() { return wireframeWidth; }

// Returns the current level of bezier subdivision
int BezierEditor::getSubdivisionSamples() { return subdivisionSamples; }

// Sets the point size to the given value
void BezierEditor::setPointSize(double value) { pointSize = value; }

// Sets the contact point size to the given value
void BezierEditor::setContactPointSize(double value) {
  contactPointSize = value;
}

// Sets the line width to the given value
void BezierEditor::setLineWidth(double value) { lineWidth = value; }

// Sets the width of the wireframe lines to the given value
void BezierEditor::setWireframeWidth(double value) { wireframeWidth = value; }

// Sets the level of bezier subdivision to the given value
void BezierEditor::setSubdivisionSamples(int value) {
  QGLWidget::makeCurrent();
  subdivisionSamples = value;
  surface.subdivide(subdivisionSamples); // Subdivide the surface appropreatly
  if (_savingMode == CONTINUOUS)
    emit continuousSave();

  updateGL();
}

// Gets the name of the currently loaded texture, so that it can be passed to
// the texture editor
string BezierEditor::getTextureName() { return textureName; }

bool BezierEditor::hasTexture() { return !textureName.empty(); }

// Gets the name of the currently loaded texture, so that it can be passed to
// the texture editor
QImage BezierEditor::getCurrentTexture() { return currentTexture; }

// Indicates whether or not there is a valid current texture
bool BezierEditor::noCurrentTexture() { return !hasCurrentTexture; }

// Updates the currently shown texture to the QImage passed
void BezierEditor::updateTexture(QImage image) {
  currentTexture = image;
  QGLWidget::makeCurrent();

  if (textureFlippedH)
    currentTexture = currentTexture.transformed(QMatrix().scale(-1, 1));
  if (textureFlippedV)
    currentTexture = currentTexture.transformed(QMatrix().scale(1, -1));
  
  //currentTexture =
  //    currentTexture.transformed(QMatrix().rotate(textureRotation));

  hasCurrentTexture = true;
    //PASCAL: For some reason the image is flipped and rotated, so we flip adn rotate it back
  /*
  currentTexture = currentTexture.transformed(QMatrix().scale(1, -1));
  currentTexture = currentTexture.transformed(QMatrix().scale(-1, 1));
  currentTexture = currentTexture.transformed(QMatrix().rotate(textureRotation));
  */
  QImage texture = QGLWidget::convertToGLFormat(currentTexture);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width(), texture.height(), 0,
               GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
  if (linearInterpolation) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}

// Updates the saved texture name to match the texture editor
void BezierEditor::updateTextureName(string filename) {

  textureName = filename;
}

// Saves the current texture so that it can be reverted to
void BezierEditor::saveCurrentTexture() { revertTexture = currentTexture; }

// Restores the current texture to the revert texture if there is one, otherwise
// indicates that there is no longer a valid current texture
void BezierEditor::restoreTexture() {
  if (!revertTexture.isNull() && hasCurrentTexture)
    currentTexture = revertTexture;
  else
    hasCurrentTexture = false;
}

// Reloads the last loaded texture
void BezierEditor::reloadTexture() {
  if (hasTexture()) {
    if (hasCurrentTexture)
      reloadCurrentTexture();
    else
      loadTexture(textureName);
  }
}

// Enables or disables symmetrical editing mode
void BezierEditor::setSymmetricalEditing(bool axis14, bool axisX, bool value) {
  saveUndoData();
  redoStack.clear();
  emit(canRedo(false));
  if (axis14) {
    if (axisX)
      symmetrical14Y = value;
    else
      symmetrical14X = value;
  } else {
    if (axisX)
      symmetrical113X = value;
    else
      symmetrical113Y = value;
  }
  updateGL();
}

// Clears the surface filename so that the program won't save over anything
void BezierEditor::clearSurfaceFilename() { surfaceFilename = ""; }

// Returns true if there is no saved surface filename, false otherwise
bool BezierEditor::noSurfaceFilename() { return surfaceFilename.empty(); }

// Indicates that changes have been made to the surface, so the program should
// ask to save before closing, and allow reverting
void BezierEditor::changed() {
  unsavedChanges = true;
  if (!noSurfaceFilename())
    emit(canRevert(true));
  else
    emit(canRevert(false));
}

Point BezierEditor::getContactPoint() { return surface.getContactPoint(); }

Point BezierEditor::getEndPoint() { return surface.getEndPoint(); }

Vector3 BezierEditor::getHeading() { return surface.getHeading(); }

Vector3 BezierEditor::getUp() { return surface.getUp(); }

double BezierEditor::getSize() { return surface.getSize(); }

// Set the contact point to a new position and recenter the camera on it if the
// camera was already centered on it
void BezierEditor::setContactPoint(Point point) {
  surface.setContactPoint(point);
  if (centerContact)
    centerAtContact(true); // Recenter the contact if it was moved
  changed();
  updateGL();
}

// Set the value of the surface's endpoint to the given value and indicate a
// change
void BezierEditor::setEndPoint(Point point) {
  surface.setEndPoint(point);
  changed();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}

// Set the value of the surface's heading vector to the given value and indicate
// a change
void BezierEditor::setHeading(Vector3 vector) {

  surface.setHeading(vector);
  changed();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}

// Set the value of the surface's up vector to the given value and indicate a
// change
void BezierEditor::setUp(Vector3 vector) {

  surface.setUp(vector);
  changed();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}

// Set the value of the surface's size to the given value and indicate a change
void BezierEditor::setSize(double value) {

  surface.setSize(value);
  changed();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}

// Flip all the control points in the surface horizontally about the YZ plane
// through the contact point
void BezierEditor::flipPatchHorizontal() {
  saveUndoState();
  surface.flipHorizontal();
  changed();
  surface.subdivide(subdivisionSamples); // Subdivide the surface appropreatly
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
  if (contactSelected) {
    emit(selectedPointX(
        surface.getContactPoint().X())); // Tell the combo boxes what the
                                       // coordinates of the selected point are
    emit(selectedPointY(surface.getContactPoint().Y()));
    emit(selectedPointZ(surface.getContactPoint().Z()));
  } else if (surface.getSelectedPoint() != 0) {
    emit(selectedPointX(surface.getSelectedPoint()->X()));
    emit(selectedPointY(surface.getSelectedPoint()->Y()));
    emit(selectedPointZ(surface.getSelectedPoint()->Z()));
  }
}

// Flip all the control points in the surface vertically about the XZ plane
// through the contact point
void BezierEditor::flipPatchVertical() {
  saveUndoState();
  surface.flipVertical();
  changed();
  surface.subdivide(subdivisionSamples); // Subdivide the surface appropreatly
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
  if (contactSelected) {
    emit(selectedPointX(
        surface.getContactPoint().X())); // Tell the combo boxes what the
                                       // coordinates of the selected point are
    emit(selectedPointY(surface.getContactPoint().Y()));
    emit(selectedPointZ(surface.getContactPoint().Z()));
  } else if (surface.getSelectedPoint() != 0) {
    emit(selectedPointX(surface.getSelectedPoint()->X()));
    emit(selectedPointY(surface.getSelectedPoint()->Y()));
    emit(selectedPointZ(surface.getSelectedPoint()->Z()));
  }
}

// Flip all the control points in the surface depthwise about the XY plane
// through the contact point
void BezierEditor::flipPatchDepth() {
  saveUndoState();
  surface.flipDepth();
  changed();
  surface.subdivide(subdivisionSamples); // Subdivide the surface appropreatly
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
  if (contactSelected) {
    emit(selectedPointX(
        surface.getContactPoint().X())); // Tell the combo boxes what the
                                       // coordinates of the selected point are
    emit(selectedPointY(surface.getContactPoint().Y()));
    emit(selectedPointZ(surface.getContactPoint().Z()));
  } else if (surface.getSelectedPoint() != 0) {
    emit(selectedPointX(surface.getSelectedPoint()->X()));
    emit(selectedPointY(surface.getSelectedPoint()->Y()));
    emit(selectedPointZ(surface.getSelectedPoint()->Z()));
  }
}

// Gets the i index of the selected point in the surface
int BezierEditor::getSelectedPointI() { return surface.getSelectedPointI(); }

// Gets the j index of the selected point in the surface
int BezierEditor::getSelectedPointJ() { return surface.getSelectedPointJ(); }

// Sets the patch with the given index to be the selected patch in the surface
void BezierEditor::selectPatch(int index) {
  QGLWidget::makeCurrent();
  surface.setSelectedPatch(index);
  if (contactSelected) {
    emit(selectedPointX(
        surface.getContactPoint().X())); // Tell the combo boxes what the
                                       // coordinates of the selected point are
    emit(selectedPointY(surface.getContactPoint().Y()));
    emit(selectedPointZ(surface.getContactPoint().Z()));
  } else if (surface.getSelectedPoint() != 0) {
    emit(selectedPointX(surface.getSelectedPoint()->X()));
    emit(selectedPointY(surface.getSelectedPoint()->Y()));
    emit(selectedPointZ(surface.getSelectedPoint()->Z()));
  }
  updateGL();
}

// Changes the name of the patch at the given index to the name provided
void BezierEditor::changePatchName(string name, int index) {
  surface.changePatchName(name, index);
}

// Sets the visibility of the patch at the given index
void BezierEditor::changePatchVisibility(bool visibility, int index) {
  surface.changePatchVisibility(visibility, index);
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}

// Adds a new patch to the surface
void BezierEditor::addNewPatch(int shape) {
  QGLWidget::makeCurrent();
  saveUndoState();
  changed();

  QString name = QString();
  int nameral = surface.numPatches() + 1;
  name = name.setNum(nameral);
  name.prepend("Patch_");

  QStringList names = getPatchNames();
  while (names.contains(name)) {
    nameral++;
    name = name.setNum(nameral);
    name.prepend("Patch_");
  }

  surface.addNewPatch(name.toStdString(), shape);
  emit(addPatchToList(name.toStdString()));
  surface.subdivide(subdivisionSamples);
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
  if (surface.numPatches() ==
      1) { // If this is the first patch to be added, take some extra steps...
    surface.setSelectedPatch(0);
    emit(updatePatchSelection(0));
    if (contactSelected) {
      emit(selectedPointX(surface.getContactPoint()
                              .X())); // Tell the combo boxes what the coordinates
                                    // of the selected point are
      emit(selectedPointY(surface.getContactPoint().Y()));
      emit(selectedPointZ(surface.getContactPoint().Z()));
    } else {
      emit(selectedPointX(surface.getSelectedPoint()
                              ->X())); // Tell the combo boxes what the
                                     // coordinates of the selected point are
      emit(selectedPointY(surface.getSelectedPoint()->Y()));
      emit(selectedPointZ(surface.getSelectedPoint()->Z()));
    }
  }
}

// Deletes the currently selected patch from the surface
void BezierEditor::deleteSelectedPatch() {
  QGLWidget::makeCurrent();
  saveUndoState();
  changed();
  surface.deletePatch(surface.getSelectedPatch());
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  if (surface.numPatches() == 0)
    updateGL();
}

// Duplicates the selected patch in the bezier editor
void BezierEditor::duplicateSelectedPatch() {
  QGLWidget::makeCurrent();
  saveUndoState();
  changed();

  QString nameRoot = QString();
  QString nameNum = QString();
  QString name = QString();

  nameRoot = surface.getPatchName(surface.getSelectedPatch())
                 .c_str();              // The actual patch name string
  nameNum = nameRoot.split('_').last(); // Takes the portion of the patch name
                                        // after the last underscore as a number
  bool success;
  int nameral = nameNum.toInt(
      &success); // Read the number as an int and see if it actually works
  if (success)
    nameRoot.chop(nameNum.length() +
                  1); // If a number was found, remove it and the underscore
                      // from the end of the name

  nameral++;                         // Incremet the number
  nameNum = nameNum.setNum(nameral); // Set the number string to the new number
  name.append(nameRoot).append("_").append(
      nameNum); // The new name is the old root name with the new number
                // appended after an underscore

  QStringList names = getPatchNames();
  while (names.contains(name)) { // If the new name exists, increment the number
                                 // until the name is unique
    nameral++;
    nameNum = nameNum.setNum(nameral);
    name.clear();
    name.append(nameRoot).append("_").append(nameNum);
  }

  if (surface.duplicatePatch(
          surface.getSelectedPatch(),
          name.toStdString())) { // Do the actual patch duplication
    changed();

    emit(addPatchToList(name.toStdString()));
    surface.subdivide(subdivisionSamples);
    surface.setSelectedPatch(surface.numPatches() - 1);
    emit(updatePatchSelection(surface.numPatches() - 1));
    if (contactSelected) {
      emit(selectedPointX(surface.getContactPoint()
                              .X())); // Tell the combo boxes what the coordinates
                                    // of the selected point are
      emit(selectedPointY(surface.getContactPoint().Y()));
      emit(selectedPointZ(surface.getContactPoint().Z()));
    } else {
      emit(selectedPointX(surface.getSelectedPoint()
                              ->X())); // Tell the combo boxes what the
                                     // coordinates of the selected point are
      emit(selectedPointY(surface.getSelectedPoint()->Y()));
      emit(selectedPointZ(surface.getSelectedPoint()->Z()));
    }
  }
}

// Returns a QStringList of the names of the patches in the surface, in the
// order they appear in the list
QStringList BezierEditor::getPatchNames() {
  QStringList names = QStringList();
  vector<string> list = surface.getPatchNames();
  for (unsigned int i = 0; i < list.size(); i++) {
    names.append(list.at(i).c_str());
  }
  return names;
}

// Translates the currently selected patch by the given vector
// A previous translation needs to be remembered so that a translation can be
// updated continuously with the latest changes
void BezierEditor::translate(Vector3 translation) {
  surface.translate(translation - previousTranslation);
  previousTranslation = translation;
  changed();
  surface.subdivide(subdivisionSamples);
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
  if (contactSelected) {
    emit(selectedPointX(
        surface.getContactPoint().X())); // Tell the combo boxes what the
                                       // coordinates of the selected point are
    emit(selectedPointY(surface.getContactPoint().Y()));
    emit(selectedPointZ(surface.getContactPoint().Z()));
  } else if (surface.getSelectedPoint() != 0) {
    emit(selectedPointX(surface.getSelectedPoint()->X()));
    emit(selectedPointY(surface.getSelectedPoint()->Y()));
    emit(selectedPointZ(surface.getSelectedPoint()->Z()));
  }
}

// Rotates the currently selected patch by the given angle about the given axis
// A previous rotation needs to be remembered so that a rotation can be updated
// continuously with the latest changes
void BezierEditor::rotate(Vector3 axis, double angle) {
  //  std::cerr<<"Rotation params: "<<axis.X()<<" - "<<axis.Y()<<" - "<<axis.Z()<<" - "<<angle<<std::endl;
  if (!axis.isZero()) { // If the axis is zero, do nothing
    axis = axis.normalize();
    //    std::cerr<<"Previous rotation: "<<previousRotationAxis.X()<<" - "<<previousRotationAxis.Y()<<" - "<<previousRotationAxis.Z()<<" - "<<previousRotationAngle<<std::endl;
    surface.rotate(previousRotationAxis,
		   -previousRotationAngle); // Undo the old rotation
    
    surface.rotate(axis, angle);            // Perform the new rotation
    previousRotationAxis = axis;   // Remember this rotation axis for next time
    previousRotationAngle = angle; // Remember this rotation angle for next time
    changed();
    //    std::cerr<<"Bezier Editor subdivide"<<std::endl;
    surface.subdivide(subdivisionSamples); // Re-subdivide the surface
    //    std::cerr<<"---------------"<<std::endl;
    
    if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
      emit continuousSave();
    
    updateGL();
    if (contactSelected) {
      emit(selectedPointX(surface.getContactPoint()
                              .X())); // Tell the combo boxes what the coordinates
                                    // of the selected point are
      emit(selectedPointY(surface.getContactPoint().Y()));
      emit(selectedPointZ(surface.getContactPoint().Z()));
    } else if (surface.getSelectedPoint() != 0) {
      //      std::cerr<<"EMIT: "<<surface.getSelectedPoint()->X()<<" - "<<surface.getSelectedPoint()->Y()<<" - "<<surface.getSelectedPoint()->Z()<<std::endl;
      emit(selectedPointX(surface.getSelectedPoint()->X()));
      emit(selectedPointY(surface.getSelectedPoint()->Y()));
      emit(selectedPointZ(surface.getSelectedPoint()->Z()));
    }
  }
}

// Scales the currently selected patch by the given vector
// A previous scale operation needs to be remembered so that a scaling can be
// updated continuously with the latest changes
void BezierEditor::scale(Vector3 factor) {
  surface.scale(
      Vector3(1 / previousScale.X(), 1 / previousScale.Y(), 1 / previousScale.Z()));
  surface.scale(factor);
  previousScale = factor;
  changed();
  surface.subdivide(subdivisionSamples);
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
  if (contactSelected) {
    emit(selectedPointX(
        surface.getContactPoint().X())); // Tell the combo boxes what the
                                       // coordinates of the selected point are
    emit(selectedPointY(surface.getContactPoint().Y()));
    emit(selectedPointZ(surface.getContactPoint().Z()));
  } else if (surface.getSelectedPoint() != 0) {
    emit(selectedPointX(surface.getSelectedPoint()->X()));
    emit(selectedPointY(surface.getSelectedPoint()->Y()));
    emit(selectedPointZ(surface.getSelectedPoint()->Z()));
  }
}

// Resets the previous translation to zero
void BezierEditor::resetTranslation() { previousTranslation = Vector3(); }

// Resets the previous rotation to zero
void BezierEditor::resetRotation() {
  previousRotationAxis = Vector3();
  previousRotationAngle = 0;
}

// Resets the previous scaling to one
void BezierEditor::resetScale() { previousScale = Vector3(1, 1, 1); }

void BezierEditor::setRotationLocked(bool value) {
  rotationLocked = value;
  // if (rotationLocked) resetView();
}

// Save undo information whenever a box is finished being edited
// TODO: The information save should be the information from BEFORE the box was
// changed, not after
void BezierEditor::boxEdited() {
  saveUndoState();
  updateGL();
}

// Gets a pointer to the surface
Surface *BezierEditor::getSurfaceLoc() { return &surface; }

void BezierEditor::setEnforceAdjacency(bool value) {
  enforceAdjacency = value;
  updateAdjacencies();
}

void BezierEditor::updateAdjacencies() {
  QGLWidget::makeCurrent();
  if (enforceAdjacency) {
    surface.updateAdjacentEdges(
        surface.getSelectedPatch()); // Update the selected patch first
    for (int i = 0; i < surface.numPatches(); i++) { // Then the rest
      if (i != surface.getSelectedPatch())
        surface.updateAdjacentEdges(i);
    }
    surface.subdivide(subdivisionSamples);
    if  (_savingMode == CONTINUOUS)
      emit continuousSave();

    updateGL();
  }
}

void BezierEditor::setLinearInterpolation(bool value) {
  if (linearInterpolation == value)
    return;
  linearInterpolation = value;
  reloadTexture();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}

void BezierEditor::setProjection(bool value) {
  if (parallelProjection == value)
    return;
  parallelProjection = value;
#ifndef __APPLE__
  glViewport(0, 0, editorWidth, editorHeight);
#endif
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (parallelProjection) {
    if (editorWidth > editorHeight)
      glOrtho(-camPos.Z() * ratio, camPos.Z() * ratio, -camPos.Z(), camPos.Z(), -10000,
              10000);
    else
      glOrtho(-camPos.Z(), camPos.Z(), -camPos.Z() / ratio, camPos.Z() / ratio, -10000,
              10000);
  } else
    gluPerspective(50.0f, ratio, 0.01f, 10000.0f);
  glMatrixMode(GL_MODELVIEW);
  centerCamera();
}

void BezierEditor::rotateTextureCW() {
  textureRotation += 90;
  textureRotation %= 360;
  changed();

  reloadTexture();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}

void BezierEditor::rotateTextureCCW() {
  textureRotation += 270;
  textureRotation %= 360;
  changed();

  reloadTexture();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}

void BezierEditor::flipTextureH(bool value) {
  textureFlippedH = value;
  reloadTexture();
  changed();

  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}

void BezierEditor::flipTextureV(bool value) {
  textureFlippedV = value;
  reloadTexture();
  changed();

  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    emit continuousSave();

  updateGL();
}
