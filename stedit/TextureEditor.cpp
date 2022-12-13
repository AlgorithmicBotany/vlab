/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "TextureEditor.h"

#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>

TextureEditor::TextureEditor(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::AlphaChannel), parent) {

  showLines = true;
  showPoints = true;
  ratio = 1.0;
  imageRatio = 1.0;
  editorWidth = 600;
  editorHeight = 600;
  bgColour = Colour();
  defBgColour = bgColour;
  pointColour = Colour(1, 0, 0);
  lineColour = Colour(1, 0, 0);
  pointSize = DEFAULT_POINT_SIZE;
  lineWidth = DEFAULT_LINE_WIDTH;

  currentPoint = 0;
  unsavedChanges = false;
  pointMoved = false;
  resized = false;
  linearInterpolation = true;
  captured = false;

  fbo = 0;
}

TextureEditor::~TextureEditor() {
  glDeleteTextures(1, &tex);
  if (fbo != 0)
    delete fbo;
}

QSize TextureEditor::minimumSizeHint() const { return QSize(100, 100); }

QSize TextureEditor::sizeHint() const { return QSize(600, 600); }

void TextureEditor::paintGL() {  
  QGLWidget::makeCurrent();
  glMatrixMode(GL_MODELVIEW);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Background checkerboard
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  if (fbo != 0 && !fbo->isBound()) {
    if (bgColour.r + bgColour.g + bgColour.b < 1)
      glColor3f(bgColour.r + 0.05, bgColour.g + 0.05, bgColour.b + 0.05);
    else
      glColor3f(bgColour.r - 0.05, bgColour.g - 0.05, bgColour.b - 0.05);
    double boundary, wInc, hInc;

    if (imageRatio < 1)
      boundary = ratio / imageRatio;
    else
      boundary = ratio * imageRatio;
    // boundary = 1;
    glBegin(GL_QUADS);
    if (editorWidth > editorHeight) {
      wInc = (10.0 / (double)editorWidth) * ratio;
      hInc = 10.0 / (double)editorHeight;
    } else {
      wInc = 10.0 / (double)editorWidth;
      hInc = (10.0 / (double)editorHeight) * ratio;
    }
    for (double x = -boundary; x < boundary; x += wInc * 2) {
      for (double y = -boundary; y < boundary; y += hInc * 2) {
        glVertex2f(x, y + hInc);
        glVertex2f(x + wInc, y + hInc);
        glVertex2f(x + wInc, y);
        glVertex2f(x, y);
        glVertex2f(x, y - hInc);
        glVertex2f(x - wInc, y - hInc);
        glVertex2f(x - wInc, y);
        glVertex2f(x, y);
      }
    }
    glEnd();
  }

  glLineWidth(1.0);         // Border
  glColor3f(0.5, 0.5, 0.5); // 50% Grey
  glBegin(GL_LINE_LOOP);
  if (imageRatio < 1) {
    glVertex2f((WIN_SIZE + 0.005) * imageRatio,
               WIN_SIZE + 0.005); // Corner points
    glVertex2f((-WIN_SIZE - 0.005) * imageRatio, WIN_SIZE + 0.005);
    glVertex2f((-WIN_SIZE - 0.005) * imageRatio, -WIN_SIZE - 0.005);
    glVertex2f((WIN_SIZE + 0.005) * imageRatio, -WIN_SIZE - 0.005);
  } else {
    glVertex2f(WIN_SIZE + 0.005,
               (WIN_SIZE + 0.005) / imageRatio); // Corner points
    glVertex2f(-WIN_SIZE - 0.005, (WIN_SIZE + 0.005) / imageRatio);
    glVertex2f(-WIN_SIZE - 0.005, (-WIN_SIZE - 0.005) / imageRatio);
    glVertex2f(WIN_SIZE + 0.005, (-WIN_SIZE - 0.005) / imageRatio);
  }
  glEnd();
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);

  for (unsigned int i = 0; i < triangles.size(); i++) { // Draw all the triangles
    glColor3f(1, 1, 1);
    triangles.at(i)->draw();
    if (showLines &&
        ((fbo != 0 && !fbo->isBound()) ||
         fbo == 0)) { // Draw tringle borders if lines are to be shown
      glColor3f(lineColour.r, lineColour.g, lineColour.b);
      triangles.at(i)->drawLines(lineWidth);
    }
  }
  if (showPoints && ((fbo != 0 && !fbo->isBound()) ||
                     fbo == 0)) { // Draw all the points if they are to be shown
    glDisable(
        GL_TEXTURE_2D); // Turn stuff off so that the points are drawn correctly
    glDisable(GL_BLEND);
    for (unsigned int i = 0; i < points.size();
         i++) { // Draw each point and highlight the selected one
      if (points.at(i) == currentPoint)
        glColor3f(1 - pointColour.r, pointColour.g, 1 - pointColour.b);
      else
        glColor3f(pointColour.r, pointColour.g, pointColour.b);
      const int retinaScale = devicePixelRatio();

      points.at(i)->draw(pointSize*retinaScale);
    }
    glEnable(GL_BLEND); // Turn stuff back on
    glEnable(GL_TEXTURE_2D);
  }
}

void TextureEditor::initializeGL() {
  QGLWidget::makeCurrent();

  glClearColor(bgColour.r, bgColour.g, bgColour.b,
               0); // Clear to the background colour

  glEnable(GL_TEXTURE_2D);

  glGenTextures(1, &tex);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Set up GL stuff
  glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_POINT_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  init();
}

