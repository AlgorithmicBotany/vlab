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
#include "mainwindow.h"
#include "rubberband.h"
#include "common.h"
#include "xmemory.h"
#include "icon.h"
#include "resources.h"
#include "SaveAs.h"
#include <QDesktopServices>

#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QApplication>
#include <QPainter>
#include <QScreen>
#include <QSplashScreen>
#include <QTimer>
#include <QFileDialog>
#include <iostream>
#include <unistd.h>
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <iostream>
#include <QPushButton>
#include <fstream>

MainWindow::MainWindow() : QMainWindow() {
  setWindowFlags(Qt::Widget | Qt::FramelessWindowHint |
                 Qt::WindowStaysOnTopHint);
  setParent(0); // Create TopLevel-Widget
  setAttribute(Qt::WA_NoSystemBackground, true);
  setAttribute(Qt::WA_TranslucentBackground, true);
  setFocusPolicy(Qt::StrongFocus);

  setPreferences();
  currentPath = QString(QDir::currentPath());
  QColor defaultColor = preferences->getColor();
  if (defaultColor.alpha() < 20)
    defaultColor.setAlpha(20);
  QColor defaultBackgroundColor = preferences->getBackgroundColor();
  if (defaultBackgroundColor.alpha() < 20)
    defaultBackgroundColor.setAlpha(20);
  rubberBandBorderSize = preferences->getBorderWidth();

  ctrl_x = 400;
  ctrl_y = 400;
  ctrl_w = ACTUAL_W + 2 * rubberBandBorderSize;
  ctrl_h = ACTUAL_H + 2 * rubberBandBorderSize;
  resize(ctrl_w, ctrl_h);
  move(ctrl_x, ctrl_y);

  rubberband = new RubberBand(this, defaultColor, defaultBackgroundColor,
                              rubberBandBorderSize);

  name = "picture.png";
  pix_format = "PNG";
  basename = "picture";
  setCentralWidget(rubberband);
  connect(rubberband, &RubberBand::open_menu, this, &MainWindow::open_menu);
  connect(preferences, &Preferences::colorChanged, rubberband,
          &RubberBand::setColor);
  connect(preferences, &Preferences::borderWidthChanged, rubberband,
          &RubberBand::setWidth);
  connect(preferences, &Preferences::backgroundColorChanged, rubberband,
          &RubberBand::setBackgroundColor);
  createActions();
  createBarMenus();
}

void MainWindow::createBarMenus() {

  // Size Menu
  menu = new QMenu();
  size_menu = menuBar()->addMenu(tr("&Size"));
  size_menu->addAction(sizeActActual);
  size_menu->addAction(sizeActDouble);
  size_menu->addAction(sizeActQuadruple);
  size_menu->addAction(sizeActAspect);
  size_menu->addAction(sizeActFree);

  // Format Menu
  /*
  format_menu = menuBar()->addMenu(tr("&File format"));
  format_menu->addAction(formatAct_rgb);
  format_menu->addAction(formatAct_bmp);
  format_menu->addAction(formatAct_jpeg);
  format_menu->addAction(formatAct_pbm);
  format_menu->addAction(formatAct_pgm);
  format_menu->addAction(formatAct_png);
  format_menu->addAction(formatAct_ppm);
  format_menu->addAction(formatAct_xbm);
  format_menu->addAction(formatAct_xpm);
  */
  // snap Menu
  snap_menu = menuBar()->addMenu(tr("&Snap"));
  snap_menu->addAction(snapAct);
  snap_menu->addAction(previewAct);
  snap_menu->addAction(saveAct);
  snap_menu->addAction(saveAsAct);
  snap_menu->addAction(snap_save_exitAct);
  snap_menu->addAction(save_exitAct);

  menu->addMenu(size_menu);
  menu->addAction(snapAct);
  menu->addAction(previewAct);
  menu->addSeparator();
  menu->addAction(saveAct);
  menu->addAction(save_exitAct);
  menu->addAction(snap_save_exitAct);
  menu->addSeparator();
  //  menu->addMenu(format_menu);
  menu->addAction(saveAsAct);

  previewAct->setEnabled(false);
  saveAct->setEnabled(false);
  saveAsAct->setEnabled(false);
  save_exitAct->setEnabled(false);


  menu->addSeparator();
  QMenu *preferenceMenu = menuBar()->addMenu("Preferences");
  preferenceMenu->addAction(preferencesAct);
  //menu->addSeparator();

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(quickHelpAct);
  helpMenu->addAction(helpAct);
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);
  //menu->addMenu(helpMenu);

  //menu->addSeparator();
  menu->addAction(exitAct);
  #ifndef __APPLE__
  menuBar()->setVisible(false);
  #endif
}

