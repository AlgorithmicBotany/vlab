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



/* Palette

   Implementation of Class: GLColourMap
                            ColourTuner

   Last Modified by: Joanne
   On Date: 14-06-01
*/
#define  GL_SILENCE_DEPRECATION
#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollArea>
#include <QVBoxLayout>
#include <chrono>
#include <ctime>
#include <qcursor.h>

using namespace Qt;
#include "glcolourmap.h"

// ==================== Class: GLColourMap
// -------------------- Construction/Destruction
GLColourMap::GLColourMap(QWidget *parent, SavingMode savingMode,
                         const char *)
    : QOpenGLWidget(parent) {
  _savingMode = savingMode;
  setMinimumHeight(175);
  setFocusPolicy(Qt::StrongFocus);
  currentTime = std::chrono::system_clock::now();
  popmenu = new QMenu(this);
  popmenu->setObjectName("popmenu");
  QAction *act = 0;
  act =
      popmenu->addAction("&Save Page", this, SLOT(SAVE()), QKeySequence::Save);
  addAction(act);
  act = popmenu->addAction("Save &All", this, SLOT(SAVE_ALL()),
                           QKeySequence("Shift+Ctrl+S"));
  addAction(act);
  act = popmenu->addAction("&Revert to saved", this, SLOT(REVERT()),
                           QKeySequence("Ctrl+R"));
  addAction(act);
  popmenu->insertSeparator(act);
  act = popmenu->addAction("S&elect All", this, SLOT(SELECT()),
                           QKeySequence::SelectAll);
  addAction(act);
  act = popmenu->addAction("Cut", this, SLOT(CUT()), QKeySequence::Cut);
  addAction(act);
  act = popmenu->addAction("C&opy", this, SLOT(COPY()), QKeySequence::Copy);
  addAction(act);
  _paste =
      popmenu->addAction("Paste", this, SLOT(PASTE()), QKeySequence::Paste);
  addAction(_paste);
  _insert = popmenu->addAction("I&nsert", this, SLOT(INSERT()),
                               QKeySequence("Ctrl+N"));
  addAction(_insert);
  act = popmenu->addAction("&Interpolate", this, SLOT(INTERPOLATE()),
                           QKeySequence("Ctrl+I"));
  addAction(act);
  act = popmenu->addAction("Fine &Tune", this, SLOT(TUNE()),
                           QKeySequence("Ctrl+T"));
  addAction(act);
  act = popmenu->addAction("HSV &Pick", this, SLOT(PICK()),
                           QKeySequence("Ctrl+H"));
  addAction(act);
  popmenu->addSeparator();

  QMenu *modeMenu = popmenu->addMenu("Refresh mode");

  _savingMenu_act = modeMenu->addAction("Explicit", this, SLOT(ModeOff()));
  _savingMenu_act->setCheckable(true);
  if (_savingMode == OFF)
    _savingMenu_act->setChecked(true);

  _savingTriggered_act =
      modeMenu->addAction("Triggered", this, SLOT(TriggeredSavingMode()));
  _savingTriggered_act->setCheckable(true);
  if (_savingMode == TRIGGERED)
    _savingTriggered_act->setChecked(true);

  _savingContinu_act =
      modeMenu->addAction("Continuous", this, SLOT(ContinuousSavingMode()));
  _savingContinu_act->setCheckable(true);
  if (_savingMode == CONTINUOUS)
    _savingContinu_act->setChecked(true);

  popmenu->addSeparator();
  act = popmenu->addAction("E&xit", parent, SLOT(close()),
                           QKeySequence("Ctrl+Q"));
  addAction(act);

  _paste->setEnabled(false);
  _insert->setEnabled(false);

  mX = mY = 0;
  selectindex = 0;
  interindex = -1;
  offset = 1;
  numcolours = 256;
  numpages = 16;
  currpage = 0;
  P = new page[numpages];
  copycolour.i = -1; // use i for flag
  copypage = -1;
  selectall = false;
  selectable = true;
  tuner = new ColourTuner(0, "tuner");
  picker = new ColourPick(this, "picker");
}

GLColourMap::~GLColourMap() {
  if (P) {
    for (int i = 0; i < numpages; i++) {
      if (P[i].colourfile)
        delete P[i].colourfile;
      delete[] P[i].colours;
    }
  }
  emit quit();
}

void GLColourMap::ContinuousSavingMode() {
  _savingMode = CONTINUOUS;
  _savingContinu_act->setChecked(true);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(false);
}

void GLColourMap::TriggeredSavingMode() {
  _savingMode = TRIGGERED;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(true);
  _savingMenu_act->setChecked(false);
}

void GLColourMap::ModeOff() {
  _savingMode = OFF;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(true);
}

// -------------------- Rendering
void GLColourMap::initializeGL() {
  glClearColor(0, 0, 0, 1);
  glShadeModel(GL_FLAT);
}

void GLColourMap::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT);
  drawcolourmap(16, 16);
}

void GLColourMap::resizeGL(int, int) { viewerSetup(); }

void GLColourMap::viewerSetup() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  gluOrtho2D(0, (GLdouble)width() + 1, 0, (GLdouble)height() + 1);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glViewport(0, 0, (GLint)width(), (GLint)height());
}

