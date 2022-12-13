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



#include "ctrl.h"
#include "qtFontUtils.h"
#include <QIcon>
#include <QtCore/QTimer>
#include <iostream>
#include <QApplication>

#ifdef __APPLE__
#include "mainwindow.h"
#endif

int main(int argc, char **argv) {
  setDefaultFont();
  QApplication::setApplicationName("funcedit");
  QApplication::setOrganizationDomain("algorithmicbotany.org");
  QApplication::setOrganizationName("Algorithmic Botany");
  QApplication qapp(argc, argv);
#ifdef __APPLE__
  MainWindow *wnd = new MainWindow();
  Ctrl *ctrl = new Ctrl(wnd, argc, argv);
  wnd->setCentralWidget(ctrl);
  wnd->setWindowIcon(QPixmap());
  QString windowTitle = "funcedit";
   QTimer::singleShot(0, wnd, SLOT(raise()));
#else
  Ctrl *ctrl = new Ctrl(0, argc, argv);
  ctrl->show();
  QTimer::singleShot(0, ctrl, SLOT(raise()));
#endif
  qapp.setWindowIcon(QPixmap(":icon.png"));
  return qapp.exec();
}
