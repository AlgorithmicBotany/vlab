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




#ifndef __ITEM_H__
#define __ITEM_H__

#include <string>

#include <QWidget>
#include <QLineEdit>
#include <QMenu>
#include <QLabel>
#include <QMouseEvent>
#include <iostream>


#include "editor.h"
#include "gallery.h"


class Gallery;
class Set;
class DirectoryWatcher;
class GLWidget;

struct Point {
  Point() : x(0.0), y(0.0), z(0.0), multiplicity(1) {}
  Point(const Point &p){
    x = p.x;
    y = p.y;
    z = p.z;
    multiplicity = p.multiplicity;
  }
  void operator=(const Point &p){
    x = p.x;
    y = p.y;
    z = p.z;
    multiplicity = p.multiplicity;
  }
  double x;
  double y;
  double z;
  unsigned int multiplicity;
  
  Point operator*(double s) const {
    Point ret;
    ret.x = x * s;
    ret.y = y * s;
    ret.z = z * s;
    return ret;
  }
  Point& operator+=(const Point& p) {
    x += p.x;
    y += p.y;
    z += p.z;
    return *this;
  }
};
typedef std::vector<Point> PtVec;


class Item : public QWidget {
  Q_OBJECT


 public:

  Item(){}
  Item(const Item& ) {std::cerr<<"NOT YET IMPLEMENTED"<<std::endl;};
  
  Item(Gallery* pGal, std::string filename, QWidget* parent, const char* name = 0, Qt::WindowFlags f = 0);
  virtual ~Item();

  virtual bool load() = 0;
  virtual void save() = 0;


  virtual float relativeSize() {return 1.0;}

  std::string getFileName() const {
    return filename;
  }

  void setName(const std::string n) {
    name = n;
    pNameTxt->setText(this->name.c_str());
  }
    
  
  QLabel* getName() const {
    return pNameTxt;
  }

  void setSet(Set* s);
  Set* getSet() {return pSet;}
 
  void setTmpDir(std::string tmpDir){
    _tmpDir = tmpDir;
  }
  Gallery* getGallery()const{
    return pGallery;
  }

  bool isVisible(){
    return _visible;
  }

  void hide(){
    _visible = false;
  }

  void visible(){
    _visible = true;
  }

  int Width(){
    return _width;
  }

  int Height(){
    return _height;
  }

  GLWidget* getGLWidget(){
    return _glwidget;
  }

  void setGLWidget(GLWidget *glwidget){
    _glwidget = glwidget;
  }

  QProcess* getProcess(){ return pEditor->getProcess();}

 public slots:
  void edit();
  bool reload();

signals:
  void openMenu(Item*);


 public:
  void mousePressEvent(QMouseEvent* pEv);
  void mouseReleaseEvent(QMouseEvent* pEv);
  void mouseDoubleClickEvent(QMouseEvent* pEv);
  void mouseMoveEvent(QMouseEvent* pEv);
  void mouseGrabEvent(QMouseEvent* );

  GLWidget* _glwidget;
  Gallery* pGallery;
  std::string filename;

  std::string name;

  std::string _tmpDir;    

  Editor* pEditor;

  QLabel*  pNameTxt;

  QColor bgClr;
  QColor dragClr;

  Set* pSet;

  float _offSet;
  bool _visible;
  int _width;
  int _height;
  QPoint _mousePos;

};

#endif
