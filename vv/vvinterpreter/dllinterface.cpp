#ifndef WIN32
#include <dlfcn.h>
#endif
#include <iostream>
#include <stdlib.h>
#include "dllinterface.hpp"

#ifdef VVSTATIC
extern "C" {
void vvp_start();
void vvp_read_parms();
void vvp_step();
void vvp_end();
void vvp_close();
void vvp_render();
void vvp_render_screen();
void vvp_render_init();
void vvp_error();
___vvproxy::Proxy& getProxy();
}
#endif

DllException::DllException(std::string message) throw() :
  exception()
{
  this->message = "DLL exception: ";
  this->message += message;
  std::cerr << message << std::endl;
}

DllException::~DllException() throw (){}

const char* DllException::what() const throw () {
  return message.c_str();
}

DllInterface::DllInterface() :
  debug(false),
  stats(false),
  dll_handle(0),
  start_handle(0),
  read_parms_handle(0),
  step_handle(0),
  end_handle(0),
  close_handle(0),
  render_handle(0),
  render_init_handle(0),
  proxy_handle(0),
#ifdef VVSTATIC
  start_available(true),
  read_parms_available(true),
  step_available(true),
  end_available(true),
  close_available(true),
  render_available(true),
  render_screen_available(true),
  render_init_available(true),
  proxy_available(true)
#else
  start_available(false),
  read_parms_available(false),
  step_available(false),
  end_available(false),
  close_available(false),
  render_available(false),
  render_screen_available(false),
  render_init_available(false),
  proxy_available(false)
#endif
{
#ifdef VVSTATIC
  start_handle = (void(*)())(&vvp_start);
  read_parms_handle = (void(*)(const std::set<std::string>&))(&vvp_read_parms);
  step_handle = (void(*)())(&vvp_step);
  end_handle = (void(*)())(&vvp_end);
  close_handle = (void(*)())(&vvp_close);
  render_handle = (void(*)())(&vvp_render);
  render_screen_handle = (void(*)())(&vvp_render_screen);
  render_init_handle = (void(*)())(&vvp_render_init);
  proxy_handle = (___vvproxy::Proxy&(*)())(&getProxy);
  error_handle = (void(*)())(&vvp_error);
#endif
}

DllInterface::~DllInterface() {
  close();
}

void DllInterface::useDebug() {
  debug = true;
}

void DllInterface::useStats() {
  stats = true;
}

