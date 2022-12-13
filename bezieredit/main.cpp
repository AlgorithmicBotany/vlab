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
#include "mainwindow.h"
#include <QtCore/QTimer>
#include <qapplication.h>

void delayedShow(QWidget *wnd) {
  // This timer ensure Qt has the time to create the OpenGL context before
  // showing the window
  QTimer *delay_show = new QTimer(wnd);
  delay_show->setSingleShot(true);
  QObject::connect(delay_show, SIGNAL(timeout()), wnd, SLOT(show()));
  QObject::connect(delay_show, SIGNAL(timeout()), wnd, SLOT(raise()));
  delay_show->start(100);
}

int main(int argc, char **argv) {
  QApplication::setApplicationName("BezierEdit");
  QApplication::setOrganizationDomain("algorithmicbotany.org");
  QApplication::setOrganizationName("Algorithmic Botany");
  QApplication qapp(argc, argv);

  MainWindow *wnd = new MainWindow();
  Ctrl *ctrl = new Ctrl(argc, argv);
  wnd->setCentralWidget(ctrl);
  wnd->setWindowIcon(QPixmap());
  wnd->setWindowTitle("BezierEdit");
  wnd->activateWindow();
  delayedShow(wnd);
  qapp.setWindowIcon(QIcon(":/icon.png"));
  wnd->setWindowIcon(QPixmap(":/icon.png"));
  return qapp.exec();
}