void MainWindow::createActions() {
  // Definition of Action for changing the size
  sizeActActual = new QAction(tr("Actual"), this);
  sizeActActual->setCheckable(true);
  connect(sizeActActual, &QAction::triggered, this, &MainWindow::size_actual);

  sizeActDouble = new QAction(tr("Double"), this);
  sizeActDouble->setCheckable(true);
  connect(sizeActDouble, &QAction::triggered, this, &MainWindow::size_double);

  sizeActQuadruple = new QAction(tr("Quadruple"), this);
  sizeActQuadruple->setCheckable(true);
  connect(sizeActQuadruple, &QAction::triggered, this,
          &MainWindow::size_quadruple);

  sizeActAspect = new QAction(tr("Aspect"), this);
  sizeActAspect->setCheckable(true);
  connect(sizeActAspect, &QAction::triggered, this, &MainWindow::size_aspect);

  sizeActFree = new QAction(tr("Free"), this);
  sizeActFree->setCheckable(true);
  connect(sizeActFree, &QAction::triggered, this, &MainWindow::size_free);

  sizeActGroup = new QActionGroup(this);
  sizeActGroup->addAction(sizeActActual);
  sizeActGroup->addAction(sizeActDouble);
  sizeActGroup->addAction(sizeActQuadruple);
  sizeActGroup->addAction(sizeActAspect);
  sizeActGroup->addAction(sizeActFree);
  sizeActActual->setChecked(true);

  // Definition of Action for changing the Format
  /*
  formatAct_rgb = new QAction(tr("RGB"), this);
  formatAct_rgb->setCheckable(true);
  connect(formatAct_rgb, &QAction::triggered, this, &MainWindow::format_rgb);

  formatAct_bmp = new QAction(tr("BMP"), this);
  formatAct_bmp->setCheckable(true);
  connect(formatAct_bmp, &QAction::triggered, this, &MainWindow::format_bmp);

  formatAct_jpeg = new QAction(tr("JPEG"), this);
  formatAct_jpeg->setCheckable(true);
  connect(formatAct_jpeg, &QAction::triggered, this, &MainWindow::format_jpeg);

  formatAct_pbm = new QAction(tr("PBM"), this);
  formatAct_pbm->setCheckable(true);
  connect(formatAct_pbm, &QAction::triggered, this, &MainWindow::format_pbm);

  formatAct_png = new QAction(tr("PNG"), this);
  formatAct_png->setCheckable(true);
  connect(formatAct_png, &QAction::triggered, this, &MainWindow::format_png);

  formatAct_ppm = new QAction(tr("PPM"), this);
  formatAct_ppm->setCheckable(true);
  connect(formatAct_ppm, &QAction::triggered, this, &MainWindow::format_ppm);

  formatAct_xbm = new QAction(tr("XBM"), this);
  formatAct_xbm->setCheckable(true);
  connect(formatAct_xbm, &QAction::triggered, this, &MainWindow::format_xbm);

  formatAct_xpm = new QAction(tr("XPM"), this);
  formatAct_xpm->setCheckable(true);
  connect(formatAct_xpm, &QAction::triggered, this, &MainWindow::format_xpm);

  formatAct_pgm = new QAction(tr("PGM"), this);
  formatAct_pgm->setCheckable(true);
  connect(formatAct_pgm, &QAction::triggered, this, &MainWindow::format_pgm);

  formatActGroup = new QActionGroup(this);
  formatActGroup->addAction(formatAct_rgb);
  formatActGroup->addAction(formatAct_bmp);
  formatActGroup->addAction(formatAct_jpeg);
  formatActGroup->addAction(formatAct_pbm);
  formatActGroup->addAction(formatAct_pgm);
  formatActGroup->addAction(formatAct_png);
  formatActGroup->addAction(formatAct_ppm);
  formatActGroup->addAction(formatAct_xbm);
  formatActGroup->addAction(formatAct_xpm);
  formatAct_png->setChecked(true);
  */
  /////////////////////////////
  // snap actions
  snapAct = new QAction(tr("Snap"), this);
  connect(snapAct, &QAction::triggered, this, &MainWindow::newScreenShot);
  previewAct = new QAction(tr("Preview"), this);
  connect(previewAct, &QAction::triggered, this, &MainWindow::preview);
  saveAct = new QAction(tr("Save icon"), this);
  connect(saveAct, &QAction::triggered, this, &MainWindow::saveIcon);
  saveAsAct = new QAction(tr("Save as ..."), this);
  connect(saveAsAct, &QAction::triggered, this, &MainWindow::save_as);
  save_exitAct = new QAction(tr("Save and exit"), this);
  connect(save_exitAct, &QAction::triggered, this, &MainWindow::save_exit);
  snap_save_exitAct = new QAction(tr("Snap, save and exit"), this);
  connect(snap_save_exitAct, &QAction::triggered, this,
          &MainWindow::snap_save_exit);

  ////////////////////////
  changeColorAct = new QAction(tr("&Change Color"), this);
  connect(changeColorAct, &QAction::triggered, rubberband,
          &RubberBand::changeColor);

  preferencesAct = new QAction(tr("Preferences..."), this);
  connect(preferencesAct, &QAction::triggered, this,
          &MainWindow::editPreferencesCB);

  ////////////////////////
  helpAct = new QAction(tr("Vlab framework"), this);
  connect(helpAct, &QAction::triggered, this, &MainWindow::help);

  quickHelpAct = new QAction(tr("&Quick Help"), this);
  connect(quickHelpAct, &QAction::triggered, this, &MainWindow::quickHelp);

  aboutAct = new QAction(tr("&About"), this);
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

  aboutQtAct = new QAction(tr("About &Qt"), this);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);
  connect(aboutQtAct, &QAction::triggered, this, &MainWindow::aboutQt);

  exitAct = new QAction(tr("Exit"), this);
  connect(exitAct, &QAction::triggered, this, &MainWindow::exit);
}

