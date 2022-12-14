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
#include "Preferences.h"
#include "resources.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QUrl>
#include <iostream>
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <iostream>
#include <QPushButton>




#include "ctrl.h"
#ifdef __APPLE__
#include "cocoabridge.h"
#endif

MainWindow::MainWindow() : QMainWindow() {
#ifdef __APPLE__
  CocoaBridge::setAllowsAutomaticWindowTabbing(false);
#endif
  QMenuBar *menu = menuBar();
  file = menu->addMenu("&File");
  view = menu->addMenu("&View ");
  QAction *preferences = file->addAction("Preferences...");
  connect(preferences, SIGNAL(triggered()), this, SLOT(editPreferencesCB()));

  
  help_menu = menu->addMenu("Help");
  /*
#ifdef __APPLE__
  help_menu->addAction("Function Edit Help", this, SLOT(help()));
#endif
  help_menu->addAction("Function Edit Help (PDF)", this, SLOT(pdfHelp()));
  */
  
  QAction *qHelp=help_menu->addAction("Quick help",this,SLOT(quickHelp()));
  qHelp->setEnabled(true);
  //help->addAction("Medit Help", this, SLOT(help()));
  help_menu->addAction("Tools manual", this, SLOT(pdfHelp()));

  resize(300, 220);
}

void MainWindow::help() {
#ifdef __APPLE__
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
#endif
}

void MainWindow::pdfHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}

void MainWindow::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
  QString path = helpDir.filePath("FunceditQuickHelp.html");
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
  msgBox->setWindowTitle("Funcedit: Quick Help");
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


void MainWindow::closeEvent(QCloseEvent *pEv) {
  int ret = QMessageBox::No;

  Ctrl *ctrl = dynamic_cast<Ctrl *>(centralWidget());
  if (ctrl && ctrl->_FuncModel.hasChanged()) {
    ret = QMessageBox::warning(this, "Save At Exit",
                               "Save changes before exit?", QMessageBox::Yes,
                               QMessageBox::No, QMessageBox::Cancel);
  }

  switch (ret) {
  case QMessageBox::Yes:
    ctrl->Save();
  case QMessageBox::No:
    pEv->accept();
    break;
  default:
    pEv->ignore();
  }
}

void MainWindow::nameChanged(QString s) {
  setWindowTitle(QString("Funcedit: " + s));
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
  strcat(bf, "funcedit.cfg");

  QString filePreferences = QString(bf);
  Preferences* window = new Preferences(this, filePreferences);

  Ctrl *ctrl = dynamic_cast<Ctrl *>(centralWidget());
  QObject::connect(window,SIGNAL(preferenceChanged()),ctrl,SLOT(ReloadColors()));
  window->show();
  window->raise();
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  ctrl->ReloadColors();
}

void MainWindow::msgBoxClosed(QAbstractButton*) {
   //react on button click (usually only needed when there > 1 buttons)
}