void TextureEditor::resizeGL(int w, int h) {
  QGLWidget::makeCurrent();

  editorWidth = w;
  editorHeight = h;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, editorWidth, editorHeight);
  if (editorWidth > editorHeight) { // Keep the texture square even if the
                                    // editor window is sized to be uneven
    ratio = (double)editorWidth / (double)editorHeight;
    if (imageRatio < 1) {
      glOrtho(-ratio, ratio, -1, 1, -1, 1);
    } else {
      glOrtho(-ratio / (min(imageRatio, ratio)),
              ratio / (min(imageRatio, ratio)), -1 / (min(imageRatio, ratio)),
              1 / (min(imageRatio, ratio)), -1, 1);
    }
  } else {
    ratio = (double)editorHeight / (double)editorWidth;
    if (imageRatio < 1) {
      glOrtho(-1 * (max(imageRatio, 1 / ratio)),
              1 * (max(imageRatio, 1 / ratio)),
              -ratio * (max(imageRatio, 1 / ratio)),
              ratio * (max(imageRatio, 1 / ratio)), -1, 1);
    } else {
      glOrtho(-1, 1, -ratio, ratio, -1, 1);
    }
  }
  glMatrixMode(GL_MODELVIEW);
}

void TextureEditor::resetProjection() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, editorWidth, editorHeight);
  if (editorWidth > editorHeight) { // Keep the texture square even if the
                                    // editor window is sized to be uneven
    if (imageRatio < 1) {
      glOrtho(-ratio, ratio, -1, 1, -1, 1);
    } else {
      glOrtho(-ratio / (min(imageRatio, ratio)),
              ratio / (min(imageRatio, ratio)), -1 / (min(imageRatio, ratio)),
              1 / (min(imageRatio, ratio)), -1, 1);
    }
  } else {
    if (imageRatio < 1) {
      glOrtho(-1 * (max(imageRatio, 1 / ratio)),
              1 * (max(imageRatio, 1 / ratio)),
              -ratio * (max(imageRatio, 1 / ratio)),
              ratio * (max(imageRatio, 1 / ratio)), -1, 1);
    } else {
      glOrtho(-1, 1, -ratio, ratio, -1, 1);
    }
  }
  glMatrixMode(GL_MODELVIEW);
}

void TextureEditor::mousePressEvent(QMouseEvent *event) {
  updateMousePosition(event->x(), event->y());
  if (event->button() == Qt::LeftButton &&
      (event->modifiers() &
       Qt::ControlModifier)) { // Add a new point on right click
    addPoint(new Point(mouseRel.X(), mouseRel.Y()));
  } else if (event->button() ==
             Qt::LeftButton) { // Select a point for draging on left click
    leftButtonDown = true;     // Remember that the button is down
    double minDistance = 10;   // The window is only -1 to 1 so this will be
                               // higher than the shortest distance
    double distance;
    for (unsigned int i = 0; i < points.size(); i++) {
      distance = points.at(i)->distanceTo(mouseRel);
      if ((distance < pointSize / 200.0) &&
          (distance <= minDistance)) { // Select the point if it is close enough
                                       // to the mouse
        currentPoint = points.at(i);
        minDistance = distance;
      }
    }
    saveUndoData(); // Save undo data when a point is selected, in case the
                    // point is actually moved. This will be thrown away if the
                    // point is not moved
  }
  updateGL();
}

void TextureEditor::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    if (!pointMoved && currentPoint != 0) {
      undo(); // If a point was selected, but not moved, the last undo state is
              // redundant. Throw it away.
    }
    pointMoved = false; // Reset for next time
    leftButtonDown = false;
    currentPoint = 0; // Deselect when the mouse is released
    if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
      saveTexture();
      emit continuousSave();
    }

    updateGL();
  }
}

void TextureEditor::mouseMoveEvent(QMouseEvent *event) {
  if (leftButtonDown &&
      currentPoint !=
          0) { // Drag while the left button is down and a point is selected
    updateMousePosition(event->x(), event->y());
    *currentPoint = mouseRel;
    pointMoved =
        true; // Remember that a point was moved so that the undo data is kept
    for (unsigned int i = 0; i < triangles.size(); i++) {
      if (triangles.at(i)->containsPoint(mouseRel) &&
          !triangles.at(i)->containsVertex(
              currentPoint)) { // Can't drag a point into a neighbouring
                               // triangle

        *currentPoint = getClosestPointToOppositeEdge(
            currentPoint); // Snap the point to the edge of the triangle instead
                           // of moving it outside
        break;
      }
    }
    for (unsigned int i = 0; i < triangles.size(); i++) {
      for (unsigned int j = 0; j < points.size(); j++) {
        if (triangles.at(i)->containsPointExcludingEdges(*points.at(j)) &&
            !triangles.at(i)->containsVertex(points.at(j))) {
          // If moving this point would overlap with any other points, move them
          // too This appears in the program as pushing the other points out of
          // the way if the move would otherwise be illegal
          *points.at(j) = getClosestPointToOppositeEdge(points.at(j));
        }
      }
    }
    changed();

    if (_savingMode == CONTINUOUS) {
      saveTexture();
      emit continuousSave();
    }

    updateGL();
  }
}

