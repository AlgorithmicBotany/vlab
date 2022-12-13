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




/* Material World

   Definition of Class: MWViewer

*/

#ifndef MWVIEWER_H
#define MWVIEWER_H
#define GL_SILENCE_DEPRECATION

#include <iostream>
#include <cstdlib>
#include <time.h>
#include <cmath>


#include <QMimeData>
#include <QOpenGLWidget>
#ifdef __APPLE__
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/glu.h>
#  include <GL/glut.h>
#endif
#include <QWidget>
#include <QFile>
#include <QClipboard>
#include <QApplication>
#include <QMouseEvent>
#include <QMimeData>
#include <QMenu>

#include "colour.h" 

// floating point values to use w/in program
struct Material{
  GLfloat ambvec[4];
  GLfloat difvec[4];
  GLfloat emivec[4];
  GLfloat spevec[4];
  GLfloat shiny;
  GLfloat trans;
  bool isD; // default flag
};

enum SavingMode {CONTINUOUS,TRIGGERED, OFF};

void matcpy(Material*,Material); // copies 2nd into 1st
bool matcmp(Material,Material); // return true if equal
void matprint(Material);

class MWViewer:public QOpenGLWidget{
  Q_OBJECT
    public:
  MWViewer(QWidget* parent, const char* name=0,Qt::WindowFlags f=0);

  void getselect(int*,Material*);
  void selectOff(bool);
  inline int getSmoothness(){return smoothness;};
  inline bool getPageSmooth(){return pagesmooth;};
  inline bool getEnhance(){return enhance;};

  public slots:
    void read(QFile*);
  void write(QFile*);
  void selectall();
  void selectpage();
  void selectrange(int,int);
  void select(int);
  void Medit();
  void cut();  // cut and copy selected, shuffle up, adds new to end
  void copy(); // copy selected
  void paste(); // paste over selected
  void insert(); // insert copied (or new) before selected and shuffle down, delete from end
  void interpolate();
  void defaultmat();
  void setMaterial(Material);
  void showMaterial(Material);
  void setDefault(Material);
  void xspage();
  void smpage();
  void mdpage();
  void lgpage();
  void nextpage();
  void prevpage();
  void firstpage();
  void gotopage(int);
  void pagerange(int,int);
  void showBackground(bool);
  void rangeDialog();
  void enablePaste();
  void setSmoothness(int);
  void setEnhance(bool);
  void setPageSmooth(bool);

  void newfile();
  void newWindow();
  void loadWindow();
  void load();
  void save();
  void saveas();
  void revertsaved();

  void setmodified_save(bool);
  void setmodified_save_as(bool);
  
  void ContinuousSavingMode();
  void TriggeredSavingMode();
  void ModeOff();
  void setSavingMode(SavingMode savingMode);
  void saveInContinuouMode(bool modified);
  void saveInTriggeredMode();


 signals:
  void notice(const QString&);  // message
  void error(const QString&,int);  // message & error code
  void pageflip(int,int);
  void setselect(int,Material);
  void edit();
  void modify(bool);
  void setdefault(Material);
  void getRange(int,int);
 
  void newfile_signal();
  void newWindow_signal();
  void loadWindow_signal();
  void load_signal();
  void save_signal();
  void saveas_signal();
  void revertsaved_signal();
  void changeSavingMode(SavingMode);

 private:

 void setCopied(int,int);
 int getCopied(Material*);


  void initializeGL();
  void resizeGL(int,int);
  void viewSetup();
  void paintGL();
  void setSpheres();
  void backsplash(GLfloat,GLfloat,GLfloat,GLfloat);
  void backsplash_N(GLfloat,GLfloat,GLfloat,GLfloat);
  void backcheck(GLfloat,GLfloat,GLfloat,GLfloat);
  void randomcolour(GLfloat*);
  void randomgrey(GLfloat*);

  void mousePressEvent(QMouseEvent*);
  void mouseDoubleClickEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  int selection_test(int,int);

  void setdim();

  GLUquadricObj* sphereobject;
  int smoothness; // the smoothness coefficient
  Material materials[256];
  Material defmat; 
  int pg; // how many spheres to show per page
  int row;  // spheres per row (row x row = pg)
  GLfloat range;
  GLfloat near,far;
  GLfloat bgplane;
  GLfloat dimX,dimY; 
  GLfloat celldimX,celldimY;
  GLfloat radius; // of each sphere, given dim and pg
  int divisions; // sphere into polygons
  int selectindex; // index of selected material
  int selectoffset; // the distance to the next un-selected index
  bool selecting;
  int pageindex; // index of first material on page
  int pagenumber; // current page number
  int numpages; // number of pages
  bool selectable; // selectabe only if nothing is currently being edited
  GLfloat lightpos[4];
  GLfloat lightmodel[4];
  bool imageON,enhance,pagesmooth;
  QString background;
  int seed;
  QMenu *menu;
  QMimeData* D;
  QAction* _xs,*_sm,*_md,*_lg,*_cs,*_nx,*_pv,*_fp,*_ps,*_en,*_rs,*_sv;
  int base; // all your base are belong to us

  SavingMode _savingMode;
  QAction *_savingContinu_act;
  QAction *_savingTriggered_act;
  QAction *_savingMenu_act;

};

#endif

//eof: mwviewer.h
