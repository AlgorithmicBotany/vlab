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



#include "QTGLObject.h"
#include "about.h"
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <QCursor>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "MessageBox.h"
#include "Preferences.h"
#include "objectMessageBox.h"
#include "resources.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFontMetrics>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QOpenGLFunctions_1_1>
#include <QPushButton>
#include <QTime>
#include <QUrl>
#include <QtOpenGL>
#include <qtFontUtils.cpp>
#include <stdio.h>    // perror(), printf()
#include <stdlib.h>   // exit(), EXIT_FAILURE
#include <unistd.h>   // execlp(), fork(), pid_t
#include <sys/wait.h> // wait()


#include "ImportExport.h"
#include "Mem.h"
#include "auxMenuItem.h"
#include "dirList.h"
#include "dsprintf.h"
#include "labutil.h"
#include "object.h"
#include "platform.h"
#include "rgbToAbgr.h"
#include "saveChanges.h"
#include "sgiFormat.h"
#include "xmemory.h"
#include "xstring.h"
#include "xutils.h"
#include <cmath>

#ifndef min
using std::min;
#endif

using namespace Qt;

#include "directorywatcher.h"

namespace {
// timer for idle function
QTimer *idleTimer;
} // namespace

// ------------------------------------------------------------------------------
// openGL convenience functions
//
/* // Not used anymore...
static void glRectangle(double x1, double y1, double x2, double y2) {
  glBegin(GL_QUADS);
  glVertex2d(x1, y1);
  glVertex2d(x2, y1);
  glVertex2d(x2, y2);
  glVertex2d(x1, y2);
  glEnd();
}
static void glCircle(double x, double y, double radius, int count = 100) {
  glBegin(GL_TRIANGLE_FAN);
  glVertex2d(x, y);
  for (int i = 0; i <= count; i++) {
    double alpha = double(i) / count * 2 * M_PI;
    glVertex2d(x + radius * sin(alpha), y + radius * cos(alpha));
  }
  glEnd();
}
static void glPie(double x, double y, double radius, double a1, double a2,
                  int count = 100) {
  glBegin(GL_TRIANGLE_FAN);
  glVertex2d(x, y);
  for (int i = 0; i <= count; i++) {
    double alpha = a1 * (1 - double(i) / count) + a2 * (double(i) / count);
    alpha *= 2 * M_PI;
    glVertex2d(x + radius * sin(alpha), y + radius * cos(alpha));
  }
  glEnd();
}
*/
//
// openGL convenience functions
// ------------------------------------------------------------------------------

// ------------------------------------------------------------------------------
// class Progress implementation
//
Progress::Progress(QTGLObject &qtglobject) : _qtglobject(qtglobject) {
  _total = 100;
  _inc = 1;
  _curr = 0;
  _show = false;
  _effectiveShow = false;
  _hideRequestTime = 0;
  _operationTimeThreshold = 0;
}
void Progress::show(bool on, bool ignoreTimeThreshold) {
  // disable if min duration and fade time are both zero
  // otherwise (occasionally) one or two render calls sneak through
  // and the object window flickers
  if (obj.progressMinDuration + obj.progressFadeTime == 0)
    return;

  if (on) {
    // if we are displaying progress:
    _show = true;
    _effectiveShow = true;
    _time.start();             // start the timer
    _qtglobject.animate(true); // request periodic refresh
    _hideRequestTime = 0;
    _operationTimeThreshold = ignoreTimeThreshold ? 0 : obj.operationTimeThreshold;
  } else {
    // indicate that the caller wants the progress bar hidden
    _show = false;
    // record the time hide was requested
    _hideRequestTime = _time.elapsed();
  }
  //_qtglobject.repaint();//update();
}
void Progress::advance() {
  if (_show) {
    _curr += _inc;
    _qtglobject.repaint();//update();
  }
}
void Progress::setup(double total, double inc, double curr) {
  _total = total;
  _inc = inc;
  _curr = curr;
}

void Progress::render() {

  // disable any rendering if we don't need to show anything
  if (!_effectiveShow)
    return;
  
  // measure the time elapsed since show
  int elapsed = _time.elapsed();

  // deactivate progress if the time threshold has not been reached
  // (should the window even be open?)
  if (elapsed < _operationTimeThreshold)
    return;
  if (_hideRequestTime > 0 && _hideRequestTime < _operationTimeThreshold) {
    _qtglobject.animate(false);
    _effectiveShow = false;
    return;
  }

  // deactivate progess if fading has finished
  int fadeFinish = _hideRequestTime + obj.progressFadeTime;
  if (_hideRequestTime < obj.progressMinDuration)
    fadeFinish = obj.progressMinDuration + obj.progressFadeTime;
  if (!_show && elapsed > fadeFinish) {
    _qtglobject.animate(false);
    _effectiveShow = false;
    return;
  }
  
  //
  // otherwise we render progress...
  //
  
  // determine what progress to render
  double progress = _curr / _total;
  // if the progress has been shown for less than the minDuration,
  // fake the progress so that it does not get to 100% before the
  // minDuration is over
  if (elapsed <= obj.progressMinDuration) {
    double t = double(elapsed) / double(obj.progressMinDuration);
    if (progress > t)
      progress = t;
  }
  
  // determine alpha
  double alpha = 1.0;
  // if we are in the fading mode, adjust the alpha
  if( ! _show && elapsed > obj.progressMinDuration ) {
   	alpha = (fadeFinish - elapsed) / double( obj.progressFadeTime );
   	alpha = std::log( alpha + 1. ) / 0.7;
  }
  // set alpha of user-defined colors
  obj.bgColor.setAlphaF(alpha);
  obj.circleColor.setAlphaF(alpha);
  obj.pieColor.setAlphaF(alpha);
  obj.fontColor.setAlphaF(alpha);
  
  // draw using QPainter...
  QPainter painter(&_qtglobject);
  painter.setRenderHint(QPainter::Antialiasing);
  QSize size = _qtglobject.size();
  QPoint center(size.width()/2,size.height()/2);
  QRect rect(QPoint(0,0),size);

  // dim background
  QPixmap result = QPixmap(size);
  result.fill(obj.bgColor);//QColor(0,0,0,(int)(alpha*255.0)));
  painter.drawPixmap(0,0,result);
  
  // draw circle and pie showing progress
  painter.setPen(Qt::NoPen);
  painter.setBrush(obj.circleColor);//QColor(0,51,0,(int)(255.0*alpha)));
  int minR = std::min(size.width()/2, size.height()/2);
  painter.drawEllipse(center, minR, minR);

  painter.setBrush(obj.pieColor);//QColor(70, 131, 0, (int)(255.0*alpha)));
  // drawPie() takes integer parameters between 0 and 360
  // in 1/16th of a degree: a circle is 16 * 360 = 5760
  painter.drawPie(rect, 90 * 16, (int)(progress * 360.0) * (-16));
  
  // draw progress text
  painter.setPen(obj.fontColor);
  painter.setBrush(obj.fontColor);
  QString txt( QString( "%1%").arg( int( progress * 100 )));
  QFont font( "Arial", 32, QFont::Normal );
  painter.setFont(font);
  painter.drawText(rect, Qt::AlignCenter, txt);

  // Original code: draw using OpenGL calls...
  // commented out because the icon is now rendered with
  // QPainter and the renderText function is no longer
  // supported by Qt.
/*
  //    // dim the background (if any)
  //glColor4f(0.0, 0.0, 0.0, alpha);
  //glRectangle(0, 0, 1, 1);
  glColor4f(0.0, 0.2, 0.0, 1. * alpha);
  glCircle(0.5, 0.5, 0.45);
  // circle progress
  glColor4f(0.1, 0.3, 0.0, 1.);
  glPie(0.5, 0.5, 0.45, 0, progress);
  // display centered progress text
  glColor4f(1, 1, 1, alpha);
  QString txt(QString("%1%").arg(int(progress * 100)));
  setDefaultFont();
  QFont font("Arial", 32, QFont::Normal, false);
  QRect br = QFontMetrics(font).boundingRect(txt); // bounding rect. of the text
  QSize s = _qtglobject.size();                    // size of our opengl widget
  // figure out where to start drawing the text so that it's centered
  double x = s.width() / 2.0 - (br.right() + br.left()) / 2.0;
  double y = s.height() / 2.0 - (br.top() + br.bottom()) / 2.0;
  // Render text
  //glEnable(GL_LIGHTING);
  //_qtglobject.renderText(int(x), int(y), 0.1,  txt,QColor(125,125,125), font);
  //glDisable(GL_LIGHTING);
  QPainter p(& _qtglobject);
  p.setPen(Qt::white);
  p.setFont(font);
  p.drawText(int(x), int(y),txt);
*/
}
//
// class Progress implementation
// ------------------------------------------------------------------------------

