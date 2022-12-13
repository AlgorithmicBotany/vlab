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
#include <QGLFramebufferObject>
#include <QImage>
#include "Globals.h"
#include "Triangle.h"
#include "Point.h"
#include "Vector3.h"
#include "TextureWindow.h"
#include "ResizeImageDialog.h"
#include "Colour.h"
#include "Globals.h"
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>
#include <iostream>

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

using namespace std;

class TextureWindow;

class TextureEditor : public QGLWidget {
  Q_OBJECT
public:
  TextureEditor(QWidget *parent = 0);
  ~TextureEditor();

  QSize minimumSizeHint() const;
  QSize sizeHint() const;

  void reset();
  void preload(string filename);
  void load(string filename);
  void load(QImage image);
  void undo();
  void redo();
  void resizeTexture();
  void capture();
  void saveTexture(string filename);
  void saveTexture();
  QImage getImage();
  void reloadTexture();
  void saveProject(string filename);
  void saveProject();
  void loadProject(string filename);
  void reloadProject();
  void resetBgColour();
  void changeBgColour(Colour colour);
  void changePointColour(Colour colour);
  void changeLineColour(Colour colour);
  void showHidePoints(bool value);
  void showHideLines(bool value);
  Colour getBgColour();
  Colour getPointColour();
  Colour getLineColour();
  void clearFilename();
  void clearProjectFilename();
  bool noFilename();
  bool noProjectFilename();
  bool unsavedChanges;
  double getPointSize();
  double getLineWidth();
  void setPointSize(int size);
  void setLineWidth(int width);
  void resetView();
  void setLinearInterpolation(bool value);
  QString getFilename();
  void setSavingMode(SavingMode mode) { _savingMode = mode; }

signals:

  void canUndo(bool);
  void canRedo(bool);
  void canRevert(bool);
  void sendTexture(QImage);
  void textureLoaded(string);
  void currentOpenFile(string);
  void continuousSave();

public slots:
  void getImageSize(int width, int height);
  void rotateCW();
  void rotateCCW();
  void flipH();
  void flipV();

protected:
  void paintGL();
  void initializeGL();
  void resizeGL(int w, int h);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);

private:
  void init();
  void loadImage(const char *filename);
  void saveUndoData();
  void saveRedoData();
  void addPoint(Point *newPoint);
  void updateMousePosition(int x, int y);
  Point getClosestPointToOppositeEdge(Point *point);
  string getDir(string fileStr);
  Point *findPoint(Point point);
  void changed();
  void resetProjection();
  QImage getFrameBuffer();

  bool leftButtonDown; // Specifies whether the left mouse button is being held
                       // down
  bool pointMoved;     // Indicates whether a point has been moved
  int editorWidth;     // The width of the editor in screen coordinates
  int editorHeight;    // The height of the editor in screen coordinates
  int mouseX;          // Mouse X position in screen coordinates
  int mouseY;          // Mouse Y position in screen coordinates
  Point mouseRel;      // Mouse position relative to the gl coordinates
  double ratio;        // Screen ratio for resizing
  double imageRatio;   // Ratio of the image dimensions
  Point *currentPoint; // The currently selected point
  GLuint tex;          // Current texture being edited
  Colour bgColour;     // Background colour for the editor
  Colour defBgColour;  // Default background colour for the editor
  Colour pointColour;  // Point colour for the editor
  Colour lineColour;   // Line colour for the editor
  int pointSize;       // Size that the points will be displayed at
  int lineWidth;       // Width that the lines will be displayed at
  bool showPoints;     // Specifies whether the points should be shown
  bool showLines;      // Specifies whether the lines should be shown
  bool linearInterpolation; // Specifies whether or not to use linear
                            // interpolation when displaying textures

  QGLFramebufferObject *fbo;

  QSize resizeResult;
  bool resized;
  bool captured;

  string filename; // The name of the texture to save over when saving normally
  string projectFilename;   // The name of the project to save over when saving
                            // normally
  string loadedTextureName; // The name of the last loaded texture for project
                            // saving

  vector<Point *> points;       // List of points in the scene
  vector<Triangle *> triangles; // List of triangles in the scene

  vector<vector<Point>>
      undoStackPoints; // List of old lists of points for undoing
  vector<vector<Triangle>>
      undoStackTriangles; // List of old lists of triangles for undoing
  vector<vector<Point>>
      redoStackPoints; // List of old lists of points for redoing
  vector<vector<Triangle>>
      redoStackTriangles; // List of old lists of triangles for redoing

  SavingMode _savingMode;
};
