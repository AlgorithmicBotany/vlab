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



#include "mainwindow.h"
#include "resources.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QCloseEvent>
#include <QMessageBox>
#include <signal.h>
#include <unistd.h>

#include "Preferences.h"
#include "about.h"
#include "gallery.h"
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <iostream>
#include <QPushButton>

#ifdef __APPLE__
#include "cocoabridge.h"
#endif


MainWindow::MainWindow() : QMainWindow() {
#ifdef __APPLE__
  CocoaBridge::setAllowsAutomaticWindowTabbing(false);
#endif
  // Construct menubar
  QMenuBar *menu;
#if defined(Q_WS_MAC)
  menu = new QMenuBar(0);
#else
  menu = menuBar();
#endif
  file = menu->addMenu("&File");
  view = new QMenu(this);
  view->setTitle("&View ");
  menu->addMenu(view);
  QAction *preferences = file->addAction("Preferences...");
  connect(preferences, SIGNAL(triggered()), this, SLOT(editPreferencesCB()));
  QAction *about = file->addAction("About...");
  connect(about, SIGNAL(triggered()), this, SLOT(about()));

  help_menu = menu->addMenu("&Help");
  /*
#ifdef __APPLE__
  help_menu->addAction("Gallery Help", this, SLOT(help()));
  help_menu->addAction("Gallery Help (PDF)", this, SLOT(pdfHelp()));
#endif
  */
  QAction *qHelp=help_menu->addAction("Quick help",this,SLOT(quickHelp()));
  qHelp->setEnabled(true);
  help_menu->addAction("Tools manual", this, SLOT(pdfHelp()));
}

MainWindow::~MainWindow() {
  delete _gallery;
}

void MainWindow::help() {
#ifdef __APPLE__
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("gallery.html")));
#endif
}

void MainWindow::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
  QString path = helpDir.filePath("GalleryQuickHelp.html");
  QFile f(path);
  if (!f.open(QFile::ReadOnly | QFile::Text)) {
    std::cerr<<"Path: "<<path.toStdString()<<"doesn't exist"<<std::endl;
    return;
  }
  QTextStream in(&f);
  QString message = in.readAll();
  QTextBrowser *tb = new QTextBrowser(this);
  tb->setOpenExternalLinks(true);
  tb->setHtml(message);

  QDialog *msgBox = new QDialog;
  msgBox->setWindowTitle("Gallery: Quick Help");
  msgBox->setWindowFlags(Qt::Dialog);
  msgBox->setModal(false);
  QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
  QPushButton* okBtn = bb->button(QDialogButtonBox::Ok);
  connect(okBtn, SIGNAL(clicked()),msgBox,SLOT(close()));
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(tb);
  layout->addWidget(bb);
  msgBox->setLayout(layout);
  msgBox->resize(400,300);
 
  msgBox->show();
}

void MainWindow::pdfHelp() {
   QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}

void MainWindow::closeEvent(QCloseEvent *pEv) {
  int ret = QMessageBox::No;
  if (_gallery && _gallery->hasChanged()) {
    ret = QMessageBox::warning(
        this, "Save At Exit", "Save changes before exit?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Cancel);
  }

  switch (ret) {
  case QMessageBox::Yes:
    _gallery->saveAll();
  case QMessageBox::No:
    pEv->accept();
    break;
  default:
    pEv->ignore();
  }
  _gallery->CleanUp();
//  delete _gallery; // moved to destructor
}

void MainWindow::about() {
  vlab::about(this,"Gallery");
  return;
  QMessageBox::information(
      this, "About",
      "VLAB Gallery\n\nDeveloppers:\nColin Smith, Pascal Ferraro\n\nDocumentation:\nColin Smith, Lynn Mercer\n",
      "Close");
}


void MainWindow::nameChanged(QString s) {
  setWindowTitle(QString("Function editor: " + s));
}

/******************************************************************************
 *
 * callback for the 'edit preferences'
 *
 */
void MainWindow::editPreferencesCB() {

  QString userConfigDir = "";
#ifdef __APPLE__
  userConfigDir = Vlab::getUserConfigDir(false);
#else
#endif

  char bf[PATH_MAX + 1];
  const char *cdir = userConfigDir.toStdString().c_str();
  if (NULL == cdir)
    return;
  else {
    strcpy(bf, cdir);
    strcat(bf, "/");
  }
  strcat(bf, "gallery.cfg");

  QString filePreferences = QString(bf);
  Preferences *window = new Preferences(this, filePreferences);
  //Gallery *gallery = dynamic_cast<Gallery *>(centralWidget());
  QObject::connect(window,SIGNAL(preferenceChanged()),_gallery,SLOT(changeSize()));
  window->show();
  window->raise();
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  _gallery->changeSize();
}