void MainWindow::open_menu() {
  menu->raise();
  menu->exec(QCursor::pos());
}

void MainWindow::draw_frame() {}

void MainWindow::about() {
  vlab::about(this,"Snapicon");
  return;
  char message[] = "Snapicon2.0.0\n"
                   "\n"
                   "by Pascal Ferraro\n"
                   "\n"
                   "May 23, 2019\n"
                   "\n"
                   "Dept. of Computer Science\n"
                   "University of Calgary\n";

  QMessageBox::about(this, tr("About Menu"), tr(message));
}

void MainWindow::aboutQt() {}

// Display help
void MainWindow::help() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("VLABFramework.pdf")));
}

// Display help
void MainWindow::quickHelp() {
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif  
  QString path = helpDir.filePath("SnapiconQuickHelp.html");
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
  msgBox->setWindowTitle("Snapicon: Quick Help");
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

// Toggle size mode
void MainWindow::size_actual() { rubberband->setToActualSize(); }

void MainWindow::size_double() { rubberband->setToDoubleSize(); }

void MainWindow::size_quadruple() { rubberband->setToQuadrupleSize(); }

void MainWindow::size_aspect() { rubberband->setToAspectSize(); }

void MainWindow::size_free() { rubberband->setToFreeSize(); }

void MainWindow::format_rgb() {
  name = basename + QString(".rgb");
  pix_format = "RGB";
}

void MainWindow::format_bmp() {
  name = basename + QString(".bmp");
  pix_format = "BMP";
}

void MainWindow::format_jpeg() {
  name = basename + QString(".jpeg");
  pix_format = "JPG";
}

void MainWindow::format_pbm() {
  name = basename + QString(".pbm");
  pix_format = "PBM";
}

void MainWindow::format_pgm() {
  name = basename + QString(".pgm");
  pix_format = "PGM";
}

void MainWindow::format_png() {
  name = basename + QString(".png");
  pix_format = "PNG";
}

void MainWindow::format_ppm() {
  name = basename + QString(".ppm");
  pix_format = "PPM";
}

void MainWindow::format_xbm() {
  name = basename + QString(".xbm");
  pix_format = "XBM";
}

void MainWindow::format_xpm() {
  name = basename + QString(".xpm");
  pix_format = "XPM";
}


void MainWindow::saveIcon() {

  // save as PNG
  QImage image = snap_image.toImage();
  if (image.isNull()) {
    std::cerr << "Image has not been snapped, cannot be saved !!" << std::endl;
    return;
  }
  // resize
  int w = (ACTUAL_W);
  int h = (ACTUAL_H);
  if (!sizeActFree->isChecked()) {
    image = image.scaled(w, h, Qt::KeepAspectRatio,
			 Qt::SmoothTransformation);
    //   std::cerr<<"Saved Image has alpha: "<<image.hasAlphaChannel()<<std::endl;
  }
  
  QString newName = "icon";
  if (!image.save(newName, "PNG")) {
    std::cerr << "[SAVE] Could not write to file " << name.toStdString()
              << "\n";
  }
}


void MainWindow::save() {

  if (!QString::compare(pix_format, QString("RGB"))) {
    if (saveRGB()){
    // if icon.png is in the directory, we want to delete icon.png to keep only one file icon
      // [PASCAL] this is commented to keep compatibility with Lstudio
      /*
      QString filePNG = basename+QString(".png");
      if (QFile::exists(filePNG)){
	//then delete icon.png
	QFile file (filePNG);
	file.remove();
      }
      */
    }
    return;
  }

  // save as PNG
  QImage image = snap_image.toImage();
   if (image.isNull()) {
    std::cerr << "Image has not been snapped, cannot be saved !!" << std::endl;
    return;
  }
 // resize
  int w = (ACTUAL_W);
  int h = (ACTUAL_H);
  if (!sizeActFree->isChecked()) {
    image = image.scaled(w, h, Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    //   std::cerr<<"Saved Image has alpha: "<<image.hasAlphaChannel()<<std::endl;
  }
  
  //const char *format = pix_format.toStdString().c_str();
  QByteArray ba = pix_format.toLocal8Bit();
  const char *format = ba.data();
  QString newName = name;
  std::ifstream ifile;
  bool savefile = true;
  QByteArray ba2 = name.toLocal8Bit();
  const char *filename = ba2.data();
  ifile.open(filename);
  if (ifile)
    savefile = OverwriteDialogBox(filename);
  if (!savefile)
    return;

  //  if (QString::compare(newName, QString("icon"))) {
  //   newName = basename + QString(".") + pix_format.toLower();
  //}
  if (!image.save(newName, format)) {
    std::cerr << "[SAVE] Could not write to file " << name.toStdString()
              << "\n";
    //qApp->quit();
  }
  // [PASCAL] this is commented to keep compatibility with Lstudio
 /*
    // if icon is in the directory, we want to delete icon to keep only one file icon.png
  QString filePNG = basename+QString(".png");
  QString fileRGB = basename;
  if (QFile::exists(filePNG)){
	//then keep icon.png and remove icon if it exists
      if (QFile::exists(fileRGB)){
	QFile file (fileRGB);
	file.remove();
      }
  }
  */
}

bool MainWindow::saveRGB() {
  QImage image = snap_image.toImage();
  // resize
  int w = (ACTUAL_W);
  int h = (ACTUAL_H);
  if (!sizeActFree->isChecked()) {
    image = image.scaled(w, h, Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    //   std::cerr<<"Saved Image has alpha: "<<image.hasAlphaChannel()<<std::endl;
 }

  //
  // convert the QImage into a raw RGB format
  // --------------------------------------------------
  char *res = (char *)xmalloc(image.width() * image.height() * 3);
  if (res == NULL) {
    std::cerr << "Could not malloc() memory for RGB "
      "image. \n "
      " You are trying to save an image of size: "<<image.width()<<" x "<<image.height()<<" = "<<image.width() * image.height() * 3 / 1000000<<" M"<<std::endl;
    std::cerr<<"Give another try.\n";
    return false;
    //qApp->quit();
  }
  char *ptr = res;
  int x, y;
  for (long i = 0; i < long(image.width() * image.height()); i++) {
    y = i / image.width();
    x = i - y * image.width();
    *ptr++ = (char)qRed(image.pixel(x, y));
    *ptr++ = (char)qGreen(image.pixel(x, y));
    *ptr++ = (char)qBlue(image.pixel(x, y));
  }
  std::string filename = name.toStdString();

  // now save the image into a file
  FILE *fp = fopen(filename.c_str(), "w");
  if (fp == NULL) {
    std::cerr << "[SAVERGB] Could not write to file " << filename << "\n";
    return false;
    //qApp->quit();
  }
  if (WriteIRIS(fp, (byte *)res, PIC24, image.width(), image.height(), NULL,
                NULL, NULL, F_FULLCOLOR)) {
    std::cerr << "Could not write to file " << filename << "\n";
    return false;
    //qApp->quit();
  }
  fclose(fp);
  delete res;
  return true;

}

/*
void MainWindow::save_as() {
  QString oldName = name;
  QString newName = QFileDialog::getSaveFileName(this, tr("Save Icon"), name,"",nullptr,QFileDialog::DontUseCustomDirectoryIcons);
  if (!newName.isEmpty()) {
    name = newName;
    QFileInfo fi(newName);
    basename = fi.absolutePath() + "/" + fi.baseName();
    if (newName == "./icon")
      is_latest_save = 0;
    save();
    return;
  }
  name = oldName;
}
*/

void MainWindow::save_as(){

  SaveAs *window = new SaveAs(this, basename,
                              currentPath, pix_format);
  QPoint snapiconPosition = this->pos();
  window->move(snapiconPosition);
  QString savePath;
  window->setAttribute(Qt::WA_DeleteOnClose, false);

  int result = window->exec();
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  window->close();
  if (result) {
    QString pathToSave = window->getPath();
    currentPath = pathToSave;
    int imageType = window->getImageType();
    basename =  window->getImageBaseName();
    name = pathToSave + QString("/") +  window->getImageBaseName() + QString(".") +  window->getExtension();
    switch (imageType) {
      case 0:
        pix_format = "BMP";
        break;
      case 1:
        pix_format = "GIF";
        break;
      case 2:
        pix_format = "JPG";
        break;
      case 3:
        pix_format = "PBM";
        break;
      case 4:
        pix_format = "PDF";
        break;
      case 5:
        pix_format = "PNG";
        break;
      case 6:
        pix_format = "TIFF";
        break;
      default:
        pix_format = "PNG";
        break;
      }
    
    save();
  }
}


// returns true if the user wants to overwrite the old file
// false otherwise
bool MainWindow::OverwriteDialogBox(const char *sfilename) {

  bool overwrite = false;

  QString message = " ";
  message.append(sfilename);
  QMessageBox box(this);
  box.setWindowTitle("Overwrite File");
  box.setText("File " + message + " exists.\nOverwrite?");
  box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  box.setDefaultButton(QMessageBox::Cancel);
  box.setEscapeButton(QMessageBox::Cancel);
  box.setAttribute(Qt::WA_DeleteOnClose, false);
  int ret = box.exec();

  switch (ret) {
  case QMessageBox::Ok:
    overwrite = true;
    break;
  case QMessageBox::Cancel:
    overwrite = false;
    break;
  default:
    break;
  }

  QWidget::setCursor(Qt::ArrowCursor); // change to arrow cursor */

  return overwrite;
}



void MainWindow::preview() {
  // resize
  int w = (ACTUAL_W);
  int h = (ACTUAL_H);

  int frame_x = pos().x();
  int frame_y = pos().y();
  QScreen *screen = QGuiApplication::screenAt(QPoint(frame_x, frame_y));
 // if screen is retina, we need to scale
  deviceRatio = screen->devicePixelRatio();

  QPixmap piximage = snap_image.scaled(
      w*deviceRatio,  h*deviceRatio,
      Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  if (sizeActFree->isChecked()) { 
    piximage = snap_image;
  }
  
  QSplashScreen *preview =
      new QSplashScreen(QPixmap(piximage), Qt::WindowStaysOnTopHint);
  preview->show();
}

void MainWindow::snap_save_exit() {
  hide();
  QTimer::singleShot(500, this, &MainWindow::snapSaveExit);
}

void MainWindow::snapSaveExit() {
  snap();
  saveIcon();
  exit();
}

void MainWindow::save_exit() {
  if (!snap_image.isNull())
    saveIcon();
  exit();
}

void MainWindow::newScreenShot() {
  hide();
  QTimer::singleShot(500, this, &MainWindow::snap);
}

void MainWindow::snap() {
  previewAct->setEnabled(true);
  saveAct->setEnabled(true);
  saveAsAct->setEnabled(true);
  save_exitAct->setEnabled(true);

  rubberBandBorderSize = rubberband->getBorderSize();
  int frame_x = pos().x();
  int frame_y = pos().y();
  int frame_w = size().width();
  int frame_h = size().height();
  QList<QScreen *> screenList = QGuiApplication::screens();
  QList<QPixmap> scrs;
  QScreen *screen = QGuiApplication::screenAt(QPoint(frame_x, frame_y));

  // Main screen
  QScreen *screen0 = screenList.at(0);

  // if screen is retina, we need to scale
  deviceRatio = screen->devicePixelRatio();

  int x_pos = frame_x + rubberBandBorderSize;
  int y_pos = frame_y + rubberBandBorderSize;

  // if screen is not 0 we need to adjust coordinate
  if (screen != screen0) {
    int screen_x = screen->geometry().x();
    int screen_y = screen->geometry().y();
    x_pos -= screen_x;
    y_pos -= screen_y;
  }

  snap_image =
      screen->grabWindow(0, screen->geometry().x(), screen->geometry().y(),
                         screen->size().width(), screen->size().height());
  // std::cerr<<"Snapped image has alpha: "<<snap_image.hasAlphaChannel()<<std::endl;
  // then crop at the right position
  QRect rect(deviceRatio * x_pos, deviceRatio * y_pos,
             (frame_w - 2*rubberBandBorderSize) * deviceRatio,
             (frame_h - 2*rubberBandBorderSize) * deviceRatio);
  snap_image = snap_image.copy(rect);

  // resize
  /*
  int w = (ACTUAL_W);
  int h = (ACTUAL_H);
  if (!sizeActFree->isChecked()) {
    QPixmap my_snap_image = snap_image.scaled(w, h, Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
   } else {
    // nothing needs to be done for free size or actual size
  }
  */
  is_image_modified = 1;
  show();
}

void MainWindow::exit() {
  // test if image is Null
  qApp->quit();
}

void MainWindow::setPreferences() {

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
  strcat(bf, "snapicon.cfg");

  QString filePreferences = QString(bf);
  preferences = new Preferences(this, filePreferences);
  preferences->setModal(false);
  preferences->loadConfig();
}

void MainWindow::editPreferencesCB() {
  setPreferences();
  connect(preferences, &Preferences::colorChanged, rubberband,
          &RubberBand::setColor);
  connect(preferences, &Preferences::borderWidthChanged, rubberband,
          &RubberBand::setWidth);
  connect(preferences, &Preferences::backgroundColorChanged, rubberband,
          &RubberBand::setBackgroundColor);
  preferences->show();
}

void MainWindow::resizeEvent(QResizeEvent *) {}