bool GLColourMap::drawcolourmap(int m, int n) {
  GLdouble w, h, x, y;
  int c;

  if ((m * n) != numcolours)
    return false;

  mapH = m;
  mapW = n;

  h = (GLdouble)height() / (GLdouble)mapH;
  w = (GLdouble)width() / (GLdouble)mapW;

  x = 0;
  c = 0;
  for (int i = 0; i < m; i++) {
    y = height();
    for (int j = 0; j < n; j++) {
      glBegin(GL_QUADS);
      {
        glColor3f(P[currpage].colours[c].r, P[currpage].colours[c].g,
                  P[currpage].colours[c].b);
        glVertex2d(x, y);
        glVertex2d(x + w, y);
        glVertex2d(x + w, y - h);
        glVertex2d(x, y - h);
      }
      glEnd();

      if (selectall)
        glColor3f(.9, .9, .9);
      else if ((P[currpage].colours[c].r <= 0.07) &&
               (P[currpage].colours[c].g <= 0.07) &&
               (P[currpage].colours[c].b <= 0.07))
        glColor3f(0.15, 0.15, 0.15);
      else
        glColor3f(0, 0, 0);
      glLineWidth(1);
      glBegin(GL_LINE_LOOP);
      {
        glVertex2d(x, y);
        glVertex2d(x + w, y);
        glVertex2d(x + w, y - h);
        glVertex2d(x, y - h);
      }
      glEnd();
      y -= h;
      c++;
    }
    x += w;
  }

  if (!selectall) {
    glColor3f(.85, .9, .9);
    for (int i = 0; i < offset; i++) {
      x = w * ((selectindex + i) / mapH);
      y = height() - h * ((selectindex + i) % mapH);

      glColor3f(.85, .9, .9);

      glBegin(GL_LINE_LOOP);
      {
        glVertex2d(x, y);
        glVertex2d(x + w, y);
        glVertex2d(x + w, y - h);
        glVertex2d(x, y - h);
      }
      glEnd();
    }
  }

  glFlush();

  return true;
}

void GLColourMap::interpolate() {
  int steps, A, B;
  GLfloat redscale, greenscale, bluescale;

  if ((selectindex == interindex) || (selectindex == interindex + 1) ||
      (selectindex == interindex - 1))
    return;

  if (selectindex > interindex) {
    A = interindex;
    B = selectindex;
  } else {
    A = selectindex;
    B = interindex;
  }

  steps = B - A;

  redscale = (GLfloat)(P[currpage].colours[B].bytes[0] -
                       P[currpage].colours[A].bytes[0]) /
             (GLfloat)(steps * 255);
  greenscale = (GLfloat)(P[currpage].colours[B].bytes[1] -
                         P[currpage].colours[A].bytes[1]) /
               (GLfloat)(steps * 255);
  bluescale = (GLfloat)(P[currpage].colours[B].bytes[2] -
                        P[currpage].colours[A].bytes[2]) /
              (GLfloat)(steps * 255);

  for (int i = A + 1; i < B; i++) {

    P[currpage].colours[i].r = P[currpage].colours[i - 1].r + redscale;
    P[currpage].colours[i].g = P[currpage].colours[i - 1].g + greenscale;
    P[currpage].colours[i].b = P[currpage].colours[i - 1].b + bluescale;

    P[currpage].colours[i].bytes[0] = (GLubyte)(P[currpage].colours[i].r * 255);
    P[currpage].colours[i].bytes[1] = (GLubyte)(P[currpage].colours[i].g * 255);
    P[currpage].colours[i].bytes[2] = (GLubyte)(P[currpage].colours[i].b * 255);

    P[currpage].modified = true;
    emit MODIFIED(true);

  }
  update();

  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
    SAVE();
  }

}

// ---------------------- Colour File Generation / IO Functions
int GLColourMap::defaultPalette(int index) {
  QFile *C = new QFile("../CONFIG/sys.map");
  return 0;

  char col[3];

  C->open(QIODevice::ReadOnly);

  for (int i = 0; i < numcolours; i++) {

    C->read(col, 3);

    P[index].colours[i].bytes[0] = P[index].colours[i].oldbytes[0] =
        (GLubyte)((unsigned char)col[0]);
    P[index].colours[i].bytes[1] = P[index].colours[i].oldbytes[1] =
        (GLubyte)((unsigned char)col[1]);
    P[index].colours[i].bytes[2] = P[index].colours[i].oldbytes[2] =
        (GLubyte)((unsigned char)col[2]);
    P[index].colours[i].r = (GLfloat)((unsigned char)col[0]) / (GLfloat)255;
    P[index].colours[i].g = (GLfloat)((unsigned char)col[1]) / (GLfloat)255;
    P[index].colours[i].b = (GLfloat)((unsigned char)col[2]) / (GLfloat)255;
    P[index].colours[i].i = i;
  }

  C->close();
}

void GLColourMap::generateGreyScale(int index) {
  for (int i = 0; i < numcolours; i++) {
    P[index].colours[i].bytes[0] = P[index].colours[i].oldbytes[0] =
        (GLubyte)(i);
    P[index].colours[i].bytes[1] = P[index].colours[i].oldbytes[1] =
        (GLubyte)(i);
    P[index].colours[i].bytes[2] = P[index].colours[i].oldbytes[2] =
        (GLubyte)(i);
    P[index].colours[i].r = (GLfloat)(i) / (GLfloat)255;
    P[index].colours[i].g = (GLfloat)(i) / (GLfloat)255;
    P[index].colours[i].b = (GLfloat)(i) / (GLfloat)255;
    P[index].colours[i].i = i;
  }
}

void GLColourMap::generateBlankPage(int index) {
  for (int i = 0; i < numcolours; i++) {
    P[index].colours[i].bytes[0] = P[index].colours[i].oldbytes[0] = 0;
    P[index].colours[i].bytes[1] = P[index].colours[i].oldbytes[1] = 0;
    P[index].colours[i].bytes[2] = P[index].colours[i].oldbytes[2] = 0;
    P[index].colours[i].r = 0;
    P[index].colours[i].g = 0;
    P[index].colours[i].b = 0;
    P[index].colours[i].i = i;
  }
}

