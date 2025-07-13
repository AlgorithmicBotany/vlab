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




#include <QApplication>
#include <QMenuBar>
#include <QGroupBox>
#include <QRandomGenerator>
#include <QScrollArea>
#include <QDir>
#include <QDesktopServices>
#include <QDockWidget>
#include <QPainter>
#include <QPrinter>
#include <QGridLayout>
#include <QTextBrowser>
#include <QTextStream>
#include <iostream>
#include "about.h"
#include "resources.h"
#include "lpfg.h"
#include "directorywatcher.h"
#include "SaveAs.h"
#include "animparam.h"

#include "glwidget.h"
using namespace Qt;

namespace {
// timer for idle function
QTimer *idleTimer;
} // namespace

View::View(const std::string &, const Rect &r, LPFG *pLpfg, int id)
    : _idTimer(0), new_model_pending(false), directoryWatcher(NULL)

{
  const WindowBorderParams &wbp = drawparams.GetWindowBorderParams();
  _numberingImageName = 0;

  if (comlineparam.NoBorder())
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
  QPalette pal = this->palette();
#ifndef __APPLE__
  // On Linux-based systems with the menu bar shown inside the lpfg window,
  // we need to be careful with setting the window's background color:
  // In single-view mode, we don't want to change the color because the
  // menu background color will change (e.g., black (default) hides the menu)
  // In multi-view mode, we want to set the border color but, by setting
  // the background, the menu color is affected. There must be a better way!
  if (drawparams.IsMultiView())
#endif
  // to define the color of borders
  pal.setColor(QPalette::Window, QColor(wbp.red(), wbp.green(), wbp.blue()));
  this->setAutoFillBackground(true);
  this->setPalette(pal);
  _alphaChannel = false;
  this->raise();

  QTimer::singleShot(0, this, SLOT(raise()));
  _proportion = 2;
  _resizeTimer.setSingleShot(true);
  connect(&_resizeTimer, SIGNAL(timeout()), SLOT(resizeDone()));
  setMouseTracking(true);

  setDontPaint(false);

  _pLpfg = pLpfg;
  _id = id;
  _fileNameid = _id;
  _AxisOn = false;
  _flist = 0;
  //_pQ = 0; // not used and never created!
  clear = false;
  _glWidgetClicked = -1;

  if (animparam.ClearBetweenFrames())
    _openGlBehavior = QOpenGLWidget::NoPartialUpdate; // should enter into
                                                      // glWidget constructor
  else
    _openGlBehavior = QOpenGLWidget::PartialUpdate;

  _pathToSave = QString(QDir::currentPath()).toStdString();
  QFileInfo fi(comlineparam.Lsystemfile());
  _imageBaseName = fi.baseName().toStdString();
  _pixFormat = PNG;
  _outputFormat = 0;
  SetTitle();
  move(r.left, r.top);
  resize(r.right, r.bottom);

  InitializeDocks();  

  if (comlineparam.OutputfileSpecified())
    this->showMinimized();
  else
    this->show();

  // create Menu
  setUnifiedTitleAndToolBarOnMac(true);
  createMenu();
  
}

void View::InitializeDocks(){
  // in multiview is a gridlayout 100 x 100 square, we made them identical (taking account borderWidth)
  _centralWidget = new QWidget(this);

   const WindowBorderParams &wbp = drawparams.GetWindowBorderParams();
   m_layout = new QGridLayout();
   m_layout->setSpacing(wbp.width());
   m_layout->setContentsMargins(0, 0, 0, 0);
   
   int borderWidth = wbp.width();

  if (drawparams.IsMultiView()) {
    for (int i = 0; i < drawparams.multiViewSize(); ++i){
    const WindowParams &wp = drawparams.GetView(i);
      Rect ir;
      if (wp.Left() < 0) {
	ir.left = 0;
	Utils::Message("Warning Windows parameters are non positive, set to 0\n");
      }
      if (wp.Left() > 1) {
	ir.left = 1;
	Utils::Message("Warning Windows parameters are greater than 1, set to 1\n");
      }
      ir.left = static_cast<int>(100 * wp.Left());
     
      if (wp.Top() < 0) {
	ir.left = 0;
	Utils::Message("Warning Windows parameters are non positive, set to 0\n");
      }
      if (wp.Top() > 1) {
	ir.left = 1;
	Utils::Message("Warning Windows parameters are greater than 1, set to 1\n");
      } 
      ir.top = static_cast<int>(100 * wp.Top());
      
      if (wp.Width() < 0) {
	ir.left = 0;
	Utils::Message("Warning Windows parameters are non positive, set to 0\n");
      }
      if (wp.Width() > 1) {
	ir.left = 1;
	Utils::Message("Warning Windows parameters are greater than 1, set to 1\n");
      } 
      ir.right = static_cast<int>(100 * wp.Width());
      if (wp.Height() < 0) {
	ir.left = 0;
	Utils::Message("Warning Windows parameters are non positive, set to 0\n");
      }
      if (wp.Height() > 1) {
	ir.left = 1;
	Utils::Message("Warning Windows parameters are greater than 1, set to 1\n");
      } 
      ir.bottom = static_cast<int>(100 * wp.Height());
      QPalette palette;
      Vector3d bgColor = gl.GetBgColor();
      palette.setColor(QPalette::Window, QColor(bgColor[0]*255, bgColor[1]*255, bgColor[2]*255));
      QLabel *label = new QLabel("");
      label->setAutoFillBackground(true);
      label->setPalette(palette);
      m_layout->addWidget(label,ir.top, ir.left, ir.bottom, ir.right);
      layoutWidgets.push_back(label);
    }
  }
     // in multiview is a gridlayout 100 x 100 square, we made them identical (taking account borderWidth)
  _centralWidget->setLayout(m_layout);

  _centralWidget->show();
  setCentralWidget(_centralWidget);


}