// Loads a texture with the given filename into the given GL texture
void TextureEditor::loadImage(const char *filename) {
  QGLWidget::makeCurrent();
  QImage image;

  bool success = image.load(filename);
  if (!success) { // If the texture was not found send an error message...
    QMessageBox::critical(
        this, "Error", QString("Cannot load texture file ").append(filename));
    return;
  }


  fbo = new QGLFramebufferObject(image.width(), image.height(),
                                 QGLFramebufferObject::Depth);
  resized = false;
  captured = false;

  imageRatio = (double)image.width() / (double)image.height();
      //PASCAL: For some reason the image is flipped and rotated, so we flip adn rotate it back
  /*
  image = image.transformed(QMatrix().scale(1, -1));
  image = image.transformed(QMatrix().scale(-1, 1));
  image = image.transformed(QMatrix().rotate(-90));
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
  resetProjection();
}

// Initialize the corner points and starting triangles
void TextureEditor::init() {
  points.clear();
  triangles.clear();
  Point *a, *b, *c, *d;

  if (imageRatio < 1) {
    a = new Point((WIN_SIZE)*imageRatio, WIN_SIZE);
    b = new Point((-WIN_SIZE) * imageRatio, WIN_SIZE);
    c = new Point((-WIN_SIZE) * imageRatio, -WIN_SIZE);
    d = new Point((WIN_SIZE)*imageRatio, -WIN_SIZE);
  } else {
    a = new Point(WIN_SIZE, (WIN_SIZE) / imageRatio);
    b = new Point(-WIN_SIZE, (WIN_SIZE) / imageRatio);
    c = new Point(-WIN_SIZE, (-WIN_SIZE) / imageRatio);
    d = new Point(WIN_SIZE, (-WIN_SIZE) / imageRatio);
  }

  points.push_back(a);
  points.push_back(b);
  points.push_back(c);
  points.push_back(d);
  triangles.push_back(new Triangle(a, b, c, imageRatio));
  triangles.push_back(new Triangle(a, c, d, imageRatio));
}

// Resets the list of points and triangles to their initial state
void TextureEditor::reset() {
  QGLWidget::makeCurrent();
  init();

  undoStackPoints
      .clear(); // Can't undo or redo after this, it would just mess up
  undoStackTriangles.clear();
  redoStackPoints.clear();
  redoStackTriangles.clear();

  changed();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
    saveTexture();
    emit continuousSave();
  }

  updateGL();
}

// Loads a texture on startup
void TextureEditor::preload(string filename) {
  bool supported =
      QImageFormatStrings::isSupportedFormat(QString(filename.c_str()));
  if (!supported) { // If the file type of texture is not supported
     qDebug() << QString("File type of ")
                    .append(filename.c_str())
                    .append(" is not supported")
                    .toStdString()
                    .c_str();
    return;
  }
  loadedTextureName = filename; // Remember which texture is open
  emit(textureLoaded(filename));
  this->filename = filename;
  emit(currentOpenFile(filename));
  loadImage(filename.c_str());
}

// Loads a new texture
void TextureEditor::load(string filename) {
  QGLWidget::makeCurrent();

  /*
  bool supported =
      QImageFormatStrings::isSupportedFormat(QString(filename.c_str()));
  
  if (!supported) { // If the texture was not found send an error message...
    qDebug() << QString("File type of ")
                    .append(filename.c_str())
                    .append(" is not supported")
                    .toStdString()
                    .c_str();
    return;
  }
  */
  clearFilename(); // Clear the filenames so that nothing will be overwritten on
                   // saving
  clearProjectFilename();
  loadedTextureName = filename; // Remember which texture is open
  emit(textureLoaded(filename));
  this->filename = filename;
  emit(currentOpenFile(filename));
  loadImage(filename.c_str());
  changed();
  reset();
}

// Loads a new texture
void TextureEditor::load(QImage image) {
  QGLWidget::makeCurrent();

  clearFilename(); // Clear the filenames so that nothing will be overwritten on
                   // saving
  clearProjectFilename();
  changed();
  /*
  image = image.transformed(QMatrix().scale(1, -1));
  image = image.transformed(QMatrix().scale(-1, 1));
  image = image.transformed(QMatrix().rotate(-90));
  */
  image = QGLWidget::convertToGLFormat(image);

  fbo = new QGLFramebufferObject(image.width(), image.height(),
                                 QGLFramebufferObject::Depth);
  resized = false;
  captured = false;

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
  reset();
}

// Saves the current triangle and point information into their respective undo
// stacks
void TextureEditor::saveUndoData() {
  vector<Point> undoPoints;
  vector<Triangle> undoTriangles;

  emit(canUndo(true)); // Let the window know that there is something to undo
  for (unsigned int i = 0; i < points.size(); i++) {
    undoPoints.push_back(
        *points.at(i)); // Dereferencce and remember the point data
  }
  for (unsigned int i = 0; i < triangles.size(); i++) {
    undoTriangles.push_back(
        *triangles.at(i)); // Dereferencce and remember the triangle data
    undoTriangles.at(i).saveValues(); // Remember the actual points in this
                                      // triangle, not just the pointers
  }

  undoStackPoints.push_back(
      undoPoints); // Save these states on their respective stacks
  undoStackTriangles.push_back(undoTriangles);

  if (undoStackPoints.size() >
      MAX_UNDOS) { // Throw away old states if their number exceeds the maximum
    undoStackPoints.erase(undoStackPoints.begin());
    undoStackTriangles.erase(undoStackTriangles.begin());
  }
}

