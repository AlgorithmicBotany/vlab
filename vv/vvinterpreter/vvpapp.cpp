#include "vvpapp.hpp"

#include <cstdlib>

#include <QAction>
#include <QActionGroup>
#include <QImageReader>
#include <QMessageBox>
#include <QString>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QImage>
#include <QTimer>
#include <QRegExp>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QPicture>
#include <QPixmap>
#include <QDir>
#include <QPainter>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QScreen>

#include <util/materials.hpp>
#include <util/palette.hpp>
#include <util/function.hpp>
#include <util/contour.hpp>
#include <external/gl2ps.h>
#include <util/dir.hpp>
#include <util/forall.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>

#include "sgiFormat.h"
#include "about.h"

QString getHelpDirectory();

#ifdef __APPLE__
void OpenHelpBook();
#endif

VVPApp::VVPApp(QWidget* parent) :
  QMainWindow(parent),
  dll_interface(),
  view(0),
  stopped(false),
  exitAfterStart(false),
  animateOnStart(false),
  glsamples(4),
  rec(NO_REC),
  filemenu(0),
  imagemenu(0),
  animatemenu(0),
  helpmenu(0),
  frame_counter(0)
{
  view = new VVPViewer(this, &dll_interface, glsamples);

  QPixmap icon(":/icons/logo.png");
  setWindowIcon(icon);
  setWindowTitle("VV Interpreter");

  filemenu = new QMenu("&File", this);
  imagemenu = new QMenu("&Image", this);
  animatemenu = new QMenu("&Animation", this);
  helpmenu = new QMenu("&Help", this);

  filemenu->addAction("&Open Model", this, SLOT(open()));
  filemenu->addAction("E&xit", this, SLOT(close()));

  imagemenu->addAction("Save As &Image", this, SLOT(saveAsImage()));
  imagemenu->addAction("Save As &Postscript", this, SLOT(saveAsPostScript()));

  QAction* noRecAction = animatemenu->addAction(
    "&No Recording", this, SLOT(recNone()));
  noRecAction->setCheckable(true);
  noRecAction->setChecked(true);

  QAction* recAfterStepAction = animatemenu->addAction(
    "Record Frame After &Step", this, SLOT(recAfterStep()));
  recAfterStepAction->setCheckable(true);

  QActionGroup* animateGroup = new QActionGroup(this);
  animateGroup->addAction(noRecAction);
  animateGroup->addAction(recAfterStepAction);
  animateGroup->setExclusive(true);

#ifndef __APPLE__
  helpmenu->addAction("&Quick Help", this, SLOT(quickHelp()));
#endif
//  helpmenu->addAction("&PDF Help", this, SLOT(pdfHelp())); // To be restored 
//  when PDF Help will exist
  helpmenu->addAction("&Online Help", this, SLOT(onlineHelp()));
  helpmenu->addAction("&About", this, SLOT(about()));

  menuBar()->addMenu(filemenu);
  menuBar()->addMenu(imagemenu);
  menuBar()->addMenu(animatemenu);
  menuBar()->addMenu(helpmenu);

  connect(view, SIGNAL(start()), this, SLOT(start()));
  connect(view, SIGNAL(step()), this, SLOT(step()));
  connect(view, SIGNAL(end()), this, SLOT(end()));
  connect(view, SIGNAL(run()), this, SLOT(run()));
  connect(view, SIGNAL(animate()), this, SLOT(animate()));
  connect(view, SIGNAL(stop()), this, SLOT(stop()));
  connect(view, SIGNAL(reread(const std::set<std::string>&)), this, SLOT(reread(const std::set<std::string>&)));
  connect(view, SIGNAL(quit()), this, SLOT(close()));

  setCentralWidget(view);

  start();
}

VVPApp::~VVPApp() {}

void VVPApp::debug() {
  dll_interface.useDebug();
}

void VVPApp::nogui() {
  exitAfterStart = true;
}

void VVPApp::stats() {
  dll_interface.useStats();
}

void VVPApp::startanimate() {
  animateOnStart = true;
}

void VVPApp::setGLsize(int w, int h) {
    // is this used anymore?
    // TBD: it would be nice to be able to set the initial size, but then allow the user to resize
    // the window.
    view->setFixedSize(w, h);
}

void VVPApp::setGLSamples(int samples) {
    glsamples = samples;
}

void VVPApp::open() {
#ifdef VVSTATIC
  open( "" );
#else
#ifdef WIN32
  QString dllext = "*.dll";
#else
  QString dllext = "*.so";
#endif
  QString file = QFileDialog::getOpenFileName(this, "Choose a file", ".", dllext);
  if (file.isEmpty()) return;

  open(file.toStdString());
#endif
}