void View::Resize(int w, int h) {
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr)
      (*it)->Resize(w,h);
  }
}

void View::createMenu() {

  QMenuBar *menu = menuBar();
  QMenu *about = menu->addMenu("About");
  about->addAction("About", this, SLOT(aboutCB()));

  /* Main Menu Definition */
  _pMenu = new QMenu(this);

  _pMenu->addAction("New model", this, SLOT(NewModel()), CTRL + Key_N);
  _pMenu->addAction("New L-system", this, SLOT(NewLsystem()), CTRL + Key_L);
  _pMenu->addAction("New run", this, SLOT(Rerun()));
  _pMenu->addSeparator();
  _pMenu->addAction("New view", this, SLOT(NewView()));
  _pMenu->addAction("New rendering", this, SLOT(RereadDrawParams()));

  QMenu *pparameter = new QMenu(this);
  pparameter->addAction("Colors", this, SLOT(RereadColors()));
  pparameter->addAction("Animate", this, SLOT(RereadAnimParams()));
  pparameter->addAction("Surfaces", this, SLOT(RereadSurfaces()));
  pparameter->addAction("Contours", this, SLOT(RereadContours()));
  pparameter->addAction("Functions", this, SLOT(RereadFunctions()));
  pparameter->setTitle(QString("New ..."));

  std::string saveMenuName = "Save " + _imageBaseName + getExtension();

  _saveAction = new QAction(saveMenuName.c_str(), this);
  connect(_saveAction, SIGNAL(triggered()), this, SLOT(save()));
  _pMenu->addAction(_saveAction);
  _pMenu->addAction("Save as ...", this, SLOT(saveAs()));
  // we can start recording only in animate mode.

  QMenu *stringPopup = new QMenu(this);
  stringPopup->addAction("Input", this, SLOT(StringLoad()));
  stringPopup->addAction("Output", this, SLOT(StringOutput()));

  _pMenu->addMenu(stringPopup);
  stringPopup->setTitle("String");
  _pMenu->addSeparator();

  _pMenu->addAction("Animate", this, SLOT(Animate()));

  _pMenu->addSeparator();

  // Adding Continuous mode
  QMenu *continuousModeMenu = new QMenu(this);
  continuousModeMenu->setTitle("Refresh mode");
  explicitMode = continuousModeMenu->addAction("Explicit", this,
                                               SLOT(SetExplicitMode(bool)));
  continuousMode = continuousModeMenu->addAction("Triggered/Continuous", this,
                                                 SLOT(SetContinuousMode(bool)));

  continuousMode->setCheckable(true);
  explicitMode->setCheckable(true);
  _pMenu->addMenu(continuousModeMenu);

  ////////////
  _pMenu->addSeparator();

  _pMenu->addAction("Exit", qApp, SLOT(quit()));

  /* Secondary Menu Definition */
  _pSecondaryMenu = new QMenu(this);
  connect(_pSecondaryMenu, SIGNAL(aboutToShow()), this, SLOT(Pause()));
  connect(_pSecondaryMenu, SIGNAL(aboutToHide()), this, SLOT(RestoreState()));
  connect(_pSecondaryMenu, SIGNAL(triggered(QAction *)), this,
          SLOT(ActionTriggered()));

  addAction(
      _pSecondaryMenu->addAction("Step", this, SLOT(Step()), CTRL + Key_F));
  addAction(_pSecondaryMenu->addAction("Run", this, SLOT(Run()), CTRL + Key_R));
  addAction(_pSecondaryMenu->addAction("Forever", this, SLOT(Forever()),
                                       CTRL + Key_V));
  addAction(
      _pSecondaryMenu->addAction("Stop", this, SLOT(Stop()), CTRL + Key_S));
  addAction(
      _pSecondaryMenu->addAction("Rewind", this, SLOT(Rewind()), CTRL + Key_W));
  _clearAction = new QAction("Clear", this);
  connect(_clearAction, SIGNAL(triggered()), this, SLOT(Clear()));

  _pSecondaryMenu->addAction(_clearAction);

  _pSecondaryMenu->addSeparator();

  addAction(_pSecondaryMenu->addAction("New model", this, SLOT(NewModel()), CTRL + Key_N));
  addAction(_pSecondaryMenu->addAction("New L-system", this, SLOT(NewLsystem()), CTRL + Key_L));
  _pSecondaryMenu->addAction("New run", this, SLOT(Rerun()));
  _pSecondaryMenu->addSeparator();
  _pSecondaryMenu->addAction("New view", this, SLOT(NewView()));
  _pSecondaryMenu->addAction("New rendering", this, SLOT(RereadDrawParams()));
  _pSecondaryMenu->addAction("New animate", this, SLOT(RereadAnimParams()));

  _pSecondaryMenu->addSeparator();

  _pSecondaryMenu->addAction(_saveAction);
  _pSecondaryMenu->addAction("Save as ...", this, SLOT(saveAs()));
  _recordingId =
      _pSecondaryMenu->addAction("Start recording", this, SLOT(Recording()));

  _pSecondaryMenu->addMenu(stringPopup);
  stringPopup->setTitle("String");

  _pSecondaryMenu->addSeparator();

  _pSecondaryMenu->addAction("Don't animate", this, SLOT(Animate()));
  _pSecondaryMenu->addSeparator();

  _pSecondaryMenu->addMenu(continuousModeMenu);

  _pSecondaryMenu->addSeparator();

  _pSecondaryMenu->addAction("Exit", qApp, SLOT(quit()));

  if (!comlineparam.StartInAnimMode()) {
    _mPopmenu = true;
    _sPopmenu = false;
  } else {
    _mPopmenu = false;
    _sPopmenu = true;
  }
 

  helpAct = new QAction(tr("&Lpfg Help"), this);
  helpAct->setStatusTip(tr("Show the HTML Lpfg Help"));
  connect(helpAct, SIGNAL(triggered()), this, SLOT(helpCB()));
  pdfHelpAct = new QAction(tr("L&pfg manual"), this);
  pdfHelpAct->setStatusTip(tr("Show the PDF Lpfg Help"));
  connect(pdfHelpAct, SIGNAL(triggered()), this, SLOT(pdfHelpCB()));

  helpMenu = menuBar()->addMenu(tr("&Help"));
  //helpMenu->addAction(helpAct);
    QAction* qHelp = 
      helpMenu->addAction("Quick help", this, SLOT(quickHelp()));
  helpMenu->addAction(pdfHelpAct);
  qHelp->setEnabled(true);
  _isRunningForEver = false;
  _isRunning = false;
  QWidget::setFocusPolicy(Qt::StrongFocus);

  _savingMode = comlineparam.savingMode();
  if ((_savingMode == CONTINUOUS) || (_savingMode == TRIGGERED)) {
    continuousMode->setChecked(true);
    explicitMode->setChecked(false);
    SetContinuousMode(true);
  } else {
    explicitMode->setChecked(true);
    continuousMode->setChecked(false);
    SetExplicitMode(true);
  }

  /******** timer to run idle function ********/

  // create and set up the timer for the IdleFunction
  idleTimer = new QTimer(this);
  connect(idleTimer, SIGNAL(timeout()), SLOT(Idle()));
}

