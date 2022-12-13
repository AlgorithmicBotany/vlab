/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "Trackball.h"
#ifdef __APPLE__

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#else

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#endif
Trackball::Trackball() {

  rotMatrix[0] = 1.0f; // Initialize the rotation matrix to identity
  rotMatrix[5] = 1.0f;
  rotMatrix[10] = 1.0f;
  rotMatrix[15] = 1.0f;

  scale = 1.0f;
  this->pressed = false;
}

Trackball::Trackball(double scale) {

  rotMatrix[0] = 1.0f; // Initialize the rotation matrix to identity
  rotMatrix[5] = 1.0f;
  rotMatrix[10] = 1.0f;
  rotMatrix[15] = 1.0f;

  this->scale = scale;
  this->pressed = false;
}

// Indicate that the trackball is being pressed if it is not already, and
// calculate the initial point
void Trackball::press(Point point) {
  if (!pressed) {
    previousPoint.setX(point.X()); // Remember the point that was pressed
    previousPoint.setY(-point.Y());
    pressed = true;
  }
}

// Indicate that the trackball is no longer being pressed
void Trackball::release() { pressed = false; }

// Calculates the trackball's rotation matrix when the mouse is moved across it
void Trackball::move(Point point) {
  if (pressed) {
    calculateMatrix(point); // Calculate the new matrix based on the new point
                            // the mouse has been dragged to

    previousPoint.setX(point.X()); // Calculate the new previous point
    previousPoint.setY(-point.Y());
  }
}

// Resets the trackball's rotation matrix to identity
void Trackball::reset() {
  for (int i = 0; i < 16; i++) {
    if (i == 0 || i == 5 || i == 10 || i == 15)
      rotMatrix[i] = 1.0f;
    else
      rotMatrix[i] = 0.0f;
  }
}

// Returns the rotation matrix of the trackball
float *Trackball::getMatrix() { return rotMatrix; }

// Returns the current point of the trackball
Vector3 Trackball::getPoint() { return spherePoint; }

// Calculate the trackball's rotation matrix from its current state
void Trackball::calculateMatrix(Point point) {

  spherePoint.setX(point.X()); // Calculate the current point
  spherePoint.setY(-point.Y());

  Vector3 rot = spherePoint - previousPoint;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix(); // save the old matrix so we don't mess anything up
  glLoadIdentity();
  glRotatef(rot.Y() * scale, 1, 0, 0); // X rotation
  glRotatef(rot.X() * scale, 0, 1, 0); // Y rotation
  glMultMatrixf(rotMatrix);          // our previous rotation matrix
  glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)rotMatrix);
  glPopMatrix(); // return modelview to its old value;
}
