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



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <platform.h>
#include "gallery.h"

class QCloseEvent;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow();
  ~MainWindow();
  QMenu *file;
  QMenu *view;
  QMenu *help_menu;
  void setGallery(Gallery* gallery){
    _gallery = gallery;
  }


protected:
  void closeEvent(QCloseEvent *);

public slots:
  void help();
  void about();
  void pdfHelp();
  void quickHelp();
  void nameChanged(QString s);
  void editPreferencesCB();

private:
    Gallery* _gallery;

  
};

#endif // MAINWINDOW_H
