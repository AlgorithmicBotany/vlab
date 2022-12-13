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




/* MaterialWorld

   Definition of Class: MWeditor

   Last Modified by: Joanne
   On Date:
*/

#ifndef MWEDITOR_H
#define MWEDITOR_H

#include <iostream>

#include <qwidget.h>
#include <qapplication.h>
#include <QOpenGLWidget>
#include <QMessageBox>
#include <qlayout.h>
#include <qmenubar.h>
#include <qfile.h>
#include <qstatusbar.h>
#include <QByteArray>
#include <qcolor.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <QCloseEvent>
#include <QMouseEvent>

#include "colourpick.h"
#include "mwviewer.h"

class ColourSwatch;
class MaterialDisplay;

// the main editor panel
class MWEditor : public QWidget{
  Q_OBJECT
    
    public:
  MWEditor(QWidget* parent,const char* name,Qt::WindowFlags f);

  public slots:
    void edit(); // executes the dialog
  // handling button signals
  void ok();
  void cancel();
  void applyM();
  void defaultM();
  void revertM();
  void undoM();
  void selectM(int,Material);
  void setdefault(Material);
  // handle slider signals
  void updateMaterial();
  void updateAmb(int);
  
  void updateDif(int);

  void updateSpec(int);
  void updateEm(int);
  void updateShiny(int);
  void updateTrans(int);
  // handle line edit signals
  void editAmb();
  void editDif();
  void editSpec();
  void editEm();
  void editShiny();
  void editTrans();
  // editing colours
  void editAmbColour();
  void editDifColour();
  void editSpecColour();
  void editEmColour();
  void showPick(GLfloat*); // if colour picker made selection
   void editingYes();
  void editingNo();
  void copycolour(QString&);
  void pastecolour(QString&);
  void notify(bool);

 signals:
  void pickColour(); // execute the colourpick dialog
  void isEdited(bool); // emitted when the material has been modified
  void doneEvent(Material); // emitted when the dialog has been closed
  void applyEvent(Material); // emitted when the "apply" button is clicked
  void displayMaterial(Material);
  void triggered();

 private:
  void closeEvent(QCloseEvent*);

  Material Mselect,Moriginal,Mdefault;
  MaterialDisplay* MD;
  bool editing;
  QStatusBar* status;
  ColourSwatch *ambCS, *difCS, *specCS, *emCS;
  GLfloat amb,dif,em,spec,shin,trans; // the values from sliders
  GLfloat ambcol[3],difcol[3],speccol[3],emcol[3],copycol[3],cancol[3]; // colours at full intensity
  QString setting;
  bool edited;
  QLabel *ambLB, *difLB, *specLB, *emLB, *shinLB, *transLB;
  QSlider *ambSL, *difSL, *specSL, *emSL, *shinSL, *transSL;
  QLineEdit *ambLE, *difLE, *specLE, *emLE, *shinLE, *transLE;
  QPushButton *defaultPB,*closePB,*undoPB,*applyPB;
  ColourPick *CP;
  bool off;
  QString touching;
};

class MaterialDisplay:public QOpenGLWidget{
  Q_OBJECT
    public:
  MaterialDisplay(QWidget* parent, const char* name=0);
  void lightup(GLfloat*,GLfloat*);
  void display(Material);

 private:
  void initializeGL();
  void resizeGL(int,int);
  void paintGL();

  GLUquadricObj* sphereobject;
  GLuint spheredrawlist;
  GLfloat smoothness; 
  GLfloat range;
  GLfloat near,far;
  GLfloat radius; 
  Material M;
  GLfloat lightpos[4];
  GLfloat lightmodel[4];
  GLfloat xmin,xmax,ymin,ymax;
};

// a colourful little button
class ColourSwatch:public QOpenGLWidget{
  Q_OBJECT
    public:
  ColourSwatch(QWidget* parent, const char* name=0);

  public slots:
    void editcol();
    void display(GLfloat*,GLfloat*);
  void setDisabled(bool);
  void copy();
  void paste();

 signals:
  void edit();
  void copycolour(QString&);
  void pastecolour(QString&);


 private:
  void initializeGL();
  void resizeGL(int,int);
  void paintGL();

  void mousePressEvent(QMouseEvent*);

  GLfloat currColour[3];
  GLfloat fullColour[3];

  QMenu* menu;
  bool enabled;
  QString mynameis;
};

#endif

// EOF: mw.h
