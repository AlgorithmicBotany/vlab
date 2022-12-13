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
//! [0]
#include "about.h"
#include <iostream>
#include <QFileDialog>
#include <QAction>
#include <QTimer>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSettings>
#include <QApplication>
#include <QScrollBar>
#include <QTextStream>

#include "mainwindow.h"
#include <directorywatcher.h>
#include "resources.h"
#include <QDesktopServices>
#include <QDir>


#ifndef WIN32
#include <unistd.h>
#endif

namespace {
// timer for idle function
QTimer *idleTimer;
} // namespace

//! [0]

//! [1]
MainWindow::MainWindow(QString fileName)
//! [1] //! [2]
{
  /******** timer to run idle function ********/

  // create and set up the timer for the IdleFunction
  idleTimer = new QTimer(this);
  connect(idleTimer, SIGNAL(timeout()), SLOT(Idle()));

  saveFromTextEditor = 0;
  directoryWatcher = NULL;
  fontSize = 12;
  QFont font;
  font.setFamily("Menlo");
  font.setStyleHint(QFont::TypeWriter);
  font.setFixedPitch(true);
  font.setPointSize(fontSize);

  textEdit = new CodeEditor;
  textEdit->setFont(font);
  const int tabStop = 10; // 4 characters
  QFontMetrics metrics(font);
  textEdit->setTabStopWidth(tabStop * metrics.width(' '));

  textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);

  QPalette p = textEdit->palette();

  p.setColor(QPalette::Active, QPalette::Base, Qt::black);
  p.setColor(QPalette::Inactive, QPalette::Base, Qt::black);
  p.setColor(QPalette::Text, Qt::white);
  textEdit->setPalette(p);

  highlighter = new Highlighter(textEdit->document());

  setCentralWidget(textEdit);
  createActions();
  createMenus();
  createStatusBar();
  // continuousMode always on

  readSettings();

  connect(textEdit->document(), SIGNAL(contentsChanged()), this,
          SLOT(documentWasModified()));
  if (!fileName.isEmpty()){
    setCurrentFile(fileName);
    loadFile(fileName);
    SetContinuousMode(true);
  }
  else{
    setCurrentFile("");
    SetContinuousMode(false);
    saveAct->setEnabled(false);
    textEdit->setEnabled(false);
  }

  QFileInfo fileInfo(fileName);
  QString name(fileInfo.fileName());
  // The window title must contain a [*] placeholder (see Qt API docs)
  setWindowTitle("vlabTextEdit: " + name + "[*]");

  setUnifiedTitleAndToolBarOnMac(true);
}
//! [2]

//! [3]
void MainWindow::closeEvent(QCloseEvent *event)
//! [3] //! [4]
{
  if (maybeSave()) {
    writeSettings();
    event->accept();
  } else {
    event->ignore();
  }
}
//! [4]

void MainWindow::increaseFontSize() {
  fontSize += 1;
  // this is to workaround a Qt bug with tabs

  if (fontSize % 10 == 6)
    fontSize += 1;
  if (fontSize % 10 == 8)
    fontSize += 1;
  if (fontSize % 10 == 1)
    fontSize += 1;
  if (fontSize % 10 == 3)
    fontSize += 1;

  QFont font = textEdit->font();
  font.setPointSize(fontSize);
  font.setStyleHint(QFont::Monospace);
  font.setFixedPitch(true);
  const int tabStop = 10; // 4 characters
  QFontMetrics metrics(font);
  textEdit->setTabStopWidth(tabStop * metrics.width(' '));
  textEdit->setFont(font);
}

void MainWindow::decreaseFontSize() {
  fontSize -= 1;
  // this is to workaround a Qt bug with tabs
  if (fontSize % 10 == 6)
    fontSize -= 1;
  if (fontSize % 10 == 8)
    fontSize -= 1;
  if (fontSize % 10 == 1)
    fontSize -= 1;
  if (fontSize % 10 == 3)
    fontSize -= 1;

  QFont font = textEdit->font();
  font.setPointSize(fontSize);
  font.setStyleHint(QFont::Monospace);
  font.setFixedPitch(true);
  const int tabStop = 10; // 4 characters
  QFontMetrics metrics(font);
  textEdit->setTabStopWidth(tabStop * metrics.width(' '));
  textEdit->setFont(font);
}