bool GLColourMap::read(int index) {
  char *col = new char[3];

  if (!P[index].colourfile->exists()) {
    QString mess("Cannot read colourmap file ");
    mess += *P[index].name;
    mess += ".\nIt has disappeared.";
    emit NOTICE(mess, -1);
    return false;
  }
  if (P[index].colourfile->size() != (unsigned long long)numcolours * 3) {
    QString mess("Cannot read colourmap file ");
    mess += *P[index].name;
    mess += ".\nIt has been modified on disk.";
    emit NOTICE((const QString &)mess, -1);
    return false;
  }

  P[index].colourfile->open(QIODevice::ReadOnly);

  for (int i = 0; i < numcolours; i++) {

    P[index].colourfile->read(col, 3);

    P[index].colours[i].bytes[0] = P[index].colours[i].oldbytes[0] =
        (GLubyte)((unsigned char)col[0]);
    P[index].colours[i].bytes[1] = P[index].colours[i].oldbytes[1] =
        (GLubyte)((unsigned char)col[1]);
    P[index].colours[i].bytes[2] = P[index].colours[i].oldbytes[2] =
        (GLubyte)((unsigned char)col[2]);
    P[index].colours[i].r = (GLfloat)((unsigned char)col[0]) / (GLfloat)255;
    P[index].colours[i].g = (GLfloat)((unsigned char)col[1]) / (GLfloat)255;
    P[index].colours[i].b = (GLfloat)((unsigned char)col[2]) / (GLfloat)255;
    P[index].colours[i].i = i;
  }

  P[index].colourfile->close();
  return true;
}

// -------------------- Some Useful Little Functions
void GLColourMap::checkModified() {
  bool m = false;
  for (int i = 0; i < numpages; i++)
    if (P[i].modified) {
      m = true;
      break;
    }
  if (!m)
    emit MODIFIED(false);
}

void GLColourMap::updateColours() {
  update();
  if (!selectall) {
    emit myRED((int)P[currpage].colours[selectindex].bytes[0]);
    emit myGREEN((int)P[currpage].colours[selectindex].bytes[1]);
    emit myBLUE((int)P[currpage].colours[selectindex].bytes[2]);
  }
}

void GLColourMap::oldBytes() {
  for (int i = 0; i < numcolours; i++) {
    P[currpage].colours[i].oldbytes[0] = P[currpage].colours[i].bytes[0];
    P[currpage].colours[i].oldbytes[1] = P[currpage].colours[i].bytes[1];
    P[currpage].colours[i].oldbytes[2] = P[currpage].colours[i].bytes[2];
  }
}

// ---------------------- Mouse Events Handlers
void GLColourMap::mousePressEvent(QMouseEvent *me) {
  mX = me->x();
  mY = me->y();
  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
  if (topWidget != nullptr)
    topWidget->raise();

  if (RightButton == me->button()) {
    popmenu->exec(QCursor::pos());
  }

  if (Qt::MidButton == me->button()) {
    if (selectable) {
      if (!selectall) {
        interindex = selection_test();
        offset = interindex - selectindex + 1;
        update(); // to (briefly) highlight the square

        QString mess = "Interpolate ";
        mess += QString::number(selectindex);
        mess += " To  ";
        mess += QString::number(interindex);
        emit CONFIRM(mess);

        interpolate();
        interindex = -1;
      } else
        emit CONFIRM("Select an index to start interpolation.");
    } else
      emit CONFIRM("Selection disabled for tuning.");
  }

  if (LeftButton == me->button()) {
    SELECTINDEX(selection_test());
    if (selectable)
      emit INDEX(selectindex);
  }
}

void GLColourMap::mouseMoveEvent(QMouseEvent *me) {
  mX = me->x();
  mY = me->y();
  offset = selection_test() - selectindex + 1;
  if (offset > 1) {
    emit CONFIRM("Select indices " + QString::number(selectindex) + " to " +
                 QString::number(selectindex + offset - 1));
  }
  if (offset < 1)
    offset = 1; // FORWARDS ONLY!!!
  if (_savingMode == CONTINUOUS)
    SAVE();

  update();
}

void GLColourMap::mouseReleaseEvent(QMouseEvent *) {
  if (_savingMode == TRIGGERED) {
    SAVE();
  }
  //if (_savingMode == CONTINUOUS)
  //  SAVE();

  update();
}

int GLColourMap::selection_test() {
  GLdouble h = (GLdouble)height() / (GLdouble)mapH;
  GLdouble w = (GLdouble)width() / (GLdouble)mapW;
  int index = ((int)mapH * (int)(mX / w)) + (int)(mY / h);
  if (index > 255)
    index = 255;
  return (index);
}

// --------------------- Slots

void GLColourMap::closeEvent() {
  // 2012.03.19 PBdR: Added local variable to get the address of it. You can't
  // get the address of a temporary object, as the object is not required to
  // still exist as soon as you got its address ...
  QCloseEvent ev;
  emit closeEvent(&ev);
}

void GLColourMap::INTERPOLATE() {
  interindex = selectindex + offset - 1;
  QString mess = "Interpolate ";
  mess += QString::number(selectindex);
  mess += " To  ";
  mess += QString::number(interindex);
  emit CONFIRM(mess);
  interpolate();
  interindex = -1;
  //SAVE();
}

void GLColourMap::INIT(page *mypages) {
  bool error = false;
  int numfiles = 0;

  for (int i = 0; i < numpages; i++) {

    P[i].colourfile = mypages[i].colourfile;
    P[i].pg = mypages[i].pg;
    P[i].name = mypages[i].name;
    P[i].colours = new colour[256];
    P[i].modified = false;

    if (P[i].colourfile) {
      if (!read(i)) {
        error = true;
        break;
      }
      numfiles++;
    } else
      generateBlankPage(i);
  }

  if (!error) {
    if (!P[0].colourfile) {
      if (!defaultPalette(0))
        generateGreyScale(0);
    }

    if (numfiles) {
      QString mess(QString::number(numfiles));
      if (numfiles == 1)
        mess += " page read.";
      else
        mess += " pages read.";
      emit CONFIRM(mess);
    } else
      emit CONFIRM("Ready!");

    parentWidget()->setWindowTitle(*P[currpage].name);
    updateColours();
  }
}

void GLColourMap::SAVE_ALL() {
  int temp = currpage;
  int num = 0;
  bool m = false;

  for (int i = 0; i < numpages; i++) {
    if (P[i].modified) {
      currpage = i;
      update(); // to show which one is currently being saved
      SAVE();
      if (!P[i].modified)
        num++;
      m = true;
    }
  }

  currpage = temp;
  update();

  QString mess(QString::number(num));
  if (m) {
    if (num == 1)
      mess += " file saved.";
    else
      mess += " files saved.";
    emit CONFIRM(mess);
  } else
    emit CONFIRM("No files have been modified.");
}

