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




#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <QApplication>
#include <string>
#include <vector>
#include <QMessageBox>
#include "QTGLObject.h"
#include "RA.h"
#include "libvlabd.h"
#include "mainwindow.h"

// define a type for a list of files
typedef std::vector <std::string> FileList;

// object structure created by object mgr
struct object
{
    RA_Connection * connection;
    RA_Connection * local_connection;
    std::string objName;
    std::string objDir;
    std::string tmpDir;
    // list of files from the specifications - these are the ones
    // that belong to the object
    FileList fnames;
    // list of ignore files
    FileList fnamesIgnore;
    
    /// the root directory of the database
    std::string rootDir;
    /// relative path to the object (for the
    std::string nodePath;

    std::string savingModeCommand;
    // database manipulation purposes)
    struct {
	char *filename;
	int x,y;
    } icon;
    struct spec *sp;
    // parameters settable through object.ini
    bool fitIcon; // whether to fit icon into size above, or vice versa
    QSize winSize; // window size if fitIcon = true
    QColor bgColor; // color to use when icon does not fit the whole window
    QColor pieColor,circleColor; // color of the progress "pie" and its circle
    QColor fontColor; // color of the text in the progress indicator
    int progressMinDuration; // minimum duration for progress to be shown to prevent flashing
    int progressFadeTime; // how long to fade progress away
    int operationTimeThreshold; // if time to complete operation is greater, show pie chart
    bool getSaveAlways; // show pie chart always (even for fast operations)
    int obj_posx;
    int obj_posy;
} ;


// global variables
extern struct object       obj;
extern QTGLObject        * iconForm;
extern VlabD             * vlabd; // connection to VLAB daemon
extern const char        * objShell;
extern const char        * objEd;
extern QApplication      * qapp;
extern MainWindow        * wnd ;

bool ReadSpecs (RA_Connection * connection, const std::string & specDir);
void RemoveTemp (void);
void warningMessage (const std::string & msg);
void fatalError (const std::string & str);
void qqWarning( const QString & s );
int  messageBox(QString, int, QStringList);
void readConfiguration(const QString & fileName);

#endif
