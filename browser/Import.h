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




#ifndef IMPORT_H
#define IMPORT_H

#include <QDialog>
#include <QDir>
#include <QFileDialog>
#include <QProgressDialog>
#include <QMessageBox>
#include "stdio.h"
#include "stdlib.h"
#include "RA.h"
#include "main.h"
#include <sys/stat.h>
#include "delete_recursive.h"
#include "dos2unix.h"
#include "FileConflict.h"
#include "TransferObject.h"


namespace Ui {
    class Import;
}

class Import : public QDialog {
    Q_OBJECT
public:
  Import(QWidget *parent = 0,QString objName = NULL,QString basePath="",int baseArchiveType=0);
    ~Import();

    int getType();
    QString getPath();
    int getFormat();
    void grabConflicts(TransferObject* to);
    void receiveTransferObject(TransferObject* inObject);
    QString getBasePath(){
      return exportPath;
    }
    int getBaseArchiveType(){
      return baseArchiveType;
    }

    void loadSettings();
    void writeSettings();


public slots:
    void setType(int);
    void setPaths(QStringList);
    void setFormat(int);
    void browse();
    void ok();
    void selectingPath(const QString &text);

    QString getNodeName(){
      return nodeName;
    }


    QString getExtension();


protected:
    void changeEvent(QEvent *e);
    void nodeList(RA_Connection *conn, QString path, bool recursive);
    int importObject();
    void closeEvent(QCloseEvent *event);


private:
    bool changeFormat;
    Ui::Import *ui;
    QStringList archiveTypes, formats; //, files, dirs, remoteDirs, remoteFiles, conflicts;
    TransferObject* to;
    FileConflict *conflictWindow;
    QString nodeName;
    QString saveName;

    QString exportPath;
    int baseArchiveType;
    QString settingsFile;
    int previousOutputFormat;


};

extern SystemInfo sysInfo;	// Global Variable used for Glx/Graphics info

#endif // EXPORT_H
