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



#include "GLWindow.h"
#include "glcanvas.h"
#include "about.h"
#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QMenu>
#include <QString>
#include <QDir>
#include <QThread>
#include "control.h"
#include "comlineparam.h"
#include <iostream>

#include <QMenu>
#include <QMenuBar>
#include <QPalette>
#include <QMessageBox>

#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <iostream>

extern COMLINEPARAM clp;


GLWindow::GLWindow(QWidget *parent, int xsize, int ysize, Qt::WindowFlags flags)
    : QMainWindow(parent, flags) {
  /**** create the qglwidget ****/
  if (clp.noborder)
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

  canvas = new glcanvas(this);
  canvas->setObjectName("canvas");

  canvas->show();
  setCentralWidget(canvas);

  _resizeTimer.setSingleShot(true);
  connect(&_resizeTimer, SIGNAL(timeout()), SLOT(resizeDone()));

  // Add Help menu
  QMenuBar *bar = menuBar();
  QMenu *help = bar->addMenu("Help");
  help->addAction("About CPFG", this, SLOT(about_cpfg_cb()));
  //  help->addAction("Cpfg Help", this, SLOT(onlineHelp()));
  QAction *qHelp = help->addAction("Quick help", this, SLOT(quickHelp()));
  help->addAction("Cpfg manual", this, SLOT(pdfHelp()));
  qHelp->setEnabled(true);
  // help->addAction("Environmental Program Help", this,
  //                SLOT(enviropdfHelp()));
  //help->addAction("Graphics extensions Help (PDF)", this, SLOT(graphpdfHelp()));
  resize(xsize,ysize);
  if (!clp.graphics_output){
    // don't display the window, the window needs to be opened to use opengl routines
    canvas->setWindowOpacity(0);
    setWindowOpacity(0);
    xsize = 1;
    ysize = 1;
    //resize(xsize,ysize);
    QTimer::singleShot(0, this, &GLWindow::savePS);

  }
}

void GLWindow::savePS(){
  if (clp.savefilename[SAVE_POSTSCRIPT]){
    SaveFile(clp.savefilename[SAVE_POSTSCRIPT], SAVE_POSTSCRIPT);
  }
  if (clp.savefilename[SAVE_GLS] != NULL){
    SaveFile(clp.savefilename[SAVE_GLS], SAVE_GLS);
  }
  qApp->quit();
}

void GLWindow::resizeEvent(QResizeEvent *pEv) {
  _resizeTimer.start(500);
  QString newWindowTitle = _title;

  QString text = QString::number(canvas->width());
  text.append("/");
  text.append(QString::number(canvas->height()));
  newWindowTitle.append(QString(" (")).append(text).append(")");
  setWindowTitle(newWindowTitle);
  canvas->resize(pEv->size());
}

void GLWindow::resizeDone() { setWindowTitle(_title); }

GLWindow::~GLWindow() {
  if (canvas)
    delete canvas;
}

/********************************************************************************/

#include "apple.h"
#include <QDesktopServices>
#include <QUrl>

void GLWindow::pdfHelp() {
  QDir helpDir(getHelpDirectory());
  QString absPath = helpDir.absolutePath();
  QDesktopServices::openUrl(QUrl::fromLocalFile(
      helpDir.filePath("CPFGManual.pdf")));
}

void GLWindow::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
  QString path = helpDir.filePath("CPFGQuickHelp.html");
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
  msgBox->setWindowTitle("CPFG: Quick Help");
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

void GLWindow::enviropdfHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(QUrl::fromLocalFile(
      helpDir.filePath("EnviroManual.pdf")));
}

void GLWindow::graphpdfHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(QUrl::fromLocalFile(helpDir.filePath("graph.pdf")));
}

void GLWindow::onlineHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("CPFGManual.pdf")));
}

// about vlab slot - displays the about dialog
void GLWindow::about_cpfg_cb() {
  vlab::about(this,"Cpfg");
  return;
  /*
  QMessageBox box(NULL);
  std::string msg =
      "<p align=\"center\">\n"
      "<b><font size=\"+4\"><font "
      "color=\"#0000f5\">CPFG</font></font></b><br>\n"
      "<br>\n"
      "http://algorithmicbotany.org/virtual_laboratory/<br>\n"
      "<br>\n"
      "Developed at the University of Calgary<br>\n"
      "Department of Computer Science<br>\n"
      "<br>\n"
      "<font size=\"-1\">Copyright (C) 1990-2013 P. Prusinkiewicz,\n"
      "University of Calgary</font>\n"
      "</p>\n";

  box.setText(QString(msg.c_str()));
  box.setIconPixmap(QPixmap::fromImage(QImage(":/images/logo1.png")));
  box.show();
  box.exec();
  */
}