// --------------------------------------------------------------------------------
// QTGLObject implementation
//
QTGLObject::QTGLObject(QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f), _progress(*this) {
  /******** timer to run idle function ********/
  // create and set up the timer for the IdleFunction
  idleTimer = new QTimer(this);
  connect(idleTimer, SIGNAL(timeout()), SLOT(Idle()));
  
  _format.setRedBufferSize(8);
  _format.setGreenBufferSize(8);
  _format.setBlueBufferSize(8);
  _format.setAlphaBufferSize(8);
  _format.setSamples(8);
  setFormat(_format);

  _init = true;
  menu = NULL;
  oldMenu = NULL;

  // Set error window
  errWin = new QWidget(NULL);
  errWin->resize(60, 60);
  imgLabel = new QLabel("", errWin);
  imgLabel->setGeometry(5, 20, 50, 20);
  imgLabel->setAlignment(AlignHCenter | AlignVCenter);
  errWin->hide();

  QPalette p = palette();
  p.setColor(backgroundRole(), Qt::black);
  setPalette(p);

  // On macOS the menu bar appears at the top of the screen,
  // but, on most other systems, the menu bar is at the top of the window.
  // Consequently, on most systems the menu bar hides part
  // of the icon and looks ugly.
  // So, for now, the menu bar is enabled on mac OS only...
#if __APPLE_
  menubar = new QMenuBar(parent);
#else
  menubar = new QMenuBar(nullptr);
#endif
  this->setFocusPolicy(Qt::StrongFocus);

  // set default icon
  // hourglass
  //iconPicture = QImage(":/hourglass.png");
  //iconPicture = iconPicture.scaled(140, 140, Qt::KeepAspectRatio,
  //		 Qt::SmoothTransformation);
  // rectangle in background color
  iconPicture = QImage(this->size(),QImage::Format_RGB32);
  iconPicture.fill(obj.bgColor);
}

/////////// Continuous Mode //////

// [Pascal] keep opening the file until the size is stable
int QTGLObject::waitOpenFile(const char *fname) {
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  FILE *fp = fopen(fname, "r");
  // This is a big hack:
  // Add a counter to avoid infinite loop if view file is missing
  while ((fp == NULL) && (counter < 10000)) {
    fp = fopen(fname, "r");
    counter++;
  }
  if (counter == 10000) {
    fprintf(stderr, "WARNING: Can't open file %s.\n", fname);
    return 0;
  } else {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size = ftell(fp);       // get current file pointer
    counter = 0;
    while (((size == 0) || (current_size != size)) && (counter < 10000)) {
      current_size = size;
      fclose(fp);

      fp = NULL;
      while (fp == NULL) {
        fp = fopen(fname, "r");
        counter++;
      }
      fseek(fp, 0, SEEK_END); // seek to end of file
      size = ftell(fp);       // get current file pointer
    }
  }
  fseek(fp, 0L, SEEK_SET);
  fclose(fp);
  if (counter >= 1000) {
    std::cerr << "we can't reload the file : " << counter << std::endl;
    return 0;
  }

  return 1;
}

void QTGLObject::SetContinuousMode(std::string tmpDir) {

  if (!directoryWatcher) {
    QStringList ignoredPrefixes;
    QStringList ignoredSuffixes;
    directoryWatcher = new DirectoryWatcher(QString::fromStdString(tmpDir),
                                            ignoredPrefixes << "."
                                                            << "lsys.",
                                            ignoredSuffixes << ".i"
                                                            << ".log"
                                                            << ".ii"
                                                            << ".o"
                                                            << ".o",
                                            this);
    connect(directoryWatcher, SIGNAL(fileChanged(QString)), this,
            SLOT(RequestReload(QString)));
  }
}

void QTGLObject::RequestReload(const QString &f) {

  if (f.compare("specifications") == 0) {
    std::string file = f.toStdString();
    if (waitOpenFile(file.c_str()) == 0)
      return;

    if (!idleTimer->isActive()) {
      idleTimer->setSingleShot(false);
      idleTimer->start(0);
      NewMenuCb();
    } else {
      new_reload_pending = false;
    }
  }
}

void QTGLObject::Idle() {
  idleTimer->stop();
  QWidget::setCursor(Qt::ArrowCursor);
  if (new_reload_pending != 0) {
    new_reload_pending = 0;
    NewMenuCb();
  }
}
///////////////////////////////////////////

// Destructor
QTGLObject::~QTGLObject() {
  delete errWin;
  if (menu)
    delete menu;
}

// turns on/off periodic refresh of the GL window
void QTGLObject::animate(bool on) {
  static QTimer *timer = 0;
  static bool first_run = true;
  if (first_run) {
    first_run = false;
    timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  }
  if (on) {
    if (!timer->isActive())
      timer->start();
  } else {
    timer->stop();
  }
}

