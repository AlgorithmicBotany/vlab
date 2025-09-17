#ifndef __ALGORITHMS__CONSISTENCY_HPP__
#define __ALGORITHMS__CONSISTENCY_HPP__

#include <algebra/abstractvertex.hpp>
#include <algebra/abstractmesh.hpp>

namespace algorithms {
  template <typename V>
  bool symmetry(algebra::AbstractMesh<V>& mesh& m) {
    class Check : public V::NFunc {
    public:
      Check() : symmetric = true {}
      void operator()(
	  typename V::VPtr v,
	  typename V::VPtr n
      ) {
	if (!n->in(v)) symmetric = false;
      }
      bool symmetric;
    } check;

    class CheckAll : public algebra::AbstractMesh<V>::VFunc {
    public:
      virtual void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
	if (symmetric) v->forEachNeighbour(check);
      }
    } checkall;

    m.forEachVertex(checkall);
    return check.symmetric;
  }
}

#endif
