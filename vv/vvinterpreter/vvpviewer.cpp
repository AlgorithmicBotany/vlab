#include <QMenu>
#include <QMouseEvent>
#include <util/point.hpp>
#include <util/glerrorcheck.hpp>

#include "vvpviewer.hpp"
#include "vvpapp.hpp"
#include "dllinterface.hpp"

#include <util/dir.hpp>

#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#ifdef FAM_THREAD
#include <qevent.h>
#include <qdir.h>
#include <util/forall.hpp>
#endif

VVPViewer::VVPViewer(QWidget* parent, DllInterface *dll, int glsamples = 1) : 
  QOpenGLWidget(parent),
  dll_interface(dll),
  min(-1.0, -1.0, -1.0),
  max(1.0, 1.0, 1.0),
  upp(1),
  rotx(),
  roty(),
  panx(),
  pany(),
  scalefactor(0.0)
{
  _format.setRedBufferSize(8);
  _format.setGreenBufferSize(8);
  _format.setBlueBufferSize(8);
  _format.setAlphaBufferSize(8);
  if (glsamples > 0){
    _format.setSamples(glsamples);
  }
  setFormat(_format);

#ifdef FAM_THREAD
  timer.setSingleShot(true);
  connect( &timer, SIGNAL( timeout() ), this, SLOT( send_reread() ) );
  fam_thread = new FamThread( this );
  fam_thread->start();
#endif

  pContextMenu = new QMenu(this);
  buildContextMenu();
}

void VVPViewer::setInterface(DllInterface* pInterface) {
  dll_interface = pInterface;
}

void VVPViewer::initializeGL() {
  
  initializeOpenGLFunctions();

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glEnable(GL_DEPTH_TEST);

  if (dll_interface) dll_interface->render_init();
}

VVPViewer::~VVPViewer()
{
#ifdef FAM_THREAD
  std::cerr << "Windows closing" << std::endl;
  fam_thread->stop();
  int wait_count = 0;
  while( !fam_thread->wait(1000) )
    {
    if( wait_count > 5 )
      break;
    std::cerr << "Waiting FAM thread to stop ... " << wait_count++ << std::endl;
    };
  delete fam_thread;
#endif
}

void VVPViewer::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (dll_interface) {
    if (dll_interface->isRenderAvailable()) {

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glTranslatef(panx, pany, 0.0);
      glRotatef(rotx, 1.0, 0.0, 0.0);
      glRotatef(roty, 0.0, 1.0, 0.0);

      dll_interface->render();
    }
    if (dll_interface->isProxyAvailable()) {
      dll_interface->proxy().rotx = rotx;
      dll_interface->proxy().roty = roty;
    }
    // not sure what the difference between Render and RenderScreen is
    if (dll_interface->isRenderScreenAvailable()) {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();

      GLdouble w = width();
      GLdouble h = height();

      if (w > h) {
        GLdouble r = w / h;
        gluOrtho2D(-r, r, -1.0, 1.0);
      }
      else {
        GLdouble r = h / w;
        gluOrtho2D(-1.0, 1.0, -r, r);
      }

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      glViewport(0,0,w,h);
      glTranslatef(panx, pany, 0.0);
      glRotatef(rotx, 1.0, 0.0, 0.0);
      glRotatef(roty, 0.0, 1.0, 0.0);

      dll_interface->render_screen();
      glPopMatrix();

      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
    }
  }
  glFlush();
}

void VVPViewer::resizeGL(int w, int h) {
  if (dll_interface->isProxyAvailable()) {
    float x = dll_interface->proxy().view_maxx;
    float y = dll_interface->proxy().view_maxy;
    float z = dll_interface->proxy().view_maxz;
    util::Point<float> pmax(x, y, z);

    x = dll_interface->proxy().view_minx;
    y = dll_interface->proxy().view_miny;
    z = dll_interface->proxy().view_minz;
    util::Point<float> pmin(x, y, z);

    min = pmin;
    max = pmax;
  }
  applyProjection(w,h);

  glViewport(0, 0, w, h);
  //const int retinaScale = devicePixelRatio();
  //OrthoViewer::resizeGL(w * retinaScale, h * retinaScale);
}

void VVPViewer::applyProjection(int w, int h) {
  makeCurrent();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  const GLfloat ratio = GLfloat(w) / GLfloat(h);
  const GLfloat scale = pow(1.003f, scalefactor);
  const GLfloat size  = min.distance(max) * scale;

  if (ratio > 1.0) {
    glOrtho(
      -size * ratio, size * ratio,
      -size, size,
      -size / scale, size / scale
    );
    upp = 2.0 * size / GLfloat(h);
  }
  else {
    glOrtho(
      -size, size,
      -size / ratio, size / ratio,
      -size / scale, size / scale
    );
    upp = 2.0 * size / GLfloat(w);
  }
}

void VVPViewer::send_quit(){
  emit quit();
}

void VVPViewer::send_start() {
  emit start();
}