// ---------------------------------------------------------------------------
// Reads an icon from the object's lab table if possible.
// If unsuccessful, it uses a default icon.
// ---------------------------------------------------------------------------
void QTGLObject::loadIcon() {
  // try to load in the icon from the object directory
  // [PASCAL] uncomment the following to add png support
  /*
  QString iconFname = QString("%1/icon.png").arg(obj.tmpDir.c_str());
  // check if iconFname exists otherwise check if there is an icon
  QFileInfo check_file(iconFname);
  if (!check_file.exists()){
    iconFname = QString("%1/icon").arg(obj.tmpDir.c_str());
  }
  else{
    std::cerr<<"Deleting icon"<<std::endl;
    // there is an icon.png so remove icon from the labtable
    QString icontodelete = QString("%1/icon").arg(obj.tmpDir.c_str());
   if (QFile::exists(icontodelete)){
	QFile file (icontodelete);
	file.remove();
      } 
  }
  iconPicture = QImage(iconFname);
  */
  
  QString iconFname = QString("%1/icon").arg(obj.tmpDir.c_str());
  // if unsuccessful, silenty ignore it and use the default icon
  if (iconPicture.isNull()) {
    // plugins have changed in QT5, I don't understand how to handle them so simply
    // call the function that read a SGI. the problem is if QImage doesn't
    // recognize rgb format, to deal with this case we first try load from Qt if
    // it doesn't work, we try from SGI format !
    iconPicture = readSGI(iconFname.toLatin1());
    if (iconPicture.isNull()) {
      //then try to load in png forma:
      iconPicture = QImage(iconFname);
      if (iconPicture.isNull()){
        iconFname = ":/default-icon.png";
        iconPicture = QImage(iconFname);
        iconPicture = iconPicture.scaled(139, 139, Qt::KeepAspectRatio,
					  Qt::SmoothTransformation);
      }
    }
  }

  if (obj.fitIcon) {
    // if we fitting the icon inside a fixed window, then have QT
    // rescale the icon so that opengl does not have to do it
    iconPicture = iconPicture.scaled(obj.winSize, Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);
  } else {
    // if we are fitting the window to match the icon, then resize
    // the window but don't make it resizable
    iconForm->setMinimumSize(iconPicture.size());
    iconForm->setMaximumSize(iconPicture.size());
    iconForm->resize(iconPicture.size());
    findTopLevel();
    _topLevel->setMinimumSize(iconPicture.size());
    _topLevel->setMaximumSize(iconPicture.size());
    _topLevel->resize(iconPicture.size());
  }
  // set the application icon

  QPixmap icon = QPixmap::fromImage(iconPicture);

  qapp->setWindowIcon(icon);
  this->setWindowIcon(icon);
}

/******************************************************************************
 *
 * changesMade( )
 *
 * - checks if the contents of the lab table is different from the
 *   content of the original object
 *
 * - if there is any diference, the function returns True, otherwise
 *   it returns False ( i.e. no changes need to be made
 */
int QTGLObject::changesMade() {
  char filename1[4096];
  char filename2[4096];

  // get the list of files on the lab table
  std::vector<std::string> list = dirList2(obj.tmpDir.c_str());
  size_t n = list.size();
  // if there is diferent number of files, return false ( 2 accounts
  // for the '.' and '..' directories
  bool changes;
  if (n != obj.fnames.size()) {
    changes = 1;
  } else {
    // now we must compare all the files with the ones in the
    // specification file
    changes = 0;
    for (size_t i = 0; i < obj.fnames.size() && !changes; i++) {
      sprintf(filename1, "%s/%s", obj.objDir.c_str(), obj.fnames[i].c_str());
      sprintf(filename2, "%s/%s", obj.tmpDir.c_str(), obj.fnames[i].c_str());

      if (0 != RA::Compare_files(obj.connection, filename1,
                                 obj.local_connection, filename2)) {
        changes = 1;
      }
    }
  }
  // return the value that we have 'calculated'
  return changes;
}
/******************************************************************************
 *
 * loadAllFiles()
 *
 * - checks the list of all files in the object on the server and compares it
 *   with the list of files already checked out
 *
 * - if there is any diference, the missing files are checked out and put on
 *   the fnames list so that they can be saved later properly
 *
 * - if we are here, then we already know we can access the object directory
 *   and we don't need to do all of our possible failure checking over again
 *
 */
void QTGLObject::loadAllFiles() {
  char **dir_list;
  RA_Stat_Struc stat;
  int load;
  QStringList list, uniqueNames;
  int ret = RA::Get_dir(obj.connection, obj.objDir.c_str(), &dir_list);
  if (ret != 0) {
    for (int i = 0; i < ret; i++) {
      std::string specFile = obj.objDir + "/" + dir_list[i];
      RA::Stat(obj.connection, specFile.c_str(), &stat);
      if (stat.type == RA_REG_TYPE)
        list << dir_list[i];
    }

    // now we have excluded everything that isn't a file in the directory
    bool found;
    uniqueNames.clear(); // just to be safe
    for (int i = 0; i < list.size(); i++) {
      found = false;
      for (size_t j = 0; j < obj.fnames.size(); j++) {
        // we will now compare everything on our list to the files already
        // loaded to make sure we don't load them twice
        if (list.at(i).compare(obj.fnames.at(j).c_str(), Qt::CaseSensitive) ==
            0) {
          found = true;
        }
      }
      if (!found)
        uniqueNames.push_back(list.at(i)); // push it on the list of unique
                                           // names if it does not already exist
    }
    if (uniqueNames.size() > 0) {
      load = messageBox("Loading all files", uniqueNames.size(), uniqueNames);
      if (load == QMessageBox::Yes) {
        // load all files on uniqueNames, then push the contents of uniqueNames
        // on the back of obj.fnames
        for (int i = 0; i < uniqueNames.size(); i++) {
          std::string remoteFile =
              obj.objDir + "/" + uniqueNames.at(i).toStdString();
          std::string localFile =
              obj.tmpDir + "/" + uniqueNames.at(i).toStdString();
          if (RA::Fetch_file(obj.connection, remoteFile.c_str(),
                             localFile.c_str()) != 0) {
            std::cerr << "Error reading file " << remoteFile << std::flush;
          }
          obj.fnames.push_back(uniqueNames.at(i).toStdString());
        }
      }
    }
  }
}

void QTGLObject::initializeGL()
//-------------------------------------------------------------------------
// Initialize GL settings
//-------------------------------------------------------------------------
{
  // According to Qt5 documentation:
  // There is no need to call makeCurrent() because this has already been
  // done when this function is called.
//  makeCurrent();
  initializeOpenGLFunctions();
//  makeCurrent();
 
  glClearColor(obj.bgColor.redF(), obj.bgColor.greenF(), obj.bgColor.blueF(),
               0);
  
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glOrtho(0, this->width(), 0, this->height(), 0, 1);
  glMatrixMode(GL_MODELVIEW);
  
  glShadeModel(GL_FLAT);
}

void QTGLObject::resizeGL(int width, int height) {
  /* initialize the graphics */
  glViewport(0, 0, width, height);
  /* This will setup a simple 2D orthographic  viewport */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, 1, 0, 1, -1, 1);

  /* Set the Model Transformation to I */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void QTGLObject::paintGL()
//-------------------------------------------------------------------------
// Paint icon
//-------------------------------------------------------------------------
{
  QPainter p(this);
  p.drawImage(0,0,iconPicture,0,0,-1,-1,Qt::NoFormatConversion);
  progress().render();
}

void QTGLObject::closeEvent(QCloseEvent *ev)
// ---------------------------------------------------------------------------
// Traps window close event
// ---------------------------------------------------------------------------
{
  QuitCb();
  // ignore the event (to prevent object from closing if the user
  // clicks cancel). This OK because QuitCb() will actually quit
  // if the user selects one of the quit options.
  ev->ignore();
}

void QTGLObject::findTopLevel() {
  _topLevel = this;
  QWidget *p = dynamic_cast<QWidget *>(parent());
  while (p) {
    _topLevel = p;
    p = dynamic_cast<QWidget *>(_topLevel->parent());
  };
}