#ifndef VVSTATIC
void DllInterface::open(std::string sofile) {
  filename = sofile;
#else
void DllInterface::open(std::string /*sofile*/) {
#endif
}

void DllInterface::close() {
#ifndef VVSTATIC
  if (dll_handle) {
    if (close_available) {
      try {
        if (stats) timer.start();
        (*close_handle)();
        if (stats) std::cerr << "close time is " << timer.elapsed() << " ms." << std::endl;
      }
      catch (...) {
        if (debug) (*error_handle)();
      }
    }
#ifdef WIN32
    if (dll_handle) {
      if (!FreeLibrary(dll_handle)) {
        throw DllException("Could not close the DLL.");
      }
      else dll_handle = 0;
    }
#else
    if (dlclose(dll_handle)) {
      throw DllException(dlerror());
    }
#endif
  }
  dll_handle = 0;
  start_handle = 0;
  read_parms_handle = 0;
  step_handle = 0;
  end_handle = 0;
  close_handle = 0;
  render_handle = 0;
  render_init_handle = 0;
  error_handle = 0;
  proxy_handle = 0;
  start_available = false;
  read_parms_available = false;
  step_available = false;
  end_available = false;
  render_available = false;
  render_screen_available = false;
  render_init_available = false;
  proxy_available = false;
#endif
}

void DllInterface::init()
{
#ifndef VVSTATIC
  close();

  if (filename.empty()) return;

#ifdef WIN32
  dll_handle = LoadLibrary(filename.c_str());

  if (!dll_handle) throw DllException(std::string("Could not open the DLL file:") + filename);

  start_handle = (fvoidproc)GetProcAddress(dll_handle, "vvp_start");
  read_parms_handle = (fparamproc)GetProcAddress(dll_handle, "vvp_read_parms");
  step_handle = (fvoidproc)GetProcAddress(dll_handle, "vvp_step");
  end_handle = (fvoidproc)GetProcAddress(dll_handle, "vvp_end");
  close_handle = (fvoidproc)GetProcAddress(dll_handle, "vvp_close");
  render_handle = (fvoidproc)GetProcAddress(dll_handle, "vvp_render");
  render_screen_handle = (fvoidproc)GetProcAddress(dll_handle, "vvp_render_screen");
  render_init_handle = (fvoidproc)GetProcAddress(dll_handle, "vvp_render_init");
  proxy_handle = (fproxyproc)GetProcAddress(dll_handle, "proxy");
  error_handle = (fvoidproc)GetProcAddress(dll_handle, "vvp_error");
#else
  dll_handle = dlopen(filename.c_str(), RTLD_NOW);
  if (!dll_handle) throw DllException(dlerror());

  start_handle = (void(*)())dlsym(dll_handle, "vvp_start");
  read_parms_handle = (void(*)(const std::set<std::string>&))dlsym(dll_handle, "vvp_read_parms");
  step_handle = (void(*)())dlsym(dll_handle, "vvp_step");
  end_handle = (void(*)())dlsym(dll_handle, "vvp_end");
  close_handle = (void(*)())dlsym(dll_handle, "vvp_close");
  render_handle = (void(*)())dlsym(dll_handle, "vvp_render");
  render_screen_handle = (void(*)())dlsym(dll_handle, "vvp_render_screen");
  render_init_handle = (void(*)())dlsym(dll_handle, "vvp_render_init");
  proxy_handle = (___vvproxy::Proxy&(*)())dlsym(dll_handle, "getProxy");
  error_handle = (void(*)())dlsym(dll_handle, "vvp_error");
#endif

  start_available = start_handle ? true : false;
  read_parms_available = read_parms_handle ? true : false;
  step_available = step_handle ? true : false;
  end_available = end_handle ? true : false;
  close_available = close_handle ? true : false;
  render_available = render_handle ? true : false;
  render_screen_available = render_screen_handle ? true : false;
  render_init_available = render_init_handle ? true : false;
  proxy_available = proxy_handle ? true : false;
#endif

}

void DllInterface::start() {
  if (start_available) {
    try {
      if (stats) timer.start();
      (*start_handle)();
      if (stats) std::cerr << "start time is " << timer.elapsed() << " ms." << std::endl;
    }
    catch (...) {
      if (debug) (*error_handle)();
    }
  }
}

void DllInterface::step() {
  if (step_available) {
    try {
      if (stats) timer.start();
      (*step_handle)();
      if (stats) std::cerr << "step time is " << timer.elapsed() << " ms." << std::endl;
    }
    catch (...) {
      if (debug) (*error_handle)();
    }
  }
}

void DllInterface::end() {
  if (end_available) {
    try {
      if (stats) timer.start();
      (*end_handle)();
      if (stats) std::cerr << "end time is " << timer.elapsed() << " ms." << std::endl;
    }
    catch (...) {
      if (debug) (*error_handle)();
    }
  }
}

void DllInterface::read_parms( const std::set<std::string>& filenames ) {
  if (read_parms_available) {
    try {
      if (stats) timer.start();
      (*read_parms_handle)( filenames );
      if (stats) std::cerr << "read_parms time is " << timer.elapsed() << " ms." << std::endl;
    }
    catch (...) {
      if (debug) (*error_handle)();
    }
  }
}

void DllInterface::render() {
  if (render_available) {
    try {
      if (stats) timer.start();
      (*render_handle)();
      if (stats) std::cerr << "render time is " << timer.elapsed() << " ms." << std::endl;
    }
    catch (...) {
      if (debug) (*error_handle)();
    }
  }
}

void DllInterface::render_screen() {
  if (render_screen_available) {
    try {
      if (stats) timer.start();
      (*render_screen_handle)();
      if (stats) std::cerr << "render_screen time is " << timer.elapsed() << " ms." << std::endl;
    }
    catch (...) {
      if (debug) (*error_handle)();
    }
  }
}

void DllInterface::render_init() {
  if (render_init_available) {
    try {
      if (stats) timer.start();
      (*render_init_handle)();
      if (stats) std::cerr << "render_init time is " << timer.elapsed() << " ms." << std::endl;
    }
    catch (...) {
      if (debug) (*error_handle)();
    }
  }
}

___vvproxy::Proxy& DllInterface::proxy() {
  if( proxy_available )
    {
#ifdef WIN32
    return (*proxy_handle);
#else
    return (*proxy_handle)();
#endif
    }
  std::cerr << "Error, no proxy ! Cannot continue." << std::endl;
  exit( 10 );
}

bool DllInterface::isStartAvailable() {
  return start_available;
}

bool DllInterface::isRereadAvailable() {
  return read_parms_available;
}

bool DllInterface::isStepAvailable() {
  return step_available;
}

bool DllInterface::isEndAvailable() {
  return end_available;
}

bool DllInterface::isRenderAvailable() {
  return render_available;
}

bool DllInterface::isRenderScreenAvailable() {
  return render_screen_available;
}

bool DllInterface::isRenderInitAvailable() {
  return render_init_available;
}

bool DllInterface::isProxyAvailable() {
  return proxy_available;
}

void DllInterface::setViewer(VVPViewer* pViewer) {
  //QQQ if (proxy_available) proxy().viewer = pViewer;
}
