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

class GLprimitive {
protected:
  GLprimitive(GLenum mode) {
    assert(!_exists);
#ifndef NDEBUG
    _exists = true;
#endif
    glBegin(mode);
  }

public:
  ~GLprimitive() {
    glEnd();
#ifndef NDEBUG
    _exists = false;
#endif
  }

  void Vertex(double x, double y) { glVertex3d(x, y, 0.0); }
  void Vertex(WorldPoint wp) { glVertex3d(wp.X(), wp.Y(), wp.Z()); }

private:
#ifndef NDEBUG
  static bool _exists;
#endif
};

class GLlines : public GLprimitive {
public:
  GLlines() : GLprimitive(GL_LINES) {}
};

class GLlineloop : public GLprimitive {
public:
  GLlineloop() : GLprimitive(GL_LINE_LOOP) {}
};

class GLlinestrip : public GLprimitive {
public:
  GLlinestrip() : GLprimitive(GL_LINE_STRIP) {}
};

class GLpoints : public GLprimitive {
public:
  GLpoints() : GLprimitive(GL_POINTS) {}
};

class GLcircles {
public:
  static void Record();
  void Draw(WorldPoint, double) const;

private:
  enum { eListId = 1 };
};

class GLfilledCircle
{
public:
  GLfilledCircle(void);
  void Draw(WorldPoint, double) const;
private:
  GLdouble _v[18][2];
};

class GLellipse
{
public:
  GLellipse(void);
  void Draw(WorldPoint, double, double) const;
private:
  GLdouble _v[18][2];
};


class GLlist {
public:
  GLlist(GLuint id, GLenum mode) { glNewList(id, mode); }
  ~GLlist() { glEndList(); }
};

class PushPopMatrix {
public:
  PushPopMatrix() { glPushMatrix(); }
  ~PushPopMatrix() { glPopMatrix(); }
};

#else
#error File already included
#endif
