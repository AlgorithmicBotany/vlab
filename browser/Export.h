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




#ifndef EXPORT_H
#define EXPORT_H

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
    class Export;
}

class Export : public QDialog {
    Q_OBJECT
public:
  Export(QWidget *parent = 0,NODE* root=NULL, QString objName = NULL,QString basePath="",int baseArchiveType=0,int hyperLinks = 0);
    ~Export();

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
    void setNodeName();

    void changeExtension(int);
    void preserveExtension();
    void preserveFormat();

    QString getExtension();
    void setLineEdit();

    QString getNodeName(){
      return nodeName;
    }

    bool getRecursive() ;

    void setRecursive(bool inValue);



protected:
    void changeEvent(QEvent *e);
    void nodeList(RA_Connection *conn, NODE* root, QString path, QString dirPath,bool recursive);
    void nodeList(RA_Connection *conn,  QString path, bool recursive);
    int exportObject();
    void closeEvent(QCloseEvent *event);


private:
    bool changeFormat;
    Ui::Export *ui;
    QStringList archiveTypes, formats; //, files, dirs, remoteDirs, remoteFiles, conflicts;
    TransferObject* to;
    FileConflict *conflictWindow;
    QString nodeName;
    QString saveName;

    QString exportPath;
    int baseArchiveType;
    QString settingsFile;
    int previousOutputFormat;
    int saveHyperLinks;
    NODE* _root;


};

extern SystemInfo sysInfo;	// Global Variable used for Glx/Graphics info

#endif // EXPORT_H
