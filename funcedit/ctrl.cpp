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



#include <cassert>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include "colors.h"
#include "ctrl.h"
#include "drawflags.h"
#include "dynarray.h"
#include "file.h"
#include "functask.h"
#include "gridview.h"
#include "mainwindow.h"
#include "model.h"
#include "about.h"
#include <QMainWindow>
#include <QMenuBar>
#include <QPainter>
#include <QWheelEvent>
#include <iostream>

#include <QFileDialog>
#include <QCursor>
#include <QMessageBox>
#include <QString>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <unistd.h>

Ctrl::Ctrl(QWidget *parent, int argc, char **argv)
    : QOpenGLWidget(parent), _caption("Funcedit"), _contextmenu(0),
      _AddRemoveTask(this), _DragPointTask(this), _IncMultiplicityTask(this), _default(true)
{
  ReadColors();

  SetCenter(0.5, 0.0, 0.0);
  SetScale(0.5);

  
  QWidget *_topLevel =
      this; // 2012.03.19 PBdR: If not on Apple, topLevel is 'this'
#ifdef __APPLE__
  QWidget *p = dynamic_cast<QWidget *>(parent);
  while (p) {
    _topLevel = p;
    p = dynamic_cast<QWidget *>(_topLevel->parent());
  };
  MainWindow *mw = dynamic_cast<MainWindow *>(_topLevel);
#else
  QWidget *mw = this;
#endif

  _savingMode = OFF;

  // parsing command line
  int xpos = 100;
  int ypos = 100;
  int xsize = 300;
  int ysize = 240;
  QDesktopWidget widget;
  QRect mainScreenSize = widget.availableGeometry(widget.primaryScreen());

  int wscr = mainScreenSize.width();
  int hscr = mainScreenSize.height();

  //std::string filename = "noname.func";
  std::string filename = "";
  while (--argc > 0) {
    if ((*++argv)[0] == '-') {
      if (!strcmp(argv[0], "-wp")) {
        /* window position */
        xpos = atoi(*++argv);
        ypos = atoi(*++argv);
        --argc;
        --argc;
      } else if (!strcmp(argv[0], "-wpr")) {
        float xr, yr;
        xr = atof(*++argv);
        yr = atof(*++argv);
        --argc;
        --argc;

        xpos = xr * wscr / 100;
        ypos = yr * hscr / 100;
      } else if (!strcmp(argv[0], "-wr")) {
        float xr, yr;
        xr = atof(*++argv);
        yr = atof(*++argv);
        --argc;
        --argc;
        xsize = xr * wscr / 100;
        ysize = yr * hscr / 100;
      } else if (!strcmp(argv[0], "-ws")) {
        xsize = atof(*++argv);
        ysize = atof(*++argv);
        --argc;
        --argc;
      } else if ((strcmp(argv[0], "--refreshMode") == 0) ||
                 (strcmp(argv[0], "-rmode") == 0)) {
        const char *opt = *++argv;
        if ((strcmp(opt, "exp") == 0) || (strcmp(opt, "explicit") == 0))
          _savingMode = OFF;
        if ((strcmp(opt, "cont") == 0) || (strcmp(opt, "continuous") == 0))
          _savingMode = CONTINUOUS;
        if ((strcmp(opt, "trig") == 0) || (strcmp(opt, "triggered") == 0))
          _savingMode = TRIGGERED;
        --argc;

      } else {
        std::cout << "Wrong options: " << argv[0]
                  << " - usage funcedit [--refreshMode|-rmode cont|trig|exp] "
                     "[-wp xpos ypos][-wpr xpos ypos] [filename]"
                  << std::endl;
        exit(EXIT_FAILURE);
      }
    } else {
      filename = argv[0];
      --argc;
    }
  }
  //if (filename.compare("noname.func") != 0) {
  if (!filename.empty()) {

    Load(filename.c_str());
    std::string funcmodel_name = _FuncModel.getName();
    if (!funcmodel_name.empty())
      mw->setWindowTitle(_caption + QString(": ") +
                         QString(funcmodel_name.c_str()));
    else
      mw->setWindowTitle(_caption + QString(": ") + QString(filename.c_str()));
    _default = false;
  } else {
    filename = "noname.func";
    mw->setWindowTitle(_caption);
  }

 // initialize the context menu
  _contextmenu = new QMenu();

  QAction *save_act =
      _contextmenu->addAction("Save", this, SLOT(Save()), QKeySequence::Save);
  QAction *saveas_act = _contextmenu->addAction(
      "Save as ...", this, SLOT(SaveAs()), QKeySequence::SaveAs);
  QAction *reload_act = _contextmenu->addAction(
      "Revert to saved", this, SLOT(Reload()), QKeySequence::Refresh);
  _contextmenu->addSeparator();


  QAction *loadBackGrdImage_act = _contextmenu->addAction("Load background image", this, SLOT(LoadBackGrdImage()), QKeySequence("Ctrl+L"));
  mw->addAction(loadBackGrdImage_act);
  _contextmenu->addSeparator();

  
  flip_act = _contextmenu->addAction("Flip view");
  flip_act->setCheckable(true);
  flip_act->setChecked(_FuncModel.isFlipped());
  connect(flip_act, SIGNAL(toggled(bool)), this, SLOT(FlipView(bool)));

  QAction *points_act = _contextmenu->addAction("View points");
  points_act->setCheckable(true);
  points_act->setChecked(true);
  connect(points_act, SIGNAL(toggled(bool)), this, SLOT(ViewPoints(bool)));

  QAction *seg_act = _contextmenu->addAction("View segments");
  seg_act->setCheckable(true);
  seg_act->setChecked(true);
  connect(seg_act, SIGNAL(toggled(bool)), this, SLOT(ViewSegments(bool)));

  QAction *curve_act = _contextmenu->addAction("View curve");
  curve_act->setCheckable(true);
  curve_act->setChecked(true);
  connect(curve_act, SIGNAL(toggled(bool)), this, SLOT(ViewCurve(bool)));

  QAction *limits_act = _contextmenu->addAction("View limits");
  limits_act->setCheckable(true);
  limits_act->setChecked(true);
  connect(limits_act, SIGNAL(toggled(bool)), this, SLOT(ViewLimits(bool)));

  _contextmenu->addSeparator();
#ifndef __APPLE__
  QAction *help_act = _contextmenu->addAction("Quick Help", this, SLOT(Help()));
  QAction *about_act =
      _contextmenu->addAction("About ...", this, SLOT(About()));
#else
  QAction *help_act = new QAction("Quick Help", this);
  connect(help_act, SIGNAL(triggered()), this, SLOT(Help()));
  QAction *about_act = new QAction("About ...", this);
  connect(about_act, SIGNAL(triggered()), this, SLOT(About()));
#endif

  QAction *edit_act =
      _contextmenu->addAction("Edit name ...", this, SLOT(FuncName()));
  _contextmenu->addSeparator();
   QAction *samples_act =
    _contextmenu->addAction("Edit number of samples ...", this, SLOT(SetSamples()));
  _contextmenu->addSeparator();
 QMenu *modeMenu = _contextmenu->addMenu("Refresh mode");

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

  _contextmenu->addSeparator();
  QAction *exit_act = _contextmenu->addAction("Exit", _topLevel, SLOT(close()),
                                              QKeySequence("Ctrl+Q"));

  // 2012.03.19 PBdR: This is only for apple
#ifdef __APPLE__
  if (mw) {
    QMenu *file = mw->file;
    file->addAction(save_act);
    file->addAction(saveas_act);
    file->addAction(reload_act);
    file->addAction(loadBackGrdImage_act);
    file->addSeparator();
    file->addAction(edit_act);
    file->addSeparator();
    file->addAction(samples_act);
    file->addSeparator();
    file->addAction(exit_act);

    QMenu *view = mw->view;
    view->addAction(flip_act);
    view->addAction(points_act);
    view->addAction(seg_act);
    view->addAction(curve_act);
    view->addAction(limits_act);

 
    QMenu *help = mw->help_menu;
    //    help->addAction(help_act);
    help->addAction(about_act);
    pNDlg = new NameDlg(this);
    pNDlg->setModel(&_FuncModel);
    sampleDlg = new SampleDlg(this);
    sampleDlg->setModel(&_FuncModel);

    QObject::connect(pNDlg, SIGNAL(updateName(QString)), mw,
                     SLOT(nameChanged(QString)));
    QObject::connect(sampleDlg, SIGNAL(modified()), this, SLOT(SaveInContinuousMode()));
    QObject::connect(pNDlg, SIGNAL(modified()), this, SLOT(SaveInContinuousMode()));
  }
#endif

  _WindowSize.x = width();
  _WindowSize.y = height();

  strcpy(_fname, filename.c_str());

  _GridDrawWhat = DrawPoints | DrawSegments | DrawCurve | eDrawGrid |
                  eDrawLabels | eDrawAxis;

  // There is a bug somewhere that will cause the app to
  //  crash if it is resized too small.  It is unknown
  //  where exactly what is causing this problem.  Until
  //  the error is solved, just set a minimum size to prevent
  //  the crash.
  setMinimumSize(50, 50);


  // You must enable keyboard focus for a widget if it processes keyboard events
  setFocusPolicy(Qt::StrongFocus);


  // The chosen size, (300x220), is taken from the Motif version, the
  //  location is arbitrary.
  resize(300, 220);

  mw->move(xpos, ypos);
  mw->resize(xsize, ysize);
  mw->show();
}

