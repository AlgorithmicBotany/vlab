#ifndef __DLLINTERFACE_HPP__
#define __DLLINTERFACE_HPP__

#include <string>
#include <set>
#include <exception>
#include <qdatetime.h>
#include <generation/proxy.hpp>
#include "vvpviewer.hpp"

#ifdef WIN32
  #include <windows.h>
#endif

class DllException : public std::exception {
public:
  DllException(std::string message) throw();
  virtual ~DllException() throw();
  virtual const char* what() const throw();
private:
  std::string message;
};

class DllInterface {
public:
  DllInterface();
  ~DllInterface();

  void useDebug();
  void useStats();

  void open(std::string sofile);
  void close();

  void init();
  void start();
  void step();
  void end();
  void read_parms( const std::set<std::string>& filenames );

  void run();
  void stop();

  void render();
  void render_screen();
  void render_init();
  ___vvproxy::Proxy& proxy();

  bool isStartAvailable();
  bool isRereadAvailable();
  bool isStepAvailable();
  bool isEndAvailable();
  bool isRenderAvailable();
  bool isRenderScreenAvailable();
  bool isRenderInitAvailable();
  bool isProxyAvailable();

  void setViewer(VVPViewer* pViewer);

private:
  std::string filename;

  bool debug;
  bool stats;

#ifdef WIN32
  HINSTANCE dll_handle;

  typedef void (*fvoidproc)();
  typedef void (*fparamproc)( const std::set<std::string>& );
  typedef ___vvproxy::Proxy (*fproxyproc);

  fvoidproc  start_handle;
  fparamproc  read_parms_handle;
  fvoidproc  step_handle;
  fvoidproc  end_handle;
  fvoidproc  close_handle;
  fvoidproc  render_handle;
  fvoidproc  render_screen_handle;
  fvoidproc  render_init_handle;
  fproxyproc proxy_handle;
  fvoidproc  error_handle;
#else
  void* dll_handle;

  void (*start_handle)();
  void (*read_parms_handle)( const std::set<std::string>& );
  void (*step_handle)();
  void (*end_handle)();
  void (*close_handle)();
  void (*render_handle)();
  void (*render_screen_handle)();
  void (*render_init_handle)();
  ___vvproxy::Proxy& (*proxy_handle)();
  void (*error_handle)();
#endif

  bool start_available;
  bool read_parms_available;
  bool step_available;
  bool end_available;
  bool close_available;
  bool render_available;
  bool render_screen_available;
  bool render_init_available;
  bool proxy_available;

  QTime timer;
};

#endif