void GLColourMap::SAVE() {
  QFile *f;
  if (P[currpage].modified) {
    if (!P[currpage].colourfile) {
      QString mess = "Save file ";
      mess += *P[currpage].name;
      mess += " ?";
      QMessageBox msgBox;
      msgBox.setText("Save File ");
      msgBox.setInformativeText(mess);
      msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
       msgBox.setDefaultButton(QMessageBox::Save);

      int ret = msgBox.exec();
      switch (ret) {
      case QMessageBox::Save:
        f = new QFile(*P[currpage].name);
        if (f->exists()) {
          QString mess("File ");
          mess += *P[currpage].name;
          mess += " exists.  Overwrite?";
          switch (QMessageBox::information(this, "Overwrite File", mess,
                                           "Overwrite", "Cancel", 0, 1)) {
          case 0:
            break;
          case 1:
          default:
            return;
          }
        }
        P[currpage].colourfile = f;
        break;

      case QMessageBox::Discard:
        //	default:
        return;
        break;

      default:
        QString fn = QFileDialog::getSaveFileName(
            this, QString(), "ColourMap Files (*.map);;All Files (*.*)");
        if (!fn.isEmpty()) {
          f = new QFile(fn);
          if (f->exists()) {
            QString mess("File ");
            mess += *P[currpage].name;
            mess += " exists.  Overwrite?";
            switch (QMessageBox::information(this, "Overwrite File", mess,
                                             "Overwrite", "Cancel", 0, 1)) {
            case 0:
              break;
            case 1:
            default:
              return;
            }
          }
          P[currpage].colourfile = f;
          int dir = fn.lastIndexOf("/");
          if (dir != -1)
            fn.remove(0, dir + 1);
          *P[currpage].name = fn;
          parentWidget()->setWindowTitle(*P[currpage].name);
        } else
          return;
        break;
      }
    }

    P[currpage].colourfile->open(QIODevice::Unbuffered | QIODevice::WriteOnly |
                                 QIODevice::Truncate);
    for (int i = 0; i < numcolours; i++)
      P[currpage].colourfile->write((const char *)P[currpage].colours[i].bytes,
                                    3);

    P[currpage].colourfile->close();

    P[currpage].modified = false;

    oldBytes();

    QString mess("File ");
    mess += *P[currpage].name;
    mess += " saved.";
    emit CONFIRM(mess);
    std::chrono::system_clock::time_point end =
        std::chrono::system_clock::now();
    currentTime = end;
    checkModified();
  } else
    emit CONFIRM("Page not modified.");
}

void GLColourMap::REVERT() {
  if (P[currpage].colourfile) {
    if (read(currpage)) {
      updateColours();

      P[currpage].modified = false;
      checkModified();

      QString mess("Reverted file ");
      mess += *P[currpage].name;
      mess += " to last saved.";
      emit CONFIRM(mess);
    }
  } else {
    QString mess("File ");
    mess += *P[currpage].name;
    mess += " not saved yet.";
    emit CONFIRM(mess);
  }
}

void GLColourMap::updateRED(int r) {
  if ((!selectall) && (selectable)) {
    if (P[currpage].colours[selectindex].bytes[0] != (GLubyte)r) {
      P[currpage].colours[selectindex].bytes[0] = (GLubyte)r;
      P[currpage].colours[selectindex].r = (GLfloat)r / (GLfloat)255;
      P[currpage].modified = true;
      emit MODIFIED(true);
      update();
    }
  }
}

void GLColourMap::sliderTriggered() {
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS))
    SAVE();
}

void GLColourMap::sliderMoving() {
  if (_savingMode == CONTINUOUS)
    SAVE();
}

void GLColourMap::updateGREEN(int g) {
  if ((!selectall) && (selectable)) {
    if (P[currpage].colours[selectindex].bytes[1] != (GLubyte)g) {
      P[currpage].colours[selectindex].bytes[1] = (GLubyte)g;
      P[currpage].colours[selectindex].g = (GLfloat)g / (GLfloat)255;
      P[currpage].modified = true;
      emit MODIFIED(true);
      update();
    }
  }
}

void GLColourMap::updateBLUE(int b) {
  if ((!selectall) && (selectable)) {
    if (P[currpage].colours[selectindex].bytes[2] != (GLubyte)b) {
      P[currpage].colours[selectindex].bytes[2] = (GLubyte)b;
      P[currpage].colours[selectindex].b = (GLfloat)b / (GLfloat)255;
      P[currpage].modified = true;
      emit MODIFIED(true);
      update();
    }
  }
}

void GLColourMap::PAGE(int pg) {
  if (selectable) {
    currpage = pg;

    SLIDERS_ON(true);
    QString mess;

    parentWidget()->setWindowTitle(*P[currpage].name);

    mess = "Page Number ";
    mess += QString::number(currpage + 1);
    emit CONFIRM(mess);

    updateColours();
  }
}

void GLColourMap::ABOUT() {
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
}