View::~View() {
  //gl.DeleteQuadric(_pQ);
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr){
      delete (*it);
    }
  }
  close();
}

void View::contextMenuEvent(QContextMenuEvent *) {
  if (IsRecording()) {
    _recordingId->setText("Stop Recording");
  } else {
    _recordingId->setText("Start Recording");
  }

  if (_mPopmenu) {
    _pMenu->exec(QCursor::pos());
  } else {
    _pSecondaryMenu->exec(QCursor::pos());
  }
}

void View::addDockWindows(const std::string &, const Rect &r, int id) {
  // add a dockWidget
  int size = m_glWidgets.size();
  if (size < static_cast<size_t>(id +1)){
    m_glWidgets.resize(id +1);
    size = id + 1;
  }

  
  GLWidget *w = new GLWidget(this, true, qRgb(250, 0, 0),id,
                             _openGlBehavior);

  //widget size:
  int wid = width()*r.right/100.;
  int hei = height()*r.bottom/100.;
  
  m_glWidgets[id] = w;
  QObject::connect(w, SIGNAL(mouseClicked(const QPoint &)), this,
                   SLOT(mousePressed(const QPoint &)));
  QObject::connect(w, SIGNAL(mouseMoved(const QPoint &)), this,
                   SLOT(mouseMoved(const QPoint &)));
  QObject::connect(w, SIGNAL(mouseReleased(const QPoint &)), this,
                   SLOT(mouseReleased(const QPoint &)));
  QObject::connect(this, SIGNAL(keyPress(QKeyEvent *)), w,
                   SLOT(keyPressEvent(QKeyEvent *)));
  QObject::connect(this, SIGNAL(keyRelease(QKeyEvent *)), w,
                   SLOT(keyReleaseEvent(QKeyEvent *)));

  w->installEventFilter(this);
  //m_layout->removeWidget(label);
  m_layout->addWidget(w, r.top, r.left, r.bottom, r.right);
 
  if (_pLpfg->GetLEngine()._glview.size() < static_cast<size_t>(id + 1))
    _pLpfg->GetLEngine()._glview.resize(id + 1);

  _pLpfg->GetLEngine()._glview[id] = m_glWidgets[id] ;
  m_glWidgets[id]->resize(wid,hei);

 }