void VVPViewer::send_step() {
  emit step();
}

void VVPViewer::send_end() {
  emit end();
}

void VVPViewer::send_run() {
  emit run();
}

void VVPViewer::send_animate() {
  emit animate();
}

void VVPViewer::send_stop() {
  emit stop();
}

void VVPViewer::send_reread() {
#ifdef FAM_THREAD
  emit reread(modified_files);
  modified_files.clear();
#else
  std::set<std::string> empty;
  emit reread(empty);
#endif
}

void VVPViewer::buildContextMenu() {
   
  pContextMenu->clear();

  if (dll_interface->isStepAvailable()) {
    pContextMenu->addAction("&Step", this, SLOT(send_step()));
    pContextMenu->addAction("&Run",      this, SLOT(send_animate()));
    //pContextMenu->addAction("&Forever",          this, SLOT(send_run()));
  }
  pContextMenu->addAction("&Stop",         this, SLOT(send_stop()));
  if (dll_interface->isStartAvailable())
    pContextMenu->addAction("Re&wind", this, SLOT(send_start()));
    
  /*
  if (dll_interface->isEndAvailable())
    pContextMenu->addAction("&End", this, SLOT(send_end()));
  */
   
  pContextMenu->addSeparator();
 
  pContextMenu->addAction("New Model", this, SLOT(send_reread()));
  pContextMenu->addSeparator();
  pContextMenu->addAction("Exit", this, SLOT(send_quit()) );

  // moved to initializeGL:
  // if (dll_interface) dll_interface->render_init();
}

#ifdef FAM_THREAD
void VVPViewer::customEvent( QEvent* event )
{
  RereadEvent* rre = dynamic_cast< RereadEvent* >( event );
  if( rre )
    {
    forall( std::string fn, absolute_to_user_filenames[ rre->filename ] )
      {
      modified_files.insert( fn );
      }
    timer.start( EVENT_TIMER );
    }
}

void VVPViewer::registerFile( std::string filename )
{
  std::string abso = util::absoluteDir( filename );
  absolute_to_user_filenames[ abso ].insert( filename );
  fam_thread->registerFile( abso );
}

void VVPViewer::unregisterFile( std::string filename )
{
  std::string abso = util::absoluteDir( filename );
  absolute_to_user_filenames.erase( abso );
  fam_thread->unregisterFile( util::absoluteDir( filename ) );
}

void VVPViewer::reregisterFiles()
{
  if( dll_interface && dll_interface->isProxyAvailable())
    {
    ___vvproxy::Proxy& proxy = dll_interface->proxy();
    forall( std::string filename, proxy.unregisterFiles )
      {
      unregisterFile( filename );
      }
    proxy.unregisterFiles.clear();
    forall( std::string filename, proxy.registerFiles )
      {
      registerFile( filename );
      }
    proxy.registerFiles.clear();
    }
}
#endif

// moved from orthoviewer

void VVPViewer::mousePressEvent(QMouseEvent* pEv) {
  switch (pEv->button()) {
    case Qt::LeftButton:
      switch (pEv->modifiers()) {
        case Qt::ShiftModifier:
          editMode = PAN;
          break;
        case Qt::ControlModifier:
          editMode = ZOOM;
          break;
        default: 
          editMode = ROTATE;
          break;
      }
      break;
    case Qt::MidButton:
      editMode = ZOOM;
      break;
    case Qt::RightButton:
      editMode = NONE;
      if (pContextMenu) 
        pContextMenu->popup(QCursor::pos());
      break;
    default: break;
  }
  prevMouseX = pEv->x();
  prevMouseY = pEv->y();
}

void VVPViewer::mouseReleaseEvent(QMouseEvent* pEv) {
  editMode = NONE;
  pEv->accept();
}

void VVPViewer::mouseMoveEvent(QMouseEvent* pEv) {
  switch (editMode) {
  case ROTATE:
    rotate(pEv->x() - prevMouseX, pEv->y() - prevMouseY);
    break;
  case ZOOM:
    zoom(pEv->y() - prevMouseY);
    break;
  case PAN:
    pan(pEv->x() - prevMouseX, pEv->y() - prevMouseY);
    break;
  case NONE:
  default: break;
  }

  prevMouseX = pEv->x();
  prevMouseY = pEv->y();
}


void VVPViewer::zoom(int dy) {
  scalefactor += GLfloat(dy);
  applyProjection(width(), height());
  update();
}

void VVPViewer::rotate(int dx, int dy) {
  rotx += GLfloat(dy);
  if (rotx >= 360.0) rotx -= 360.0;
  else if (rotx < 0.0) rotx += 360.0;

  roty += GLfloat(dx);
  if (roty >= 360.0) roty -= 360.0;
  else if (roty < 0.0) roty += 360.0;

  update();
}

void VVPViewer::pan(int dx, int dy) {
  panx += upp * GLfloat(dx);
  pany -= upp * GLfloat(dy);

  update();
}
