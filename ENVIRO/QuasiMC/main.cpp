/* QuasiMC updated with QT4 */
/* The main application is in the event loop (in exec()) until the quasimc
 * thread sends it a close event.  If the user closes the debug window, only
 * that window is closed and the main application is still in exec(). */

#include <QApplication>
#include <QCloseEvent>
#include <QThread>
#include <QMutex>

#include <stdio.h>

#include "globjwin.h"
#include "quasiMC.h"
#include "qmc.h"
#include "randquasimc.h"

// global parameters
#ifdef __cplusplus
extern "C" {
#endif
int verbose;
int remove_objects;
int rays_from_objects;
int spectrum_samples;
int reflectance_model;
unsigned int max_depth;
int num_samples;
int use_sky_model;
int one_ray_per_spectrum;
int no_direct_light;
int return_var;
int material_parameter; // specifies which module parameter gives the material
                        // index
int number_of_rays;
float sensor_fov;
int return_type[MAX_SPECTRUM_SAMPLES]; // user-specified return type per
                                       // wavelength: absorbed flux, absorbed
                                       // irradiance, etc...
float total_spectrum_flux[MAX_SPECTRUM_SAMPLES]; // the summed radiant flux of
                                                 // each wavelength in the
                                                 // specturm
char output_filename[64];                        // optional name of output file
char RQMC_method[32];
SOURCESPECTRUM source_spectrum[MAX_SPECTRUM_SAMPLES];
RUSSIANROULETTE russian_roulette;
QUERIES queries;
SCENE scene;
int numSides; // the number of sides on the triangulated cylinder

// RQMC points
double *pointqmc, *rpointqmc, *randqmc;
double npoints = 0.0;

// functions in qmc.h
int InitQMC(void);
double *(*QMC)(void);
void (*ResetQMC)(void);
void (*FreeMethod)(void);
void FreeQMC();
double GetQMC(int request);
int SetQMC(int request, double value);
double *(*AppRandom)(double *point, double *r);
#ifdef __cplusplus
}
#endif

// A QT mutex
QMutex scene_mutex;

// ----------------------------------------------------------------------------

class quasiMCThread : public QThread {
public:
  quasiMCThread(QApplication *_app, GLObjectWindow *_debugWindow, int _argc,
                char **_argv, bool debug);
  quasiMCThread(int _argc, char **_argv);
  void CloseQTDebugWindow(void);
  void run(void);

private:
  QApplication *app;
  GLObjectWindow *debugWindow;
  int argc;
  char **argv;
  bool debug;
};

// quasiMC thread implementation
// ----------------------------------------------------------------------------
quasiMCThread::quasiMCThread(QApplication *_app, GLObjectWindow *_debugWindow,
                             int _argc, char **_argv, bool _debug)
    : app(_app), debugWindow(_debugWindow), argc(_argc), argv(_argv),
      debug(_debug) {}
// ----------------------------------------------------------------------------
quasiMCThread::quasiMCThread(int _argc, char **_argv)
    : app(NULL), debugWindow(NULL), argc(_argc), argv(_argv), debug(false) {}

// ----------------------------------------------------------------------------
void quasiMCThread::CloseQTDebugWindow(void) {
  // send a close signal to the QT application
  if (app != NULL) {
    QCloseEvent *qe = new QCloseEvent();
    QApplication::postEvent(app, qe);
    // Do not use the quit or exit signal because they are not thread safe.
  }
}
// ----------------------------------------------------------------------------

