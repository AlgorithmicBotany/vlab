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



// Based on Qt Example

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <highlighter.h>
#include <codeeditor.h>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QPlainTextEdit;

QT_END_NAMESPACE
class DirectoryWatcher;

//! [0]
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QString);

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

protected:
  void closeEvent(QCloseEvent *event);

private slots:
  void newFile();
  void open();
  bool save();
  bool saveAs();
  void about();
  void documentWasModified();
  void increaseFontSize();
  void decreaseFontSize();
  void RequestReload(const QString &);
  void reloadFile(const QString &);
  // timer slot
  void Idle();
  //helps
  void quickHelp();
  void pdfHelp();

private:
  void createActions();
  void createMenus();
  void createToolBars();
  void createStatusBar();
  void SetContinuousMode(bool enable);

  void readSettings();
  void writeSettings();
  bool maybeSave();
  void loadFile(const QString &fileName);
  bool saveFile(const QString &fileName);
  void setCurrentFile(const QString &fileName);
  QString strippedName(const QString &fullFileName);

  CodeEditor *textEdit;
  QString curFile;
  Highlighter *highlighter;

  DirectoryWatcher *directoryWatcher;
  int new_reload_pending;

  QMenu *fileMenu;
  QMenu *editMenu;
  QMenu *helpMenu;
  QMenu *viewMenu;
  QToolBar *fileToolBar;
  QToolBar *editToolBar;
  QAction *newAct;
  QAction *openAct;
  QAction *saveAct;
  QAction *saveAsAct;
  QAction *exitAct;
  QAction *cutAct;
  QAction *copyAct;
  QAction *pasteAct;
  QAction *aboutAct;
  QAction *aboutQtAct;
  QAction *increaseFontAct;
  QAction *decreaseFontAct;

  QWidget *lineNumberArea;
  int fontSize;
  int saveFromTextEditor;
};

#endif
