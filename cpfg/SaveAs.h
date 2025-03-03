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
  void changeEvent(QEvent *event);

public:
  SaveAs(QWidget *parent = 0, QString objName = NULL,
         QString currentPath = NULL, QString labTablePath = NULL,
         int number = 0, int id = 0, int outputFormat = 0, int pix_format = 4);
  ~SaveAs();

  QString getPath();
  int getFormat();
  int getImageType();
  bool getAlphaChannel();

  QString getImageBaseName();
  int getNumbering();

  void loadSettings();
  void writeSettings();
  int getId() { return _id; }

public slots:
  void setPaths(QStringList);
  void setFormat(int);
  void setNumbering(int b);
  void setImageFormat(int f);
  void setAlphaChannel(bool a);
  void setAlphaChannelBox(int inValue);

  QString getExtension();
  void setLineEdit();
  void setNodeName();
  void preserveFormat();

  void setDirectoryList();
  void browse();
  int ok();
  void setFileNumber(int id) {
    std::stringstream st1;
    st1.fill('0');
    st1.width(4);
    st1 << id;
    _id = id;
    fileNameNumber = QString(st1.str().c_str());
  }

private:
  QStringList imageTypes, formats;
  QString nodeName;
  QString saveName;
  int numbering;
  bool alphaChannel;
  QString settingsFile;
  QString fileNameNumber;
  int _id;
  int decay;
  int previousOutputFormat;
  int previousPixFormat;
  bool changeFormat;
  bool _isLightTheme;

private slots:
  void preserveExtension();
};

#endif // SAVEAS_H