void View::removeDockWindows(int id) {
  // add a et
  m_layout->removeWidget(_pLpfg->GetLEngine()._glview[id]);
}

void View::placeDockWindows(int id, const Rect &r) {
  // add a dockWidget
  m_layout->addWidget(_pLpfg->GetLEngine()._glview[id],r.top, r.left, r.bottom, r.right);
}

void View::resetDockWindowPosition(){
  /*
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr)
      delete(*it);
  }
  */
  for (QVector<QWidget *>::Iterator it = layoutWidgets.begin(); it != layoutWidgets.end(); ++it){
    if ((*it) != nullptr){
      delete(*it);
    }  
  }
  //m_glWidgets.clear();
  layoutWidgets.clear();
  _pLpfg->clearGLWidget();
  delete m_layout;
  delete _centralWidget;
  InitializeDocks();
}

void View::setlayout() {
  _centralWidget->setLayout(m_layout);
  _centralWidget->show();
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr){
      show();
      (*it)->Showing(true);
      update();
    }
  }
}

void View::addNew() {
  GLWidget *w = new GLWidget(this, false,
                             qRgb(QRandomGenerator::global()->bounded(256),
                                  QRandomGenerator::global()->bounded(256),
                                  QRandomGenerator::global()->bounded(256)),
                             m_glWidgets.size(), _openGlBehavior);
  m_glWidgets << w;
}

void View::resizeEvent(QResizeEvent *) {
  _resizeTimer.start(200); // to display the window size temporarely

  QString newWindowTitle = _title;
  QString text = QString::number(width());
  text.append("/");
  text.append(QString::number(height()));
  newWindowTitle.append(QString(" (")).append(text).append(")");
  setWindowTitle(newWindowTitle);
  update();
}

void View::resetWindowTitle() {
  setWindowTitle(_title);
}

void View::resizeDone() {
  QTimer::singleShot(5000, this, &View::resetWindowTitle);
}

void View::update() {
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr){
      (*it)->update();
    }
  }

  _glWidgetClicked = -1;
}

void View::Step() {
  if (_mPopmenu) {
    _mPopmenu = false;
    _sPopmenu = true;
  }

  _isRunning = false;
  _isRunningForEver = false;
  _pLpfg->Step();
  update();
}

void View::Run() {
  if (_mPopmenu) {
    _mPopmenu = false;
    _sPopmenu = true;
  }
  _isRunning = true;
  _isRunningForEver = false;
  _pLpfg->Run();
  update();
}

void View::Forever() {
  if (_mPopmenu)
    return;

  _isRunningForEver = true;
  _isRunning = false;
  _pLpfg->Forever();
  update();
}

void View::Clear() {
  clear = true;
  if (_glWidgetClicked == -1) {
    for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
      if ((*it) != nullptr){
	(*it)->Clear();
      }
    }
  } else
    m_glWidgets[_glWidgetClicked]->Clear();

  update();
}

void View::Show(int id) {
  show();

  m_glWidgets[id]->Showing(true);
  update();

}

void View::Show() {
  show();
  SetTitle();
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr){
      show();
      (*it)->Showing(true);
      update();
    }
  }
}

void View::Pause() { _pLpfg->Stop(); }

void View::Stop() {
  _isRunning = false;
  _isRunningForEver = false;
  _pLpfg->Stop();
}

void View::Rewind() {
  if (_mPopmenu) {
    _mPopmenu = false;
    _sPopmenu = true;
  }
  _isRunning = false;
  _isRunningForEver = false;
  _pLpfg->Rewind();
  update();
}

void View::NewModel() {
  Clear();
  if (!_pLpfg->RereadDrawParams())
    return;

  _isRunning = false;
  _isRunningForEver = false;
   if (drawparams.IsMultiView()) {
     resetDockWindowPosition();
   }
  _pLpfg->NewModel();


  if (animparam.ClearBetweenFrames()) {
    _openGlBehavior = QOpenGLWidget::NoPartialUpdate;

  } else {
    _openGlBehavior = QOpenGLWidget::PartialUpdate;
  }
  
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr)
      (*it)->setUpdateBehavior(_openGlBehavior);
  }
  
  update(); // repaints all the views not just this one
}

void View::NewLsystem() {
  setDontPaint(true);

  _isRunning = false;
  _isRunningForEver = false;
  _pLpfg->RereadColors();
  _pLpfg->RereadContoursNoRerun();
  _pLpfg->RereadSurfacesNorepaint();
  _pLpfg->RereadTexturesNorepaint();
  _pLpfg->RereadFunctionsNoRerun();

  _pLpfg->Stop();
  /*
  if (drawparams.IsMultiView()) {
    resetDockWindowPosition();
  }
  */
  _pLpfg->NewLsystem();

  setDontPaint(false);
  update();
}

