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



#ifndef _GLWINDOW_H
#define _GLWINDOW_H

#include <qgl.h>
#include <QWidget>
#include <QMainWindow>

#include "glcanvas.h"

class GLWindow : public QMainWindow {
  Q_OBJECT

private:
  QString _title;
  QTimer _resizeTimer;

public:
  // the qglwidget
  glcanvas *canvas;
  GLWindow(QWidget *parent = 0, int xsize = 320, int ysize = 240, Qt::WindowFlags flags = 0);
  ~GLWindow();
  void setTitle(QString title) { _title = title; }

public slots:
  void resizeDone();

protected:
  void resizeEvent(QResizeEvent *);

public slots:
  void about_cpfg_cb();
  void onlineHelp();
  void pdfHelp();
  void quickHelp();
  void enviropdfHelp();
  void graphpdfHelp();
  void closeEvent(QCloseEvent *);
  void savePS();
};

#endif
