#ifndef __ALGORITHMS_CLONESET_HPP__
#define __ALGORITHMS_CLONESET_HPP__

#include <algebra/abstractvertex.hpp>
#include <algebra/abstractmesh.hpp>

namespace algorithms {
  template <class V>
  class CloneVSetFunc : public algebra::AbstractMesh<V>::VFunc {
    algebra::AbstractMesh<V>* pMesh;
  public:
    CloneVSetFunc() : pMesh(0) {}
    void setMesh(algebra::AbstractMesh<V>* pM) {
      pMesh = pM;
    }
    virtual void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
      typename algebra::AbstractMesh<V>::VPtr nv = v->clone();
      pMesh->addVertex(nv);
    }
  };

  template <class V>
  void CloneVSet(algebra::AbstractMesh<V>& src, algebra::AbstractMesh<V>& dst) {
    CloneVSetFunc<V> f;
    f.setMesh(&dst);
    src.forEachVertex(f);
  }
}

#endif