Ctrl::~Ctrl() { delete _contextmenu; _exit(EXIT_SUCCESS);}

void Ctrl::closeEvent(QCloseEvent *pEv) {
  int ret = QMessageBox::No;

  if (_FuncModel.hasChanged()) {
    ret = QMessageBox::warning(this, "Save At Exit",
                               "Save changes before exit?", QMessageBox::Yes,
                               QMessageBox::No, QMessageBox::Cancel);
  }

  switch (ret) {
  case QMessageBox::Yes:
    Save();
  case QMessageBox::No:
    pEv->accept();
    break;
  default:
    pEv->ignore();
  }
}


void Ctrl::LoadBackGrdImage() {

  QString filename = QFileDialog::getOpenFileName(this,QString("Load Background Image"),".",QString("Image Files (*.png *.jpg *.jpeg *.bmp)"));
  if (filename.isEmpty()) return;

  _LoadBackGrdTexture(filename);

  // get just the file name, exluding the path
  QFileInfo fi(filename);
  QString name = fi.fileName();

  _FuncModel.SetBackgroundImage(name.toLatin1().data()); // image width and height are set in _LoadBackGrdTexture
  update();
}


void Ctrl::FuncName() { pNDlg->show(); }
void Ctrl::SetSamples()  {
  sampleDlg->setSamples(_FuncModel.getSamples());
  sampleDlg->show();
  if (_savingMode == CONTINUOUS) {
    WriteTextFile trg(_fname);
    if (trg.Valid())
      _FuncModel.Save(trg);
    else
      fprintf(stderr, "Error writing to file: %s\n", _fname);
  }
}

