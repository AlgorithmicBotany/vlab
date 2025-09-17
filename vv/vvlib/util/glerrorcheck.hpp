#ifndef ___UTIL__GLERRORCHECK_HPP__
#define ___UTIL__GLERRORCHECK_HPP__

/** @brief The following macro checks if an OpenGL has occured.
    @param cmd The command to check.

    This macro is based on that found in the notes for SIGGRAPH 2003
    Course 26.  It is modified here to use C++ (it was originally in C)
    and in the output format.
*/

#include <iostream>

#define OPENGL_ERROR_CHECK(cmd) \
  cmd; \
  { \
    GLenum error = GL_NO_ERROR; \
    while ((error = glGetError()) != GL_NO_ERROR) { \
      std::cerr << "OpenGL error found. [ " << __FILE__ \
                << " : " << __LINE__ \
                << " : " << #cmd \
                << " ] \"" << gluErrorString(error) \
                << "\"." << std::endl; \
    } \
  }

#endif