// Saves the current triangle and point information into their respective redo
// stacks
void TextureEditor::saveRedoData() {
  vector<Point> redoPoints;
  vector<Triangle> redoTriangles;

  emit(canRedo(true)); // Let the window know that there is something to redo
  for (unsigned int i = 0; i < points.size(); i++) {
    redoPoints.push_back(
        *points.at(i)); // Dereferencce and remember the point data
  }
  for (unsigned int i = 0; i < triangles.size(); i++) {
    redoTriangles.push_back(
        *triangles.at(i)); // Dereferencce and remember the triangle data
    redoTriangles.at(i).saveValues(); // Remember the actual points in this
                                      // triangle, not just the pointers
  }

  redoStackPoints.push_back(
      redoPoints); // Save these states on their respective stacks
  redoStackTriangles.push_back(redoTriangles);
}

// Adds the given point to the set of points and does the appropriate triangle
// division/flipping
void TextureEditor::addPoint(Point *newPoint) {
  vector<Triangle *> result;
  bool foundSplit = false;
  saveUndoData(); // Remember the old state
  redoStackPoints
      .clear(); // Clear the redo information if something different is done
  redoStackTriangles.clear();
  for (unsigned int i = 0; i < triangles.size(); i++) {
    if (triangles.at(i)->containsPoint(
            *newPoint)) { // Find the triangle the point is in so it can be
                          // split up
      result = triangles.at(i)->splitAt(newPoint); // Get the new triangles
      triangles.erase(triangles.begin() + i);      // Erase the old triangle
      foundSplit = true;
      break; // There will only be one split, so remember this and stop looking
    }
  }
  if (!foundSplit)
    return; // Don't do anything if no triangle is available to split
  points.push_back(newPoint);

  // Delaunay Triangulation flipping
  Triangle *current;
  Triangle *opposite;
  for (unsigned int i = 0; i < result.size(); i++) {
    current = result.at(i);
    for (unsigned int j = 0; j < triangles.size(); j++) {
      if (triangles.at(j)->containsVertex(
              result.at(i)->get(2)) && // Get the triangle that borders the new
                                       // triangle opposite to the new point
          triangles.at(j)->containsVertex(result.at(i)->get(
              3))) { // Point a in the result triange represents the new point,
                     // so it is not checked

        opposite = triangles.at(j);

        int oppositeVertex =
            1; // Find the vertex that isn't shared between the two triangles
        if (opposite->get(oppositeVertex) == current->get(2) ||
            opposite->get(oppositeVertex) == current->get(3))
          oppositeVertex = 2;
        if (opposite->get(oppositeVertex) == current->get(2) ||
            opposite->get(oppositeVertex) == current->get(3))
          oppositeVertex = 3;

        // Locate the other vertices of the opposite triangle
        int vertex2 =
            1; // this one will be swapped if the edge needs to be flipped
        if (opposite->get(vertex2) != current->get(3))
          vertex2 = 2;
        if (opposite->get(vertex2) != current->get(3))
          vertex2 = 3;

        int vertex3 = 1; // This one is just for angle checking
        if (vertex3 == oppositeVertex || vertex3 == vertex2)
          vertex3 = 2;
        if (vertex3 == oppositeVertex || vertex3 == vertex2)
          vertex3 = 3;

        Vector3 vec1 = *opposite->get(vertex2) - *opposite->get(oppositeVertex);
        Vector3 vec2 = *opposite->get(vertex3) - *opposite->get(oppositeVertex);
        double angle1 = (vec1).angleBetween(
            vec2); // Calculate the angle on the opposite the shared line in the
                   // opposite triangle
        vec1 = *current->get(2) - *current->get(1);
        vec2 = *current->get(3) - *current->get(1);
        double angle2 = (vec1).angleBetween(
            vec2); // Calculate the corresponding angle in the current triangle
        if (angle1 + angle2 >
            PI) { // If the sum of these angles is greater than 180 degrees, the
                  // edge must be flipped
          current->set(
              2, opposite->get(oppositeVertex),
              opposite->getTexCoord(
                  oppositeVertex)); // Modify the triangles to reflect this flip
          opposite->set(vertex2, current->get(1), current->getTexCoord(1));
        }
      }
    }
    triangles.push_back(result.at(i));
  }
  changed();
}

QImage TextureEditor::getFrameBuffer() {
  fbo->bind();

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, fbo->size().width(), fbo->size().height());
  if (imageRatio < 1) {
    glOrtho(-WIN_SIZE * imageRatio, WIN_SIZE * imageRatio, -WIN_SIZE, WIN_SIZE,
            -1, 1);
  } else {
    glOrtho(-WIN_SIZE, WIN_SIZE, -WIN_SIZE / imageRatio, WIN_SIZE / imageRatio,
            -1, 1);
  }
  glMatrixMode(GL_MODELVIEW);
  paintGL(); // Do this instead of updateGL in order to properly hide the lines
             // and points

  fbo->release();

  QImage image =
      fbo->toImage(); // Copy the contents of the frame buffer into a new image

  resetProjection();

  return image;
}

