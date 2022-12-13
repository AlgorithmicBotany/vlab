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
#include "ctrl.h"
#include "resources.h"
#include <QDesktopServices>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QUrl>
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <iostream>
#include <about.h>

MainWindow::MainWindow() : QMainWindow() {
  QMenuBar *menu = menuBar();
  QMenu *help = menu->addMenu("Help");
#ifdef __APPLE__
  QMenu *about = menu->addMenu("About");
  about->addAction("About", this, SLOT(about()));
  //help->addAction("Bezier Edit Help", this, SLOT(help()));
#endif
  //help->addAction("Bezier Edit Help (PDF)", this, SLOT(pdfHelp()));
  QAction *qHelp=help->addAction("Quick help",this,SLOT(quickHelp()));
  qHelp->setEnabled(true);
  help->addAction("Tools manual", this, SLOT(pdfHelp()));

  resize(800, 520);
}

void MainWindow::help() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
}

void MainWindow::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
  QString path = helpDir.filePath("BeziereditQuickHelp.html");
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
  msgBox->setWindowTitle("Browser: Quick Help");
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

void MainWindow::about() {
  vlab::about(this,"Bezieredit");
  /*
  char msg[] = "Bezieredit\n"
               "\n"
               "by Colin Smith\n"
               "August 2001\n"
               "\n"
               "Dept. of Computer Science\n"
               "University of Calgary";

  QMessageBox *mb =
      new QMessageBox("About", QString(msg), QMessageBox::Information, 1, 0, 0);
  QPixmap icon(":icon.png");
  mb->setIconPixmap(icon.scaled(icon.width() / 2, icon.height() / 2,
                                Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation));

  mb->setButtonText(1, "OK");
  mb->exec();

  delete mb;
  */
}

void MainWindow::closeEvent(QCloseEvent *pEv) {
  Ctrl *ctrl = dynamic_cast<Ctrl *>(centralWidget());
  bool q = false;
  if (ctrl) {
    q = ctrl->quitCB();
  }
  if (!q)
    pEv->ignore();
}