void quasiMCThread::run(void) {
  Cmodule_type two_modules[2];
  unsigned long module_id;
  int master;
  CTURTLE turtle;
  int polygon_num;

  // start communication with cpfg/lpfg
  // if you start QuasiMC from command line, set communication type to "pipes"
  // otherwise a seg fault is called (to fix this would require chaning
  // comm_lib).
  CSInitialize(&argc, &argv);

  InitQuery(&queries);
  InitScene(&scene);

  if (!ProcessArguments(argc, argv)) {
    FreeScene(&scene);
    FreeQuery(&queries);
    CTerminate();
    CloseQTDebugWindow();
    return;
  }

  if (use_sky_model)
    if (!InitSkyModel()) {
      fprintf(stderr, "QuasiMC - Sky model was not initialized\n");
      FreeScene(&scene);
      FreeQuery(&queries);
      CTerminate();
      CloseQTDebugWindow();
      return;
    }

  npoints = (double)number_of_rays;
  SetRQMC(RQMC_method, max_depth, num_samples, &npoints);

  if (!InitQMC()) {
    fprintf(stderr, "QuasiMC - RQMC method '%s' cannot be initialzed!\n",
            RQMC_method);
    FreeScene(&scene);
    FreeQuery(&queries);
    if (use_sky_model)
      FreeSkyModel();
    CTerminate();
    CloseQTDebugWindow();
    return;
  }

  fprintf(stderr, "Field process QuasiMC initialized.\n");

  if (verbose <= 1)
    CloseQTDebugWindow();
  else {
    debugWindow->emitUpdateVisualization();
  }

  // reset memory that holds queries and objects
  polygon_num = 0;
  ResetQuery(&queries);
  ResetScene(&scene);

  // loop until signal 'exit' comes from cpfg/lpfg
  do {
    if (verbose >= 1)
      fprintf(stderr, "QuasiMC - start processing data.\n");

    CSBeginTransmission();

    // process the data from plant simulator
    // have to lock mutex outside while loop because only once all of
    // the data is processed should it be displayed in debug window
    int new_data = 1;
    scene_mutex.lock();
    while (CSGetData(&master, &module_id, two_modules, &turtle)) {
      if (new_data) {
        new_data = 0;
        polygon_num = 0;
        ResetQuery(&queries);
        ResetScene(&scene);
      }
      ++polygon_num;
      StoreQuery(master, module_id, polygon_num, two_modules, &turtle);
    }
    scene_mutex.unlock();

    if (!new_data) {
      scene_mutex.lock();
      FindBoundingSphere(&scene);
      FillGrid(&scene);
      scene_mutex.unlock();

      if (queries.num_queries > 0) {
        scene_mutex.lock();
        DetermineResponse();
        scene_mutex.unlock();
      }

      // unfortunately, X doesn't allow this thread to update the QT window
      // so that following does not work (see gldisplay.h) as you expect
      // i.e., updateGL is not called.
      // Works in QT 4.7.2, Must test on Linux & Mac!!!!
      if (debugWindow != NULL)
        debugWindow->emitUpdateWindow();

      if (verbose >= 1)
        fprintf(stderr, "QuasiMC - data processed.\n");
    }

  } while (!CSEndTransmission());

  // if in debug mode, should only be used if QuasiMC is invoked from command
  // line without cpfg/lpfg.
  if (debug) {
    // wait until app is closed by user
    while (debugWindow->isOpen())
      this->sleep(1);
  } else {
    // close the debug window before shared memory is released
    CloseQTDebugWindow();
  }

  // free memory, but this is dangerous because the app may not be closed
  // I could not find a QT function to check if the app has closed from this
  // thread thus, I ensured that freeing memory in Scene3d will not cause faults
  // in debugwindow.
  FreeQMC();
  FreeQuery(&queries);
  // lock the mutex for freeing memory, so debug window doesn't try to draw
  // while memory is being freed!
  scene_mutex.lock();
  FreeScene(&scene);
  scene_mutex.unlock();
  if (use_sky_model)
    FreeSkyModel();

  fprintf(stderr, "Field process QuasiMC exiting.\n");

  // end communication with cpfg/lpfg
  CTerminate();

  // returning from this method will end execution of this thread
  return;
}

// ----------------------------------------------------------------------------

int main(int argc, char **argv) {
  bool no_xserver = false;
  bool debug = false;

  if (argc == 5) {
    if (!strcmp(argv[4], "-no_xserver"))
      no_xserver = true;
    else if (!strcmp(argv[4], "-debug"))
      debug = true;
    else {
      fprintf(stderr, "QuasiMC - last command line parameter unknown.\n");
      fprintf(
          stderr,
          "\tOnly the commands '-no_xserver' or '-debug' are recognized.\n");
    }
  }

  if (no_xserver) {
    // the thread which communicates with cpfg/lpfg
    quasiMCThread qmcThread(argc, argv);
    qmcThread.start();
    if (qmcThread.isRunning())
      qmcThread.wait();
  } else {
    QApplication app(argc, argv);
    GLObjectWindow debugWindow;

    // the thread which communicates with cpfg/lpfg
    quasiMCThread qmcThread(&app, &debugWindow, argc, argv, debug);
    qmcThread.start();

    // set the openGL window which shows debug info for quasiMC
    debugWindow.setWindowTitle("QuasiMC");
    debugWindow.resize(400, 300);
    debugWindow.show();
    app.exec();

    // even if the debugwindow is closed, quasiMC should keep running
    if (qmcThread.isRunning())
      qmcThread.wait();
  }

  return (0);
}