// Overwrites the current GL texture with the contents of the screen buffer
// so that the current appearance is saved without the control points and new
// changes can be made
void TextureEditor::capture() {
  if (fbo != 0) {
    QGLWidget::makeCurrent();

    QImage image = getFrameBuffer();
    /*
    image = image.transformed(QMatrix().scale(1, -1));
    image = image.transformed(QMatrix().scale(-1, 1));
    image = image.transformed(QMatrix().rotate(-90));
    */
    image = QGLWidget::convertToGLFormat(image);
    // Make this the new texture
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
    captured = true;

    reset();
    changed();
  }
}

void TextureEditor::saveTexture() {
  saveTexture(filename); // Automatically save over as the saved filename if
                         // none is given
}

void TextureEditor::saveTexture(string filename) {
  if (fbo != 0) {
    QGLWidget::makeCurrent();

    unsavedChanges = false; // No unsaved changes if you just saved

    this->filename = filename;
    emit(currentOpenFile(filename));
    loadedTextureName = filename; // Remember which texture is open

    QImage image = getFrameBuffer();

    if (resized)
      image = image.scaled(resizeResult);
    image.save(filename.c_str()); // Save the image with the given filename

    updateGL();
  }
}

void TextureEditor::getImageSize(int width, int height) {
  resizeResult = QSize(width, height);
}

void TextureEditor::resizeTexture() {
  if (fbo != 0) {
    ResizeImageDialog *rid;
    double originalImageRatio;
    if (resized)
      rid = new ResizeImageDialog(resizeResult.width(), resizeResult.height(),
                                  this);
    else
      rid = new ResizeImageDialog(fbo->width(), fbo->height(), this);
    rid->setMaximumSize(1, 1); // As small as possible
    connect(rid, SIGNAL(result(int, int)), this, SLOT(getImageSize(int, int)));
    int ret = rid->exec();
    if (ret) {
      originalImageRatio = imageRatio;
      imageRatio = (double)resizeResult.width() / (double)resizeResult.height();
      for (unsigned int i = 0; i < points.size(); i++) {
        if (originalImageRatio < 1) {
	  double p = points.at(i)->X() / originalImageRatio;
          points.at(i)->setX(p);
        } else {
	  double p = points.at(i)->Y() * originalImageRatio;
          points.at(i)->setY(p);
        }

        if (imageRatio < 1){
	  double p = points.at(i)->X() * originalImageRatio;
          points.at(i)->setX(p);
        } else {
	  double p = points.at(i)->Y() / originalImageRatio;
          points.at(i)->setY(p);
        }
      }
      resetProjection();
      resized = true;
    }
  }
}

// Returns a capture as a QImage so that it can be passed to the bezier editor
QImage TextureEditor::getImage() {
  if (fbo != 0) {
    QGLWidget::makeCurrent();

    QImage image = getFrameBuffer();

    return image;
  }
  return QImage(1, 1, QImage::Format_ARGB32);
}

void TextureEditor::saveProject() {
  saveProject(projectFilename); // Automatically save over as the saved project
                                // filename if none is given
}

void TextureEditor::saveProject(string filename) {
  unsavedChanges = false; // No unsaved changes if you just saved
  emit(canRevert(false));
  projectFilename = filename;

  string saveDir = getDir(filename);
  // Get the directory of the texture relative to the project file's location
  if (loadedTextureName.length() >= saveDir.length() &&
      loadedTextureName.find(':') != std::string::npos &&
      loadedTextureName.substr(0, saveDir.length()) == saveDir) {
    loadedTextureName = loadedTextureName.substr(
        saveDir.length() + 1, loadedTextureName.length() - saveDir.length());
  }

  ofstream outFile;
  outFile.open(filename.c_str());

  outFile << "#Texture" << endl; // Save the name of the texture being used
  outFile << loadedTextureName << endl;

  for (unsigned int i = 0; i < points.size(); i++) {
    outFile << endl
            << "#Point"
            << endl; // Save the coordinates of all the control points
    outFile << points.at(i)->X() << "\t" << points.at(i)->Y() << endl;
  }

  for (unsigned int i = 0; i < triangles.size(); i++) {
    outFile << endl << "#Triangle" << endl;
    int indices[3]; // The indices of the points in the triangle
    for (unsigned int j = 0; j < points.size();
         j++) { // Find the indices of the triangle points in order so that they
                // match with their texture coordinates later
      if (triangles.at(i)->get(1) == points.at(j)) {
        indices[0] = j;
        break;
      }
    }
    for (unsigned int j = 0; j < points.size(); j++) {
      if (triangles.at(i)->get(2) == points.at(j)) {
        indices[1] = j;
        break;
      }
    }
    for (unsigned int j = 0; j < points.size(); j++) {
      if (triangles.at(i)->get(3) == points.at(j)) {
        indices[2] = j;
        break;
      }
    }
    outFile << indices[0] << "\t" << indices[1] << "\t" << indices[2]
            << endl; // Write the point indices and texture coordinates
    outFile << triangles.at(i)->getTexCoord(1).X() << "\t"
            << triangles.at(i)->getTexCoord(1).Y() << endl;
    outFile << triangles.at(i)->getTexCoord(2).X() << "\t"
            << triangles.at(i)->getTexCoord(2).Y() << endl;
    outFile << triangles.at(i)->getTexCoord(3).X() << "\t"
            << triangles.at(i)->getTexCoord(3).Y() << endl;
  }

  outFile << endl
          << "#BGColour" << endl; // Save the user's custom colour information
  outFile << bgColour.r << "\t" << bgColour.g << "\t" << bgColour.b << endl;
  outFile << endl << "#PointColour" << endl;
  outFile << pointColour.r << "\t" << pointColour.g << "\t" << pointColour.b
          << endl;
  outFile << endl << "#LineColour" << endl;
  outFile << lineColour.r << "\t" << lineColour.g << "\t" << lineColour.b
          << endl;
  outFile << endl << "#PointSize" << endl;
  outFile << pointSize << endl;
  outFile << endl << "#LineWidth" << endl;
  outFile << lineWidth << endl;

  outFile.close();
}

