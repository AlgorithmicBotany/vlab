
#include "glwidget.h"
#include "contour.h"
#include "func.h"

#include <QPainter>
#include <QTimer>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include <iostream>
#include <cmath>



GLWidget::GLWidget(QWidget *parent)
  : QOpenGLWidget(parent),_parent(parent), scale(double()), ratio(double()),_offSet(0)
{
  //   setFixedSize(158, 158);
  setAutoFillBackground(false);
}
void GLWidget::setItem(Item *pItem) {

  this->pItem = pItem;
  setFixedSize(pItem->Width(), pItem->Height());
}


void GLWidget::mousePressEvent(QMouseEvent *pEv) {
  pItem->mousePressEvent(pEv);
}

void GLWidget::mouseReleaseEvent(QMouseEvent *pEv) {
  pItem->mouseReleaseEvent(pEv);
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *pEv) {
  pItem->mouseDoubleClickEvent(pEv);
}

void GLWidget::mouseMoveEvent(QMouseEvent *pEv)  {
  pItem->mouseMoveEvent(pEv);
}

ConViewer::ConViewer(QWidget *parent)
    : GLWidget(parent) {}

ConViewer::ConViewer(ConViewer* viewer)
  : GLWidget(viewer->parent()) {
  scale = viewer->Scale();

  pItem = viewer->getItem();
  setFixedSize(pItem->Width(), pItem->Height());

}


void ConViewer::initializeGL() {
  initializeOpenGLFunctions(); 
  makeCurrent();
    

}

void ConViewer::placeCenter(){
    const int retinaScale = devicePixelRatio();

  // glClearColor(background[0], background[1], background[2], 1.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  double ratio = double(width()) / double(height());
  Point center =  dynamic_cast<ConItem *>(pItem)->Center();
  Point max =  dynamic_cast<ConItem *>(pItem)->Max();
  Point min =  dynamic_cast<ConItem *>(pItem)->Min();

  
  double n = fabs(max.x - center.x);
  double s = fabs(min.x - center.x);
  double e = fabs(max.y - center.y);
  double w = fabs(min.y - center.y);
  
  scale = n;
  if (s > scale)
    scale = s;
  if (e > scale)
    scale = e;
  if (w > scale)
    scale = w;
  
  scale *= 1.1; // add some padding

  if (ratio > 1.0)
    glOrtho(center.x - scale * ratio,
            center.x + scale * ratio,
            center.y - scale,
            center.y + scale, -1.0, 1.0);
  else
    glOrtho(center.x - scale,
            center.x + scale,
            center.y - scale / ratio,
            center.y + scale / ratio, -1.0,
            1.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, width()*retinaScale, height()*retinaScale);
}

void ConViewer::resizeGL(int, int) {}

void ConViewer::paintGL() {
  const int retinaScale = devicePixelRatio();

  //initializeGL();
  placeCenter();
  double *background =  dynamic_cast<ConItem *>(pItem)->Background();

  int w = width()*retinaScale;
  int h = height()*retinaScale;
  double ratio = double(w) / double(h);
  Point center =  dynamic_cast<ConItem *>(pItem)->Center();
  float maxX, minX, maxY, minY;
  if (ratio > 1.0) {
    minX = center.x - scale * ratio;
    maxX = center.x + scale * ratio;
    minY = center.y - scale;
    maxY = center.y + scale;
  } else {
    minX = center.x - scale;
    maxX = center.x + scale;
    minY = center.y - scale / ratio;
    maxY = center.y + scale / ratio;
  }
  float cst = 1000.;
  minX = -cst;
  maxX = cst;
  minY = -cst;
  maxY = cst;
  // drawing axis
  makeCurrent();
  glClearColor(background[0], background[1], background[2], 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBegin(GL_LINES);
  glColor3f(1, 0, 0);
  glLineWidth(1);
  glVertex3d(minX - 0.1 - _offSet, 0,0.);
  glVertex3d(maxX + 1.1 - _offSet, 0,0.);
  glEnd();
  glColor3f(0, 1, 0);
  glBegin(GL_LINES);
  glLineWidth(1);
  glVertex3d(0 - _offSet, minY - 0.1,0.);
  glVertex3d(0 - _offSet, maxY + 1.1,0.);
  glEnd();

  PtVec ctrlpts;
  PtVec points = dynamic_cast<ConItem *>(pItem)->Points();
  for (unsigned int i = 0; i < points.size();
       i++)
    for (unsigned int j = 0;
         j < points[i].multiplicity; j++)
      ctrlpts.push_back(points[i]);
  double *curve =  dynamic_cast<ConItem *>(pItem)->Curve();
  //makeCurrent();

  glColor3f(curve[0], curve[1], curve[2]);
  ConItem* item = dynamic_cast<ConItem *>(pItem);
  if (item->EndPointInterpolation()) {
    // Render with this end-point interpolation thing
    if (item->IsClosed()) {
      for (unsigned int j = 0;
           j < points.back().multiplicity; j++)
        ctrlpts.push_back(points.back());
    }

    unsigned int n = ctrlpts.size() * 12;

    glBegin(item->IsClosed() ? GL_LINE_LOOP : GL_LINE_STRIP);
    for (unsigned int i = 0; i < n; ++i) {
      Point p = P(double(i) / double(n), ctrlpts);
      glVertex2d(p.x, p.y);
    }
    if (item->IsClosed()) {
      glVertex2d(ctrlpts[0].x, ctrlpts[0].y);
    } else {
      glVertex2d(ctrlpts.back().x, ctrlpts.back().y);
    }
    glEnd();
  } else {
    // Render as a regular B-spline
    if (item->IsClosed()) {
      ctrlpts.push_back(ctrlpts[0]);
      ctrlpts.push_back(ctrlpts[1]);
      ctrlpts.push_back(ctrlpts[2]);
    }

    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 3; i < ctrlpts.size(); i++) {
      Point G[4] = {ctrlpts[i - 3], ctrlpts[i - 2], ctrlpts[i - 1],
                             ctrlpts[i]};

      for (double t = 0.0; t < 1.0; t += 0.1) {
        double x = Basis0(t) * G[0].x + Basis1(t) * G[1].x +
                   Basis2(t) * G[2].x + Basis3(t) * G[3].x;
        double y = Basis0(t) * G[0].y + Basis1(t) * G[1].y +
                   Basis2(t) * G[2].y + Basis3(t) * G[3].y;

        glVertex2d(x, y);
      }
    }
    glEnd();
  }

  //glFlush();
}