void Ctrl::Load(const char *fname) {

  strcpy(_fname, fname);
  ReadTextFile src(fname);
  if (!src.Valid())
    return;
  _FuncModel.Load(src);
  _center = _FuncModel.getMax() * 0.5 + _FuncModel.getMin() * 0.5;
  if (_FuncModel.isFlipped())
    _center.Set(_center.Y(), _center.X(), 0);

  if (_contextmenu)
    flip_act->setChecked(_FuncModel.isFlipped());

  WorldPoint dist = _FuncModel.getMax() - _FuncModel.getMin();
  double max_dist = dist.X();
  if (dist.Y() > max_dist)
    max_dist = dist.Y();
  if (max_dist == 0)
    max_dist = 1.75;

  SetScale(max_dist / 1.75);
}

void Ctrl::Reload() {
  makeCurrent();
  ReadTextFile src(_fname);
  if (src.Valid())
    _FuncModel.Load(src);
  else
    fprintf(stderr, "Error opening file: %s\n", _fname);

  _center = _FuncModel.getMax() * 0.5 + _FuncModel.getMin() * 0.5;
  if (_FuncModel.isFlipped())
    _center.Set(_center.Y(), _center.X(), 0);

  if (_contextmenu)
    flip_act->setChecked(_FuncModel.isFlipped());

  WorldPoint dist = _FuncModel.getMax() - _FuncModel.getMin();
  double max_dist = dist.X();
  if (dist.Y() > max_dist)
    max_dist = dist.Y();

  SetScale(max_dist / 2.0);

  resize(width(), height());
  update();
}