//! [5]
void MainWindow::newFile()
//! [5] //! [6]
{
  if (maybeSave()) {
    textEdit->clear();
    setCurrentFile("");
  }
}
//! [6]

//! [7]
void MainWindow::open()
//! [7] //! [8]
{
  if (maybeSave()) {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
      loadFile(fileName);
  }
}
//! [8]

//! [9]
bool MainWindow::save()
//! [9] //! [10]
{
  if (curFile.isEmpty()) {
    return saveAs();
  } else {
    return saveFile(curFile);
  }
}
//! [10]

//! [11]
bool MainWindow::saveAs()
//! [11] //! [12]
{
  QString fileName = QFileDialog::getSaveFileName(this);
  if (fileName.isEmpty())
    return false;
  saveAct->setEnabled(true);
  textEdit->setEnabled(true);
  QFileInfo fileInfo(fileName);
  QString name(fileInfo.fileName());
  setWindowTitle("vlabTextEdit: " + name + "[*]");

  return saveFile(fileName);
}
//! [12]

//! [13]
void MainWindow::about()
//! [13] //! [14]
{
  vlab::about(this,"Vlabtexteditor");
  return;
  QMessageBox::about(
      this, tr("About"),
      tr("The <b>VlabTextEdit</b> This text editor is the one provided by "
         "default with vlab. It can be changed for any other text editor by "
         "editing Preferences in object (cf. object documentation.)\n The code is based on Qt Code Editor Example."));
}
//! [14]

//! [15]
void MainWindow::documentWasModified()
//! [15] //! [16]
{
  setWindowModified(textEdit->document()->isModified());
}
//! [16]

//! [17]
void MainWindow::createActions()
//! [17] //! [18]
{
  newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
  newAct->setShortcuts(QKeySequence::New);
  newAct->setStatusTip(tr("Create a new file"));
  connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

  //! [19]
  openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
  //! [18] //! [19]

  saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
  saveAct->setShortcuts(QKeySequence::Save);
  saveAct->setStatusTip(tr("Save the document to disk"));
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
  connect(textEdit,SIGNAL(save()),this,SLOT(save()));
  saveAsAct = new QAction(tr("Save &As..."), this);
  saveAsAct->setShortcuts(QKeySequence::SaveAs);
  saveAsAct->setStatusTip(tr("Save the document under a new name"));
  connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  //! [20]
  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcuts(QKeySequence::Quit);
  //! [20]
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  //! [21]
  cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
  //! [21]
  cutAct->setShortcuts(QKeySequence::Cut);
  cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                          "clipboard"));
  connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));

  copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
  copyAct->setShortcuts(QKeySequence::Copy);
  copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                           "clipboard"));
  connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));

  pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
  pasteAct->setShortcuts(QKeySequence::Paste);
  pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                            "selection"));
  connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));

  aboutAct = new QAction(tr("&About"), this);
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  //! [22]
  aboutQtAct = new QAction(tr("About &Qt"), this);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
  //! [22]

  //! [23]
  cutAct->setEnabled(false);
  //! [23] //! [24]
  copyAct->setEnabled(false);
  connect(textEdit, SIGNAL(copyAvailable(bool)), cutAct,
          SLOT(setEnabled(bool)));
  connect(textEdit, SIGNAL(copyAvailable(bool)), copyAct,
          SLOT(setEnabled(bool)));

  increaseFontAct = new QAction(tr("&Increase Font"), this);
  increaseFontAct->setShortcuts(QKeySequence::ZoomIn);
  increaseFontAct->setStatusTip(tr("Zoom In"));

  connect(increaseFontAct, SIGNAL(triggered()), this, SLOT(increaseFontSize()));
  decreaseFontAct = new QAction(tr("&Increase Font"), this);
  decreaseFontAct->setShortcuts(QKeySequence::ZoomOut);
  decreaseFontAct->setStatusTip(tr("Zoom Out"));

  connect(decreaseFontAct, SIGNAL(triggered()), this, SLOT(decreaseFontSize()));
}
//! [24]