double ConViewer::Basis0(double t) {
  return 1.0 / 6.0 * (-pow(t, 3) + 3 * pow(t, 2) - 3 * t + 1);
}

double ConViewer::Basis1(double t) {
  return 1.0 / 6.0 * (3 * pow(t, 3) - 6 * pow(t, 2) + 4);
}

double ConViewer::Basis2(double t) {
  return 1.0 / 6.0 * (-3 * pow(t, 3) + 3 * pow(t, 2) + 3 * t + 1);
}

double ConViewer::Basis3(double t) { return 1.0 / 6.0 * (pow(t, 3)); }

Point ConViewer::P(double d, const PtVec &pts) {
  const int n = pts.size() - 1;
  const int t = 4;
  float u = d * (n - t + 2);
  Point sum;
  for (int k = 0; k <= n; ++k)
    sum += pts[k] * N(k, t, u, n);
  return sum;
}

double ConViewer::N(int k, int t, float u, int n) {
  if (t == 1)
    return Nk1(k, u, n);
  else
    return Nkt(k, t, u, n);
}

double ConViewer::Nk1(int k, float u, int n) {
  if (Uk(k, n) <= u && u < Uk(k + 1, n))
    return 1.0;
  return 0.0;
}

double ConViewer::Nkt(int k, int t, float u, int n) {
  double sum = 0.0;
  int div = Uk(k + t - 1, n) - Uk(k, n);
  if (div)
    sum = (u - Uk(k, n)) / div * N(k, t - 1, u, n);
  div = Uk(k + t, n) - Uk(k + 1, n);
  if (0 != div)
    sum += (Uk(k + t, n) - u) / div * N(k + 1, t - 1, u, n);
  return sum;
}

int ConViewer::Uk(int j, int n) {
  const int t = 4;
  if (j < t)
    return 0;
  if (j > n)
    return n - t + 2;
  return j - t + 1;
}

FuncViewer::FuncViewer(QWidget *parent)
    : GLWidget(parent) {
  scale = 1.0;
}


FuncViewer::FuncViewer(FuncViewer *viewer)
  : GLWidget(viewer->parent()) {
  const int retinaScale = devicePixelRatio();

  scale = viewer->Scale();  
  pItem = viewer->getItem();
  setFixedSize(pItem->Width()*retinaScale, pItem->Height()*retinaScale);

}

void FuncViewer::initializeGL() {
  initializeOpenGLFunctions(); 
  makeCurrent();
  
  
  // glClearColor(background[0], background[1], background[2], 1.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  //translate();
}

void FuncViewer::resizeGL(int, int) {
}

