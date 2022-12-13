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
#include "glcolourmap.h"
#include "palette.h"
#include "resources.h"
#include <QDesktopServices>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QUrl>
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <iostream>
#include <QPushButton>
#include "about.h"

MainWindow::MainWindow() : QMainWindow() {
  QMenuBar *menu = menuBar();
  QMenu *about = menu->addMenu("About");
  about->addAction("About", this, SLOT(about()));
  QMenu *help = menu->addMenu("Help");
  //help->addAction("Palette Help", this, SLOT(help()));
     QAction *qHelp=help->addAction("Quick help",this,SLOT(quickHelp()));
  qHelp->setEnabled(true);
  //help->addAction("Medit Help", this, SLOT(help()));
  help->addAction("Tools manual", this, SLOT(pdfHelp()));

  //help->addAction("Palette Help (PDF)", this, SLOT(pdfHelp()));
  resize(300, 220);
}

void MainWindow::help() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABToolsManual.pdf")));
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
  QString path = helpDir.filePath("PaletteQuickHelp.html");
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
  msgBox->setWindowTitle("Palette: Quick Help");
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

void MainWindow::about() {
  vlab::about(this,"Palette");
  /*
  char msg[] =
      "VLAB Palette Editor\nJoanne Penner\nUniversity of Calgary, 2001";

  QMessageBox *mb =
      new QMessageBox("About", QString(msg), QMessageBox::Information, 1, 0, 0);
  QPixmap icon(":icon.png");
  mb->setIconPixmap(icon.scaled(icon.width() / 2, icon.height() / 2,
                                Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation));
  mb->setButtonText(1, "OK");
  mb->exec();
  */
}

void MainWindow::closeEvent(QCloseEvent *) { emit quit(); }
