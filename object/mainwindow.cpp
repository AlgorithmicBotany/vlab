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



#include "mainwindow.h"
#include "QTGLObject.h"
#include "resources.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QUrl>

MainWindow::MainWindow() : QMainWindow(nullptr) {
// menu bar is initialized in QTGLObject...
//  QMenuBar *menu = new QMenuBar(0); // menuBar();
//  QMenu *help = menu->addMenu("Help");
//  help->addAction("Object Help", this, SLOT(help()));
//  help->addAction("Object Help (PDF)", this, SLOT(pdfHelp()));
//  setWindowIcon(QPixmap());
//  this->setStyleSheet("MainWindow {background: 'black';}");
  this->setFocusPolicy(Qt::StrongFocus);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  QTGLObject *obj = dynamic_cast<QTGLObject *>(centralWidget());
  if (obj) {
    obj->QuitCb();
  }
  event->ignore();
}

bool MainWindow::event(QEvent *event) {
  bool value = QMainWindow::event(event);
  if (event->type() == QEvent::WindowActivate) {
    emit readyToLoad();
  }
  return value;
}
