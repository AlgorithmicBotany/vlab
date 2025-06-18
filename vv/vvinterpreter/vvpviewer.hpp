#ifndef __VVPVIEWER_HPP__
#define __VVPVIEWER_HPP__

//#include <view/orthoviewer.hpp>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMenu>

#include <string>
#include <set>

#include <util/point.hpp>

#ifdef FAM_THREAD
#include <qtimer.h>
#include "fam.hpp"
class QCloseEvent;

// Minimum time between two writing event before sending it of to the viewer 
// (in msec)
#define EVENT_TIMER 10

#endif


class DllInterface;

class VVPApp;

// class VVPViewer : public view::OrthoViewer {
class VVPViewer : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

public:
  VVPViewer(QWidget* parent, DllInterface *dll, int glsamples);

  ~VVPViewer();
  void setInterface(DllInterface* pInterface);

  //inline void renderText(int x, int y, QString s, QFont f = QFont()) {
  //  QGLWidget::renderText(x, y, s, f);
  //}
  //inline void renderText(double x, double y, double z, QString s, QFont f = QFont()) {
  //  QGLWidget::renderText(x, y, z, s, f);
  //}

#ifdef FAM_THREAD
  void registerFile( std::string filename );
  void unregisterFile( std::string filename );
  void reregisterFiles();
#endif

protected:
  enum {
      NONE,
      ROTATE,
      ZOOM,
      PAN
  } editMode;

  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;

  void mouseMoveEvent(QMouseEvent *pEv) override;
  void mousePressEvent(QMouseEvent *pEv) override;
  void mouseReleaseEvent(QMouseEvent *pEv) override;
  
  void zoom(int dy);
  void rotate(int dx, int dy);
  void pan(int dx, int dy);

  void applyProjection(int w, int h);

#ifdef FAM_THREAD
  void customEvent( QEvent* event );
#endif

private:
  DllInterface* dll_interface;
  VVPApp*       vvpapp;

  QSurfaceFormat _format;
#ifdef FAM_THREAD
  FamThread *fam_thread;
  QTimer timer;
  std::set<std::string> modified_files;
  std::map<std::string,std::set<std::string> > absolute_to_user_filenames;
#endif

  // moved from orthoviewer
  util::Point<GLfloat> min;
  util::Point<GLfloat> max;
  GLfloat              upp;
  GLfloat              rotx;
  GLfloat              roty;
  GLfloat              panx;
  GLfloat              pany;
  GLfloat              scalefactor;

  int prevMouseX;
  int prevMouseY;

  QMenu* pContextMenu;

private slots:
  void send_start();
  void send_step();
  void send_end();
  void send_run();
  void send_animate();
  void send_stop();
  void send_reread();
  void send_quit();

signals:
  void start();
  void step();
  void end();
  void run();
  void animate();
  void stop();
  void reread( const std::set<std::string>& filenames );
  void quit();

public:
  void buildContextMenu();
};

#endif