void GLColourMap::HELP() {
  QString mess;
  mess =
      "<h2>"
      "Quick Help for VLAB Palette</h2>"
      ""
      "<h2>"
      "<b><i>1 Palette Execution</i></b></h2>"
      "Execute the VLAB Palette Editor using the following arguments:"
      "<p><b>>&nbsp; palette</b>"
      "<br>Opens 16 new palette pages for editing, with greyscale in the first"
      "page."
      "<p><b>>&nbsp; palette -m[pg#]&nbsp; [file.map] ...</b>"
      "<br>Opens list of palettes on specified pages"
      "<p><b>>&nbsp; palette ... -w [width] [height]</b>"
      "<br>Opens palette with specified dimensions<b><u></u></b>"
      "<p><b>Note</b> that the only way to load a specific colourmap is via "
      "command"
      "line arguments, and each page has its own file."
      "<br>A colourmap file is always exactly 768 bytes in length, 256 entries"
      "of 1 byte each red, green and blue."
      "<br>&nbsp;"
      "<h2>"
      "<i>2 Display and Selection</i></h2>"
      ""
      "<ul>"
      "<li>"
      "Each page contains 256 colour entries, arranged as 16 columns and 16 "
      "rows,"
      "index 0 in the upper left corner with indices increasing "
      "downwards.&nbsp;&nbsp;"
      "Page numbers along the top of the window (1 to 16) indicate the 16 "
      "colormaps"
      "that may be selected with the left mouse button.</li>"
      ""
      "<li>"
      "A <b>colour entry may be selected </b>by clicking the <b>left mouse</b>"
      "button on the entry, or by entering the index in the spinbox in the "
      "lower"
      "right corner. The 3 sliders near the top will immediately display the "
      "RGB"
      "values. The color may be changed by manipulating the sliders using the"
      "mouse to slide to the desired RGB levels.</li>"
      ""
      "<li>"
      "A <b>range of indices</b> can be selected by clicking the left mouse on"
      "the lesser index, and dragging to the greater index.</li>"
      ""
      "<li>"
      "An <b>entire page</b> can be selected by activating the popup menu and"
      "selecting <b><i>Select All.</i></b></li>"
      ""
      "<li>"
      "Selected indices are <b>outlined</b> in a light colour.</li>"
      ""
      "<li>"
      "The <b>Popup Menu</b> is activated by clicking the <b>right mouse</b> "
      "button"
      "anywhere on the palette entries.</li>"
      "</ul>"
      "<b><i></i></b>"
      "<p><br><b><u>2.1 Actions on Selection</u></b><b></b>"
      "<p><b>Cut, Copy, Paste, Insert</b>"
      "<br>Cut, Copy, Paste, and Insert can be invoked by <b>activating the "
      "popup"
      "menu</b> and selecting the desired option, or by using keyboard "
      "shortcuts"
      "(displayed on the popup menu)."
      "<ul>"
      "<li>"
      "Once an index or range of indices is selected, it can be cut or copied,"
      "then pasted or inserted onto any palette page.</li>"
      ""
      "<li>"
      "If a selection is <b>cut</b>, all following entries are shuffled up, and"
      "the palette is padded with null (black) colour entries.</li>"
      ""
      "<li>"
      "If a selection is <b>inserted</b>, all following entries (<i>including"
      "the current selection</i>) are shuffled down, and extra entries are "
      "pushed"
      "out.</li>"
      ""
      "<li>"
      "If the selection that is i<b>nserted or pasted</b> is larger than the "
      "space"
      "remaing on the palette page, the extra entries are ignored.</li>"
      "</ul>"
      "<b>Colour Interpolation</b>"
      "<br>Interpolated colour ramps can be created in the following ways:"
      "<ul>"
      "<li>"
      "<b>Click</b> the <b>first</b> colour using the <b>left button</b>, then"
      "the <b>second</b> using the <b>middle button</b>.</li>"
      ""
      "<li>"
      "Select a range of indices as described above, activate the popup menu "
      "with"
      "the right mouse button, and select <b><i>Interpolate.</i></b></li>"
      "</ul>"
      "<b>Fine Tune</b>"
      "<br>Once an index or range of indices has been selected, the Fine Tuner"
      "can be invoked by <b>activating the popup menu</b> and selecting "
      "<b><i>Fine"
      "Tune</i></b>."
      "<br>Fine Tuning provides a way to add or remove up to 25% of the "
      "selections"
      "red, green or blue components, or the brightness value of the colour."
      "<br>(Note, this may affect any interpolation effects.)"
      "<p><b>HSV Pick</b>"
      "<br>The colour of a selected index may be picked from an <b>HSV colour"
      "wheel</b> by <b>activating the popup menu</b> and selecting "
      "<b><i>HSV&nbsp;Pick</i></b>."
      "<br>The HSV colour wheel, as well the colour sliders, only work on a "
      "single"
      "selection, or the first selection in a range."
      "<br>&nbsp;"
      "<h2>"
      "<i>3 Saving Colourmaps</i></h2>"
      "The user is able to <b>save the current page</b>, <b>save all pages</b>,"
      "or <b>revert to the last saved </b>version of the current page."
      "<br>If a page is not modified, it will not be saved."
      "<br>If a page was loaded at the command line, then the page is saved to"
      "that filename."
      "<br>If a page has a default name and has not yet been saved, then a "
      "message"
      "appears with options to <b><i>save</i></b>, <b><i>not save </i></b>or "
      "<b><i>rename</i></b>"
      "the colourmap."
      "<br>If modified colourmaps exist upon selecting <b><i>Exit</i></b> or "
      "closing"
      "the Palette window, the user is prompted for whether or not to save.";

  QScrollArea *helpscroll = new QScrollArea();
  QLabel *label = new QLabel("", helpscroll->viewport());
  label->setTextFormat(RichText);
  label->setText(mess);
  helpscroll->setWidget(label);
  helpscroll->show();
}

void GLColourMap::CUT() {
  COPY();

  // shuffle
  int j = 0;
  for (int i = selectindex + offset; i < 256; i++) {
    P[currpage].colours[selectindex + j].bytes[0] =
        P[currpage].colours[i].bytes[0];
    P[currpage].colours[selectindex + j].bytes[1] =
        P[currpage].colours[i].bytes[1];
    P[currpage].colours[selectindex + j].bytes[2] =
        P[currpage].colours[i].bytes[2];
    P[currpage].colours[selectindex + j].r =
        (GLfloat)P[currpage].colours[selectindex + j].bytes[0] / (GLfloat)255;
    P[currpage].colours[selectindex + j].g =
        (GLfloat)P[currpage].colours[selectindex + j].bytes[1] / (GLfloat)255;
    P[currpage].colours[selectindex + j].b =
        (GLfloat)P[currpage].colours[selectindex + j].bytes[2] / (GLfloat)255;
    j++;
  }

  // fill
  for (int i = selectindex + j; i < 256; i++) {
    P[currpage].colours[i].bytes[0] = 0;
    P[currpage].colours[i].bytes[1] = 0;
    P[currpage].colours[i].bytes[2] = 0;
    P[currpage].colours[i].r = 0;
    P[currpage].colours[i].g = 0;
    P[currpage].colours[i].b = 0;
  }

  if (selectall)
    emit CONFIRM(QString("Cut page: ") + QString::number(currpage + 1));
  else {
    if (offset == 1)
      emit CONFIRM("Cut index " + QString::number(selectindex));
    else
      emit CONFIRM("Cut indices " + QString::number(selectindex) + " to " +
                   QString::number(selectindex + offset - 1));
  }

  _paste->setEnabled(true);
  _insert->setEnabled(true);
  update();
}

