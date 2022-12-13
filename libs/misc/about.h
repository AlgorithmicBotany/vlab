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



#ifndef __VLAB_ABOUT_H__
#define __VLAB_ABOUT_H__

#include <QString>
#include <QDialog>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <iostream>
#include "resources.h"
class QWidget;

namespace vlab {

  void about(QWidget *parent = 0);
  void about(QWidget *parent = 0,QString programName= "");
  void aboutRayshade(QWidget *parent = 0);
  QDialog *openFile(QString path, QString title, QDialog *box);  
  void openPdf(QString path, QString title);

  class openPDF : public QObject
{
  Q_OBJECT
public:
  openPDF(QObject *parent = NULL):QObject(parent){}

  void openAPDF(QString filename){
    /* credits */
    QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
    helpDir.cd("files");
#endif
    QString path = helpDir.filePath(filename);
    QDesktopServices::openUrl(
			      QUrl::fromLocalFile(helpDir.filePath(path)));
  }

private slots:
    void open3rdParty(){
      openAPDF(QString("ThirdPartyNotices.pdf"));
    }
    void openLicence(){
      openAPDF(QString("License+gpl-3.0.pdf"));
    }
    void openCredits(){
      openAPDF(QString("Credits.pdf"));
    }
};

}; // namespace vlab

#endif