void FuncViewer::translate() {
    const int retinaScale = devicePixelRatio();

  Point max =  dynamic_cast<FuncItem *>(pItem)->Max();
  Point center =  dynamic_cast<FuncItem *>(pItem)->Center();
  Point min =  dynamic_cast<FuncItem *>(pItem)->Min();
  ratio = double(width()) / double(height());
  double n = fabs(max.x - center.x);
  double s = fabs(min.x - center.x);
  double e = fabs(max.y - center.y);
  double w = fabs(min.y - center.y);
  
  scale = n;
  
  if (s > scale)
    scale = s;
  if (e > scale)
    scale = e;
  if (w > scale)
    scale = w;
  
  scale *= 1.1; // add some padding;


  if (ratio > 1.0) {
    glOrtho(center.x - scale * ratio,
            center.x + scale * ratio,
            center.y - scale,
            center.y + scale, -1.0, 1.0);
  } else {
    glOrtho(center.x - scale,
            center.x + scale,
            center.y - scale / ratio,
            center.y + scale / ratio, -1.0,
            1.0);
  }



  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, width()*retinaScale, height()*retinaScale);
}

void FuncViewer::paintGL() {
  Point center =  dynamic_cast<FuncItem *>(pItem)->Center();
  double *background =  dynamic_cast<FuncItem *>(pItem)->Background();
  makeCurrent();
  // glClearColor(background[0], background[1], background[2], 1.0);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  translate();

  float maxX, minX, maxY, minY;
  if (ratio > 1.0) {
    minX = center.x - scale * ratio;
    maxX = center.x + scale * ratio;
    minY = center.y - scale;
    maxY = center.y + scale;
  } else {
    minX = center.x - scale;
    maxX = center.x + scale;
    minY = center.y - scale / ratio;
    maxY = center.y + scale / ratio;
  }
  makeCurrent();

  glClearColor(background[0], background[1], background[2], 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //drawing axis
  PtVec points = dynamic_cast<FuncItem *>(pItem)->Points();
  PtVec ctrlpts;
  for (unsigned int i = 0; i < points.size();i++)
    for (unsigned int j = 0; j < points[i].multiplicity; j++)
      ctrlpts.push_back(points[i]);

  
  glColor3f(1, 0, 0);
  glBegin(GL_LINES);
  glVertex2d((minX - 0.1 - _offSet), 0);
  glVertex2d((maxX + 1.1 - _offSet), 0);
  glColor3f(0, 1, 0);
  glVertex2d((0 - _offSet), (minY - 0.1));
  glVertex2d((0 - _offSet), (maxY + 1.1));
  glColor3f(1, 0, 0);
  glVertex2d((ctrlpts[ctrlpts.size() - 1].x - _offSet), (minY - 0.1));
  glVertex2d((ctrlpts[ctrlpts.size() - 1].x - _offSet), (maxY + 1.1));
    
  glEnd();

  // draw curve
  double *curve =  dynamic_cast<FuncItem *>(pItem)->Curve();

  glColor3f(curve[0], curve[1], curve[2]);


  int num = 6 * ctrlpts.size();
  glBegin(GL_LINE_STRIP);
  for (int i = 0; i < num; i++) {
    Point pt = P(double(i) / num,ctrlpts);
    glVertex2d((pt.x - _offSet), pt.y);
  }
  glVertex2d(
	     (ctrlpts[ctrlpts.size() - 1].x - _offSet),
      ctrlpts[ctrlpts.size() - 1].y);

  glEnd();
  glFlush();
}

Point FuncViewer::P(double d, const PtVec& pts) {
  const int n = pts.size() - 1;
  const int t = 4;
  double u = d * (n - t + 2);
  Point sum;

  for (int k = 0; k <= n; k++) {
    double coeff = N(k, t, u,n);
    sum.x += pts[k].x * coeff;
    sum.y += pts[k].y * coeff;
  }
  return sum;
}

double FuncViewer::N(int k, int t, double u, int n) {
  double res = 0.0;
  if (t == 1)
    res = Nk1(k, u,n);
  else
    res = Nkt(k, t, u,n);
  return res;
}

double FuncViewer::Nk1(int k, double u, int n) {
  if (Uk(k,n) <= u) {
    if (u < Uk(k + 1,n))
      return 1.0;
  }
  return 0.0;
}

double FuncViewer::Nkt(int k, int t, double u, int n) {
  double sum = 0.0;
  int div = Uk(k + t - 1,n) - Uk(k,n);
  if (div)
    sum = (u - Uk(k,n)) / div * N(k, t - 1, u,n);
  div = Uk(k + t,n) - Uk(k + 1,n);
  if (div)
    sum += (Uk(k + t,n) - u) / div * N(k + 1, t - 1, u,n);

  return sum;
}

int FuncViewer::Uk(int j,int n) {
  const int t = 4;
  if (j < t)
    return 0;
  if (j > n)
    return n - t + 2;
  return j - t + 1;
}
