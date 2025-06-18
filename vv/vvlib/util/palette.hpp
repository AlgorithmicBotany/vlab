#ifndef __UTIL__PALETTE_HPP__
#define __UTIL__PALETTE_HPP__

#include <string>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <util/colour.hpp>

namespace util {
  /** @brief A utility class for palettes.

      This class provides an interface for VLAB palette files
      and their use for OpenGL.
  */
  class Palette {
  public:
    typedef util::Colour<GLubyte> PaletteColour;

    Palette(std::string filename);

    void reread();

    void                 useColour(unsigned int index, double alpha = 1);
    const PaletteColour& getColour(unsigned int index);
    void                 blend(unsigned int ind1, unsigned int ind2, double w = 0.5, double alpha = 1);

    const std::string& getFilename() const { return filename; }

  private:
    std::string filename;
    PaletteColour colours[256];
  };
}

#endif