//! [25] //! [26]
void MainWindow::createMenus()
//! [25] //! [27]
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newAct);
  //! [28]
  fileMenu->addAction(openAct);
  //! [28]
  fileMenu->addAction(saveAct);
  //! [26]
  fileMenu->addAction(saveAsAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(cutAct);
  editMenu->addAction(copyAct);
  editMenu->addAction(pasteAct);

  viewMenu = menuBar()->addMenu(tr("&View"));

  viewMenu->addAction(increaseFontAct);
  viewMenu->addAction(decreaseFontAct);

  menuBar()->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);
  QAction *qHelp=helpMenu->addAction("Quick help",this,SLOT(quickHelp()));
  qHelp->setEnabled(false);
  helpMenu->addAction("Tools manual", this, SLOT(pdfHelp()));

}
//! [27]

//! [29] //! [30]
void MainWindow::createToolBars() {
  fileToolBar = addToolBar(tr("File"));
  fileToolBar->addAction(newAct);
  //! [29] //! [31]
  fileToolBar->addAction(openAct);
  //! [31]
  fileToolBar->addAction(saveAct);

  editToolBar = addToolBar(tr("Edit"));
  editToolBar->addAction(cutAct);
  editToolBar->addAction(copyAct);
  editToolBar->addAction(pasteAct);
}
//! [30]

//! [32]
void MainWindow::createStatusBar()
//! [32] //! [33]
{
  statusBar()->showMessage(tr("Ready"));
}
//! [33]

void MainWindow::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
}

void MainWindow::pdfHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}



//! [34] //! [35]
void MainWindow::readSettings()
//! [34] //! [36]
{
  QSettings settings("VlabTextEditor", "Application settings");
  QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
  QSize size = settings.value("size", QSize(400, 400)).toSize();
  resize(size);
  move(pos);
}
//! [35] //! [36]

//! [37] //! [38]
void MainWindow::writeSettings()
//! [37] //! [39]
{
  QSettings settings("VlabTextEditor", "Application settings");
  settings.setValue("pos", pos());
  settings.setValue("size", size());
}
//! [38] //! [39]

//! [40]
bool MainWindow::maybeSave()
//! [40] //! [41]
{
  if (textEdit->document()->isModified()) {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard |
                                   QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
      return save();
    else if (ret == QMessageBox::Cancel)
      return false;
  }
  return true;
}
//! [41]

void MainWindow::SetContinuousMode(bool enable) {

  if (enable && !directoryWatcher) {
    QStringList ignoredPrefixes;
    QStringList ignoredSuffixes;

    directoryWatcher = new DirectoryWatcher(QFileInfo(curFile).absolutePath(),
                                            ignoredPrefixes << "lsys.",
                                            ignoredSuffixes << ".i"
                                                            << ".log"
                                                            << ".ii"
                                                            << ".o"
                                                            << ".o",
                                            this);
    connect(directoryWatcher, SIGNAL(fileChanged(QString)), this,
            SLOT(RequestReload(QString)));
  }
}

void MainWindow::RequestReload(const QString &f) {

  QFileInfo fi(f);
  QString fname = fi.fileName();

  if (!idleTimer->isActive()) {
    idleTimer->setSingleShot(false);
    idleTimer->start(50);
    if (curFile.compare(fname) == 0)
      reloadFile(f);

  } else {
    if (curFile.compare(fname) == 0)
      new_reload_pending = true;
  }
}

void MainWindow::Idle() {
  idleTimer->stop();
  QWidget::setCursor(Qt::ArrowCursor);
  if (!new_reload_pending) {
    new_reload_pending = false;
    reloadFile(curFile);
  }
}

