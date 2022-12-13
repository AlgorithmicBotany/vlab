/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#ifndef __GLUTILS_H__
#define __GLUTILS_H__

#include <cassert>

#ifdef _WINDOWS
#include <GL/glew.h>
#include <windows.h>
#include <GL/gl.h>
#else
#include <qgl.h>
#endif

class glPPM {
public:
  glPPM() { glPushMatrix(); }
  ~glPPM() { glPopMatrix(); }
};

class glPPSM {
public:
  glPPSM(GLenum mmode) : _mmode(mmode) {
    glMatrixMode(_mmode);
    glPushMatrix();
  }
  ~glPPSM() {
    glMatrixMode(_mmode);
    glPopMatrix();
  }

private:
  const GLenum _mmode;
};

class glOnOff {
public:
  glOnOff(GLenum cap) : _cap(cap) { glEnable(_cap); }
  ~glOnOff() { glDisable(_cap); }

private:
  const GLenum _cap;
};

class glOffOn {
public:
  glOffOn(GLenum cap) : _cap(cap) { glDisable(_cap); }
  ~glOffOn() { glEnable(_cap); }

private:
  const GLenum _cap;
};

class GLprimitive {
public:
  GLprimitive(GLenum mode) { glBegin(mode); }
  ~GLprimitive() { glEnd(); }
};

class GLlist {
public:
  GLlist(GLuint id, GLenum mode) { glNewList(id, mode); }
  ~GLlist() { glEndList(); }
};

class glTwoSidedLighting {
public:
  glTwoSidedLighting() {
      glGetBooleanv (GL_LIGHT_MODEL_TWO_SIDE, &_isOnOff);
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  }
  ~glTwoSidedLighting() {
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, _isOnOff);
  }
private:
  GLboolean _isOnOff;
};

class glSelectionMode {
public:
  glSelectionMode(int &res) : _res(res) { glRenderMode(GL_SELECT); }
  ~glSelectionMode() { _res = glRenderMode(GL_RENDER); }

private:
  int &_res;
};

#define DM DumpMatrices(__FILE__, __LINE__)
void CheckGL();
void DumpMatrices(const char *, int);

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
