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



#include "browserapp.h"

#include <QDropEvent>
#include <QFileOpenEvent>
#include <QMainWindow>
#include <QMimeData>
#include <QTextStream>
#include <QUrl>

bool BrowserApp::event(QEvent *ev) {
  switch (ev->type()) {
  case QEvent::FileOpen: {
    printf("Received FileOpen\n");
    QString filename = static_cast<QFileOpenEvent *>(ev)->file();
    if (topLevelWidgets().empty()) {
      fprintf(stderr, "Warning: no main window found\n");
    } else {
      QWidget *main_widget = topLevelWidgets().front();
      QMimeData *data = new QMimeData();
      QList<QUrl> urls;
      urls << QUrl(filename);
      data->setUrls(urls);
      QDropEvent *drop = new QDropEvent(QPoint(), Qt::CopyAction, data,
                                        Qt::LeftButton, Qt::NoModifier);
      sendEvent(main_widget, drop);
    }
  }
    return true;
  default:
    return QApplication::event(ev);
  }
}
