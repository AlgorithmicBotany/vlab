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
#include "ctrl.h"
#include <QMessageBox>
#include "Preferences.h"
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <iostream>
#include <QPushButton>
#include <QWindow>
#include <QScreen>


#ifdef __APPLE__
#include "cocoabridge.h"
#endif


MainWindow::MainWindow() : QMainWindow(), m_previousDevicePixelRatio(0.0) {
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

  help_menu = menu->addMenu("&Help");
  /*
#ifdef __APPLE__
  help_menu->addAction("Cupsy Help", this, SLOT(help()));
  help_menu->addAction("Cupsy Help (PDF)", this, SLOT(pdfHelp()));
#endif
  */
  QAction *qHelp=help_menu->addAction("Quick help",this,SLOT(quickHelp()));
  qHelp->setEnabled(true);
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

void MainWindow::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
  QString path = helpDir.filePath("CuspyQuickHelp.html");
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
  msgBox->setWindowTitle("Cuspy: Quick Help");
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
  Ctrl *ctrl = dynamic_cast<Ctrl *>(centralWidget());
  if (ctrl && ctrl->_FuncModel.hasChanged()) {
    ret = QMessageBox::warning(
        this, "Save At Exit", "Save changes before exit?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Cancel);
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
  setWindowTitle(QString("Function editor: " + s));
}

void MainWindow::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);
  if (m_previousDevicePixelRatio == 0.0) {
    QWindow *window = this->windowHandle();
    if (window) {
      m_previousDevicePixelRatio = window->devicePixelRatio();
      connect(window, &QWindow::screenChanged, this, &MainWindow::handleScreenChanged);
    }
  }  
}

void MainWindow::handleScreenChanged(QScreen *screen) {
    Q_UNUSED(screen); 

    QWindow *window = this->windowHandle();
    if (!window) {
        return;
    }

    qreal currentDevicePixelRatio = window->devicePixelRatio();

    // if ratio changed, trigger a repaint
    if (m_previousDevicePixelRatio != currentDevicePixelRatio) {
      Ctrl *ctrl = dynamic_cast<Ctrl *>(centralWidget());
      if (ctrl) {
        ctrl->resetView();
        ctrl->update();
      }
    }

    m_previousDevicePixelRatio = currentDevicePixelRatio;
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
  strcat(bf, "cuspy.cfg");

  QString filePreferences = QString(bf);
  Preferences *window = new Preferences(this, filePreferences);
  Ctrl *ctrl = dynamic_cast<Ctrl *>(centralWidget());
  QObject::connect(window,SIGNAL(preferenceChanged()),ctrl,SLOT(ReloadColors()));
  window->show();
  window->raise();
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  ctrl->ReloadColors();
}
