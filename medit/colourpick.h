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




/* ColourPick - A Custom-Built Widget for your Application

   Declaration of Classes: ColourPick 
                           ColourWheel

   Last Modified by: Joanne
   On Date: 30-07-01
*/

#ifndef COLOURPICK_H
#define COLOURPICK_H

#include <iostream>
#include <cmath>

#include <QOpenGLWidget>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <qwidget.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstatusbar.h>
#include <qlineedit.h>
#include <QMouseEvent>
#include <QCloseEvent>

#include "colour.h"

class ColourWheel;

struct Point{
  GLfloat x,y;
  Colour* colour;
};

class ColourPick:public QWidget{
  Q_OBJECT
    public:
  // =========================
  // -----> INSTANTIATION INSTRUCTIONS
  //
  // new ColourPick MUST have a parent, the rest of the args may be default
  // if fixsize is true, ColourPick is minimum size and cannot be resized (default false)
  // if multiple contexts and display lists involved, set QGLWidget* globj
  // don't toy with WindowFlags !!!!!!

  ColourPick(QWidget* parent, const char* name=0, QOpenGLWidget* globj=0,bool fixsize=false, Qt::WindowFlags=Qt::Window);

  // =========================

  public slots:
  // for internal use only! don't use these slots...
  // -------> (go down for YOUR slots and signals) 
  void revertColour(); // revert to initial or "saved" colour
  void resetInit();  // reset the colour to revert to to selected colour  
  void updateRed(int); // update the components of select colour via slider
  void updateGreen(int);
  void updateBlue(int);
  void updateHue(int);
  void updateSat(int);
  void updateValue(int);
  void updateIntensity(int);
  void editHue(); // update the components of select colour via line edit
  void editSat();
  void editVal();
  void editRed();
  void editGreen();
  void editBlue();
  void updateRGB(); // update slider values
  void updateHSV();
  void rgbmode(); // set flags to know which sliders moved
  void hsvmode();
  void nomode();
  void setSelect(Colour*); // sets select colour
  void apply();// handling button press
  void closeEvent(QCloseEvent*);
  void notify(bool); // notify of selection/modification

  // =========================
  // "HEY, YOU, LOOK AT THESE SLOTS AND SIGNALS!"
  // ----> if you use this widget, these are the slots/signals you (may) need

  // slots:
  void getColour();  // opens the ColourPick for user input
  void initColour(GLfloat*);  // you can set initial colour (eg. to current colour of what your picking for)
  
 signals:
  void applyEvent(GLfloat*);  // emitted whenever user edits colour
  // =========================  

  void setInit(Colour*);  
  void displaySelect(Colour*);

 private:
  void disconnectSliders();
  void connectSliders();

  QSlider *hSl, *sSl, *vSl, *rSl, *gSl, *bSl;
  QLineEdit *hLe, *sLe, *vLe, *rLe, *gLe, *bLe;
  Colour *selectColour,*init;
  QPushButton *closebutton;
  ColourWheel *colourWheel;
  QStatusBar *status;
  bool ud,rgbud,hsvud;
  QString slidermode;
};

class ColourWheel:public QOpenGLWidget{
  Q_OBJECT
    public:
  ColourWheel(QWidget* parent, const char* name,QOpenGLWidget* globj);
  
  public slots:
    void setSelect(Colour*); // set the selected colour
  void initColour(Colour*); // initialize first select colour and colour to revert to

 signals:
  void select(Colour*);
  void reset(Colour*);
  void revert();
  void apply();
  
 private:
  void initializeGL();
  void paintGL();
  void resizeGL(int,int);
  void drawHSWheel();
  void drawVScale();
  void drawSample();
  void showHSV();

  void mousePressEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void selectionTest();
  
  void HS();
  void V();
  void setCircumferencePoints();

  int selectObject; // 1 for HS-wheel, 2 for V-scale, 3 for Sample, 4 for RevertToSample, 0 for none
  
  GLfloat S,dV,sR,R,W; // some important distances in drawing space
  GLfloat range,left,right,bottom,top;

  Colour* selectColour;
  Colour* revertColour;

  Point point;
  Point HSpoint; 
  Point wheel;
  Point value;
  Point sample;
  Point revertBox;

  Point Wcircumference[361]; //relative to center
  Point Scircumference[361];
  Point HSPcircumference[73];
  bool moved;
};

#endif

// EOF: colourpick.h
