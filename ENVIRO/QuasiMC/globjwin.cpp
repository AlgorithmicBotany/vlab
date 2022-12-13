#define GL_SILENCE_DEPRECATION
#include <QtGui>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include "globjwin.h"
#include "gldisplay.h"

#include <iostream>

GLObjectWindow::GLObjectWindow(void) : QWidget() {

  QMenuBar *menu = new QMenuBar();
  QMenu *about = menu->addMenu("About");
  about->addAction("About", this, SLOT(about()));
  QMenu *help = menu->addMenu("Help");
  help->addAction("QuasiMC Manual (PDF)", this, SLOT(pdfHelp()));

  // Create top-level layout manager
  QVBoxLayout *vlayout = new QVBoxLayout(this);
  // set layout to be full width of window
  vlayout->setContentsMargins(0, 0, 0, 0);

  // Create an openGL widget
  GLDisplay *glframe = new GLDisplay(this);
  glframe->setMinimumSize(64, 64);
  connect(this, SIGNAL(updateWindow()), glframe, SLOT(updateGLWindow()));
  connect(this, SIGNAL(updateVisualization()), glframe,
          SLOT(updateGLVisualization()));

  vlayout->addWidget(glframe);
  // On macOS, setting the menu bar makes the background black.
  // But if you don't set it, the menu bar still appears.
  // On Ubuntu, you must set the menu bar...
#ifndef __APPLE__
  vlayout->setMenuBar(menu);
#endif
  vlayout->activate();

  window_open = true;
}

void GLObjectWindow::closeEvent(QCloseEvent *event) {
  window_open = false;
  event->accept();
}

bool GLObjectWindow::isOpen(void) { return window_open; }

void GLObjectWindow::about() {
  char msg[] = "<p align=\"center\">\n"
               "<b><font size=\"+4\"><font "
               "color=\"#0000f5\">QuasiMC</font></font></b><br>\n"
               "<br>\n"
               "http://algorithmicbotany.org/virtual_laboratory/<br>\n"
               "<br>\n"
               "Developed at the University of Calgary<br>\n"
               "Department of Computer Science<br>\n"
               "<br>\n"
               "<font size=\"-1\">Copyright (C) 1990-2020 P. Prusinkiewicz,\n"
               "University of Calgary</font>\n"
               "</p>\n";
  QMessageBox box;
  box.setText(msg);
  box.setStandardButtons(QMessageBox::Ok);
  box.setDefaultButton(QMessageBox::Ok);
  box.setIcon(QMessageBox::Information);
  box.exec();
}

void GLObjectWindow::pdfHelp() {
#ifdef Q_OS_MACX
  QString path = QCoreApplication::applicationDirPath();
  QDir resource(path);
  resource.cd("../Resources/QuasiMC.help/Contents/Resources/English.lproj");
  QDir helpDir(resource);
#else
  QStringList env = QProcess::systemEnvironment();
  QStringList dir_lst = env.filter("VLABDOCDIR=");
  QDir helpDir;
  if (!dir_lst.empty()) {
    QString dir = dir_lst[0];
    if (dir.startsWith("VLABDOCDIR=")) {
      dir.remove("VLABDOCDIR=");
      if (QFile::exists(dir)) {
        helpDir = QDir(dir);
      }
    }
  }
#endif
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("EnviroManual.pdf")));
}