void Ctrl::Save() {
  if (_default) {
    QString filename = QFileDialog::getSaveFileName(
						    this, QString("Save Function"), _fname, QString(""));
    if (filename.isEmpty())
      return;
    else
      strcpy(_fname, filename.toLatin1());
  }

  WriteTextFile trg(_fname);
  if (trg.Valid())
    _FuncModel.Save(trg);
  else
    fprintf(stderr, "Error writing to file: %s\n", _fname);
}

void Ctrl::SaveAs() {
  QString filename = QFileDialog::getSaveFileName(this, QString("Save Function"), _fname, QString("*"));
  if (filename.isEmpty())
    return;

  _default = false;

  strcpy(_fname, filename.toLatin1());
  Save();
}

void Ctrl::SaveInContinuousMode(){
  if (_savingMode == CONTINUOUS) {
    Save();
  }
}

void Ctrl::FlipView(bool checked) {
  makeCurrent();
  _FuncModel.setFlip(checked);
  update();
}

void Ctrl::ViewPoints(bool checked) {
  makeCurrent();
  if (checked) {
    _GridDrawWhat |= DrawPoints;
  } else {
    _GridDrawWhat &= ~DrawPoints;
  }
  update();
}

void Ctrl::ViewSegments(bool checked) {
  makeCurrent();
  if (checked) {
    _GridDrawWhat |= DrawSegments;
  } else {
    _GridDrawWhat &= ~DrawSegments;
  }
  update();
}

void Ctrl::ViewCurve(bool checked) {
  makeCurrent();
  if (checked) {
    _GridDrawWhat |= DrawCurve;
  } else {
    _GridDrawWhat &= ~DrawCurve;
  }
  update();
}

void Ctrl::ViewLimits(bool checked) {
  makeCurrent();
  if (checked) {
    _GridDrawWhat |= eDrawAxis;
  } else {
    _GridDrawWhat &= ~eDrawAxis;
  }
  update();
}

void Ctrl::ContinuousSavingMode() {
  _savingMode = CONTINUOUS;
  _savingContinu_act->setChecked(true);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(false);
}

void Ctrl::TriggeredSavingMode() {
  _savingMode = TRIGGERED;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(true);
  _savingMenu_act->setChecked(false);
}

void Ctrl::ModeOff() {
  _savingMode = OFF;
  _savingContinu_act->setChecked(false);
  _savingTriggered_act->setChecked(false);
  _savingMenu_act->setChecked(true);
}

void Ctrl::ReloadColors() {
  ReadColors();
  makeCurrent();
  GridView::GInit();
  repaint();
}

void Ctrl::initializeGL() {
  const int retinaScale = devicePixelRatio();

  initializeOpenGLFunctions(); // MC - Oct. 2015 - Qt setup for OpenGL
                               // extensions
  makeCurrent();
  GridView::GInit();
  GridView::_SetView();

  // if there is a background image, load it
  if (_FuncModel.backgrdImage()) {
    _LoadBackGrdTexture(QString::fromStdString(_FuncModel.Filename()));
  }
}

void Ctrl::resetView(void) {
  const int retinaScale = devicePixelRatio();
  _WindowSize.x = width()*retinaScale;
  _WindowSize.y = height()*retinaScale;
  GridView::_SetView();
}

void Ctrl::resizeGL(int, int) {
  resetView();
}

