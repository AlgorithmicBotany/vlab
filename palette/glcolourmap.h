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



/* Palette

   Definition of Class: GLColourMap
                        ColourTuner

   Last Modified by: Joanne
   On Date: 14-06-01
*/

#ifndef GLCOLOURMAP_H
#define GLCOLOURMAP_H

#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include <QOpenGLWidget>
#include <qthread.h>
//#include <qgl.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <QCloseEvent>
#include <QDial>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QInputDialog>
#include <QLCDNumber>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>

#include "colourpick.h"

class QMenu;

struct colour {
  GLubyte bytes[3]; // R-G-B
  GLubyte oldbytes[3];
  GLfloat r;
  GLfloat g;
  GLfloat b;
  int i;
};

struct page {
  int pg;
  bool modified;
  QFile *colourfile;
  QString *name;
  colour *colours;
};

enum SavingMode { CONTINUOUS, TRIGGERED, OFF };

class ColourTuner;

class GLColourMap : public QOpenGLWidget {
  Q_OBJECT

public:
  GLColourMap(QWidget *parent = 0, SavingMode savingMode = OFF,
              const char *name = 0);
  ~GLColourMap();

  inline ColourTuner *tune() { return tuner; };
  inline ColourPick *pick() { return picker; };

  // "wow, take a look at all those slots!"
public slots:
  void INIT(page *);
  void SAVE_ALL();
  void SAVE();
  void REVERT();
  void updateRED(int);
  void updateGREEN(int);
  void updateBLUE(int);
  void PAGE(int);
  void closeEvent();
  void ABOUT();
  void COPY();
  void HELP();
  void PASTE();
  void SELECT();
  void SELECTINDEX(int);
  void SELECTABLE();
  void TUNE();
  void finetuneRED(int);
  void finetuneGREEN(int);
  void finetuneBLUE(int);
  void BRIGHT(int);
  void modeR();
  void modeG();
  void modeB();
  void modeD();
  void PICK();
  void SHOWPICK(GLfloat *);
  void DONEPICK(GLfloat *);
  void INTERPOLATE();
  void CUT();
  void INSERT();

  void ContinuousSavingMode();
  void TriggeredSavingMode();
  void ModeOff();
  void sliderTriggered();
  void sliderMoving();

signals:
  void CONFIRM(const QString &);
  void closeEvent(QCloseEvent *);
  void NOTICE(const QString &, int);
  void INDEX(int);
  void myRED(int);
  void myGREEN(int);
  void myBLUE(int);
  void MODIFIED(bool);
  void SLIDERS_ON(bool);
  void SELECT_ON(bool);
  void MAKEPICK();
  void quit();

private:
  // openGL rendering
  void initializeGL();
  void paintGL();
  void resizeGL(int, int);
  void viewerSetup();
  bool drawcolourmap(int, int);
  void interpolate();

  // file generators
  int defaultPalette(int);
  void generateGreyScale(int);
  void generateBlankPage(int);
  bool read(int);

  // some functional functions
  void updateColours();
  void oldBytes();
  void checkModified();

  // handling mouse events
  void mousePressEvent(QMouseEvent *);
  void mouseMoveEvent(QMouseEvent *);
  void mouseReleaseEvent(QMouseEvent *);
  int selection_test();

  // data
  page *P;
  int currpage;
  int mX, mY;
  int mapH, mapW;
  QMenu *popmenu;
  int selectindex;
  int interindex;
  int offset;
  int numcolours;
  int numpages;
  colour copycolour;
  int copypage, copyindex, copyoffset;
  bool selectall;
  bool selectable;
  ColourTuner *tuner;
  ColourPick *picker;
  char tuneMode;
  QAction *_paste, *_insert;
  int copy[256][3];

  SavingMode _savingMode;
  QAction *_savingContinu_act;
  QAction *_savingTriggered_act;
  QAction *_savingMenu_act;
  std::chrono::system_clock::time_point currentTime;
};

class ColourTuner : public QDialog {
  Q_OBJECT
public:
  ColourTuner(QWidget *parent = 0, const char *name = 0);

  inline QDial *r() { return red; };
  inline QDial *g() { return green; };
  inline QDial *b() { return blue; };
  inline QDial *br() { return bright; };

public slots:
  void DISRED(int);
  void DISGRE(int);
  void DISBLU(int);
  void DISBRI(int);

signals:
  void SELECTABLE();

public:
  void closeEvent(QCloseEvent *);

private:
  QDial *red, *green, *blue, *bright;
  QLCDNumber *rn, *gn, *bn, *brn;
};

#endif

// EOF: glcolourmap.h