// This eventFilter never gets called because it needs to be initialized!
// There should be a call to QObject::installEventFilter();
bool QTGLObject::eventFilter(QObject *object, QEvent *e) {
  if ((object == menu) && (e->type() == QEvent::Leave)) {
    menu->close();
    return QObject::eventFilter(object, e);
  }
  return QObject::eventFilter(object, e);
}

void QTGLObject::mousePressEvent(QMouseEvent *ev)
// ---------------------------------------------------------------------------
// Activate context menu
// ---------------------------------------------------------------------------
{

  findTopLevel();
  _topLevel->raise();
  if (ev->button() == RightButton) {

    menu->exec(QCursor::pos());

    return;
    // using exec causes the menu to sometimes hang, it appears there is some OS
    // synchronization required when using exec()
  }
  if (ev->button() == Qt::LeftButton) {
    mouseLeftPressed = true;

    findTopLevel();
    _buttonDownPos = ev->globalPos();
    setCursor(Qt::OpenHandCursor);
  }

  if (oldMenu) {
    delete oldMenu;
    oldMenu = NULL;
  }
}

void QTGLObject::mouseReleaseEvent(QMouseEvent *ev) {

  if (ev->button() == Qt::LeftButton)
    mouseLeftPressed = false;

  unsetCursor();
}

void QTGLObject::mouseMoveEvent(QMouseEvent *ev) {
 
  if (ev->buttons() == Qt::LeftButton) {
    _topLevel->move(_topLevel->pos() + ev->globalPos() - _buttonDownPos);
    _buttonDownPos = ev->globalPos();
  }
}

void QTGLObject::MakeMenu()
// ---------------------------------------------------------------------------
// MakeMenu( )
//	- makes a menu for the object
//	- includes a standard  submenu using UtilMenu( )
// ---------------------------------------------------------------------------
{
  // create popup menus
  // QPopupMenu has been replaced by QMenu in Qt 4
  menu = new QMenu("menu", this);

  QFont f = menu->font();
  f.setItalic(true);
  f.setBold(true);
  QAction *act = menu->addAction(QString(obj.objName.c_str()));
  act->setFont(f);
  act->setEnabled(false);
  menu->addSeparator();

  SpecMenu(menu, obj.sp);
  UtilMenu();

  /* Quit */
  menu->addSeparator();
  menu->addAction("Quit", this, SLOT(QuitCb()));
}

void QTGLObject::SpecMenu(QMenu *parent, struct spec *sp)
// ---------------------------------------------------------------------------
// SpecMenu( )
//	- creates the menu entries based on the specification file
// ---------------------------------------------------------------------------
{
  while (sp != NULL) {
    if (sp->subitems != NULL) {

      sp->submenu = new QMenu(QString(sp->sname), parent);

      QFont f = sp->submenu->font();
      f.setItalic(true);
      sp->sbutton = parent->addMenu(sp->submenu);
      SpecMenu(sp->submenu, sp->subitems);
    } else {
      sp->sbutton =
          parent->addAction(QString(sp->sname), this, SLOT(SpecHandler()));
    }
    sp = sp->nextspec;
  }
}

//  completes the object's menu by adding a Utilities submenu
void QTGLObject::UtilMenu() {
  QAction *disableID;
#ifdef __APPLE__
  QAction *disableIDFinder;
#endif

  QMenu *utilmenu = new QMenu("Utilities", menu);
  menu->addMenu(utilmenu);

  QFont f = utilmenu->font();
  f.setItalic(true);
  /* Icon */

  QMenu *imenu = new QMenu("Icon", menu);
  utilmenu->addMenu(imenu);
  imenu->addAction("Snap", this, SLOT(ExecSnap()));
  imenu->addAction("Reread", this, SLOT(NewIconCb()));

  /* Update specs */
  QMenu *specmenu = new QMenu("Specifications", menu);
  utilmenu->addMenu(specmenu);


  specmenu->addAction("Edit", this, SLOT(editSpecCb()));
  specmenu->addAction("Reread", this, SLOT(NewMenuCb()));

  /* Shells */
  QMenu *shellmenu = new QMenu("Shells", menu);
  utilmenu->addMenu(shellmenu);

  shellmenu->addAction("Lab table", this, SLOT(labTableShellCb()));
#ifdef __APPLE__
  shellmenu->addAction("Lab table (Finder)", this,
                       SLOT(openLabTableFinderCb()));
#endif
  disableID = shellmenu->addAction("Storage", this, SLOT(storageShellCb()));
  if (obj.connection->connection_type == RA_REMOTE_CONNECTION)
    disableID->setEnabled(false);
#ifdef __APPLE__
  disableIDFinder = shellmenu->addAction("Storage (Finder)", this,
                                         SLOT(openStorageFinderCb()));
  if (obj.connection->connection_type == RA_REMOTE_CONNECTION)
    disableIDFinder->setEnabled(false);
#endif

#ifdef _DONT_COMPILE
  /* Directories */
  QMenu *dirmenu = new QMenu("Directory", menu);
  utilmenu->addMenu(dirmenu);

  QAction *iddirmenu = dirmenu->addAction("Directory");
  iddirmenu->setFont(f);
  dirmenu->addSeparator();

  dirmenu->addAction("Lab table", this, SLOT(dirviewLab()));
  disableID = dirmenu->addAction("Storage", this, SLOT(dirviewStorage()));
  if (!isLocal)
    disableID->setEnabled(false);
#endif


  /* Version control */
  utilmenu->addAction("Save changes", this, SLOT(saveChangesCb()));
  utilmenu->addAction("New version", this, SLOT(NewVersionCb()));

  /* Positioning object */
  utilmenu->addSeparator();
  QAction *posid = utilmenu->addAction("Position object", this, SLOT(posObj()));
  if (!vlabd->valid())
    posid->setEnabled(false);

 
  if (_init) {
    QMenu *file = new QMenu("&File", menubar);
    QAction *preferences = file->addAction("Preferences...");
    connect(preferences, SIGNAL(triggered()), this, SLOT(editPreferencesCB()));
    exportName = obj.objName.c_str();
    exportPath = QString();
    exportArchiveType = 1;
    file->addAction("Export", this, SLOT(exportObject()));
    file->addAction("Load all files", this, SLOT(loadAllFiles()));
    menubar->addMenu(file);
    QMenu *help = menubar->addMenu("&Help");
    help->addAction("About object", this, SLOT(about_object_cb()));
    //    help->addAction("Help (online)", this, SLOT(help()));
    //   QAction *qHelp = help->addAction("Quick help", this, SLOT(quickHelp()));
   help->addAction("Vlab framework", this, SLOT(pdfHelp()));
   //qHelp->setEnabled(false);
    _init = false;
  }
 }

void QTGLObject::SpecHandler()
//-------------------------------------------------------------------------
// Handle Spec menu item activation
//-------------------------------------------------------------------------
{
  QObject *caller = sender();
  QAction *item = dynamic_cast<QAction *>(caller);
  struct spec *entry;

  // find spec entry (obj.sp) and invoke ExecuteCommand with sp->cmdline
  entry = LocateSpecEntry(obj.sp, item);
  if (entry) {
    // execute the line
    ExecuteCommand(entry->cmdline);
  }
}

