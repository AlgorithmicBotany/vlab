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
#include "Surface.h"
#include "Colour.h"
#include "Point.h"
#include "Trackball.h"

class SurfaceViewport;

class SurfaceViewport : public QGLWidget {
  Q_OBJECT
public:
  SurfaceViewport(Surface *surface, Trackball *trackball, QWidget *parent = 0,
                  Colour borderColour = Colour());
  ~SurfaceViewport();

  QSize minimumSizeHint() const;
  QSize sizeHint() const;


  void centerCamera();
  void resetView();
  void update();

signals:
  void trackballMoved();

public slots:
  void setCurrentPatch(int index);

protected:
  void paintGL();
  void initializeGL();
  void resizeGL(int w, int h);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);

private:
  void updateMousePosition(int x, int y);
  Point getScreenPosition(int x, int y);
  void drawBorder();

  Point camPos;      // Position of the camera
  Point camLook;     // Point that the camera is looking at
  Vector3 camUp;     // Up vector for the camera
  Point centerPoint; // Center point of the model, in order to translate to the
                     // origin
  int currentPatch;

  int editorWidth;  // The width of the editor in screen coordinates
  int editorHeight; // The height of the editor in screen coordinates
  int mouseX;       // Mouse X position in screen coordinates
  int mouseY;       // Mouse Y position in screen coordinates
  double ratio;     // Screen ratio for resizing

  Colour bgColour;     // Background colour for the editor
  Colour borderColour; // The colour of the border

  Surface *surface;       // The bezier surface
  int subdivisionSamples; // The amount to subdivide the bezier surface

  Trackball *trackball; // The trackball for rotating the surface
};