void Ctrl::paintGL() {
  const int retinaScale = devicePixelRatio();

  makeCurrent();
  GridView::_SetView();

  glClear(GL_COLOR_BUFFER_BIT);

  if (_FuncModel.backgrdImage()) {
    _DrawBackGrdTexture();
  }
  
  _PaintGrid(retinaScale);
  if (_GridDrawWhat & eDrawAxis) {
    glColor3dv(GetColor(XAxis));
    GLlines gll;
    gll.Vertex(1.0, _GridMin.Y()*retinaScale);
    gll.Vertex(1.0, _GridMax.Y()*retinaScale);
  }
  _FuncModel.Draw(_GridDrawWhat,retinaScale);

  glFlush();
}

void Ctrl::wheelEvent(QWheelEvent *pEv) {
  const int retinaScale = devicePixelRatio();

  makeCurrent();
  if (pEv->orientation() == Qt::Vertical) {
    float d = pEv->delta();
    Zoom(-d*retinaScale);
    update();
  }
}

void Ctrl::mousePressEvent(QMouseEvent *pEv) {
  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
   const int retinaScale = devicePixelRatio();

  if (topWidget != nullptr)
    topWidget->raise();
  if (pEv->button() == Qt::RightButton) {
    _contextmenu->exec(QCursor::pos());
  } else if (pEv->button() == Qt::LeftButton) {
    if (pEv->modifiers() & Qt::ShiftModifier) {
      _pTask = &_TranslateTask;
    } else if (pEv->modifiers() & Qt::AltModifier) {
      _pTask = &_ZoomTask;
    } else {
      _pTask = &_DragPointTask;
    }
  }
  if (pEv->button() == Qt::MidButton) {
    _pTask = &_ZoomTask;
  }

  bool addRemove = false;
  if (pEv->button() == Qt::LeftButton){
    if  (pEv->modifiers() & Qt::ControlModifier){
      addRemove = true;
      _pTask = &_AddRemoveTask;
    }
  }
 
  _pTask->ButtonDown(pEv,retinaScale);

  if (addRemove)
    if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
      WriteTextFile trg(_fname);
      if (trg.Valid())
        _FuncModel.Save(trg);
      else
        fprintf(stderr, "Error writing to file: %s\n", _fname);
    }

  update();
}

void Ctrl::mouseDoubleClickEvent(QMouseEvent *pEv) {
  const int retinaScale = devicePixelRatio();

  if (pEv->button() == Qt::LeftButton) {
    _pTask = &_IncMultiplicityTask;
    _pTask->ButtonDown(pEv,retinaScale);
    update();
  }
}


void Ctrl::mouseReleaseEvent(QMouseEvent *pEv) {
  const int retinaScale = devicePixelRatio();

  _pTask->ButtonUp(pEv,retinaScale);
  _pTask = &_IdleTask;
  if ((_savingMode == TRIGGERED) || (_savingMode == CONTINUOUS)) {
    WriteTextFile trg(_fname);
    if (trg.Valid())
      _FuncModel.Save(trg);
    else
      fprintf(stderr, "Error writing to file: %s\n", _fname);
  }
  update();
}

void Ctrl::mouseMoveEvent(QMouseEvent *pEv) {
  const int retinaScale = devicePixelRatio();

  _pTask->PointerMotion(pEv,retinaScale);
  if (_savingMode == CONTINUOUS) {
    WriteTextFile trg(_fname);
    if (trg.Valid())
      _FuncModel.Save(trg);
    else
      fprintf(stderr, "Error writing to file: %s\n", _fname);
  }
  update();
}

void Ctrl::Help() {
  char msg[] = "Mouse Commands:\n"
               "  Pan - Shift + Left Button\n"
               "  Zoom - Alt + Left Button\n"
               "  Add/Remove Point - Command + Right Button or Alt + Shift + Left Button\n"
               "Keyboard Commands:\n"
               "  Translate Background Image - Arrow keys\n"
               "  Translate all - Command + Arrow keys\n"
               "  (Hold Shift for larger steps)\n";

  QMessageBox *mb =
      new QMessageBox("Help", QString(msg), QMessageBox::Information, 1, 0, 0);
  QPixmap icon(":icon.png");
  mb->setIconPixmap(icon.scaled(icon.width() / 8, icon.height() / 8,
                                Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation));

  mb->setStandardButtons( QMessageBox::Ok );
  mb->setModal(false);
  mb->open(this,SLOT(msgBoxClosed(QAbstractButton*)));

  //delete mb;
}
void Ctrl::msgBoxClosed(QAbstractButton*) {
   //react on button click (usually only needed when there > 1 buttons)
}