void TextureEditor::loadProject(string filename) {
  QGLWidget::makeCurrent();
  QFileInfo fi(QString(filename.c_str()));
  QString suf = fi.suffix();
  int supported = suf.compare(QString("dtx"));
  
  if (supported!=0) { // If the file format is invalid, send an error message
    QMessageBox::critical(this, "Error",
                          QString("File type of ")
                              .append(filename.c_str())
                              .append(" is not supported"));
    return;
  }
  
  unsavedChanges = false; // No unsaved changes if you just loaded
  emit(canRevert(false));
  projectFilename = filename;
  undoStackPoints
      .clear(); // Clear the undo and redo stacks when a file is loaded
  undoStackTriangles.clear();
  redoStackPoints.clear();
  redoStackTriangles.clear();

  ifstream inFile;
  inFile.open(filename.c_str());

  points.clear(); // Clear the previous information
  triangles.clear();

  double x, y;
  double r, g, b;
  int indexA, indexB, indexC;
  Point texCoordA, texCoordB, texCoordC;
  string tag, textureName;
  while (getline(inFile, tag)) {
    if (tag == "#Texture") { // Load the saved texture image
      getline(inFile, textureName);
      loadedTextureName = textureName;
      loadImage(textureName.c_str());
    }
    if (tag == "#Point") { // Load a point
      inFile >> x >> y;
      points.push_back(new Point(x, y));
    }
    if (tag ==
        "#Triangle") { // Load a triangle with the given point indices and
                       // texture coordinates. The points must be loaded first
      inFile >> indexA >> indexB >> indexC;
      inFile >> x >> y;
      Point texCoordA = Point(x, y);
      inFile >> x >> y;
      Point texCoordB = Point(x, y);
      inFile >> x >> y;
      Point texCoordC = Point(x, y);
      triangles.push_back(new Triangle(points.at(indexA), points.at(indexB),
                                       points.at(indexC), texCoordA, texCoordB,
                                       texCoordC));
    }
    if (tag == "#BGColour") { // Load the colours
      inFile >> r >> g >> b;
      bgColour = Colour(r, g, b);
    }
    if (tag == "#PointColour") {
      inFile >> r >> g >> b;
      pointColour = Colour(r, g, b);
    }
    if (tag == "#LineColour") {
      inFile >> r >> g >> b;
      lineColour = Colour(r, g, b);
    }
    if (tag == "#PointSize") {
      inFile >> x;
      pointSize = x;
    }
    if (tag == "#LineWidth") {
      inFile >> x;
      lineWidth = x;
    }
  }

  inFile.close();

  emit(sendTexture(getImage()));
  updateGL();
}

// Load the most recently loaded project again to revert it to its most recently
// saved state
void TextureEditor::reloadProject() { loadProject(projectFilename); }

// Load the most recently loaded project again to revert it to its most recently
// saved state
void TextureEditor::reloadTexture() {
  if (!loadedTextureName.empty())
    load(loadedTextureName);
}

// Returns the editor to the state saved on the top of the undo stack
void TextureEditor::undo() {
  vector<Point> undoPoints;
  vector<Triangle> undoTriangles;

  if (undoStackPoints.size() > 0) {
    saveRedoData(); // Save the current state so that it can be redone
    undoPoints = undoStackPoints.back(); // Get the state to be restored
    undoTriangles = undoStackTriangles.back();
    undoStackPoints
        .pop_back(); // Pop the stacks so that the next undo state is on top
    undoStackTriangles.pop_back();

    if (undoStackPoints.size() == 0)
      emit(canUndo(false));
    unsigned int oldPointsSize = points.size(); // Need to remember this information
                                       // because it changes during the loops
    unsigned int oldTrianglesSize = triangles.size();

    for (unsigned int i = 0; i < oldPointsSize; i++) {
      if (i < undoPoints.size()) {
        *points.at(i) = undoPoints.at(
            i); // Overwrite the old points with the ones to be restored
      } else {
        points.pop_back(); // Remove one if there are too many
      }
    }
    for (unsigned int i = 0; i < oldTrianglesSize; i++) {
      if (i < undoTriangles.size()) {
        *triangles.at(i) = undoTriangles.at(
            i); // Overwrite the old triangles with the ones to be restored
        triangles.at(i)->restoreValues();
      } else {
        triangles.pop_back(); // Remove one if there are too many
      }
    }
    changed();

    if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
      saveTexture();
      emit continuousSave();
    }
    updateGL();
  }
}

