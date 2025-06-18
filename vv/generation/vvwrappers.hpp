#ifndef __VVWRAPPERS_HPP__
#define __VVWRAPPERS_HPP__

namespace ___vvwrappers {
  inline void Flag(vertex::VPtr t, vertex::VPtr v) {
    v->flag(t);
  }

  inline bool In(vertex::VPtr t, vertex::VPtr v) {
    return v->in(t);
  }

  inline bool In(vertex::VPtr t, mesh& m) {
    return m.in(t);
  }

  inline vertex::VPtr Next(vertex::VPtr t, vertex::VPtr v) {
    return v->next(t);
  }

  inline vertex::VPtr Prev(vertex::VPtr t, vertex::VPtr v) {
    return v->prev(t);
  }

  inline vertex::VPtr Next(unsigned int k, vertex::VPtr t, vertex::VPtr v) {
    return v->next(t, k);
  }

  inline vertex::VPtr Prev(unsigned int k, vertex::VPtr t, vertex::VPtr v) {
    return v->prev(t, k);
  }

  inline void Erase(vertex::VPtr t, vertex::VPtr v) {
    v->remove(t);
  }

  inline void EraseFlagged(vertex::VPtr v) {
    v->remove_flagged();
  }

  enum direction {PREV, NEXT};

  inline void Splice(vertex::VPtr x, direction d, vertex::VPtr t, vertex::VPtr v) {
    switch(d) {
    case PREV: v->splicePrev(t, x); break;
    case NEXT: v->spliceNext(t, x); break;
    default: break;
    };
  }

  inline void Splice(vertex::VPtr x, direction d, vertex::VPtr v) {
    switch(d) {
    case PREV: v->splicePrev_flagged(x); break;
    case NEXT: v->spliceNext_flagged(x); break;
    default: break;
    };
  }

  inline void Replace(vertex::VPtr t, vertex::VPtr x, vertex::VPtr v) {
    v->replace(t, x);
  }

  inline void Replace(vertex::VPtr x, vertex::VPtr v) {
    v->replace_flagged(x);
  }

  inline void Add(vertex::VPtr v, mesh& m) {
    m.addVertex(v);
  }

  inline void Remove(vertex::VPtr v, mesh& m) {
    m.removeVertex(v);
  }

  vertex::Neighbourhood nb;

  inline vertex::VPtr Select(mesh& m) {
    return m.select();
  }

  inline vertex::VPtr Select(vertex::VPtr v) {
    return v->select();
  }
}

#endif
