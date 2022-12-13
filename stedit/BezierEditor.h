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
#include "Globals.h"
#include "Surface.h"
#include "Colour.h"
#include "Point.h"
#include "TextureWindow.h"
#include "Trackball.h"
#include "QImageFormatStrings.h"
#include <stdlib.h>
#include <math.h>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

class BezierWindow;

class BezierEditor : public QGLWidget {
  Q_OBJECT
public:
  BezierEditor(QWidget *parent = 0);
  ~BezierEditor();
  void forceExtendedFormat();

  QSize minimumSizeHint() const;
  QSize sizeHint() const;
  bool initSurfaceFilename(string filename);
  bool initTextureFilename(string filename);
  void reset();
  bool loadTexture(string filename);
  bool load(string filename);
  bool reload();
  void save(string filename);
  void save();
  void resetView();
  void setWireframe(bool value);
  void showHidePoints(bool value);
  void showHideLines(bool value);
  void showHideSurface(bool value);
  void showHideVectors(bool value);
  Colour getBgColour();
  Colour getPointColour();
  Colour getSelectedPointColour();
  Colour getLineColour();
  Colour getWireframeColour();
  Colour getVectorColour();
  Colour getSelectedPatchColour();
  Colour getPatchColour();
  void resetBgColour();
  void changeBgColour(Colour colour);
  void changePointColour(Colour colour);
  void changeSelectedPointColour(Colour colour);
  void changeLineColour(Colour colour);
  void changeWireframeColour(Colour colour);
  void changeVectorColour(Colour colour);
  void changeSelectedPatchColour(Colour colour);
  void changePatchColour(Colour colour);
  double getPointSize();
  double getContactPointSize();
  double getLineWidth();
  double getWireframeWidth();
  int getSubdivisionSamples();
  void setPointSize(double value);
  void setContactPointSize(double value);
  void setLineWidth(double value);
  void setWireframeWidth(double value);
  void setSubdivisionSamples(int value);
  string getTextureName();
  QImage getCurrentTexture();
  void saveCurrentTexture();
  void restoreTexture();
  bool noCurrentTexture();
  void reloadTexture();
  void reloadCurrentTexture();
  void centerCamera();
  void centerAtContact(bool value);
  void clearSurfaceFilename();
  bool noSurfaceFilename();
  bool unsavedChanges;
  Point getContactPoint();
  Point getEndPoint();
  Vector3 getHeading();
  Vector3 getUp();
  double getSize();
  void flipPatchHorizontal();
  void flipPatchVertical();
  void flipPatchDepth();
  int getSelectedPointI();
  int getSelectedPointJ();
  bool hasTexture();
  void selectPatch(int index);
  void changePatchName(string name, int index);
  void changePatchVisibility(bool visibility, int index);
  void addNewPatch(int shape);
  void deleteSelectedPatch();
  void duplicateSelectedPatch();
  QStringList getPatchNames();
  void resetTranslation();
  void resetRotation();
  void resetScale();
  Surface *getSurfaceLoc();
  void saveUndoState();
  void setLinearInterpolation(bool value);
  void setProjection(bool value);
  void setSavingMode(SavingMode mode) { _savingMode = mode; }

signals:
  void selectedPointX(double x);
  void selectedPointY(double y);
  void selectedPointZ(double z);
  void pointSelected(int i, int j);
  void contactPointSelected();
  void contactCenteredChanged(bool value);
  void canUndo(bool value);
  void canRedo(bool value);
  void canRevert(bool value);
  void addPatchToList(string name);
  void newSurface();
  void updatePatchSelection(int index);
  void updateContactPoint(Point point);
  void continuousSave();

public slots:
  void selectPoint(int index, int jndex);
  void selectContactPoint();
  void saveXYZValue();
  void setXValue(double value);
  void setYValue(double value);
  void setZValue(double value);
  void undo();
  void redo();
  void updateTexture(QImage image);
  void updateTextureName(string filename);
  void setSymmetricalEditing(bool axis14, bool axisX, bool value);
  void setContactPoint(Point point);
  void setEndPoint(Point point);
  void setHeading(Vector3 vector);
  void setUp(Vector3 vector);
  void setSize(double value);
  void translate(Vector3 translation);
  void rotate(Vector3 axis, double angle);
  void scale(Vector3 factor);
  void setRotationLocked(bool value);
  void boxEdited();
  void setEnforceAdjacency(bool value);
  void updateAdjacencies();
  void rotateTextureCW();
  void rotateTextureCCW();
  void flipTextureH(bool value);
  void flipTextureV(bool value);

protected:
  void paintGL();
  void initializeGL();
  void resizeGL(int w, int h);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);
  void keyPressEvent(QKeyEvent *event);

