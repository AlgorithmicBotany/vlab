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
#include "about.h"
#include <QDesktopServices>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QUrl>

MainWindow::MainWindow() : QMainWindow() {
  QMenuBar *menu = menuBar();
  QMenu *about = menu->addMenu("About");
  about->addAction("About", this, SLOT(about()));
  QMenu *help = menu->addMenu("Help");
  QAction *qHelp=help->addAction("Quick help",this,SLOT(quickHelp()));
  qHelp->setEnabled(true);
  //help->addAction("Medit Help", this, SLOT(help()));
  help->addAction("Tools manual", this, SLOT(pdfHelp()));
  resize(300, 220);
}

void MainWindow::help() {
  //  OpenHelpBook();
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("colormanip.html")));
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
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("QuickHelpExample.html")));
}

void MainWindow::about() {
  vlab::about(this,"Medit");
  return;
  char msg[] = "VLAB Medit Editor\nJoanne Penner\nUniversity of Calgary, 2001";

  QMessageBox *mb =
      new QMessageBox("About", QString(msg), QMessageBox::Information, 1, 0, 0);
  QPixmap icon(":icon.png");
  mb->setIconPixmap(icon.scaled(icon.width() / 2, icon.height() / 2,
                                Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation));
  mb->setButtonText(1, "OK");
  mb->exec();
}

void MainWindow::closeEvent(QCloseEvent *) { emit quit(); }
