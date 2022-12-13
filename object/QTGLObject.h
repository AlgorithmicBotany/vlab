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




#ifndef __QTGLOBJECT_H
#define __QTGLOBJECT_H

#include <string>
#include <QLabel>
#include <qsocketnotifier.h>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QImage>
#include <QMenu>
#include <QTime>
#include <QTimer>
#include <iostream>
#include "icon.h"
#include <QGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>

class QMenuBar;

class QTGLObject;
class DirectoryWatcher;


class Progress : public QObject {
    Q_OBJECT
public:
    Progress( QTGLObject & qtglobject );
    virtual ~Progress() {}
    void show( bool on = true, bool ignoreTimeThreshold = false );
    void advance();
    void setup( double total = 100, double inc = 1, double curr = 0 );
    double _total, _inc, _curr;
    bool _show, _effectiveShow; // caller request show status, effective status
    void render(); // renders itself
    QTGLObject & _qtglobject; // pointer to the GL window
    QTime _time; // measure the time from when show was called
    int  _hideRequestTime; // when was the hide called since show
    int _operationTimeThreshold;
};

class QTGLObject : public QOpenGLWidget, public QOpenGLFunctions {
    Q_OBJECT
public:
    QTGLObject(QWidget* parent=0, Qt::WindowFlags f=0);
    ~QTGLObject();
    void loadIcon();
    void MakeMenu();
    void animate( bool on );

    Progress & progress() { return _progress; }
    Progress _progress;
    int waitOpenFile(const char* fname);
    void SetContinuousMode(std::string tmpDir);


public slots:
    void connectSocket();
    void SpecHandler();
    void ExecSnap();
    void QuitCb();
    void NewIconCb();
    void editSpecCb();
    void NewMenuCb();
    void labTableShellCb();
  #ifdef __APPLE__
    void openLabTableFinderCb();
    void openStorageFinderCb();
   //void ConsoleShellCb();
  #endif
    void storageShellCb();
    void dirviewLab();
    void dirviewStorage();
    void dirviewCB(int flag);
    void PutFileCb();
    void GetFileCb();
    void saveChangesCb();
    void exportObject();

    void NewVersionCb();
    void posObj();
    void help();
    void pdfHelp();
  void quickHelp();
    void about_object_cb();
//    void read_messages(int socket);
    void editPreferencesCB();
    void rereadPreferencesCB();
    void shellPreferencesCB();
    void loadAllFiles();

    void RequestReload(const QString&);
    //timer slot
    void Idle();

//protected slots:
//    void updatePixmap();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void closeEvent(QCloseEvent* ev);
    virtual void mousePressEvent(QMouseEvent* ev);
    virtual void mouseReleaseEvent(QMouseEvent* ev);
    virtual void mouseMoveEvent( QMouseEvent * ev );
    bool eventFilter(QObject *object,QEvent *e);
	
    //void contextMenuEvent(QContextMenuEvent *e);

    void findTopLevel();

    QPoint _buttonDownPos;
    QWidget *_topLevel;

private:
    DirectoryWatcher* directoryWatcher;
    int new_reload_pending;


    void ExecuteCommand( QString cmd );
    void positionCb(const std::string & objName);
    int  changesMade();
    void SpecMenu(QMenu* parent, struct spec* sp);
    void UtilMenu();
    struct spec* LocateSpecEntry(struct spec* sp, QAction* item);
//    void ProcessMessage( long type, char * msg);

    QLabel* imgLabel;            //Image label widget
    QWidget* errWin;             //Error window widget
    QMenu* menu;            //Menu widget
    QMenu* oldMenu;         //Menu widget
//    QSocketNotifier* notifier;   //Network socket handler
    QImage iconPicture;         //Icon picture

    QString exportPath;
    QString exportName;
    int exportArchiveType;
    bool _init;
    bool mouseLeftPressed ;

    QMenuBar *menubar;
    QSurfaceFormat _format;
};

#endif
