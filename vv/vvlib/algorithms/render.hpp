#ifndef __ALGORITMS__REDNER_HPP__
#define __ALGORITMS__REDNER_HPP__

#include <string>
//#include <qgl.h>
//#include <qfont.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <algebra/abstractvertex.hpp>
#include <algebra/abstractmesh.hpp>

namespace algorithms {
  /** @brief Draw text.
      @param x The x-coordinate of where to draw.
      @param y The y-coordinate of where to draw.
      @param z The z-coordinate of where to draw.
      @param text The string to be rendered

      This function renders a string of text to the current OpenGL
      canvas using the X font facilities.  The xyz-coordinates are the
      lower-left corner of the first character to be rendered.  The
      string is drawn to the raster, not the scene, and so is
      guaranteed to be parallel to the front clipping plane.
  */
  void DrawText(float x, float y, float z, std::string text) {
//    static bool initialised = false;
//    static GLint fontBaseList = 0;
//    if (!initialised) {
//      initialised = true;

//      Display* xdisplay = XOpenDisplay(0);
//      XFontStruct* pF = XLoadQueryFont(xdisplay, "-*-*-*-r-*-*-18-*-*-*-*-*-*-*");
//      unsigned int id = pF->fid;
//      unsigned int first = pF->min_char_or_byte2;
//      unsigned int last = pF->max_char_or_byte2;

//      fontBaseList = glGenLists(last + 1);
//      glXUseXFont(id, first, last - first + 1, fontBaseList + first);
//    }
//    glListBase(fontBaseList);
//    glRasterPos3d(x, y, z);
//    glCallLists(text.size(), GL_BYTE, text.c_str());
  }

  /** @brief The base class for per-vertex draw functions.
   */
  template <class V>
  class DrawFunc : public algebra::AbstractMesh<V>::VFunc {
  public:
    virtual ~DrawFunc() {}
    virtual void begin() {}
    virtual void end() {}
    virtual void operator()(typename algebra::AbstractMesh<V>::VPtr v) {}
  };

  /** @brief Wrapper function to call draw functions on a mesh.
      @param mesh The set of vertices to be rendered.
      @param func A supplied draw function.
  */
  template <class V>
  void Draw(algebra::AbstractMesh<V>& mesh, DrawFunc<V>& func) {
    func.begin();
    mesh.forEachVertex(func);
    func.end();
  }

