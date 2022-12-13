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




/* MaterialWorld

   Definition of Class: MW

*/

#ifndef MW_H
#define MW_H

#include <cstdlib>
#include <iostream>
#include <fstream>

#include <qapplication.h>
#include <qgl.h>
#include <qwidget.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qstatusbar.h>
#include <QByteArray>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <QScrollArea>
#include <QStatusBar>
#include <QSpinBox>
#include <QFileDialog>
#include <QTextStream>
#include <QUrl>
#include <QLabel>
#include <QCloseEvent>

#include "mweditor.h"
#include "mwviewer.h"
#include "mwdialogs.h"

class MW : public QMainWindow{
  Q_OBJECT
    
    public:
  MW(int,char**,QWidget* parent=0);

  public slots:
    void editSmooth();
  void onlinehelp();
  void quickHelp();
  void pdfHelp();
  void setModified(bool);
  void newfile();
  void newWindow();
  void loadWindow();
  void load();
  void save();
  void saveas();
  void pref();
  void list();
  void pageflip(int,int);
  void notice(const QString&);
  void error(const QString&,int);
  void bg();
  void about();
  void selectOff(bool);
  void setselect(int,Material);
  void enablePaste();
  void revertsaved();
  void saveconfig();
  void setSavingMode(SavingMode savingMode){
    _savingMode = savingMode;
  }
  void saveInTriggeredMode(){
    emit triggered();
  }

 signals:
  void read(QFile*);
  void write(QFile*);
  void pagerange(int,int);
  void select(int,int);
  void imageON(bool);
  void modify_save_viewer(bool);
  void modify_saveas_viewer(bool);
  void quit();
  void triggered();
  
 private:
  void init();
  int loadfile(QString);
  void loadnewfile(QString);
  void parse(int,SavingMode &savingMode,char**);
  void loadconfiguration(QFile*);
  void saveconfiguration(QFile*);
  void closeEvent(QCloseEvent*);
  bool askSave(QString message=QString());

  QFile* materialfile;
  QString* filename;
  QMenu *file,*options,*edit,*help;
  QLabel* indexlabel;
  QLabel* pagelabel;
  QLabel* pglabel;
  QSpinBox* pagestepper;
  QLabel* rangelabel;
  QSpinBox* indexstepper;
  MWViewer* viewer;
  MWEditor* editor;
  bool modified; 
  SelectRangeDialog* RD;
  ImageDialog* imqual;
  QAction* _xs,*_sm,*_md,*_lg,*_cs,*_nx,*_pv,*_fp,*_ps,*_bg,*_rs,*_sv,*_in;
  SavingMode _savingMode;
};

#endif

// EOF: mw.h