struct spec *QTGLObject::LocateSpecEntry(struct spec *sp, QAction *item)
//-------------------------------------------------------------------------
// Locate the spec entry containing menu entry with desired itemID
//-------------------------------------------------------------------------
{
  struct spec *tmpSp;

  while (sp) {
    if (sp->sbutton == item) {
      if (sp->subitems)
        return NULL;
      else
        return sp;
    }
    tmpSp = LocateSpecEntry(sp->subitems, item);
    if (tmpSp) {
      return tmpSp;
    }
    sp = sp->nextspec;
  }

  return NULL;
}

void QTGLObject::ExecSnap()
//-------------------------------------------------------------------------
// Execute snapicon program to capture icon snapshot
//-------------------------------------------------------------------------
{
  ExecuteCommand("snapicon");
}

void QTGLObject::QuitCb()
// ---------------------------------------------------------------------------
// Quit( ) - if the user has write privileges to the original directory,
// checks whether the temp version has been saved - its files should all be
// the same as the original version - if not, the "Save Changes?" menu is
// produced, and saveChanges( ) may be called - the temp directory is
// removed ( RemoveTemp( )), and the process exits.
// ---------------------------------------------------------------------------
{
  bool removeTemp = true;

  // get the changes made to the object
  FileList modified, unknown, missing;

  getChanges(modified, unknown, missing);
  std::sort(modified.begin(),modified.end());
  std::sort(unknown.begin(),unknown.end());
  std::sort(missing.begin(),missing.end());
  // if changes were made, report to the user
  if (modified.size() + unknown.size() + missing.size() > 0) {
    std::string msg = "<qt><font family=arial><b>";
    std::string simpleMsg = "";
    if (modified.size()) {
      simpleMsg += "Modified files:\n";
      msg += "<big>Modified files:</big>";
      msg += "<ul>";
      for (size_t i = 0; i < modified.size(); i++) {
        simpleMsg += " - " + modified[i] + "\n";
	if (i < 5 )
	  msg += "<li>" + modified[i] + "</li>";
      }
      if (modified.size() > 5) {
	msg += "<li>Plus " +
	  QString::number(modified.size() - 5).toStdString() +
	  " more...</li>";
      }
      msg += "</ul>";
    }
    if (missing.size()) {
      simpleMsg += "Missing files:\n";

      msg += "<big>Missing files:</big> <small>"
             "(in specs but not on table)</small>";
      msg += "<ul>";
      for (size_t i = 0; i < missing.size(); i++) {
        simpleMsg += " - " + missing[i] + "\n";

	if (i <5 )
	  msg += "<li>" + missing[i] + "</li>";
      }
      if (missing.size() > 5) {
          msg += "<li>Plus " +
                 QString::number(missing.size() - 5).toStdString() +
                 " more...</li>";
      }
      msg += "</ul>";
    }
    if (unknown.size()) {
      simpleMsg += "Unknown files:\n";

      msg += "<big>Unknown files:</big> <small>"
             "(on table but not in specs)</small>";
      msg += "<ul>";
      for (size_t i = 0; i < unknown.size(); i++) {
 	if (i <5 )
	  msg += "<li>" + unknown[i] + "</li>";
        simpleMsg += " - " + unknown[i] + "\n";

      }
      if (unknown.size() > 5) {
	msg += "<li>Plus " +
	  QString::number(unknown.size() - 5).toStdString() +
	  " more...</li>";
      }
      msg += "</ul>";
    }
    msg += "</font></b></qt>";
    /*
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(QString(msg.c_str()));
    if ((missing.size() > 5) && (unknown.size() > 5) &&
        (modified.size() > 5 )) {
      msgBox.setDetailedText(QString(simpleMsg.c_str()));
    }
    QString wtitle = "Save: " + QString(obj.objName.c_str());
    msgBox.QDialog::setWindowTitle(wtitle);
    msgBox.addButton("Cancel", QMessageBox::ActionRole);
    msgBox.addButton("Ignore && Quit", QMessageBox::ActionRole);
    msgBox.addButton("Save && Quit", QMessageBox::ActionRole);
    int option = msgBox.exec();
    */
    QPoint posDialog = QCursor::pos();
    bool details = false;
    if ((unknown.size() > 5) ||(missing.size()>5)|| (modified.size() >5)){
      details = true;
    }
    ObjectMessageBox msgBox(this,msg,simpleMsg, obj.objName.c_str(),"",details,1,true);
    msgBox.move(posDialog);

    //    window.setDetailedText(QString(simpleMsg.c_str()));
    int ret = msgBox.exec();
    msgBox.setAttribute(Qt::WA_DeleteOnClose, true);
    msgBox.close();

    int option = msgBox.getAnswer();
    if (ret == QMessageBox::Cancel)
	return;

    
    if (option == QMessageBox::Ok) {
      // try to save changes
      SaveStatus status = save_changes();
      // but make sure the pie chart is finished before quitting
      //obj.progressFadeTime = 0;
      iconPicture.fill(obj.bgColor);
      while (_progress._effectiveShow) {
        repaint();
      }
      // if the changes were not saved, give the user
      // some options
      if (status != SAVE_OK) {
        std::string msg = "Changes to the object were not "
                          "saved.\n"
                          "What would you like to do?\n"
                          "\n";
         int answer = QMessageBox::critical(
            NULL, "Object: Save failed...", msg.c_str(), "Don't quit",
            "Don't remove but quit", "Remove && quit", 0, -1);
        if (answer == 1)
          removeTemp = false;
        else if (answer == 2)
          ;
        else
          return;
      } else {
        if (vlabd->valid())
          vlabd->va_send_message(REFRESHICON, "%s@%s:%s",
                                 obj.connection->login_name,
                                 obj.connection->host_name, obj.objDir.c_str());
      }
    } else if (option == QMessageBox::Discard)
      ;
    else
      return;
  }
  
  // remove the lab table
  if (removeTemp)
    RemoveTemp();
  // kill all processes invoked from within the object
  kill(0, SIGKILL);
  // and we are done
  ::exit(0);
}

// Execute given command
// - all %s are replaced in the cmd by the name of the object
void QTGLObject::ExecuteCommand(QString cmd) {
  // replace '%s' with object's name (for example to give the terminal a proper
  // name)
  cmd.replace(QString("%s"), QString(obj.objName.c_str()));
  // try to vfork
  pid_t pid = fork();
  if (pid < 0) {
    qqWarning(
        QString("object: Cannot execute '%1' because fork() failed.").arg(cmd));
  } else if (pid == 0) {
    QByteArray ba = cmd.toLatin1();
    const char* str = ba.constData();
    //fflush(stdout);
    execlp("/bin/bash", "/bin/bash", "-c", str, (char *)NULL);
    qqWarning("object:execlp failed.");
    _exit(0);
  } else {
    // fork was created...
  }
}

// Re-reads and re-displays the icon image
void QTGLObject::NewIconCb() {
  iconPicture = QImage(); // this is to force reloading the icon
  loadIcon();
  update();
}

/******************************************************************************
 *
 * callback for the 'edit preferences'
 *
 */
void QTGLObject::editPreferencesCB() {

  QString filePreferences = QString("%1/object").arg(Vlab::getUserConfigDir());

  QString configPreferences = QString("%1/object.ini").arg(Vlab::getUserConfigDir());
  Preferences window(this, filePreferences, configPreferences);
  window.exec();
  window.setAttribute(Qt::WA_DeleteOnClose, true);
  window.close();

  NewMenuCb();

  readConfiguration(configPreferences);
}