void VVPApp::open(std::string filename) {
  QDir libpath(QDir::cleanPath(QString::fromStdString(filename)));
  libpath.makeAbsolute();
  libpath.cdUp();
#ifdef __APPLE__
  QString iconFname = "./icon";
  if(libpath.exists(iconFname))
  {
    // try to load in the icon from the object directory
    QImage iconPicture = QImage(iconFname);
    // if unsuccessful, silenty ignore it and use the default icon
    if (iconPicture.isNull()) {
      // if QImage doesn't recognize rgb format, resort to vlab's sgi read function
      iconPicture = readSGI(iconFname.toLatin1().data());
      if (iconPicture.isNull()) {
        iconFname = ":/icons/logo.png";
        iconPicture = QImage(iconFname);
      }
    }

    QPixmap icon = QPixmap::fromImage(iconPicture);
    //QPixmap icon(libpath.absoluteFilePath("icon"));

    icon = icon.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap overlay(":/icons/model_overlay.png");
    {
      QPainter paint(&icon);
      paint.drawPixmap(0, icon.height()-overlay.height(), overlay);
    }
    qApp->setWindowIcon(icon);
    setWindowIcon(QIcon());
  }
  else
  {
    qApp->setWindowIcon(QIcon());
  }
#endif
  dll_interface.open(filename);
  start();
}

void VVPApp::keyPressEvent(QKeyEvent* pEv) {
  if (pEv->modifiers() == Qt::ControlModifier) {
    switch (pEv->key()) {
    case Qt::Key_W: pEv->accept(); start();   break;
    case Qt::Key_F: pEv->accept(); step();    break;
    case Qt::Key_E: pEv->accept(); end();     break;
    case Qt::Key_R: pEv->accept(); animate();     break;
    //case Qt::Key_R: pEv->accept(); run();     break;
    //case Qt::Key_A: pEv->accept(); animate(); break;
    case Qt::Key_S: pEv->accept(); stop();    break;
    default:        pEv->ignore();            break;
    }
  }
  else pEv->ignore();
}

void VVPApp::closeEvent(QCloseEvent* pEv) {
  dll_interface.close();
  pEv->accept();
}

void VVPApp::saveAsImage() {
  static QString saveDir(".");
  QString filter("Image Files (*.");
  QStringList img_formats;
  foreach(QByteArray format, QImageReader::supportedImageFormats())
  {
    img_formats << format;
  }
  filter += img_formats.join(" *.");
  filter += ")";

  QString filename = QFileDialog::getSaveFileName(this, "Save as Image", saveDir, filter);

  if (filename.isEmpty()) return;

  QString ext = filename.right(filename.length() - filename.lastIndexOf(".") - 1);

  if(ext.length() == filename.length()) {
    ext = "png";
    filename += "." + ext;
  }
  saveImage(filename.toStdString(), ext.toStdString());

#ifdef WIN32
  saveDir = filename.left(filename.length() - filename.lastIndexOf("\\") + 1);
#else
  saveDir = filename.left(filename.length() - filename.lastIndexOf("/") + 1);
#endif
}

void VVPApp::saveAsPostScript() {
  QString filename = QFileDialog::getSaveFileName(this, "Save as PostScript", ".", "*.ps");
  if (filename.isEmpty()) return;

  savePostScript(filename.toStdString());
}

void VVPApp::recNone() {
  rec = NO_REC;
}

void VVPApp::recAfterStep() {
  rec = AFTER_STEP;
}

void VVPApp::saveImage(std::string filename, std::string ext) {
  if (filename.empty() || ext.empty()) return;

  int w = view->width();
  int h = view->height();

  //GLubyte* pixels = new GLubyte[w * h * 4];
static GLubyte pixels[4000000];
  Q_ASSERT(4000000 > w * h * 4);

  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  QImage image(w, h, QImage::Format_RGB32);
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      QRgb rgb = qRgb(
        pixels[(x + w * y) * 4 + 0],
        pixels[(x + w * y) * 4 + 1],
        pixels[(x + w * y) * 4 + 2]);
      image.setPixel(x, h - y - 1, rgb);
    }
  }

  image.save(filename.c_str(), ext.c_str());
//  delete[] pixels;
}

void VVPApp::savePostScript(std::string /* filename */) {
//  if (filename.empty()) return;

//  FILE* fp = fopen(filename.c_str(), "w");
//  int buffsize = 0, state = GL2PS_OVERFLOW;

//  while (state == GL2PS_OVERFLOW) {
//    buffsize += 1048576; // 1024^2
//    gl2psBeginPage(
//      "vvinterpreter PostScript Output", "vvinterpreter",
//      GL2PS_PS, GL2PS_BSP_SORT,
//      GL2PS_SIMPLE_LINE_OFFSET,
//      GL_RGBA, 0, 0, buffsize, fp, 0
//    );
//    view->update();
//    state = gl2psEndPage();
//  }
}

void VVPApp::start() {
  std::set<std::string> empty_list;
  stopped = false;
  dll_interface.init();
  dll_interface.read_parms(empty_list);
  dll_interface.start();
  frame_counter = 0;
  if (exitAfterStart) std::exit(0);
  dll_interface.setViewer(view);
  view->buildContextMenu();
  view->update();
  if (animateOnStart)
    QTimer::singleShot(dll_interface.proxy().delay_msec, this, SLOT(animate()));
#ifdef FAM_THREAD
  view->reregisterFiles();
#endif
}

