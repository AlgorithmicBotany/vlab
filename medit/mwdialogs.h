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

   Definition of Assorted Dialog Classes

   Last Modified by: Joanne
   On Date:
*/

#ifndef MWDIALOGS_H
#define MWDIALOGS_H

#include <iostream>

#include <qwidget.h>
#include <qdialog.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <QButtonGroup>
#include <qlayout.h>
#include <qlabel.h>
#include <qgl.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <QMainWindow>
#include <QCloseEvent>
#include <QGroupBox>


class SelectRangeDialog:public QWidget{
  Q_OBJECT
    public:
  SelectRangeDialog(QWidget* parent,const char* name,Qt::WindowFlags f);

  public slots:
    void getRange(int,int);
  void newOffset(int);
  void newEnd(int);
  void newBegin(int);
  void accept();
  void reject();

 signals:
  void rangeEvent(int,int);

 private:
  void closeEvent(QCloseEvent*);

  QSpinBox *begin,*end,*offset;
  QPushButton *okay,*cancel;
};

class PreferencesDialog:public QWidget{ //will have tabs
  Q_OBJECT
};

class MaterialListDialog:public QWidget{
  Q_OBJECT
};

class ImageDialog:public QWidget{
  Q_OBJECT
    public:
  ImageDialog(QWidget* parent, const char* name,Qt::WindowFlags f);
  
  public slots:
    void getImageQuality(int,bool,bool);
  void editQuality(int);
  void setAdjust(int);
  void save();

 signals:
  void setSmoothness(int);
  void adjustResize(bool);
  void adjustPageSize(bool);
  void saveconfig();

 private:
  void closeEvent(QCloseEvent*);

  QPushButton* butt,*savebutt;
  QRadioButton* pc,*rs;
  QGroupBox* bg;
  QButtonGroup *bgButtonGroup;
  QSlider* slide;
  int R,P;
};

#endif

// eof: mwdialogs.h