  /** @brief The simplest per-vertex rendering.

      This calls glRender of each vertex with no additional OpenGL
      settings.
   */
  template <class V>
  class DrawEach : public DrawFunc<V> {
  public:
    void begin() {}
    void end()   {}
    void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
      v->getPosition().glRender();
    }
  };

  /** @brief A draw function that draw points.

      This calls glRender of each vertex inside a GL_POINTS statement.
   */
  template <class V>
  class DrawPoints : public DrawFunc<V> {
  public:
    void begin() {glBegin(GL_POINTS);}
    void end()   {glEnd();}
    void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
      v->getPosition().glRender();
    }
  };

  /** @brief A function that draws a line between each pair of vertices.

      A GL_LINE is drawn once for each pair of neighbouring vertices using the
      glRender to set the end conditions of the line.
  */
  template <class V>
  class DrawWireframe : public DrawFunc<V> {
  public:
    void begin() {glBegin(GL_LINES);}
    void end()   {glEnd();}
    void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
      v->forEachNeighbour(func);
    }
  private:
    class WireFunc : public V::NFunc {
    public:
      void operator()(
	  typename V::VPtr v,
	  typename V::VPtr n
      ) {
	if (v->getLabel() < n->getLabel()) return;
	v->getPosition().glRender();
	n->getPosition().glRender();
      }
    };

    WireFunc func;
  };

  /** @brief A function to draw the mesh as triangles.

      This function iterates over adjacent pairs in a neighbourhood to
      find the triangles in the mesh.  GL_TRIANGLES is used.  This
      function assumes that only triangles will result from polygon
      interpretation and so will graphical artifacts for meshes where
      this is not the case.
  */
  template <class V>
  class DrawTriangles : public DrawFunc<V> {
  public:
    void begin() {glBegin(GL_TRIANGLES);}
    void end()   {glEnd();}
    void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
      v->forEachNeighbour(func);
    }
  private:
    class TriangleFunc : public V::NFunc {
    public:
      void operator()(	
	  typename V::VPtr v,
	  typename V::VPtr n
      ) {
	if (v->getLabel() < n->getLabel()) return;
	typename V::VPtr m = v->next(n);
	if (v->getLabel() < m->getLabel()) return;

	v->getPosition().glRender();
	n->getPosition().glRender();
	m->getPosition().glRender();
      }
    };

    TriangleFunc func;
  };

  /** @brief A function to draw the mesh as triangles.

      This function is a variation of DrawTriangles that performs an
      extra check to ensure only triangles are drawn where the
      topology allows.  This version should be used for surfaces that
      are not closed under a polygon interpretation that only permits
      triangles.
  */
  template <class V>
  class DrawTrianglesChecked : public DrawFunc<V> {
  public:
    void begin() {glBegin(GL_TRIANGLES);}
    void end()   {glEnd();}
    void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
      v->forEachNeighbour(func);
    }
  private:
    class TriangleFunc : public V::NFunc {
    public:
      void operator()(	
	  typename V::VPtr v,
	  typename V::VPtr n
      ) {
	if (v->getLabel() < n->getLabel()) return;
	typename V::VPtr m = v->next(n);
	if (v->getLabel() < m->getLabel()) return;

        if (!n->in(m)) return;

	v->getPosition().glRender();
	n->getPosition().glRender();
	m->getPosition().glRender();
      }
    };

    TriangleFunc func;
  };

  /** @brief A function to draw the mesh as quads.

      This function traces diamond paths to find all the quads.  It
      uses GL_QUADS for rendering.  This function assumes that only
      quads will result from polygon interpretation and so will
      graphical artifacts for meshes where this is not the case.
  */
  template <class V>
  class DrawQuads : public DrawFunc<V> {
  public:
    void begin() {glBegin(GL_QUADS);}
    void end()   {glEnd();}
    void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
      v->forEachNeighbour(func);
    }
  private:
    class QuadFunc : public V::NFunc {
    public:
      void operator()(
	  typename V::VPtr v,
	  typename V::VPtr n
      ) {
	if (v->getLabel() < n->getLabel()) return;
	typename V::VPtr a = v->next(n);
	if (v->getLabel() < a->getLabel()) return;
	typename V::VPtr b = n->prev(v);
	if (v->getLabel() < n->getLabel()) return;

	v->getPosition().glRender();
	n->getPosition().glRender();
	b->getPosition().glRender();
	a->getPosition().glRender();
      }
    };

    QuadFunc func;
  };

  /** @brief A function to draw the mesh as quads with checking.

      This function traces diamond paths to find all the quads.  It
      uses GL_QUADS for rendering.  This function assumes that only
      quads will result from polygon interpretation and so will
      graphical artifacts for meshes where this is not the case.
  */
  template <class V>
  class DrawQuadsChecked : public DrawFunc<V> {
  public:
    void begin() {glBegin(GL_QUADS);}
    void end()   {glEnd();}
    void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
      v->forEachNeighbour(func);
    }
  private:
    class QuadFunc : public V::NFunc {
    public:
      void operator()(
	  typename V::VPtr v,
	  typename V::VPtr n
      ) {
	if (v->getLabel() < n->getLabel()) return;
	typename V::VPtr a = v->next(n);
	if (v->getLabel() < a->getLabel()) return;
	typename V::VPtr b = n->prev(v);
	if (v->getLabel() < n->getLabel()) return;

	if (a->next(v) != b) return;

	v->getPosition().glRender();
	n->getPosition().glRender();
	b->getPosition().glRender();
	a->getPosition().glRender();
      }
    };

    QuadFunc func;
  };

  /** @brief A fucntion that handles both triangles and quads.

      This function combines triangles and quads.  However, since it
      must differentiate the two cases, it is not as efficient as
      doing only trianlges or only quads.  This function assumes that
      only triangles or quads will result from polygon interpretation
      and so will graphical artifacts for meshes where this is not the
      case.
  */
  template <class V>
  class DrawTriAndQuads : public DrawFunc<V> {
  public:
    void begin() {}
    void end() {}
    void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
      v->forEachNeighbour(func);
    }
  private:
    class TQFunc : public V::NFunc {
    public:
      void operator()(
	  typename V::VPtr v,
	  typename V::VPtr n
      ) {
	if (v->getLabel() < n->getLabel()) return;
	typename V::VPtr a = v->next(n);
	if (v->getLabel() < a->getLabel()) return;
	typename V::VPtr b = n->prev(v);
	if (v->getLabel() < n->getLabel()) return;

	if (a == b) { // it's a triangle
	  glBegin(GL_TRIANGLES);
	  v->getPosition().glRender();
	  n->getPosition().glRender();
	  a->getPosition().glRender();
	  glEnd();
	}
	else if (a->in(b)) { //it's a quad
	  glBegin(GL_QUADS);
	  v->getPosition().glRender();
	  n->getPosition().glRender();
	  b->getPosition().glRender();
	  a->getPosition().glRender();
	  glEnd();
	}
      }

    };
    TQFunc func;
  };
}

#endif