void VVPApp::step() {
  dll_interface.step();
  view->update();
  if (rec == AFTER_STEP || dll_interface.proxy().record_frame) rec_frame();
#ifdef FAM_THREAD
  view->reregisterFiles();
#endif
}

void VVPApp::end() {
  dll_interface.end();
  view->update();
#ifdef FAM_THREAD
  view->reregisterFiles();
#endif
}

void VVPApp::run() {
  if (dll_interface.isProxyAvailable()) {
      if (stopped)
          stopped = false;
      else {
          if (dll_interface.proxy().steps) {
            --(dll_interface.proxy().steps);
            dll_interface.step();
            view->update();
            if (rec == AFTER_STEP || dll_interface.proxy().record_frame) rec_frame();
            QTimer::singleShot(dll_interface.proxy().delay_msec, this, SLOT(run()));
          }
      }
  }
}

void VVPApp::animate() {
  if (dll_interface.isProxyAvailable()) {
      if (dll_interface.proxy().no_animate) 
          return;
      if (stopped) 
          stopped = false;
      else {
        dll_interface.step();
        view->update();
        if (rec == AFTER_STEP || dll_interface.proxy().record_frame)
            rec_frame();
        QTimer::singleShot(dll_interface.proxy().delay_msec, this, SLOT(animate()));
      }
  }
}

void VVPApp::stop() {
  stopped = true;
}

void VVPApp::reread(const std::set<std::string>& filenames)
{
  using util::absoluteDir;
  std::set<std::string> absolute_filenames;
  forall( std::string fn, filenames )
    {
    absolute_filenames.insert( util::absoluteDir( fn ) );
    }

  for (std::list< util::Materials* >::iterator i = dll_interface.proxy().materials.begin();
       i != dll_interface.proxy().materials.end();
       i++)
    {
    if( filenames.empty() ||
        ( absolute_filenames.find( absoluteDir( (*i)->getFilename() ) ) != absolute_filenames.end() ) )
      (*i)->reread();
    }

  for (std::list< util::Palette* >::iterator i = dll_interface.proxy().palettes.begin();
       i != dll_interface.proxy().palettes.end();
       i++)
    {
    if( filenames.empty() ||
        ( absolute_filenames.find( absoluteDir( (*i)->getFilename() ) ) != absolute_filenames.end() ) )
      (*i)->reread();
    }

  for (std::list< util::Function* >::iterator i = dll_interface.proxy().functions.begin();
       i != dll_interface.proxy().functions.end();
       i++)
    if( filenames.empty() ||
        ( absolute_filenames.find( absoluteDir( (*i)->getFilename() ) ) != absolute_filenames.end() ) )
      (*i)->reread();

  for (std::list< util::Contour* >::iterator i = dll_interface.proxy().contours.begin();
       i != dll_interface.proxy().contours.end();
       i++)
    if( filenames.empty() ||
        ( absolute_filenames.find( absoluteDir( (*i)->getFilename() ) ) != absolute_filenames.end() ) )
      (*i)->reread();

  dll_interface.read_parms( filenames );
#ifdef FAM_THREAD
  view->reregisterFiles();
#endif
  view->update();
}

void VVPApp::quickHelp() {
  char info[] =
    "Rotate View - Left Mouse\n"
    "Pan View - Shift + Left Mouse\n"
    "Zoom View - Control + Left Mouse\n"
    "\n"
    "Start - Ctrl-w\n"
    "Step - Ctrl-f\n"
    "End - Ctrl-e\n"
    "\n"
    "Run - Ctrl-r\n"
    "Animate - Ctrl-a\n"
    "Stop - Ctrl-s\n";
  QMessageBox::information(this, "Quick Help", info, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

void VVPApp::pdfHelp()
{
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(QUrl::fromLocalFile(helpDir.filePath("refman.pdf")));
}

void VVPApp::onlineHelp() {
#ifdef __APPLE__
  OpenHelpBook();
#else
  QDir helpDir(getHelpDirectory());
  QDesktopServices::openUrl(QUrl::fromLocalFile(helpDir.filePath("index.html")));
#endif
}

void VVPApp::about() {
  vlab::about(this,"VV");
  //char info[] =
  //  "Software written by Colin Smith\nUniversity of Calgary";
  //QMessageBox::information(this, "Quick Help", info, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

void VVPApp::rec_frame() {
  frame_counter++;
#ifdef WIN32
  QString name = QString("c:/scratch/f%1.png").arg(frame_counter, 7);
#else
  QString name = QString("/scratch/f%1.png").arg(frame_counter, 7);
#endif
  name = name.replace(QRegExp(" "), "0");
  saveImage(name.toStdString(), "png");
}

void VVPApp::showEvent(QShowEvent* ev)
{
  QMainWindow::showEvent(ev);
}