private:
  void init();
  void updateMousePosition(int x, int y);
  Point getScreenPosition(int x, int y);
  bool loadImage(const char *filename);
  void changed();
  void saveUndoData();
  void saveRedoData();
  void numberPoints();
  void drawAxes();

  Point camPos;      // Position of the camera
  Point camLook;     // Point that the camera is looking at
  Vector3 camUp;     // Up vector for the camera
  Point centerPoint; // Center point of the model, in order to translate to the
                     // origin

  string surfaceFilename; // The name of the project to save over when saving
                          // normally
  string textureName;     // The name of the last loaded texture
  QImage currentTexture; // The current texture in QImage format, for use in the
                         // case that the current texture isn't actually saved
                         // anywhere externally
  QImage revertTexture;  // Use this if the user discards changes in the editor

  int editorWidth;  // The width of the editor in screen coordinates
  int editorHeight; // The height of the editor in screen coordinates
  int mouseX;       // Mouse X position in screen coordinates
  int mouseY;       // Mouse Y position in screen coordinates
  float mouseZ;    // The value of the depth buffer at the position of the mouse
  double xrot;     // The x rotation of the surface
  double yrot;     // The y rotation of the surface
  double ratio;    // Screen ratio for resizing
  GLuint tex;      // Current texture being edited
  Colour bgColour; // Background colour for the editor
  Colour defBgColour;         // Default background colour for the editor
  Colour pointColour;         // Point colour for the editor
  Colour selectedPointColour; // Selected point colour for the editor
  Colour lineColour;          // Line colour for the editor
  Colour wireframeColour;     // Wireframe colour for the editor
  Colour vectorColour;        // Vector colour for the editor
  Colour patchColour;         // Patch colour for the editor
  Colour selectedPatchColour; // Selected patch colour for the editor
  double pointSize;           // Size that the points will be displayed at
  double contactPointSize; // Size that the contact point will be displayed at
  double lineWidth;        // Width that the lines will be displayed at
  double wireframeWidth;   // Width that the wireframe will be displayed at
  int textureRotation;     // The angle by which to rotate the texture
  bool textureFlippedH; // Specifies whether the texture is flipped horizontally
                        // or not
  bool textureFlippedV; // Specifies whether the texture is flipped vertically
                        // or not

  double model[16]; // The modelview matrix, in column major order as an array
                    // of 16 elements for use by openGL
  double proj[16];  // The projection matrix, in column major order as an array
                    // of 16 elements for use by openGL
  int view[4];      // The viewport information for use by openGL

  bool showPoints;  // Specifies whether the points should be shown
  bool showLines;   // Specifies whether the lines should be shown
  bool showSurface; // Specifies whether the surface should be shown
  bool showVectors; // Specifies whether the vectors should be shown
  bool wireframe; // Specifies whether the surface should be viewed in wireframe
  bool symmetrical14Y;  // Specifies whether points in the 1-4 direction should
                        // be moved symmetrically in the X axis or not
  bool symmetrical14X;  // Specifies whether points in the 1-4 direction should
                        // be moved symmetrically in the Y axis or not
  bool symmetrical113X; // Specifies whether points in the 1-13 direction should
                        // be moved symmetrically in the X axis or not
  bool symmetrical113Y; // Specifies whether points in the 1-13 direction should
                        // be moved symmetrically in the Y axis or not
  bool centerContact;   // Specifies whether or not to center the camera around
                        // the contact point of the surface
  bool hasCurrentTexture; // Specifies whether or not the surface has a current
                          // texture saved for future use
  bool enforceAdjacency;  // Specifies whether the edges of faces specified as
                          // adjacent should be snapped together
  bool rotationLocked; // Specifies whether or not rotation of the trackball is
                       // currently disabled
  bool dragPoint; // Specifies whether or not a point is currently selected for
                  // dragging
  bool dragContact; // Specifies whether or not the contact point is currently
                    // selected for dragging
  bool contactSelected;     // Specifies whether the contact point is marked as
                            // selected, rather than a point in the surface
  bool extendedFormat;      // Specifies whether or not to use the extended file
                            // format when saving surfaces
  bool linearInterpolation; // Specifies whether or not to use linear
                            // interpolation when displaying textures
  bool parallelProjection;  // Specifies whether or not to use a parallel
                            // projection

  Vector3 previousTranslation; // The previously used translation, saved so that
                               // it can be undone on cancel
  Vector3 previousRotationAxis; // The axis of the previous rotation, saved so
                                // that it can be undone on cancel
  double previousRotationAngle; // The angle of the previous rotation, saved so
                                // that it can be undone on cancel
  Vector3 previousScale; // The previously used scale vector, saved so that it
                         // can be undone on cancel

  Surface surface;        // The bezier surface
  int subdivisionSamples; // The amount to subdivide the bezier surface

  vector<Surface> undoStack; // List of old surfaces for undoing
  vector<Surface> redoStack; // List of old surfaces for redoing

  Trackball trackball; // The trackball for rotating the surface

  SavingMode _savingMode;

  //QGLFramebufferObject *fbo;

};