void Ctrl::keyPressEvent(QKeyEvent* pEv) {
  float scale = GetScale();
  float modifier = 0.01f;
  if(pEv->modifiers() & Qt::ShiftModifier)
    modifier *= 10.f;

  if (pEv->key() == Qt::Key_Left) {
    _FuncModel.Shift(-modifier*scale,0.f);
    if (pEv->modifiers() & Qt::ControlModifier)
      _FuncModel.TranslateAll(-modifier*scale,0.f);
  }
  else if (pEv->key() == Qt::Key_Right) {
    _FuncModel.Shift(modifier*scale,0.f);
    if (pEv->modifiers() & Qt::ControlModifier)
      _FuncModel.TranslateAll(modifier*scale,0.f);
  }
  else if (pEv->key() == Qt::Key_Up) {
    _FuncModel.Shift(0.f,modifier*scale);
    if (pEv->modifiers() & Qt::ControlModifier)
      _FuncModel.TranslateAll(0.f,modifier*scale);
  }
  else if (pEv->key() == Qt::Key_Down) {
    _FuncModel.Shift(0.f,-modifier*scale);
    if (pEv->modifiers() & Qt::ControlModifier)
      _FuncModel.TranslateAll(0.f,-modifier*scale);
  }
  else {
    // pass on the event to the base class
    QWidget::keyPressEvent(pEv);
  }
  update();
}

void Ctrl::keyReleaseEvent(QKeyEvent* pEv)
{
  QWidget::keyReleaseEvent(pEv);
}


/*
void Ctrl::About() {
  char msg[] = "Funcedit\n"
               "\n"
               "by Colin Smith\n"
               "August 2001\n"
               "\n"
               "Dept. of Computer Science\n"
               "University of Calgary";

  QMessageBox *mb =
      new QMessageBox("About", QString(msg), QMessageBox::Information, 1, 0, 0);
  QPixmap icon(":icon.png");
  mb->setIconPixmap(icon.scaled(icon.width() / 8, icon.height() / 8,
                                Qt::IgnoreAspectRatio,
                                Qt::SmoothTransformation));

  mb->setButtonText(1, "OK");
  mb->exec();

  delete mb;
}
*/


void Ctrl::About(){
  vlab::about(this,"Funcedit");
}


void Ctrl::AddPoint(WorldPoint wp) {
  if (wp.X() < 0.0)
    wp.X(0.0);
  else if (wp.X() > 1.0)
    wp.X(1.0);
  _FuncModel.AddPoint(wp);
}

void Ctrl::DeletePoint(int i) {
  if (_FuncModel.Count() > 4)
    _FuncModel.DeletePoint(i);
}

void Ctrl::MovePoint(int i, WorldPoint wp) { _FuncModel.MovePoint(i, wp); }

int Ctrl::FindClosest(WorldPoint wp) const {
  const int retinaScale = devicePixelRatio();
    
  int found = _FuncModel.FindClosest(wp);
  WorldPoint rel = wp - _FuncModel[found];
  rel.X(fabs(rel.X()));
  rel.Y(fabs(rel.Y()));

  if (rel.X() / (5.0*retinaScale) > _upp)
    found = -1;
  else if (rel.Y() / (5.0*retinaScale) > _upp)
    found = -1;

  return found;
}

void Ctrl::IncPointMultiplicity(int i) {
  _FuncModel.IncPointMultiplicity(i);
}