void GLColourMap::INSERT() {
  int CO = copyoffset;
  if ((selectindex + CO) > 255)
    CO = 255 - selectindex + 1;
  int j = 0;
  for (int i = 255 - CO; i >= selectindex; i--) {
    P[currpage].colours[255 - j].bytes[0] = P[currpage].colours[i].bytes[0];
    P[currpage].colours[255 - j].bytes[1] = P[currpage].colours[i].bytes[1];
    P[currpage].colours[255 - j].bytes[2] = P[currpage].colours[i].bytes[2];
    P[currpage].colours[255 - j].r =
        (GLfloat)P[currpage].colours[255 - j].bytes[0] / (GLfloat)255;
    P[currpage].colours[255 - j].g =
        (GLfloat)P[currpage].colours[255 - j].bytes[1] / (GLfloat)255;
    P[currpage].colours[255 - j].b =
        (GLfloat)P[currpage].colours[255 - j].bytes[2] / (GLfloat)255;
    j++;
  }

  for (int i = 0; i < CO; i++) {
    P[currpage].colours[selectindex + i].bytes[0] = copy[i][0];
    P[currpage].colours[selectindex + i].bytes[1] = copy[i][1];
    P[currpage].colours[selectindex + i].bytes[2] = copy[i][2];
    P[currpage].colours[selectindex + i].r =
        (GLfloat)P[currpage].colours[selectindex + i].bytes[0] / (GLfloat)255;
    P[currpage].colours[selectindex + i].g =
        (GLfloat)P[currpage].colours[selectindex + i].bytes[1] / (GLfloat)255;
    P[currpage].colours[selectindex + i].b =
        (GLfloat)P[currpage].colours[selectindex + i].bytes[2] / (GLfloat)255;
  }
  if (selectall)
    emit CONFIRM(QString("Insert page: ") + QString::number(currpage + 1));
  else {
    if (CO == 1)
      emit CONFIRM("Insert index " + QString::number(selectindex));
    else
      emit CONFIRM("Insert indices " + QString::number(selectindex) + " to " +
                   QString::number(selectindex + CO - 1));
  }
  updateColours();
  update();
}

void GLColourMap::COPY() {
  if (selectable) {
    if (selectall) {
      copypage = currpage;
      copyindex = 0;
      copyoffset = 256;
      emit CONFIRM(QString("Copy page ") + QString::number(currpage + 1));
    } else {
      copypage = currpage;
      copyindex = selectindex;
      copyoffset = offset;
      if (offset > 1)
        emit CONFIRM(QString("Copy indices ") + QString::number(selectindex) +
                     " to " + QString::number(selectindex + offset - 1));
      else
        emit CONFIRM(QString("Copy index ") + QString::number(selectindex));
    }

    for (int i = 0; i < copyoffset; i++) {
      copy[i][0] = P[currpage].colours[selectindex + i].bytes[0];
      copy[i][1] = P[currpage].colours[selectindex + i].bytes[1];
      copy[i][2] = P[currpage].colours[selectindex + i].bytes[2];
    }

    _paste->setEnabled(true);
    _insert->setEnabled(true);
  }
}

void GLColourMap::PASTE() {
  int CO = copyoffset;

  if (selectable) {
    if ((selectindex + CO) > 255)
      CO = 255 - selectindex + 1;
    if ((CO == 1) && (offset > 1)) {
      for (int i = 0; i < offset; i++) {
        P[currpage].modified = true;

        P[currpage].colours[selectindex + i].bytes[0] = copy[0][0];
        P[currpage].colours[selectindex + i].bytes[1] = copy[0][1];
        P[currpage].colours[selectindex + i].bytes[2] = copy[0][2];
        P[currpage].colours[selectindex + i].r =
            (GLfloat)P[currpage].colours[selectindex + i].bytes[0] /
            (GLfloat)255;
        P[currpage].colours[selectindex + i].g =
            (GLfloat)P[currpage].colours[selectindex + i].bytes[1] /
            (GLfloat)255;
        P[currpage].colours[selectindex + i].b =
            (GLfloat)P[currpage].colours[selectindex + i].bytes[2] /
            (GLfloat)255;
      }
    } else {
      for (int i = 0; i < CO; i++) {
        P[currpage].modified = true;

        P[currpage].colours[selectindex + i].bytes[0] = copy[i][0];
        P[currpage].colours[selectindex + i].bytes[1] = copy[i][1];
        P[currpage].colours[selectindex + i].bytes[2] = copy[i][2];
        P[currpage].colours[selectindex + i].r =
            (GLfloat)P[currpage].colours[selectindex + i].bytes[0] /
            (GLfloat)255;
        P[currpage].colours[selectindex + i].g =
            (GLfloat)P[currpage].colours[selectindex + i].bytes[1] /
            (GLfloat)255;
        P[currpage].colours[selectindex + i].b =
            (GLfloat)P[currpage].colours[selectindex + i].bytes[2] /
            (GLfloat)255;
      }
    }
    if (selectall)
      emit CONFIRM(QString("Paste page: ") + QString::number(currpage + 1));
    else {
      if (CO == 1)
        emit CONFIRM("Paste index " + QString::number(selectindex));
      else
        emit CONFIRM("Paste indices " + QString::number(selectindex) + " to " +
                     QString::number(selectindex + CO - 1));
    }
    updateColours();
    update();
  }
}