void QTGLObject::shellPreferencesCB() {
  QDir::setCurrent(Vlab::getUserConfigDir());
  ExecuteCommand(objShell);
}

/******************************************************************************
 *
 * callback for the 'edit specifications'
 *
 */
void QTGLObject::editSpecCb() {
  QString cmd = QString("%1 %2/specifications ")
                    .arg(QString(objEd))
                    .arg(QString(obj.tmpDir.c_str()));
  ExecuteCommand(cmd);
}

void QTGLObject::NewMenuCb()
// ---------------------------------------------------------------------------
//  NewMenuCb( )
// 	- creates a new menu on Re-read Specifications
// ---------------------------------------------------------------------------
{
  oldMenu = menu;

  ReadSpecs(obj.local_connection, obj.tmpDir.c_str());
  MakeMenu();
}

void QTGLObject::rereadPreferencesCB() { NewMenuCb(); }

void QTGLObject::labTableShellCb() {
  if (chdir(obj.tmpDir.c_str()) != 0) 
    qqWarning("object: chdir failed in labTableShellCb().");
  ExecuteCommand(objShell);
}

#ifdef __APPLE__

// open current directory in finder
void QTGLObject::openLabTableFinderCb() {
  chdir(obj.tmpDir.c_str());
  QString cmd = QString("open . ");

  ExecuteCommand(cmd);
}

// open current directory in finder
void QTGLObject::openStorageFinderCb() {
  chdir(obj.objDir.c_str());
  QString cmd = QString("open . ");

  ExecuteCommand(cmd);
  chdir(obj.tmpDir.c_str());
}

#endif

void QTGLObject::storageShellCb() {
  if (chdir(obj.objDir.c_str()) != 0)
    qqWarning("object: chdir failed in storageShellCb().");
  ExecuteCommand(objShell);
  if (chdir(obj.tmpDir.c_str()) != 0)
    qqWarning("object: chdir failed in storageShellCb().");
}

void QTGLObject::dirviewLab()
//-------------------------------------------------------------------------
// Call dirviewCB with lab table option
//-------------------------------------------------------------------------
{
  dirviewCB(0);
}

void QTGLObject::dirviewStorage()
//-------------------------------------------------------------------------
// Call dirviewCB with storage option
//-------------------------------------------------------------------------
{
  dirviewCB(1);
}

/******************************************************************************
 *
 * callback for the 'directory' buttons
 *
 */
void QTGLObject::dirviewCB(int /*flag */) {
}

/*
 * PutFileCb( )
 *	- clipboard function
 *	- selection dialog called with list of all files
 *	- selected file is put on clipboard
 *
 * GetFileCb( )
 *	- gets file from clipboard
 *	- displays dialog message giving file found
 */
void QTGLObject::PutFileCb() {
#ifdef __DONT_COMPILE
  char *putfile, buf[STRLEN];

  if ((putfile = xSelect(topShell, "Select file", NULL, obj.fname,
                         obj.nfiles)) != NULL) {
    sprintf(buf, "%s/%s", obj.tempdir, putfile);
    clipb_putfile(buf);
  }
#endif
}

void QTGLObject::GetFileCb() { /* char * getfile = clipb_getfile( ); */ }

/******************************************************************************
 *
 * callback for the 'Save Changes' button
 *
 */
void QTGLObject::saveChangesCb() {
  // get the changes made to the object
  FileList modified, unknown, missing;
  getChanges(modified, unknown, missing);
  std::sort(modified.begin(),modified.end());
  std::sort(unknown.begin(),unknown.end());
  std::sort(missing.begin(),missing.end());

  // if discrepancies exist, report to the user
  // if (unknown.size () + missing.size () > 0)
  {
    std::string simpleMsg = "";

    std::string msg = "<qt><font family=arial><b>";
    if (modified.size()) {
      simpleMsg += "Modified files:\n";
      msg += "<big>Modified files:</big>";
      msg += "<ul>";
      for (size_t i = 0; i < modified.size(); i++) {
        simpleMsg += " - " + modified[i] + "\n";
	if (i < 5 )
	  msg += "<li>" + modified[i] + "</li>";
      }
      if (modified.size() > 5) {
	msg += "<li>Plus " +
	  QString::number(modified.size() - 5).toStdString() +
	  " more...</li>";
      }
      msg += "</ul>";
    } else {
      simpleMsg += "No modified files\n";
      msg += "<big>Modified files:</big>";
      msg += "<ul>";
      msg += "<li>None</li>";
      msg += "</ul>";
    }
    if (missing.size()) {
      simpleMsg += "Missing files:\n";

      msg += "<big>Missing files:</big> <small>"
             "(in specs but not on table)</small>";
      msg += "<ul>";
      for (size_t i = 0; i < missing.size(); i++) {
        simpleMsg += " - " + missing[i] + "\n";

	if (i <5 )
	  msg += "<li>" + missing[i] + "</li>";
      }
      if (missing.size() > 5) {
          msg += "<li>Plus " +
                 QString::number(missing.size() - 5).toStdString() +
                 " more...</li>";
      }
      msg += "</ul>";
    }
    if (unknown.size()) {
      simpleMsg += "Unknown files:\n";

      msg += "<big>Unknown files:</big> <small>"
             "(on table but not in specs)</small>";
      msg += "<ul>";
      for (size_t i = 0; i < unknown.size(); i++) {
 	if (i <5 )
	  msg += "<li>" + unknown[i] + "</li>";
        simpleMsg += " - " + unknown[i] + "\n";

      }
      if (unknown.size() > 5) {
	msg += "<li>Plus " +
	  QString::number(unknown.size() - 5).toStdString() +
	  " more...</li>";
      }
      msg += "</ul>";
    }
    msg += "</font></b></qt>";

    if ((missing.size() == 0) && (unknown.size() == 0) &&
        (modified.size() == 0)) {
      msg += "<b><big>Nothing to be saved</big></b>";
    }
    QPoint posDialog = QCursor::pos();
    bool details = false;
    if ((unknown.size() > 5) ||(missing.size()>5)|| (modified.size() >5)){
      details = true;
    }
    ObjectMessageBox msgBox(this,msg,simpleMsg, obj.objName.c_str(),"",details,1);
    msgBox.move(posDialog);

    //    window.setDetailedText(QString(simpleMsg.c_str()));
    int ret = msgBox.exec();
    msgBox.setAttribute(Qt::WA_DeleteOnClose, true);
    msgBox.close();

    ret = msgBox.getAnswer();
    if (ret == QMessageBox::Cancel)
	return;

    /*
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    QString wtitle = "Save: " + QString(obj.objName.c_str());
    msgBox.QDialog::setWindowTitle(wtitle);
    
    if ((missing.size() == 0) && (unknown.size() == 0) &&
        (modified.size() == 0)) {
      msg += "<b><big>Nothing to be saved</big></b>";
      msgBox.setInformativeText(msg.c_str());
      msgBox.setStandardButtons(QMessageBox::Save |QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Save);
      int option = msgBox.exec();
      if (option == QMessageBox::Cancel)
	return;
    }
    else{
      msgBox.setText(QString(msg.c_str()));
      if ((missing.size() > 5) || (unknown.size() > 5) ||
        (modified.size() > 5)) {
	msgBox.setDetailedText(QString(simpleMsg.c_str()));
      }
      msgBox.setInformativeText("<b><big>Save?</big></b>");
      msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Save);
      int option = msgBox.exec();
      if (option == QMessageBox::Cancel)
	return;
    }
    */
  }

  // try to save changes
  SaveStatus status = save_changes();
  // if the changes were not saved, give the user
  // some options
  if (status == SAVE_ERROR) {
    QMessageBox::critical(NULL, "object", "Object was not saved");
  } else {
    if (vlabd->valid())
      vlabd->va_send_message(REFRESHICON, "%s@%s:%s",
                             obj.connection->login_name,
                             obj.connection->host_name, obj.objDir.c_str());
  }
  return;
}

