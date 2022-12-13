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



#include <string.h>
#include <stdlib.h>
#include <QPixmap>
#include <QIcon>
#include "comlineparam.h"
#include "lpfg.h"
#include "sgiFormat.h"
#include <QPainter>
#include "platform.h"

QApplication *pApp = 0;

int main(int argc, char **argv) {

  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

  //QApplication a(argc, argv);
  //  a.setAttribute(Qt::AA_ShareOpenGLContexts);
  //pApp = &a;
  pApp = new QApplication(argc, argv);
  pApp->setQuitOnLastWindowClosed(true);

  comlineparam.Parse(argc, argv);
  if (!comlineparam.BatchMode()) {
    std::string iconFname = "./icon";

    // try to load in the icon from the object directory
    QImage iconPicture = QImage(iconFname.c_str());
    // if unsuccessful, silenty ignore it and use the default icon
    if (iconPicture.isNull()) {
      // plugins have changed in QT5, I don't understand to handle them so
      // simply call the function that read a SGI. the problem is if QImage
      // doesn't recognize rgb format, to deal with this case we first try load
      // from Qt if it doesn't work, we try from SGI format !
      iconPicture = readSGI(iconFname.c_str());
      if (iconPicture.isNull()) {
        iconFname = ":/default-icon.png";
        iconPicture = QImage(iconFname.c_str());
      }
    }

    QPixmap icon = QPixmap::fromImage(iconPicture);

    QPainter painter(&icon);
    painter.fillRect(0, icon.height() - 53, 53, 53,
                     QColor::fromRgbF(0, 0, 0, 1));
    painter.setPen(Qt::red);
    painter.setFont(QFont("Times", 50));
    painter.drawText(3, icon.height() - 3, QString("L"));
    //a.setWindowIcon(QIcon(icon));
    pApp->setWindowIcon(QIcon(icon));
  }
#ifdef MAKE_BUNDLE
    // Update environment variables
  std::string resources = Vlab::getVlabRoot().toStdString() +
                          "/Contents/Plug-ins/lpfg.app/Contents/Resources";
  setenv("LPFGRESOURCES", resources.c_str(), 1);
  setenv("LPFGPATH", resources.c_str(), 1);
#endif

  Utils::SetEnv(comlineparam.Path());

  comlineparam.Apply();
  LPFG lpfg;

  if (comlineparam.BatchMode())
    return lpfg.RunBatchMode();
  else if (comlineparam.CompileOnly())
      return lpfg.CompileOnly();
  else
    //return lpfg.Run(a);
    return lpfg.Run(*pApp);
}