// Returns the editor to the state saved on the top of the redo stack
void TextureEditor::redo() {
  vector<Point> redoPoints;
  vector<Triangle> redoTriangles;

  if (redoStackPoints.size() > 0) {
    saveUndoData(); // Save the current state so that the redo can be undone
    redoPoints = redoStackPoints.back(); // Get the state to be restored
    redoTriangles = redoStackTriangles.back();
    redoStackPoints
        .pop_back(); // Pop the stacks so that the next redo state is on top
    redoStackTriangles.pop_back();

    if (redoStackPoints.size() == 0)
      emit(canRedo(false));
    unsigned int oldPointsSize = points.size(); // Need to remember this information
                                       // because it changes during the loops
    unsigned int oldTrianglesSize = triangles.size();

    for (unsigned int i = 0; i < redoPoints.size(); i++) {
      if (i >= oldPointsSize) {
        Point *newPoint = new Point(redoPoints.at(i).X(), redoPoints.at(i).Y());
        points.push_back(newPoint);
      } else {
        *points.at(i) = redoPoints.at(
            i); // Overwrite the old points with the ones to be restored
      }
    }
    for (unsigned int i = 0; i < redoTriangles.size(); i++) {
      if (i >= oldTrianglesSize) {
        Triangle *newTri =
            new Triangle(0, 0, 0, redoTriangles.at(i).getTexCoord(1),
                         redoTriangles.at(i).getTexCoord(2),
                         redoTriangles.at(i).getTexCoord(3));
        triangles.push_back(newTri); // Create new triangles with null points
                                     // and the old texture coordinates
      } else {
        *triangles.at(i) = redoTriangles.at(
            i); // Overwrite the old triangles with the ones to be restored
        triangles.at(i)->restoreValues();
      }
      triangles.at(i)->set(1, findPoint(redoTriangles.at(i).getVal(
                                  1))); // Get the pointers to the new points
                                        // with the appropriate values
      triangles.at(i)->set(2, findPoint(redoTriangles.at(i).getVal(2)));
      triangles.at(i)->set(3, findPoint(redoTriangles.at(i).getVal(3)));
    }
    changed();

    if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
      saveTexture();
      emit continuousSave();
    }
    updateGL();
  }
}

// Updates the mouse position in screen coordinates as well as recalculating the
// relative coordinates
void TextureEditor::updateMousePosition(int x, int y) {
  const int retinaScale = devicePixelRatio();
  mouseX = x * retinaScale;
  mouseY = y * retinaScale;
  x = mouseX;
  y = mouseY;
  if (editorWidth > editorHeight) { // calculate the mouse position
    if (imageRatio < 1) {
      mouseRel.setX(((((x * 1.0) / editorWidth) * 2.0) - 1) * ratio);
      mouseRel.setY((((y * -1.0) / editorHeight) * 2.0) + 1);
    } else {
      mouseRel.setX(((((x * 1.0) / editorWidth) * 2.0) - 1) *
	(ratio / (min(imageRatio, ratio))));
      mouseRel.setY(((((y * -1.0) / editorHeight) * 2.0) + 1) *
	(1 / (min(imageRatio, ratio))));
    }
  } else {
    if (imageRatio < 1) {
      mouseRel.setX(((((x * 1.0) / editorWidth) * 2.0) - 1) *
	(max(imageRatio, 1 / ratio)));
      mouseRel.setY(((((y * -1.0) / editorHeight) * 2.0) + 1) *
	(ratio * max(imageRatio, 1 / ratio)));
    } else {
      mouseRel.setX((((x * 1.0) / editorWidth) * 2.0) - 1);
      mouseRel.setY(((((y * -1.0) / editorHeight) * 2.0) + 1) * ratio);
    }
  }
  if (imageRatio < 1) {
    if (mouseRel.X() > (WIN_SIZE * imageRatio))
      mouseRel.setX(WIN_SIZE * imageRatio); // Clamp the points to within the texture
    if (mouseRel.X() < (-WIN_SIZE * imageRatio))
      mouseRel.setX(-WIN_SIZE * imageRatio);
    if (mouseRel.Y() > WIN_SIZE)
      mouseRel.setY(WIN_SIZE);
    if (mouseRel.Y() < -WIN_SIZE)
      mouseRel.setY(-WIN_SIZE);
  } else {
    if (mouseRel.X() > WIN_SIZE)
      mouseRel.setX( WIN_SIZE); // Clamp the points to within the texture
    if (mouseRel.X() < -WIN_SIZE)
      mouseRel.setX( -WIN_SIZE);
    if (mouseRel.Y() > (WIN_SIZE / imageRatio))
      mouseRel.setY(WIN_SIZE / imageRatio);
    if (mouseRel.Y() < (-WIN_SIZE / imageRatio))
      mouseRel.setY(-WIN_SIZE / imageRatio);
  }
}

// Returns the closest point to the given point on the opposite edge of any
// triangle it is in
Point TextureEditor::getClosestPointToOppositeEdge(Point *point) {
  vector<Point> possiblePoints;
  Point result;
  double minDistance = 10; // The window is only -1 to 1 so this will be higher
                           // than the shortest distance
  for (unsigned int i = 0; i < triangles.size(); i++) {
    if (triangles.at(i)->containsVertex(
            point)) { // For all the triangles that contain the given point...
      Edge edge = triangles.at(i)->getOppositeEdge(
          point); // Find the closest point ont their opposite edge...
      possiblePoints.push_back(edge.getClosestPoint(*point)); // And remember it
    }
  }
  double distance;
  for (unsigned int i = 0; i < possiblePoints.size();
       i++) { // For each of the possible points...
    distance = possiblePoints.at(i).distanceTo(
        *point); // Determine the distance to the actual point...
    if (distance < minDistance) {
      result = possiblePoints.at(i); // And find the shortest one
      minDistance = distance;
    }
  }
  return result;
}