/******************************************************************************
 *
 * Slot for Export Object
 * 1) Open dialog to confirm export parameters
 * 2) Retrieve object from RA
 * 3) package and/or convert the format
 * 4) move the package to the destination
 * 5) clean up
 *
 */
void QTGLObject::exportObject() {

  // 1) Open dialog to confirm export parameters
  exportName = QString(obj.objName.c_str());
  ImportExport *window =
      new ImportExport(this, exportName, exportPath, exportArchiveType);
  //    window->setTab(2);
  window->setAttribute(Qt::WA_DeleteOnClose,
                       false); // make sure we can get our information after the
                               // dialog is finished interacting with the user
  window->exec();
  exportPath = window->getPath();
  exportArchiveType = window->getType();
  exportName = window->getNodeName();
 // 2->5) are handled within the window
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  window->close();
}

/******************************************************************************
 *
 * NewVersionCb( )
 *	- creates a new version of the object in its 'ext' directory
 *	- a dialog box is produced to request a name for the new version
 *	- a subdirectory of the object's 'ext' directory is created
 *	  and the files in the temp directory are linked/copied,
 *	  depending on whether they are the same/different.
 *	- the new object's 'ext' directory is also created
 */
void QTGLObject::NewVersionCb() {

  // get the changes made to the object
  FileList modified, unknown, missing;
  getChanges(modified, unknown, missing);
  std::sort(modified.begin(),modified.end());
  std::sort(unknown.begin(),unknown.end());
  std::sort(missing.begin(),missing.end());

  // if changes were made, report to the user
  // if (modified.size () + unknown.size () + missing.size () > 0)
  // the message shouldn't appear if there is only modified files (which is
  // almost always the case).
  //   if (unknown.size () + missing.size () + modified.size () > 0)
  //{
  std::string simpleMsg = "";
  std::string msg = "<qt> <font family=arial><b>";
  if (modified.size()) {
    simpleMsg += "Modified files:\n";
    msg += "<big>Modified files:</big>";
    msg += "<ul>";
    for (size_t i = 0; i < modified.size(); i++) {
      simpleMsg += " - " + modified[i] + "\n";
      if (i < 5 )
	msg += "<li>" + modified[i] + "</li>";
    }
    if (modified.size() > 5) {
      msg += "<li>Plus " +
	QString::number(modified.size() - 5).toStdString() +
	" more...</li>";
    }
    msg += "</ul>";
  }

  else {
    msg += "<big>Modified files:</big>";
    msg += "<ul>";
    msg += "<li>None</li>";
    msg += "</ul>";
  }
  if (missing.size()) {
    simpleMsg += " Missing files (in specs but not on table):\n";

    msg += "<big>Missing files:</big> <small>"
           "(in specs but not on table)</small>";
    msg += "<ul>";
    for (size_t i = 0; i < missing.size(); i++) {
      simpleMsg += " - " + missing[i] + "\n";
      if (i < 5) 
	msg += "<li>" + missing[i] + "</li>";
      if (i > 5) {
        msg += "<li>Plus " + QString::number(missing.size() - 5).toStdString() +
               " more...</li>";
        break;
      }
    }
    msg += "</ul>";
  }
  if (unknown.size()) {
    simpleMsg += "Unknown files (on table but not in specs):\n";

    msg += "<big>Unknown files:</big> <small>"
           "(on table but not in specs)</small>";
    msg += "<ul>";
    for (size_t i = 0; i < unknown.size(); i++) {
      simpleMsg += " - " + unknown[i] + "\n";
      if (i < 5) 
	msg += "<li>" + unknown[i] + "</li>";
      if (i == 5) {
        msg += "<li>Plus " + QString::number(unknown.size() - 5).toStdString() +
               " more...</li>";
      }
    }
    msg += "</ul>";
  }

  msg += "</font></b><hr>";

  std::string newDir;
  std::string prompt = "";

  struct waitcurs {
    waitcurs(QWidget *w) : _w(w) { _w->setCursor(Qt::BusyCursor); }
    ~waitcurs() { _w->unsetCursor(); }
    QWidget *_w;
  };

  std::string newName = obj.objName.c_str();
  bool positionToNewVersion = false;
  QPoint posDialog = QCursor::pos();
  while (1) {
    bool details = false;
    if ((unknown.size() > 5) ||(missing.size()>5)|| (modified.size() >5)){
      details = true;
    }

    msg += prompt;
    ObjectMessageBox window(this, msg, simpleMsg,newName, positionToNewVersion,details,0);
    window.move(posDialog);

    //    window.setDetailedText(QString(simpleMsg.c_str()));
    int ret = window.exec();
    window.setAttribute(Qt::WA_DeleteOnClose, true);
    window.close();

    ret = window.getAnswer();

    if (ret == QMessageBox::Cancel)
      return;

    newName = window.getNewName();
    positionToNewVersion = window.getNewPosition();


    if (newName.size() == 0) {
      prompt = "<qt><font color=red>"
               "Name cannot be empty!</font><hr>";
      continue;
    }
    // replace non-unix characters with underscores and let the user confirm
    std::string validNewName = validateObjectName(newName);
    if (newName != validNewName) {
      newName = validNewName;
      posDialog = window.pos();
      prompt = "<qt><font color=red>"
               "Some illegal characters have been replaced <br> with "
               "underscores.</font><hr>";
      continue;
    }
    // we have a valid name, let's test to see if extension with this name
    // exists
    newDir = obj.objDir + "/ext/" + newName;
    //std::cerr << "object: newdir = '" << newDir << "'\n";
    if (RA::Access(obj.connection, newDir.c_str(), F_OK) == 0) {
      posDialog = window.pos();
      prompt = "<qt><font color=red>"
               "Object already has extension with this name.</font><hr>";
      continue;
    }

    // create the directory
    RA::Mkdir(obj.connection, (obj.objDir + "/ext").c_str(), 0755);
    RA::Mkdir(obj.connection, newDir.c_str(), 0755);
    if (RA::Access(obj.connection, newDir.c_str(), F_OK) != 0) {
      auto button = QMessageBox::warning(nullptr, "Object",
          "<qt>"
          "Could not create this extension.<br>"
          "Check permissions on the directory.<br><br>"
          "Would you like to create a new version in lost_found? <hr>",
          QMessageBox::Ok | QMessageBox::Cancel,
          QMessageBox::Cancel);

      if (QMessageBox::Ok == button) {
        SaveStatus status = save_to_lost_found();
        if (SAVE_ERROR == status) {
          QMessageBox::critical(nullptr, "Object", "Object was not saved");
        }
      }
      return;
    }
    // the new object name is now valid, break out of the loop
    break;
  } // while 1 loop

  // set cursor to wait, until the function exits (the variable will go out of
  // scope)
  waitcurs tmpcurs(this);

  // copy the files over and then prototype the object
  progress().show(true,obj.getSaveAlways);
  progress().setup(obj.fnames.size() + 1);
  std::vector<std::string> errors;
  for (size_t i = 0; i < obj.fnames.size(); i++) {
    progress().advance();
    std::string oldfile = obj.objDir + "/" + obj.fnames[i];
    std::string tempfile = obj.tmpDir + "/" + obj.fnames[i];
    std::string newfile = newDir + "/" + obj.fnames[i];
    if (1 == RA::Put_file(tempfile.c_str(), obj.connection, newfile.c_str())) {
      errors.push_back("File " + obj.fnames[i] + " was not copied.");
    }
  }
  progress().advance();

  if (0 != RA::Prototype_object(obj.connection, newDir.c_str())) {
    errors.push_back("Prototyping the new extension failed.");
  }

  progress().show(false,obj.getSaveAlways);

  if (errors.size() > 0) {
    std::string msg = "<qt><font color=red>There were errors:<hr><ul>";
    for (size_t i = 0; i < errors.size(); i++)
      msg += "<li>" + errors[i] + "</li>";
    msg += "</ul>";
    vlabxutils::infoBox(this, msg.c_str());
  }
  // send a message to anyone listening (e.g. browsers) to
  // update the directory with this object
  if (vlabd->valid())
    vlabd->va_send_message(UPDATE, "%s@%s:%s", obj.connection->login_name,
                           obj.connection->host_name, obj.objDir.c_str());

  if (positionToNewVersion) {
    obj.objDir = newDir;
    obj.objName = newName;
    obj.nodePath = newDir;
    // set the environment variable to tell all applications we invoke
    // the name of the object
    setenv("VLAB_OBJECT_NAME", obj.objName.c_str(), 1);
    MakeMenu();

    this->setWindowTitle(newName.c_str());
    _topLevel->setWindowTitle(newName.c_str());
    positionCb(newDir);
  }

  return;
}

