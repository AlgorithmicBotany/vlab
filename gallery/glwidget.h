

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "contour.h"

class Item;

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
  GLWidget(QWidget *parent);
  void setItem(Item* pItem);
  Item* getItem(){
    return pItem;
  }

  QWidget* parent(){
    return _parent;
  }
  double Scale(){
    return scale;
  }

  double Ratio(){
    return ratio;
  }
 					     
public slots:

protected:
  void mousePressEvent(QMouseEvent* pEv);
  void mouseReleaseEvent(QMouseEvent* pEv);
  void mouseDoubleClickEvent(QMouseEvent* pEv);
  void mouseMoveEvent(QMouseEvent* pEv);

  virtual void initializeGL() = 0;
  virtual void resizeGL(int w, int h) = 0;
  virtual void paintGL() = 0;

  Item* pItem;
  QWidget* _parent;
  double scale;
  double ratio;
  float _offSet;


private:
};

class ConViewer : public GLWidget {
    Q_OBJECT
public:
  ConViewer(QWidget* parent);
  ConViewer(ConViewer* viewer);
  
  
 protected:
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  void placeCenter();

  double Basis0(double t);
  double Basis1(double t);
  double Basis2(double t);
  double Basis3(double t);

  Point P(double d, const PtVec& pts);
  double         N(int k, int t, float u, int n);
  double         Nk1(int k, float u, int n);
  double         Nkt(int k, int t, float u, int n);
  int            Uk(int j, int n);

};

class FuncViewer : public GLWidget {
   Q_OBJECT
 public:
  FuncViewer(QWidget* parent);
  FuncViewer(FuncViewer* viewer);

 protected:
  void translate();
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  void placeCenter();


  // evaluator functions
  Point P(double d, const PtVec& pts);
  double N(int k, int t, double u,int n);
  double Nk1(int k, double u,int n);
  double Nkt(int k, int t, double u, int n);
  int    Uk(int j, int n);

};




#endif
