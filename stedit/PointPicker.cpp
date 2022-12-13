/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "PointPicker.h"

PointPicker::PointPicker(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::AlphaChannel), parent) {

  ratio = 1.0;
  editorWidth = 150;
  editorHeight = 150;
  bgColour = Colour(parent->palette().color(
      QPalette::Window)); // Match the window's background colour
  pointColour = Colour(1, 0, 0);

  init();
}

PointPicker::~PointPicker() {}

QSize PointPicker::minimumSizeHint() const { return QSize(80, 80); }

QSize PointPicker::sizeHint() const { return QSize(150, 150); }

// Selects the point with the given indices in the control point array
void PointPicker::selectPoint(int index, int jndex) {
  selectedPoint = controlPoints.at(index).at(jndex);
  updateGL();
}

void PointPicker::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glViewport(0, 0, editorWidth, editorHeight);

  for (unsigned int i = 0; i < controlPoints.size(); i++) {
    for (unsigned int j = 0; j < controlPoints.at(i).size(); j++) {

      // Draw all the points, with the selected point in a different colour
      if (controlPoints.at(i).at(j) == selectedPoint)
        glColor3f(1 - pointColour.r, pointColour.g, 1 - pointColour.b);
      else
        glColor3f(pointColour.r, pointColour.g, pointColour.b);
      controlPoints.at(i).at(j)->draw(10);
    }
  }
  glFlush();
}

void PointPicker::initializeGL() {
  QGLWidget::makeCurrent();

  glClearColor(bgColour.r, bgColour.g, bgColour.b,
               0); // Clear to the background colour

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_POINT_SMOOTH);

  selectedPoint = controlPoints.at(0).at(0);
}

// Initialize a 4x4 array of control points
void PointPicker::init() {
  for (int x = -9; x <= 9; x += 6) {
    controlPoints.push_back(vector<Point *>());
    for (int y = 9; y >= -9; y -= 6) {
      controlPoints.at((x + 9) / 6)
          .push_back(new Point((double)x / 11.0, (double)y / 11.0, 0));
    }
  }
}

void PointPicker::resizeGL(int w, int h) {
  editorWidth = w;
  editorHeight = h;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, editorWidth, editorHeight);
  if (editorWidth > editorHeight) { // Keep the texture square even if the
                                    // editor window is sized to be uneven
    ratio = (double)editorWidth / (double)editorHeight;
    glOrtho(-ratio, ratio, -1, 1, -1, 1);
  } else {
    ratio = (double)editorHeight / (double)editorWidth;
    glOrtho(-1, 1, -ratio, ratio, -1, 1);
  }
  glMatrixMode(GL_MODELVIEW);
}

void PointPicker::mousePressEvent(QMouseEvent *event) {
  updateMousePosition(event->x(), event->y());
  if (event->button() == Qt::LeftButton) {
    double minDistance = 10;
    double distance;
    for (unsigned int i = 0; i < controlPoints.size(); i++) {
      for (unsigned int j = 0; j < controlPoints.at(i).size(); j++) {
        distance = controlPoints.at(i).at(j)->distanceTo(mouseRel);
        if ((distance < 0.2) &&
            (distance < minDistance)) { // Select the point if it is close
                                        // enough to the mouse
          selectedPoint = controlPoints.at(i).at(j);
          emit(pointSelected(i, j));
          minDistance = distance;
        }
      }
    }
  }
  updateGL();
}

void PointPicker::mouseReleaseEvent(QMouseEvent *) {}

void PointPicker::mouseMoveEvent(QMouseEvent *) {}

// Updates the mouse position in screen coordinates as well as recalculating the
// relative coordinates
void PointPicker::updateMousePosition(int x, int y) {
  mouseX = x;
  mouseY = y;
  if (editorWidth > editorHeight) { // calculate the mouse position
    mouseRel.setX(((((x * 1.0) / editorWidth) * 2.0) - 1) * ratio);
    mouseRel.setY((((y * -1.0) / editorHeight) * 2.0) + 1);
  } else {
    mouseRel.setX((((x * 1.0) / editorWidth) * 2.0) - 1);
    mouseRel.setY(((((y * -1.0) / editorHeight) * 2.0) + 1) * ratio);
  }
}