void QTGLObject::posObj()
//-------------------------------------------------------------------------
// Call vlabdaemon to position itself by passing it the object directory
//-------------------------------------------------------------------------
{
  if (RA::Access(obj.connection, obj.objDir.c_str(), F_OK) != 0) {
    QMessageBox::critical(NULL, "object", "Object can't be positionned");
  } else {
    setenv("VLAB_OBJECT_NAME", obj.objName.c_str(), 1);
    positionCb(obj.objDir);
  }
}

/******************************************************************************
 *
 * callback for the 'position' button - which will only send a message to
 * the vlabdaemon to position itself to this object
 *
 */
void QTGLObject::positionCb(const std::string &objName) {
  // send message to all browsers to position themselves...
  if (vlabd->valid())
    vlabd->va_send_message(POSITIONOBJ, "%s@%s:%s", obj.connection->login_name,
                           obj.connection->host_name, objName.c_str());
}

/******************************************************************************
 *
 * callback routine for the 'help' button in specification menu
 *
 */
void QTGLObject::help() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(QUrl::fromLocalFile(
      helpDir.filePath("VLABFramework.pdf")));
}

void QTGLObject::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
}

void QTGLObject::pdfHelp() {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(QUrl::fromLocalFile(
      helpDir.filePath("VLABFramework.pdf")));
}

// about vlab slot - displays the about dialog
void QTGLObject::about_object_cb() {
  vlab::about(this,"Object");
  return;
  QMessageBox box(NULL);
  std::string msg =
      "<p align=\"center\">\n"
      "<b><font size=\"+4\"><font "
      "color=\"#0000f5\">OBJECT</font></font></b><br>\n"
      "<br>\n"
      "http://algorithmicbotany.org/virtual_laboratory/<br>\n"
      "<br>\n"
      "Developed at the University of Calgary<br>\n"
      "Department of Computer Science<br>\n"
      "<br>\n"
      "<font size=\"-1\">Copyright (C) 1990-2022 P. Prusinkiewicz,\n"
      "University of Calgary</font>\n"
      "</p>\n";

  box.setText(QString(msg.c_str()));
  box.setIconPixmap(QPixmap::fromImage(QImage(":/images/logo1.png")));
  box.show();
  box.exec();
}

// Commented out the code below because object does not receive
// any message from vlab demon. (Pavol)
void QTGLObject::connectSocket() {}

/* NOT USED:
// Routine to render text
GLint QTGLObject::project(GLdouble objx, GLdouble objy, GLdouble objz,
                               const GLdouble model[16],
                               const GLdouble proj[16], const GLint viewport[4],
                               GLdouble *winx, GLdouble *winy, GLdouble *winz) {
  GLdouble in[4], out[4];
  in[0] = objx;
  in[1] = objy;
  in[2] = objz;
  in[3] = 1.0;
  transformPoint(out, model, in);
  transformPoint(in, proj, out);

  if (in[3] == 0.0)
    return GL_FALSE;

  in[0] /= in[3];
  in[1] /= in[3];
  in[2] /= in[3];

  *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
  *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;

  *winz = (1 + in[2]) / 2;
  return GL_TRUE;
}

void QTGLObject::transformPoint(GLdouble out[4], const GLdouble m[16],
                                     const GLdouble in[4]) {
#define M(row, col) m[col * 4 + row]
  out[0] =
      M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
  out[1] =
      M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
  out[2] =
      M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
  out[3] =
      M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

void QTGLObject::renderText(double x, double y, double z, const QString &str,
                          const QColor color, const QFont &font) {
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  int height = this->height();

  GLdouble model[4][4], proj[4][4];
  GLint view[4];
  glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
  glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
  glGetIntegerv(GL_VIEWPORT, &view[0]);
  GLdouble textPosX = 0, textPosY = 0, textPosZ = 0;

  project(x, y, z, &model[0][0], &proj[0][0], &view[0], &textPosX, &textPosY,
          &textPosZ);

  textPosY = height - textPosY; // y is inverted

  int fontSize = font.pointSize();

  QFontMetrics metrics(font);
  int text_width = metrics.width(QString(str)) + 10;
  int text_height = fontSize + 5;
  QPixmap textimg(text_width, text_height);
  textimg.fill(Qt::transparent);

  QPainter painter(&textimg);
  QColor backgroundColor(0,0,0);
  painter.setRenderHints(QPainter::HighQualityAntialiasing |
                         QPainter::TextAntialiasing |
                         QPainter::NonCosmeticDefaultPen);

  painter.setBrush(color);
  painter.setPen(color);
  painter.setFont(font);
  painter.drawText(5, text_height - 2, str);
  painter.end();
  QPainter p1(this);
  p1.drawPixmap(textPosX - 5, textPosY - text_height + 2, textimg);
  glPopAttrib();
  glPopClientAttrib();

  glEnable(GL_DEPTH_TEST);
  // Enable blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
*/
