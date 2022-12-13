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



/* Material World

   Main Routine
*/

#include "mw.h"
#include <QtCore/QTimer>
#ifdef __APPLE__
#include "mainwindow.h"
#endif

// -------------------- Main Routine
int main(int argc, char **argv) {
  MW *mw;
  int w = 400;
  int h = 400;

  QApplication::setColorSpec(QApplication::CustomColor);
  QApplication app(argc, argv);
  mw = new MW(argc, argv);

 #ifdef __APPLE__
  mw->setWindowIcon(QPixmap()); // use qt3support
  mw->setWindowTitle("Material Editor");
  mw->resize(w, h);
  QObject::connect(mw, SIGNAL(quit()), mw, SLOT(close()));

  mw->show();
  QTimer::singleShot(0, mw, SLOT(raise()));
  mw->setWindowIcon(QPixmap());
#else
  mw->resize(w, h);
  mw->show();
  QTimer::singleShot(0, mw, SLOT(raise()));
  // On Ubuntu: the next line disables the icon...
  //mw->setWindowIcon(QPixmap());
#endif
  app.setWindowIcon(QPixmap(":icon.png"));
  return app.exec();
}

// EOF: main.cc
