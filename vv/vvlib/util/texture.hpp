#ifndef __UTIL__TEXTURE_HPP__
#define __UTIL__TEXTURE_HPP__

#include <string>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace util {
  /** @brief A utility class for one-dimensional textures. */
  class Texture1D {
  public:
    Texture1D();
    Texture1D(const Texture1D& texture);
    Texture1D(std::string filename);
    ~Texture1D();

    const Texture1D& operator=(const Texture1D& texture);

    void bind();

    // tex parameter functions
    static void clamp(bool enable = true);
    static void filter(bool enable = true);

    // tex environment functions
    static void modulate();
    static void decal();
    static void blend();
    static void replace();

  private:
    GLubyte*     data;
    unsigned int size;
    GLuint       tex_name;

    bool clamped;
  };

  /** @brief A utility class for two-dimensional textures. */
  class Texture2D {
  public:
    Texture2D();
    Texture2D(const Texture2D& texture);
    Texture2D(std::string filename);
    ~Texture2D();

    const Texture2D& operator=(const Texture2D& texture);

    void bind();

    // tex parameter functions
    static void clamp(bool enable = true);
    static void filter(bool enable = true);

    // tex environment functions
    static void modulate();
    static void decal();
    static void blend();
    static void replace();

  private:
    GLubyte*     data;
    unsigned int size;
    unsigned int width;
    unsigned int height;
    GLuint       tex_name;

    bool clamped;
  };
}

#endif