void View::Rerun() {
  /*
  if (_mPopmenu) {
    _mPopmenu = false;
    _sPopmenu = true;
  }
  */
  setDontPaint(true);

  // [Pascal] Jan 14th,2021 we have decided not to change the current animate/not animate mode
  // on a new run
  //_isRunning = true;
  //_isRunningForEver = false;

  _pLpfg->RereadColors();
  _pLpfg->RereadContoursNoRerun();
  _pLpfg->RereadSurfacesNorepaint();
  _pLpfg->RereadTexturesNorepaint();
  _pLpfg->RereadFunctionsNoRerun();

  _pLpfg->Rerun();

  if (_pLpfg->IsOutputFrame())
    _pLpfg->OutputFrame();

  setDontPaint(false);
  update();
}

void View::NewView() {
  _pLpfg->RereadColors();
  if (!_pLpfg->RereadDrawParams())
    return;
  //  resetDockWindowPosition();
  if (_glWidgetClicked == -1)
    ResetView();
  else
    m_glWidgets[_glWidgetClicked]->ResetView();
  _glWidgetClicked = -1;

  _pLpfg->Repaint();
  update();
}

void View::RereadDrawParams() {

   _pLpfg->RereadColors();
   if (!_pLpfg->RereadDrawParams())
    return;  
  update();
}

void View::RereadAnimParams() {
  _pLpfg->RereadAnimParams();
  if (animparam.ClearBetweenFrames()) {
    _openGlBehavior = QOpenGLWidget::NoPartialUpdate;

  } else {
    _openGlBehavior = QOpenGLWidget::PartialUpdate;
  }
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr)
      (*it)->setUpdateBehavior(_openGlBehavior);
  }

}

void View::RereadColors() {
  _pLpfg->RereadColors();
  update();
}

void View::RereadSurfaces() { _pLpfg->RereadSurfaces(); }

void View::RereadContours() { _pLpfg->RereadContours(); }

void View::RereadFunctions() { _pLpfg->RereadFunctions(); }

// This is main window
void View::UserContextMenu(int x, int y) {
  Q_UNUSED(x);
  Q_UNUSED(y);
}

bool View::IsRecording() const { return _pLpfg->IsRecording(); }

bool View::isRunning() { return _isRunning; }

bool View::isRunningForever() { return _isRunningForEver; }

void View::save() {

  _saveAction->setText(
      QString(("Save " + _imageBaseName + getExtension()).c_str()));

  std::string filename = _pathToSave + "/" + _imageBaseName + getExtension();
  _filename = filename;

  std::ifstream ifile;
  bool savefile = true;
  ifile.open(filename.c_str());
  if (ifile)
    savefile = OverwriteDialogBox((filename).c_str());
  if (!savefile)
    return;

  setOutputFormat(_outputFormat);

  switch (_outputFormat) {
  case 0: {
    setFormat(_pixFormat);
    SaveFrame(filename.c_str(), getFilenameId());
    break;
  }
  case 1: {
    OutputPostscript();
    break;
  }
  case 2:
    OutputObj();
    break;
  case 3:
    OutputPovray();
    break;
  case 4:
    OutputRayshade();
    break;
  case 5:
    OutputView();
    break;
  default:
    break;
  }
  updateFilename();
}

std::string View::getExtension() {
  std::string ext = "";
  if (_numberingImageName) {
    std::stringstream st1;
    st1.fill('0');
    st1.width(4);
    st1 << getFilenameId();
    ext = st1.str();
  }

  switch (_outputFormat) {
  case 0: {
    switch (_pixFormat) {
    case 0:
      ext += ".bmp";
      break;
    case 1:
      ext += ".gif";
      break;
    case 2:
      ext += ".jpg";
      break;
    case 3:
      ext += ".pbm";
      break;
    case 4:
      ext += ".pdf";
      break;
    case 5:
      ext += ".png";
      break;
    case 6:
      ext += ".tiff";
      break;
    default:
      ext += ".png";
      break;
    }
    break;
  }
  case 1: {
    ext += ".ps";
    break;
  }
  case 2:
    ext += ".obj";
    break;
  case 3:
    ext += ".pov";
    break;
  case 4:
    ext += ".ray";
    break;
  case 5:
    ext += ".view";
    break;
  default:
    ext += ".png";
    break;
  }
  return ext;
}

void View::Animate() {
  if (_mPopmenu) {
    _mPopmenu = false;
    _sPopmenu = true;
  } else {
    _isRunningForEver = false;
    _isRunning = false;
    _mPopmenu = true;
    _sPopmenu = false;
  }
  _pLpfg->Rewind();
  update();

}

void View::StringLoad() {
  _pLpfg->LoadString();
}

void View::StringOutput() {
  _pLpfg->OutputString();
}

void View::Idle() {
  QWidget::setCursor(ArrowCursor);
  if (new_model_pending != 0) {
    int code = new_model_pending;
    new_model_pending = 0;
    ExecuteAction(code);
    if ((code == 1) || (code == 2)) {
      setWindowOpacity(1.0);
      setWindowTitle(_title);
    }
  }
}

