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



#include "about.h"
#include "version.h"
#include "resources.h"
#include <iostream>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <QFile>
#include <QTextStream>
#include <QTextBrowser>
#include <QPushButton>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDesktopServices>
#include <QSpacerItem>

#include "ui_about.h"
using namespace Qt;

// void qInitImages_misc();

namespace vlab {

void about(QWidget *parent) {
  QMessageBox box(parent);
  std::string msg =
    "<p align=\"center\">\n"
    "<b><font size=\"+4\"><font "
    "color=\"#0000f5\">VLAB</font></font></b><br>\n"
    "Version " +
    vlab::version_string() +
    " <br>\n"
    "(" +
    vlab::build_info() +
    ")<br>\n"
    "<br>\n"
    "http://algorithmicbotany.org/virtual_laboratory/<br>\n"
    "<br>\n"
    "Developed at the University of Calgary<br>\n"
    "Department of Computer Science<br>\n"
    "<br>\n"
    "<p align=\"justify\">\n"
    "<font size=\"-1\">Copyright (C) 1990-2022 P. Prusinkiewicz,\n"
    "University of Calgary</font>\n"
    "\n"
    "<font size=\"-1\">Note:<br>\n"
    //"This build does not include modifications from Radek Karwowski made
    // since April, 30th 2011.<br>\n"
    " Version tested on Mac OS 10.13.6<br>\n"
    "</p>\n";

  box.setText(QString(msg.c_str()));
  box.setIconPixmap(QPixmap::fromImage(QImage(":/images/logo1.png")));
  //    box.setIconPixmap( QPixmap( vlab::root_dir() +
  //    "/resources/images/about-icon.png" ));
  box.show();
  box.exec();
}

  
  void about(QWidget *parent, QString programName ) {

    std::string version =
      "Version " +
      vlab::version_string() +
      " - " +
      vlab::build_info() + std::string("\n") ;
    
    QDialog box(parent);
  

    Ui::About ui;
    ui.setupUi(&box);

    box.setStyleSheet("background-color: white;");
    ui.appName->setText(programName);
    QPixmap pix(":/vlabLogo.png");
    pix = pix.scaled(QSize(150,150),  Qt::KeepAspectRatio,Qt::SmoothTransformation);
    ui.logo->setPixmap(pix);
    ui.version->setText(version.c_str());

    /* credits */
    //QDialog *credits = openFile("credits.html","Credits",&box);
    //QDialog *license = openFile("license.html","License",&box);
    //QDialog *notices = openFile("vlabCopyrights.pdf","Third party notices",&box);
    openPDF *pdfFiles = new openPDF();
    /****/
    //QObject::connect ( ui.credits_button, SIGNAL( clicked() ), credits, SLOT(show() ));
    //QObject::connect ( ui.LicenceButton, SIGNAL( clicked() ), license, SLOT(show() ));
    QObject::connect ( ui.LicenceButton, SIGNAL( clicked() ),  pdfFiles, SLOT(openLicence()) );
    QObject::connect ( ui.credits_button, SIGNAL( clicked() ),  pdfFiles, SLOT(openCredits()) );
    QObject::connect ( ui.notices_button, SIGNAL( clicked() ),  pdfFiles, SLOT(open3rdParty()) );
    box.move(100,100);
    box.setWindowTitle(programName);
    box.setFixedSize(411,393);
#ifndef __APPLE__
    // adjust font sizes based on screenDpi so they fit in the 411x393 window
    // macOS seems to handle this?!
    qreal screenDpi = box.logicalDpiX();

    QFont f(ui.appName->font());
    f.setPointSizeF(f.pointSizeF() * 72. / screenDpi);
    ui.appName->setFont(f);

    f = ui.label_2->font();
    f.setPointSizeF(f.pointSizeF() * 72. / screenDpi);
    ui.label_2->setFont(f);

    f = ui.logo->font();
    f.setPointSizeF(f.pointSizeF() * 72. / screenDpi);
    ui.logo->setFont(f);
    
    f = ui.label_3->font();
    f.setPointSizeF(f.pointSizeF() * 72. / screenDpi);
    ui.label_3->setFont(f);

    f = ui.version->font();
    f.setPointSizeF(f.pointSizeF() * 72. / screenDpi);
    ui.version->setFont(f);

    f = ui.label_5->font();
    f.setPointSizeF(f.pointSizeF() * 72. / screenDpi);
    ui.label_5->setFont(f);
#endif
    box.show();
    box.exec();
  }


  QDialog* openFile(QString dialogPath, QString title, QDialog *box){
  /* credits */
  QDir helpDir(getHelpDirectory());
 #ifndef __APPLE__
  helpDir.cd("files");
#endif
 QString path = helpDir.filePath(dialogPath);
  QFile f(path);
  if (!f.open(QFile::ReadOnly | QFile::Text)) {
    std::cerr<<"Path: "<<path.toStdString()<<"doesn't exist"<<std::endl;
    return NULL;
  }
  QTextStream in(&f);
  QString message = in.readAll();
  QTextBrowser *tb = new QTextBrowser(box);
  tb->setOpenExternalLinks(true);
  tb->setHtml(message);

  QDialog *msgBox = new QDialog;
  msgBox->setWindowTitle(title);
  msgBox->setWindowFlags(Qt::Dialog);
  msgBox->setModal(false);
  QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
  QPushButton* okBtn = bb->button(QDialogButtonBox::Ok);
  QObject::connect(okBtn, SIGNAL(clicked()),msgBox,SLOT(close()));
  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(tb);
  layout->addWidget(bb);
  msgBox->setLayout(layout);
  msgBox->resize(400,300);
  return msgBox;
}


void aboutRayshade(QWidget *parent) {
  QMessageBox box(parent);
  std::string msg =
      "<p align=\"Left\">\n"
      "<b><font size=\"+2\">Rayshade</font></b><br>\n"
      "<font size=\"-1\">Version 4.0<br>\n"
      "<br>\n"
      "Vlab includes Rayshade v4.0, which was written by Craig Kolb and Rod "
      "Bogart,<br>\n"
      "with the kind permission of Craig Kolb.<br>\n"
      " <br>\n"
      "For compatibility with vlab, Rayshade has been modify to accept "
      "textures and <br>\n"
      "produce image files in the .png format. <br>\n<\font>"
      "<br>\n"
      "<font size=\"-1\">Copyright (C) 1989, 1991 by Craig E. Kolb\n"
      "</p>\n";

  box.setText(QString(msg.c_str()));
  box.setIconPixmap(QPixmap::fromImage(QImage(":/images/logo1.png")));
  box.show();
  box.exec();
}

}; // namespace vlab