// Find a point in the set of points with the same coordinates as the given
// point
Point *TextureEditor::findPoint(Point point) {
  for (unsigned int i = 0; i < points.size(); i++) {
    if (points.at(i)->X() == point.X() && points.at(i)->Y() == point.Y())
      return points.at(i);
  }
  return 0; // Return null if none is found
}

Colour TextureEditor::getBgColour() { return bgColour; }

Colour TextureEditor::getPointColour() { return pointColour; }

Colour TextureEditor::getLineColour() { return lineColour; }

void TextureEditor::resetBgColour() { changeBgColour(defBgColour); }

void TextureEditor::changeBgColour(Colour colour) {
  QGLWidget::makeCurrent();

  bgColour = colour;
  glClearColor(bgColour.r, bgColour.g, bgColour.b, 0);
  updateGL();
}

void TextureEditor::changePointColour(Colour colour) {
  pointColour = colour;
  updateGL();
}

void TextureEditor::changeLineColour(Colour colour) {
  lineColour = colour;
  updateGL();
}

void TextureEditor::showHidePoints(bool value) {
  showPoints = value;
  updateGL();
}

void TextureEditor::showHideLines(bool value) {
  showLines = value;
  updateGL();
}

// Clears the filename so that the program won't save over anything
void TextureEditor::clearFilename() {
  filename = "";
  emit(currentOpenFile(filename));
}

// Returns true if there is no saved filename, false otherwise
bool TextureEditor::noFilename() { return filename.empty(); }

// Clears the project filename so that the program won't save over anything
void TextureEditor::clearProjectFilename() { projectFilename = ""; }

// Returns true if there is no saved project filename, false otherwise
bool TextureEditor::noProjectFilename() { return projectFilename.empty(); }

// Returns the directory of the given file string
string TextureEditor::getDir(string fileStr) {
  return (fileStr.substr(0, fileStr.find_last_of('/')));
}

// Indicates that changes have been made to the texture, so the program should
// ask to save before closing, allow reverting, and send a signal to update the
// texture in the bezier editor
void TextureEditor::changed() {
  unsavedChanges = true;
  emit(sendTexture(getImage()));
  QGLWidget::makeCurrent();

  if (!noProjectFilename())
    emit(canRevert(true));
  else
    emit(canRevert(false));
}

double TextureEditor::getPointSize() { return pointSize; }

double TextureEditor::getLineWidth() { return lineWidth; }

void TextureEditor::setPointSize(int size) {
  pointSize = size;
  updateGL();
}

void TextureEditor::setLineWidth(int width) {
  lineWidth = width;
  updateGL();
}

void TextureEditor::resetView() {
  showPoints = true;
  showLines = true;
  updateGL();
}

void TextureEditor::setLinearInterpolation(bool value) {
  if (linearInterpolation == value)
    return;
  linearInterpolation = value;
  glBindTexture(GL_TEXTURE_2D, tex);
  if (linearInterpolation) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  emit(sendTexture(getImage()));
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
    saveTexture();
    emit continuousSave();
  }

  updateGL();
}

QString TextureEditor::getFilename() { return QString(filename.c_str()); }

void TextureEditor::rotateCW() {
  if (fbo != 0) {
    if (resized)
      imageRatio = (double)resizeResult.height() / (double)resizeResult.width();
    else
      imageRatio = (double)fbo->height() / (double)fbo->width();
    for (unsigned int i = 0; i < points.size(); i++) {
      Point point = *points.at(i);
      points.at(i)->setX(point.Y());
      points.at(i)->setY(-point.X());
    }
    resetProjection();
    if (!resized)
      resizeResult = QSize(fbo->width(), fbo->height());
    resized = true;
    resizeResult.transpose();
  }
  changed();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
    saveTexture();
    emit continuousSave();
  }

  updateGL();
}

void TextureEditor::rotateCCW() {
  if (fbo != 0) {
    if (resized)
      imageRatio = (double)resizeResult.height() / (double)resizeResult.width();
    else
      imageRatio = (double)fbo->height() / (double)fbo->width();
    for (unsigned int i = 0; i < points.size(); i++) {
      Point point = *points.at(i);
      points.at(i)->setX(-point.Y());
      points.at(i)->setY(point.X());
    }
    resetProjection();
    if (!resized)
      resizeResult = QSize(fbo->width(), fbo->height());
    resized = true;
    resizeResult.transpose();
  }
  changed();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
    saveTexture();
    emit continuousSave();
  }

  updateGL();
}

void TextureEditor::flipH() {
  saveUndoData(); // Remember the old state
  redoStackPoints
      .clear(); // Clear the redo information if something different is done
  redoStackTriangles.clear();
  for (unsigned int i = 0; i < points.size(); i++)
    points.at(i)->setX(-points.at(i)->X());
  changed();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
    saveTexture();
    emit continuousSave();
  }

  updateGL();
}

void TextureEditor::flipV() {
  saveUndoData(); // Remember the old state
  redoStackPoints
      .clear(); // Clear the redo information if something different is done
  redoStackTriangles.clear();
  for (unsigned int i = 0; i < points.size(); i++)
    points.at(i)->setY(-points.at(i)->Y());
  changed();
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
    saveTexture();
    emit continuousSave();
  }

  updateGL();
}