// Routine to render text
inline GLint Ctrl::project(GLdouble objx, GLdouble objy, GLdouble objz,
                           const GLdouble model[16], const GLdouble proj[16],
                           const GLint viewport[4], GLdouble *winx,
                           GLdouble *winy, GLdouble *winz) {
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

void Ctrl::setupViewport(int width, int height) {
  int side = qMin(width, height);
  glViewport((width - side) / 2, (height - side) / 2, side, side);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
#ifdef QT_OPENGL_ES
  glOrthof(-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#else
  glOrtho(-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#endif
  glMatrixMode(GL_MODELVIEW);
}

inline void Ctrl::transformPoint(GLdouble out[4], const GLdouble m[16],
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

void Ctrl::renderText(double x, double y, double z, const QString &str,
                      const QColor color, const QFont &font) {
  const int retinaScale = devicePixelRatio();

// save GL attributes before calling QPainter
  glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  // [PASCAL] QPainter is clearing the depth buffer for the shadow map
  // Needs to be fixed

  int height = this->height();

  GLdouble model[4][4], proj[4][4];
  GLint view[4];
  glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
  glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
  glGetIntegerv(GL_VIEWPORT, &view[0]);
  GLdouble textPosX = 0, textPosY = 0, textPosZ = 0;

  project(x, y, z, &model[0][0], &proj[0][0], &view[0], &textPosX, &textPosY,
          &textPosZ);

  textPosX/=retinaScale;
  textPosY/=retinaScale;
  textPosY = height - textPosY; // y is inverted

  int fontSize = font.pointSize()*retinaScale;
  QFontMetrics metrics(font);
  int text_width = metrics.width(QString(str)) + 10;

  int text_height = fontSize;
  QPixmap textimg(text_width, text_height + text_height / 3 + 1);
  textimg.fill(Qt::transparent);

  QPainter painter(&textimg);
  //painter.scale(1./retinaScale,1./retinaScale);
  /*
    painter.setRenderHints(QPainter::HighQualityAntialiasing |
                         QPainter::TextAntialiasing |
                         QPainter::NonCosmeticDefaultPen);
  */
  painter.setRenderHints(QPainter::Antialiasing);

  painter.setBrush(color);
  painter.setPen(color);
  painter.setFont(font);
  painter.drawText(5, text_height, str);
  painter.end();
  QPainter p1(this);
  p1.drawPixmap(static_cast<int>(std::round(textPosX)) - 5,
                static_cast<int>(std::round(textPosY)) - text_height ,
                textimg);
  p1.end();

  glPopAttrib();
  glPopClientAttrib();

}

void Ctrl::rtext(double x, double y, char *bf) {

  const GLdouble *colors = GetColor(Labels);
  QColor qcolor((int)(colors[0] * 255), (int)(colors[1] * 255),
                (int)colors[2] * 255);

  renderText(x , y , 0.0, QString(bf), qcolor, QFont());
}

void Ctrl::rtext(double x, double y, const char *bf) {

  const GLdouble *colors = GetColor(Labels);
  QColor qcolor((int)(colors[0] * 255), (int)(colors[1] * 255),
                (int)colors[2] * 255);

  renderText(x, y , 0.0, QString(bf), qcolor, QFont());
}

void Ctrl::_LoadBackGrdTexture(const QString &filename) {
	
  bool success = _backGrdImage.load(filename);
  if (!success) { // If the texture was not found send an error message...
    QMessageBox::critical(this, "Error", QString("Cannot load texture file ").append(filename));
    return;
  }
}

void Ctrl::_DrawBackGrdTexture(void) {

  const float* pos = _FuncModel.backgrdTranslate(); 
  WorldPoint wp = WorldPoint(pos[0],1.f+pos[1],0.f);
  int x,y;
  MapWorldToScreen(wp,x,y);

  // compute width keeping aspect ratio so that height=1
  WorldPoint pt(pos[0]+float(_backGrdImage.width())/float(_backGrdImage.height()),pos[1],0.f);
  int w,h;
  MapWorldToScreen(pt,w,h);

  QPainter painter(this);
  painter.drawPixmap(x,height()-y,w-x,y-h,_backGrdImage);
  painter.end();
}