// [Pascal] keep opening the file until the size is stable
int waitOpenFile(const char *fname) {
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  FILE *fp = fopen(fname, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  //  file is missing
  while ((fp == NULL) && (counter < 10000)) {
    fp = fopen(fname, "r");
    counter++;
  }
  if (counter == 10000) {
    fprintf(stderr, "WARNING: Can't open file %s.\n", fname);
    return 0;
  } else {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size = ftell(fp);       // get current file pointer
    counter = 0;
    while ((size == 0) || (current_size != size)) {
      current_size = size;
      fclose(fp);
      if (counter >= 10000)
        break;
      fp = NULL;
      while (fp == NULL) {
        fp = fopen(fname, "r");
        counter++;
      }
      fseek(fp, 0, SEEK_END); // seek to end of file
      size = ftell(fp);       // get current file pointer
    }
  }
  fseek(fp, 0L, SEEK_SET);
  fclose(fp);
  if (counter >= 10000) {
    std::cerr << "VLabTextEdit can't reload the file : " << counter
              << std::endl;
    return 0;
  }

  return 1;
}

void MainWindow::reloadFile(const QString &f) {

  QFileInfo fi(f);
  QString fname = fi.fileName();

  if (curFile.compare(fname) != 0)
    return;
  std::string onefile = f.toStdString();
  if (waitOpenFile(onefile.c_str()) == 0)
    return;
  int position = textEdit->textCursor().position();

  int vScrollPosition = textEdit->verticalScrollBar()->value();
  int hScrollPosition = textEdit->horizontalScrollBar()->value();

  int cpt = 0;
  std::string file = f.toStdString();
#ifndef WIN32
  while ((access(file.c_str(), 0) != 0) && (cpt < 1000))
    cpt++;

  if (cpt >= 1000) {
    std::cerr << "WARNING THE FILE " << f.toStdString() << " doesn't exist"
              << std::endl;
    return;
  }
#endif
  loadFile(curFile);
  // Restore settings
  QTextCursor cursor = textEdit->textCursor();
  cursor.setPosition(position);
  textEdit->setTextCursor(cursor);

  textEdit->verticalScrollBar()->setValue(vScrollPosition);
  textEdit->horizontalScrollBar()->setValue(hScrollPosition);
  statusBar()->showMessage(tr("File reloaded"), 2000);
}

//! [42]
void MainWindow::loadFile(const QString &fileName)
//! [42] //! [43]
{
  QFile file(fileName);
  if (!file.open(QFile::ReadWrite | QFile::Text)) {
    QMessageBox::warning(this, tr("Application"),
                         tr("Cannot write file %1:\n%2. \n In order to save "
                            "this file you will have to change the path.")
                             .arg(fileName)
                             .arg(file.errorString()));

    return;
  }

  QFileInfo fileInfo(fileName);
  QString name(fileInfo.fileName());
  setWindowTitle("vlabTextEdit: " + name + "[*]");

  QTextStream in(&file);

#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
  textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif

  setCurrentFile(fileName);
  statusBar()->showMessage(tr("File loaded"), 2000);
  saveAct->setEnabled(true);
  textEdit->setEnabled(true);
}
//! [43]

//! [44]
bool MainWindow::saveFile(const QString &fileName)
//! [44] //! [45]
{
  QFile file(fileName);
  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(
        this, tr("Application"),
        tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
    return false;
  }

  QTextStream out(&file);

#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
  // start saving file
  saveFromTextEditor = 1;
  out << textEdit->toPlainText();
#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif

  setCurrentFile(fileName);
  statusBar()->showMessage(tr("File saved"), 2000);
  // file is saved
  saveFromTextEditor = 2;
  saveAct->setEnabled(true);
  textEdit->setEnabled(true);
  return true;
}
//! [45]

//! [46]
void MainWindow::setCurrentFile(const QString &fileName)
//! [46] //! [47]
{
  curFile = fileName;
  textEdit->document()->setModified(false);
  setWindowModified(false);

  QString shownName = curFile;
  if (curFile.isEmpty())
    shownName = "untitled.txt";
  setWindowFilePath(shownName);
}
//! [47]

//! [48]
QString MainWindow::strippedName(const QString &fullFileName)
//! [48] //! [49]
{
  return QFileInfo(fullFileName).fileName();
}
//! [49]