void View::SetExplicitMode(bool enable) {
  if (enable && directoryWatcher) {
    delete directoryWatcher;
    directoryWatcher = NULL;
  }
  continuousMode->setChecked(false);
  explicitMode->setChecked(true);
}

void View::SetContinuousMode(bool enable) {
  if (enable && !directoryWatcher) {
    QStringList ignoredPrefixes;
    QStringList ignoredSuffixes;
    directoryWatcher = new DirectoryWatcher(QString::fromStdString(_pathToSave),
                                            ignoredPrefixes << "."
                                                            << "lsys.",
                                            ignoredSuffixes << ".i"
                                                            << ".log"
                                                            << ".ii"
                                                            << ".o"
                                                            << "~",
                                            this);
    connect(directoryWatcher, SIGNAL(fileChanged(QString)), this,
            SLOT(RequestNewModel(QString)));
  }
  continuousMode->setChecked(true);
  explicitMode->setChecked(false);
}

void View::ActionTriggered() {
  _actionTriggered = true;
}

// [Pascal] keep opening the file until the size is stable
int View::waitOpenFile(const char *fname) {
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  FILE *fp = fopen(fname, "r");
  // This is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
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
    while ((size == 0) || (current_size != size)) {
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
    Utils::Message("we can't reload the file : %d/n",counter);
    return 0;
  }

  return 1;
}

int View::ActionToBePerformedInContinuousMode(QString filename) {
  QFileInfo fileInfo(filename);
  QString f(fileInfo.fileName());
  string file = f.toStdString();
  // action to be performed according to the modified file
  // LSystem has been modified
  QString lSystemFile = QString(comlineparam.Lsystemfile());
  if (f.compare(lSystemFile) == 0) {
    if (waitOpenFile(file.c_str()) == 0)
      return 0;
    return 1;
  }
  QString colormapfile = QString(comlineparam.Colormapfile());
  if (f.compare(colormapfile) == 0)
    return 3;
  QString materialfile = QString(comlineparam.Materialfile());
  if (f.compare(materialfile) == 0)
    return 3;
  QString functionfile = QString(comlineparam.FunctionsFile());
  if (f.compare(functionfile) == 0)
    return 4;
  QString tfunctionfile = QString(comlineparam.TFunctionsFile());
  if (f.compare(tfunctionfile) == 0)
    return 4;
  QString vfunctionfile = QString(comlineparam.VFunctionsFile());
  if (f.compare(vfunctionfile) == 0)
    return 4;

  std::vector<std::string> listFunctions =
      comlineparam.IndividualFunctionsFile();
  for (size_t i = 0; i < listFunctions.size(); ++i) {
    std::string functionfile = listFunctions[i];
    if (f.toStdString().compare(functionfile) == 0)
      return 4;
  }
  QString contourfile = QString(comlineparam.ContoursFile());
  if (f.compare(contourfile) == 0)
    return 5;
  std::vector<std::string> listContours = comlineparam.IndividualContoursFile();
  for (size_t i = 0; i < listContours.size(); ++i) {
    std::string contourfile = listContours[i];

    if (f.toStdString().compare(contourfile) == 0)
      return 5;
  }
  QString animFile = QString(comlineparam.AnimparamFile());
  if (f.compare(animFile) == 0)
    return 6;

  QString viewFile = QString(comlineparam.DrawparamFile());
  if (f.compare(viewFile) == 0)
    return 7;

  std::vector<std::string> listTextures = comlineparam.TextureFiles();
  for (size_t i = 0; i < listTextures.size(); ++i) {
    std::string texturefile = listTextures[i];
    if (f.toStdString().compare(texturefile) == 0)
      return 8;
  }

  QString fileExtension = QFileInfo(f).suffix();
  if (fileExtension.compare("s") == 0)
    return 8;

  return 0;
}

void View::ExecuteAction(int code) {
  QString newWindowTitle = _title;

  QString text = QString("Executing: ");
  switch (code) {
  case 1:
    text.append(QString("New LSystem"));
    newWindowTitle.append(QString(" (")).append(text).append(")");
    setWindowTitle(newWindowTitle);
    setWindowOpacity(0.75);
    NewLsystem();
    setWindowTitle(_title);
    setWindowOpacity(1.0);
    break;
  case 2:
    text.append(QString("New Run"));
    newWindowTitle.append(QString(" (")).append(text).append(")");
    setWindowTitle(newWindowTitle);
    setWindowOpacity(0.75);
    Rerun();
    setWindowTitle(_title);
    setWindowOpacity(1.0);
    break;
  case 3:
    RereadColors();
    break;
  case 4:
    RereadFunctions();
    break;
  case 5:
    RereadContours();
    break;
  case 6:
    RereadAnimParams();
    break;
  case 7:
    NewView();
    break;
  case 8:
    RereadDrawParams();
    break;
  default:
    break;
  }

  return;
}

void View::RequestNewModel(QString f) {
  int code = this->ActionToBePerformedInContinuousMode(f);
  if (!idleTimer->isActive()) {
    idleTimer->setSingleShot(true);
    idleTimer->start(0);
    ExecuteAction(code);

  } else {
    new_model_pending = code;
  }
}

