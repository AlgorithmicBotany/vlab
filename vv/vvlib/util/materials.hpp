#ifndef __UTIL__MATERIALS_HPP__
#define __UTIL__MATERIALS_HPP__

#include <string>
//#include <qgl.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace util {
  /** @brief A utility class for materials.

      This class provides an interface for VLAB material files and
      their use for OpenGL.  The material files can contain upto 255
      materials.  The class allows access to these by an index.  If a
      material index is called that is not in the file, a default
      material is provided that uses the default values for OpenGL
      1.2.
  */

  class Materials {
  public:
    /** @brief The material data structure. */
    struct Material {
      GLfloat ambient[4];
      GLfloat diffuse[4];
      GLfloat emission[4];
      GLfloat specular[4];
      GLfloat shiny;
      GLfloat transparency;
    };

    Materials(std::string filename);

    void reread();

    void            useMaterial(unsigned int index);
    const Material& getMaterial(unsigned int index);
    void            blend(unsigned int ind1, unsigned int ind2, float t);

    const std::string& getFilename() const { return filename; }

  private:
    std::string filename;
    Material mats[256];
  };
}

#endif