void GLColourMap::SELECTINDEX(int index) {
  if (selectable) {

    GLfloat col[3];
    col[0] = P[currpage].colours[selectindex].r;
    col[1] = P[currpage].colours[selectindex].g;
    col[2] = P[currpage].colours[selectindex].b;
    picker->initColour(col);
    picker->setWindowTitle("Colour Pick : " + parentWidget()->windowTitle() +
                           " : index " + QString::number(selectindex));

    selectindex = index;
    offset = 1;
    selectall = false;
    SLIDERS_ON(true);
    updateColours();
  } else
    emit CONFIRM("Selection disabled for tuning.");
}

void GLColourMap::SELECT() {
  if (selectable) {
    selectall = true;
    selectindex = 0;
    offset = 256;
    update();
    emit SLIDERS_ON(false);
  } else
    emit CONFIRM("Selection disabled for tuning.");
}

void GLColourMap::TUNE() {
  oldBytes();
  tuneMode = 'N';

  emit SLIDERS_ON(false);
  emit SELECT_ON(false);
  selectable = false;

  tuner->r()->setValue(0);
  tuner->g()->setValue(0);
  tuner->b()->setValue(0);
  tuner->br()->setValue(0);

  tuner->setWindowTitle("Fine Tune : " + *P[currpage].name);
  tuner->resize(100, 100);
  tuner->setFixedWidth(tuner->width());
  tuner->setFixedHeight(tuner->height());
  tuner->show();
}

void GLColourMap::PICK() {
  if (!selectall) {
    GLfloat col[3];
    col[0] = P[currpage].colours[selectindex].r;
    col[1] = P[currpage].colours[selectindex].g;
    col[2] = P[currpage].colours[selectindex].b;
    picker->initColour(col);
    picker->setWindowTitle("Colour Pick : " + parentWidget()->windowTitle() +
                           " : index " + QString::number(selectindex));
    emit MAKEPICK();
  } else
    emit CONFIRM("Select a single index first.");
}

void GLColourMap::SHOWPICK(GLfloat *col) {
  P[currpage].colours[selectindex].r = col[0];
  P[currpage].colours[selectindex].g = col[1];
  P[currpage].colours[selectindex].b = col[2];
  P[currpage].colours[selectindex].bytes[0] =
      (GLubyte)(P[currpage].colours[selectindex].r * 255);
  P[currpage].colours[selectindex].bytes[1] =
      (GLubyte)(P[currpage].colours[selectindex].g * 255);
  P[currpage].colours[selectindex].bytes[2] =
      (GLubyte)(P[currpage].colours[selectindex].b * 255);
  P[currpage].modified = true;
  update();
  updateColours();
  emit MODIFIED(true);
}

void GLColourMap::DONEPICK(GLfloat *col) {
  if (col) {
    SHOWPICK(col);
    P[currpage].modified = true;
    update();
    emit MODIFIED(true);
  }
}

void GLColourMap::finetuneRED(int r) {
  int start, end;
  if (selectall) {
    start = 0;
    end = numcolours;
  } else {
    start = selectindex;
    end = selectindex + offset;
  }
  for (int i = start; i < end; i++) {
    GLfloat newred = (GLfloat)P[currpage].colours[i].oldbytes[0] +
                     ((GLfloat)P[currpage].colours[i].oldbytes[0] *
                      ((GLfloat)r / (GLfloat)1000));

    if (newred >= 255)
      P[currpage].colours[i].bytes[0] = 255;
    else if (newred <= 0)
      P[currpage].colours[i].bytes[0] = 0;
    else
      P[currpage].colours[i].bytes[0] = (GLubyte)newred;

    P[currpage].colours[i].r = P[currpage].colours[i].bytes[0] / (GLfloat)255;
    P[currpage].modified = true;
  }
  emit MODIFIED(true);
  updateColours();
  update();
}

void GLColourMap::finetuneGREEN(int g) {
  int start, end;
  if (selectall) {
    start = 0;
    end = numcolours;
  } else {
    start = selectindex;
    end = selectindex + offset;
  }
  for (int i = start; i < end; i++) {
    GLfloat newgreen = (GLfloat)P[currpage].colours[i].oldbytes[1] +
                       ((GLfloat)P[currpage].colours[i].oldbytes[1] *
                        ((GLfloat)g / (GLfloat)1000));

    if (newgreen >= 255)
      P[currpage].colours[i].bytes[1] = 255;
    else if (newgreen <= 0)
      P[currpage].colours[i].bytes[1] = 0;
    else
      P[currpage].colours[i].bytes[1] = (GLubyte)newgreen;

    P[currpage].colours[i].g = P[currpage].colours[i].bytes[1] / (GLfloat)255;
    P[currpage].modified = true;
  }
  emit MODIFIED(true);
  updateColours();
  update();
}

void GLColourMap::finetuneBLUE(int b) {
  int start, end;
  if (selectall) {
    start = 0;
    end = numcolours;
  } else {
    start = selectindex;
    end = selectindex + offset;
  }
  for (int i = start; i < end; i++) {
    GLfloat newblue = (GLfloat)P[currpage].colours[i].oldbytes[2] +
                      ((GLfloat)P[currpage].colours[i].oldbytes[2] *
                       ((GLfloat)b / (GLfloat)1000));

    if (newblue >= 255)
      P[currpage].colours[i].bytes[2] = 255;
    else if (newblue <= 0)
      P[currpage].colours[i].bytes[2] = 0;
    else
      P[currpage].colours[i].bytes[2] = (GLubyte)newblue;

    P[currpage].colours[i].b = P[currpage].colours[i].bytes[2] / (GLfloat)255;
    P[currpage].modified = true;
  }
  emit MODIFIED(true);
  updateColours();
  update();
}

void GLColourMap::BRIGHT(int br) {
  finetuneRED(br);
  finetuneGREEN(br);
  finetuneBLUE(br);
}