void View::RestoreState() {
  if (!_actionTriggered) {
    _actionTriggered = false;
  }
  if (_isRunning)
    _pLpfg->Run();
  else if (_isRunningForEver)
    _pLpfg->Forever();
}

void View::aboutCB() {
  vlab::about(this,"Lpfg");
  return;
  char msg[] = "<p align=\"center\">\n"
               "<b><font size=\"+4\"><font "
               "color=\"#0000f5\">LPFG</font></font></b><br>\n"
               "<br>\n"
               "http://algorithmicbotany.org/virtual_laboratory/<br>\n"
               "<br>\n"
               "Developed at the University of Calgary<br>\n"
               "Department of Computer Science<br>\n"
               "<br>\n"
               "<font size=\"-1\">Copyright (C) 1990-2018 P. Prusinkiewicz,\n"
               "University of Calgary</font>\n"
               "</p>\n";
  QMessageBox box;

  box.setText(msg);
  box.setStandardButtons(QMessageBox::Ok);
  box.setDefaultButton(QMessageBox::Ok);
  box.setIcon(QMessageBox::Information);
  box.exec();
}

void View::helpCB() const {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(
      QUrl::fromLocalFile(helpDir.filePath("LPFGManual.htm")));
}

void View::quickHelp(){
  QDir helpDir(getHelpDirectory());
#ifndef __APPLE__
  helpDir.cd("Quick_Help");
#endif
  QString path = helpDir.filePath("LPFGQuickHelp.html");
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
  msgBox->setWindowTitle("Lpfg: Quick Help");
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

void View::pdfHelpCB() const {
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(QUrl::fromLocalFile(
      helpDir.filePath("LPFGManual.pdf")));
}

// Interaction
void View::mousePressed(const QPoint &) {
}
void View::keyPressEvent(QKeyEvent *event) { emit keyPress(event); }
void View::keyReleaseEvent(QKeyEvent *event) { emit keyRelease(event); }

void View::mouseMoved(const QPoint &) {
}

void View::mouseReleased(const QPoint &) {
}

void View::_SaveFrame(int) const {
}

void View::SaveFrame(const char *fmt, int) {
  
  if (_glWidgetClicked == -1)
    _glWidgetClicked = 0;

  // grab the frame buffer(s)
  const int retinaScale = devicePixelRatio();
  int imageNb = m_glWidgets.size();
  std::vector<QImage> images(imageNb);
  for (int i = 0; i < imageNb; ++i) {
    images[i] = m_glWidgets[i]->grabFrameBuffer(_alphaChannel);
  }

  QImage *image = nullptr; // image to be saved depends on view mode

  // if single view,
  if (imageNb == 1) {
    // there is not much to do
    image = &images[0];
  } // else if multi view,
  else if (imageNb > 1) {
    // use QT painter to combine buffers 
    image = new QImage(this->width()*retinaScale, this->height()*retinaScale, QImage::Format_ARGB32);
    QPainter painter(image);

    // if transparency is used, ensure alpha value is used from the source image
    if (_alphaChannel)
      painter.setCompositionMode(QPainter::CompositionMode_Source);

    // paint the entire image with the window border color
    const WindowBorderParams &wbp = drawparams.GetWindowBorderParams();
    painter.fillRect(0, 0, image->width(), image->height(),
                     QColor(wbp.red(), wbp.green(), wbp.blue()));

    // next paint the buffers from all views
    for (int i = 0; i < imageNb; ++i) {
      // if required, make background (but not border) of each view transparent
      if (_alphaChannel) {
        painter.fillRect(m_glWidgets[i]->x() * retinaScale, 
                         m_glWidgets[i]->y() * retinaScale, 
                         images[i].width(), images[i].height(),
                         QColor(0,0,0,0));
      }
      // paint each frame buffer
      painter.drawImage(m_glWidgets[i]->x() * retinaScale,
                        m_glWidgets[i]->y() * retinaScale,
                        images[i]);
    }
  }

  // not sure if this is "the" way to handle Retina displays
  // but here we scale the image's DPI so the image appears
  // in the same size on the screen as the lpfg window
  if (image && retinaScale > 1) {
    image->setDotsPerMeterX(image->dotsPerMeterX() * retinaScale);
    image->setDotsPerMeterY(image->dotsPerMeterY() * retinaScale);
    // in addition, we should set the color space of the image
    // to match the display device (requires Qt >= 5.14)
    // How do you detect a wide-gamut screen? To set QColorSpace::DisplayP3?
  }
  
  // save the image to a file
  std::string fnm = fmt;
  QString filename = QString::fromStdString(fnm);
  if (_outputFormat == 0) {

    switch (_pixFormat) {
    case BMP:
      image->save(filename, "BMP");
      break;
    case JPG:
      image->save(filename, "JPG");
      break;
    case PNG:
      image->save(filename, "PNG");
      break;
    case TIFF:
      image->save(filename, "TIFF");
      break;
    case PBM:
      image->save(filename, "PBM");
      break;
    case GIF:
      image->save(filename, "GIF");
      break;
    case PDF: {
      // TODO: check if this works on Retina display
      QPrinter printer(QPrinter::ScreenResolution);
      printer.setOutputFormat(QPrinter::PdfFormat);
      printer.setColorMode(QPrinter::Color);
      printer.setOutputFileName(filename);
      printer.setPaperSize(QSizeF(image->size()), QPrinter::DevicePixel);
      printer.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
      printer.setFullPage(true);
      printer.setOrientation(QPrinter::Portrait);

      QPainter painter(&printer);
      painter.drawImage(QPoint(0, 0), *image);
      painter.end();
      break;
    }
    default:
      Utils::Message("%s not implemented\n",_pixFormat);
    }
  }
}

/********************************************************************************/

// returns true if the user wants to overwrite the old file
// false otherwise
bool View::OverwriteDialogBox(const char *sfilename) {

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

  QWidget::setCursor(ArrowCursor); // change to arrow cursor */

  return overwrite;
}

void View::saveAs() {

  SaveAs *window = new SaveAs(this, QString::fromStdString(_imageBaseName),
                              QString::fromStdString(_pathToSave),
                              QString(QDir::currentPath()), _numberingImageName,
                              getFilenameId(), _outputFormat, _pixFormat);
  QString savePath;
  window->setAttribute(Qt::WA_DeleteOnClose, false);
  window->setAlphaChannel(_alphaChannel);

  int result = window->exec();
  window->setAttribute(Qt::WA_DeleteOnClose, true);
  window->close();
  if (result) {
    int id = window->getId();
    setFileNameId(id);
    _outputFormat = window->getFormat();
    _pathToSave = window->getPath().toStdString();
    _alphaChannel = window->getAlphaChannel();
    setPathToSave(_pathToSave);
    int imageType = window->getImageType();
    _imageBaseName = window->getImageBaseName().toStdString();
    _numberingImageName = window->getNumbering();
    //setNumberingImage(_numberingImageName);
    setImageName(_imageBaseName);
    if (_outputFormat == 0) {
      switch (imageType) {
      case 0:
        _pixFormat = BMP;
        break;
      case 1:
        _pixFormat = GIF;
        break;
      case 2:
        _pixFormat = JPG;
        break;
      case 3:
        _pixFormat = PBM;
        break;
      case 4:
        _pixFormat = PDF;
        break;
      case 5:
        _pixFormat = PNG;
        break;
      case 6:
        _pixFormat = TIFF;
        break;
      default:
        _pixFormat = PNG;
        break;
      }
    }
    save();
  }
}

void View::updateFilename() {
  if (_numberingImageName) {
    setFileNameId(getFilenameId() + 1);
    _saveAction->setText(
        QString(("Save " + _imageBaseName + getExtension()).c_str()));
    _filename = _pathToSave + "/" + _imageBaseName + getExtension();
  }
}

void View::StartTimer() {
  if (_idTimer == 0)
    _idTimer = startTimer(animparam.Timeout());
}

void View::StopTimer() {
  if (_idTimer != 0) {
    killTimer(_idTimer);
    _idTimer = 0;
  }
}

void View::timerEvent(QTimerEvent *) { _pLpfg->Timer(); }

Rect  View::getRectangleFromView(int id){

  Rect ir;

  if (drawparams.IsValidViewId(id)) {
    const WindowParams &wp = drawparams.GetView(id);

    if (wp.Left() < 0) {
      ir.left = 0;
      Utils::Message("Warning Windows parameters are non positive, set to 0\n");
    } else if (wp.Left() > 1) {
      ir.left = 1;
      Utils::Message("Warning Windows parameters are greater than 1, set to 1\n");
    } else
      ir.left = static_cast<int>(100 * wp.Left());

    if (wp.Top() < 0) {
      ir.left = 0;
      Utils::Message("Warning Windows parameters are non positive, set to 0\n");
    } else if (wp.Top() > 1) {
      ir.left = 1;
      Utils::Message("Warning Windows parameters are greater than 1, set to 1\n");
    } else
      ir.top = static_cast<int>(100 * wp.Top());

    if (wp.Width() < 0) {
      ir.left = 0;
      Utils::Message("Warning Windows parameters are non positive, set to 0\n");
    } else if (wp.Width() > 1) {
      ir.left = 1;
      Utils::Message("Warning Windows parameters are greater than 1, set to 1\n");
    } else
      ir.right = static_cast<int>(100 * wp.Width());

    if (wp.Height() < 0) {
      ir.left = 0;
      Utils::Message("Warning Windows parameters are non positive, set to 0\n");
    } else if (wp.Height() > 1) {
      ir.left = 1;
      Utils::Message("Warning Windows parameters are greater than 1, set to 1\n");
    } else
      ir.bottom = static_cast<int>(100 * wp.Height());
  } else {
    ir.left = 0;
    ir.top = 0;
    ir.right = 1;
    ir.bottom = 1;
  }

  return ir;
}
