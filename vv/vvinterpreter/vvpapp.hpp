#ifndef __VVPAPP_HPP__
#define __VVPAPP_HPP__

#include <string>
#include <QMainWindow>
class QKeyEvent;
class QCloseEvent;
class QMenu;

#include "vvpviewer.hpp"
#include "dllinterface.hpp"

class QShowEvent;

class VVPApp : public QMainWindow {
  Q_OBJECT

public:
  VVPApp(QWidget* parent = 0);
  ~VVPApp();

  void debug();
  void nogui();
  void stats();
  void startanimate();
  void setGLsize (int w, int h);
  void setGLSamples (int samples);

public slots:
  void open();
  void open(std::string filename);

protected:
  void keyPressEvent(QKeyEvent* pEv);
  void closeEvent(QCloseEvent* pEv);
  void showEvent(QShowEvent* ev);

private:
  DllInterface dll_interface;
  VVPViewer*   view;

private slots:
  void saveAsImage();
  void saveAsPostScript();

  void saveImage(std::string filename, std::string extension);
  void savePostScript(std::string filename);

  void recNone();
  void recAfterStep();

  void start();
  void step();
  void end();

  void run();
  void animate();
  void stop();

  void reread(const std::set<std::string>& filenames);

  void quickHelp();
  void onlineHelp();
  void pdfHelp();
  void about();

private:
  bool stopped;
  bool exitAfterStart;
  bool animateOnStart;
  int glsamples;
  enum RecState {NO_REC, AFTER_STEP} rec;
  QAction* noRecAction;
  QAction* recAfterStepAction;

  QMenu* filemenu;
  QMenu* imagemenu;
  QMenu* animatemenu;
  QMenu* helpmenu;

  unsigned int frame_counter;

  void rec_frame();
};

#endif
