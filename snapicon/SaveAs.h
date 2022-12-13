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



#ifndef SAVEAS_H
#define SAVEAS_H

#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QProgressDialog>
#include <QMessageBox>
#include <sstream>
#include "stdio.h"
#include "stdlib.h"
#include "ui_SaveAs.h"

class SaveAs : public QDialog, public Ui_SaveAs {
  Q_OBJECT

protected:
  void closeEvent(QCloseEvent *event);

public:
  SaveAs(QWidget *parent = 0, QString objName = NULL,
         QString currentPath = NULL,
          QString pix_format = "PNG");
  ~SaveAs();

  QString getPath();
  int getFormat();
  int getImageType();
  
  QString getImageBaseName();

  void loadSettings();
  void writeSettings();

  int getId() { return _id; }

public slots:
  void setPaths(QStringList);
  void setImageFormat(int f);

  QString getExtension();
  void setLineEdit();
  void setNodeName();
  void preserveFormat();

  void setDirectoryList();
  void browse();
  int ok();

private:
  QStringList imageTypes;
  QString nodeName;
  QString saveName;
  QString settingsFile;
  int _id;

  int decay;
  QString previousOutputFormat;
  QString previousPixFormat;
  bool changeFormat;

private slots:
  void preserveExtension();
};

#endif // SAVEAS_H
