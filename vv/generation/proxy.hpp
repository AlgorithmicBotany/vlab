#ifndef __PROXY_HPP__
#define __PROXY_HPP__

#include <list>
#include <set>
#include <string>
//#include <vvinterpreter/vvpviewer.hpp>

namespace util {
  class Materials;
  class Palette;
  class Function;
  class Contour;
}

#ifdef WIN32
  #ifdef DLL_INTERFACE_IMPORT
    #define DLL_INTERFACE __declspec(dllimport)
  #else
    #define DLL_INTERFACE __declspec(dllexport)
  #endif
#endif

namespace ___vvproxy {
  struct Proxy {
    float view_maxx;
    float view_maxy;
    float view_maxz;
    float view_minx;
    float view_miny;
    float view_minz;

    unsigned int steps;
    unsigned int delay_msec;
    bool         no_animate;
    bool         record_frame;

    float rotx;
    float roty;

    std::list<util::Materials*> materials;
    std::list<util::Palette*>   palettes;
    std::list<util::Function*>  functions;
    std::list<util::Contour*>   contours;
    std::set<std::string>       registerFiles;
    std::set<std::string>       unregisterFiles;

    //VVPViewer* viewer;

    Proxy() :
      view_maxx(1.0),
      view_maxy(1.0),
      view_maxz(1.0),
      view_minx(-1.0),
      view_miny(-1.0),
      view_minz(-1.0),
      steps(0),
      delay_msec(0),
      no_animate(false),
      record_frame(false)//,
      //viewer(0)
    {}

    void setViewVolume(float maxx, float maxy, float maxz, float minx, float miny, float minz) {
      this->view_maxx = maxx;
      this->view_maxy = maxy;
      this->view_maxz = maxz;
      this->view_minx = minx;
      this->view_miny = miny;
      this->view_minz = minz;
    }
  };
}

#endif
