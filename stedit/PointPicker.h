/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include <QGLWidget>
#include <QMouseEvent>
#include <QImage>
#include "Colour.h"
#include "Point.h"
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

class PointPicker : public QGLWidget {
  Q_OBJECT
public:
  PointPicker(QWidget *parent = 0);
  ~PointPicker();

  QSize minimumSizeHint() const;
  QSize sizeHint() const;

public slots:
  void selectPoint(int index, int jndex);

signals:
  void pointSelected(int index, int jndex);
  void contactPointSelected();

protected:
  void paintGL();
  void initializeGL();
  void resizeGL(int w, int h);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);

private:
  void init();
  void updateMousePosition(int x, int y);

  int editorWidth;      // The width of the editor in screen coordinates
  int editorHeight;     // The height of the editor in screen coordinates
  int mouseX;           // Mouse X position in screen coordinates
  int mouseY;           // Mouse Y position in screen coordinates
  Point mouseRel;       // Mouse position relative to the gl coordinates
  double ratio;         // Screen ratio for resizing
  Colour bgColour;      // Background colour for the editor
  Colour pointColour;   // Point colour for the editor
  Point *selectedPoint; // The point that is currently selected

  vector<vector<Point *>> controlPoints; // List of control points in the scene
};