void GLColourMap::modeR() {
  if (tuneMode != 'R') {
    oldBytes();
    tuner->r()->setValue(0);
    tuner->g()->setValue(0);
    tuner->b()->setValue(0);
    tuner->br()->setValue(0);
    tuneMode = 'R';
  }
}

void GLColourMap::modeG() {
  if (tuneMode != 'G') {
    oldBytes();
    tuner->r()->setValue(0);
    tuner->g()->setValue(0);
    tuner->b()->setValue(0);
    tuner->br()->setValue(0);
    tuneMode = 'G';
  }
}

void GLColourMap::modeB() {
  if (tuneMode != 'B') {
    oldBytes();
    tuner->r()->setValue(0);
    tuner->g()->setValue(0);
    tuner->b()->setValue(0);
    tuner->br()->setValue(0);
    tuneMode = 'B';
  }
}

void GLColourMap::modeD() {
  if (tuneMode != 'D') {
    oldBytes();
    tuner->r()->setValue(0);
    tuner->g()->setValue(0);
    tuner->b()->setValue(0);
    tuner->br()->setValue(0);
    tuneMode = 'D';
  }
}

void GLColourMap::SELECTABLE() {
  selectable = true;
  if (!selectall)
    SLIDERS_ON(true);
  emit SELECT_ON(true);
}

// ==================== Class: ColourTuner
// -------------------- Construction/Destruction
ColourTuner::ColourTuner(QWidget *parent, const char *) : QDialog(parent) {

  QBoxLayout *topper = new QVBoxLayout(this);
  QGridLayout *top = new QGridLayout();
  top->setVerticalSpacing(25);
  topper->addLayout(top);
  QLabel *rl = new QLabel("%Red", this);
  rl->setFixedHeight(20);
  rl->setAlignment(AlignCenter);
  rl->setPalette(QPalette(QColor(255, 125, 125), QColor(255, 125, 125)));
  top->addWidget(rl, 0, 0);
  red = new QDial(parent);
  red->setMaximum(250);
  red->setMinimum(-250);
  red->setPageStep(1);
  red->setFixedSize(QSize(30, 30));
  red->setObjectName("red");
  red->setNotchTarget(100);
  red->setPalette(QPalette(QColor(255, 125, 125), QColor(255, 125, 125)));
  red->setWrapping(false);
  top->addWidget(red, 1, 0, Qt::AlignCenter);
  rn = new QLCDNumber(this);
  rn->setFixedHeight(20);
  rn->setAccessibleName("rn");
  rn->setPalette(QPalette(QColor(255, 10, 10), QColor(0, 0, 0)));
  top->addWidget(rn, 2, 0);
  QLabel *gl = new QLabel("%Green", this);
  gl->setFixedHeight(20);
  gl->setAlignment(AlignCenter);
  gl->setPalette(QPalette(QColor(125, 255, 125), QColor(125, 255, 125)));
  green = new QDial(this);
  green->setFixedSize(QSize(30, 30));
  green->setMaximum(250);
  green->setMinimum(-250);
  green->setPageStep(1);
  green->setObjectName("green");

  green->setPalette(QPalette(QColor(125, 255, 125), QColor(125, 255, 125)));
  green->setWrapping(false);
  gn = new QLCDNumber(this);
  gn->setFixedHeight(20);
  gn->setPalette(QPalette(QColor(10, 255, 10), QColor(0, 0, 0)));
  top->addWidget(gl, 0, 1);
  top->addWidget(green, 1, 1, Qt::AlignCenter);
  top->addWidget(gn, 2, 1);

  QLabel *bl = new QLabel("%Blue", this);
  bl->setFixedHeight(20);
  bl->setAlignment(AlignCenter);
  bl->setPalette(QPalette(QColor(125, 125, 255), QColor(125, 125, 255)));
  blue = new QDial(this);
  blue->setFixedSize(QSize(30, 30));

  blue->setMaximum(250);
  blue->setMinimum(-250);
  blue->setPageStep(1);
  blue->setObjectName("blue");

  blue->setPalette(QPalette(QColor(125, 125, 255), QColor(125, 125, 255)));
  blue->setWrapping(false);
  bn = new QLCDNumber(this);
  bn->setFixedHeight(20);
  bn->setPalette(QPalette(QColor(10, 10, 255), QColor(0, 0, 0)));
  top->addWidget(bl, 0, 2);
  top->addWidget(blue, 1, 2, Qt::AlignCenter);
  top->addWidget(bn, 2, 2);

  QLabel *brl = new QLabel("%Bright", this);
  brl->setFixedHeight(20);
  brl->setAlignment(AlignCenter);
  brl->setPalette(QPalette(QColor(255, 255, 255), QColor(75, 75, 75)));
  bright = new QDial(this);
  bright->setFixedSize(QSize(30, 30));
  bright->setMaximum(250);
  bright->setMinimum(-250);
  bright->setPageStep(1);
  bright->setObjectName("blue");

  bright->setPalette(QPalette(QColor(255, 255, 255), QColor(75, 75, 75)));
  bright->setWrapping(false);
  brn = new QLCDNumber(this);
  brn->setFixedHeight(20);
  brn->setPalette(QPalette(QColor(255, 255, 255), QColor(0, 0, 0)));
  top->addWidget(brl, 0, 3);
  top->addWidget(bright, 1, 3, Qt::AlignCenter);
  top->addWidget(brn, 2, 3);

  top->activate();

  QBoxLayout *bottom = new QVBoxLayout();
  topper->addLayout(bottom);
  QPushButton *closebutton = new QPushButton("Close", this);
  bottom->addWidget(closebutton);

  connect(closebutton, SIGNAL(clicked()), this, SLOT(close()));
}

void ColourTuner::DISRED(int r) { rn->display(r / 10); }
void ColourTuner::DISGRE(int g) { gn->display(g / 10); }
void ColourTuner::DISBLU(int b) { bn->display(b / 10); }
void ColourTuner::DISBRI(int br) { brn->display(br / 10); }

void ColourTuner::closeEvent(QCloseEvent *ce) {
  emit SELECTABLE();
  ce->accept();
}

// EOF: glcolourmap.cc
