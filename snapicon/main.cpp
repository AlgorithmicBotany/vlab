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

#include <QApplication>
#include <QPainterPath>
#include <QPainter>
#include <QMap>
#include <QDebug>

int main(int argc, char **argv) {
  // Initialize QT application
  // QApplication::setColorSpec(QApplication::CustomColor);
  QApplication::setApplicationName("snapicon");
  QApplication::setOrganizationDomain("algorithmicbotany.org");
  QApplication::setOrganizationName("Algorithmic Botany");
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QApplication app(argc, argv);
  app.setWindowIcon(QPixmap(":icon.png"));
  MainWindow mainWin;
  mainWin.show();
  return app.exec();
}
