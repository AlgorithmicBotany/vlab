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

   Definition of Class: Palette

   Last Modified by: Joanne
   On Date: 14-06-01
*/

#ifndef PALETTE_H
#define PALETTE_H

#include <fstream>
#include <iostream>

#include <QApplication>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QColor>
#include <QFile>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QPushButton>
#include <QRadioButton>
#include <QRect>
#include <QSpinBox>
#include <QStatusBar>
#include <QWidget>
#include <QtWidgets/QSlider>
#include <qgl.h>

#include "glcolourmap.h"

class Palette;

class Palette : public QWidget {
  Q_OBJECT

public:
  Palette(QString *mess, page *pages, SavingMode savingMode = OFF,
          QWidget *parent = 0, const char *name = 0);
  ~Palette();

public slots:
  void CONFIRM(const QString &);
  void NOTICE(const QString &, int);
  void MODIFIED(bool);
  void SLIDERS_ON(bool);
  void SELECT_ON(bool);

signals:
  void INIT(page *);
  void SAVE();
  void PAGE(int);
  void quit();

private:
  void init();
  void connect();
  void closeEvent(QCloseEvent *);

  QString *colourFileName;
  QStatusBar *status;
  QLabel *indexlabel;
  QSlider *redslide;
  QSlider *blueslide;
  QSlider *greenslide;
  QSlider *lightslide;
  QSpinBox *redspin;
  QSpinBox *bluespin;
  QSpinBox *greenspin;
  QRadioButton **pagebuttons;
  QGroupBox *pager;
  QButtonGroup *pagerButtonGroup;
  GLColourMap *colourmap;
  QSpinBox *jumper;
  SavingMode _savingMode;
  bool modified;
};

#endif

// EOF: palette.h
