#ifndef __ALGORITHMS_STELLAR_HPP__
#define __ALGORITHMS_STELLAR_HPP__

#include <algebra/opqueue.hpp>

namespace algorithms {
  /** @brief A function object to remove all the relations of
             a particular vertex.

      This function is an implementation of bistellar move 0 from
      ``Simplicial moves on complexes and manifolds'' (Lickorish,
      1999).
  */
  template <class V, bool do_checks = true> class Remove {
  public:
    /** @brief A function object to remove all the relations of
               a particular vertex.
	@param v The vertex to act on.

	This function iterates over the neighbourhood of v and for each
	neighbour, it removes itself from the neighbour's neighbourhood
	and the neighbour from its own.
    */
    void operator()(typename V::VPtr& v) {
      // This code path should never occur
      std::cerr << "Error: Remove operator() stub entered." << std::endl;
      throw 0;
    }
  };
  template <class V> class Remove<V, true> {
  public:
    void operator()(typename V::VPtr& v) {
      if (!v) {
	std::cerr << "Error: Attempt to remove a null vertex." << std::endl;
	throw 0;
      }

      while (v->getNeighbourCount()) {
	typename V::VPtr x = v->select();
	if (!x) {
	  std::cerr << "Error: Attempt to remove a vertex with a null neighbour." << std::endl;
	  throw 0;
	}
	v->remove(x);
	x->remove(v);
      }
    }
  };
  template <class V> class Remove<V, false> {
  public:
    void operator()(typename V::VPtr& v) {
      while (v->getNeighbourCount()) {
	typename V::VPtr x = v->select();
	v->remove(x);
	x->remove(v);
      }
    }
  };

  /** @brief A function object to flip an edge.

      This function is an implementation of bistellar move 1 from
      ``Simplicial moves on complexes and manifolds'' (Lickorish,
      1999).
  */
  template <class V, bool use_old_nb = false, bool do_checks = true>
  class Flip {
    public:
    /** @param a The first vertex on the edge.
        @param b The second vertex on the edge.

        This function removes the edge from a to b and add a new edge
        from (nextto b in a) to (prevto b in a).  If a and b are not in
        each others neighbourhoods, a warning is printed to the terminal
        and no change is effected.  This function is intended only for
        use with triangular meshes.
    */
    void operator()(typename V::VPtr& a, typename V::VPtr& b) {
      // This code path should never occur
      std::cerr << "Error: Flip operator() stub entered." << std::endl;
      throw 0;
    };
  };
  template <class V> class Flip<V, true, true> {
    public:
    void operator()(typename V::VPtr& a, typename V::VPtr& b) {
      if (!a || !b) {
	std::cerr << "Error: Attempt to flip an edge with a null vertex." << std::endl;
	throw 0;
      }

      unsigned int check = 0;
      if (a->in(b)) check++;
      if (b->in(a)) check++;

      switch (check) {
      case 0: std::cerr << "Warning: Attempt to flip an edge between vertices that have no relation." << std::endl; break;
      case 1: std::cerr << "Warning: Attempt to flip an edge between vertices that have an assymetric relation." << std::endl; return; break;
      default: break;
      }
      
      typename V::VPtr x = a->getOld()->next(b);
      typename V::VPtr y = a->getOld()->prev(b);

      if (!x || !y) {
	std::cerr << "Error: Attempt to flip an edge to a neighbouring null vertex." << std::endl;
	throw 0;
      }

      a->remove(b);
      b->remove(a);
      x->spliceNext(a, y);
      y->splicePrev(a, x);
    }
  };
  template <class V> class Flip<V, true, false> {
    public:
    void operator()(typename V::VPtr& a, typename V::VPtr& b) {
      typename V::VPtr x = a->getOld()->next(b);
      typename V::VPtr y = a->getOld()->prev(b);

      a->remove(b);
      b->remove(a);
      x->spliceNext(a, y);
      y->splicePrev(a, x);
    }
  };
  template <class V> class Flip<V, false, true> {
    public:
    void operator()(typename V::VPtr& a, typename V::VPtr& b) {
      if (!a || !b) {
	std::cerr << "Error: Attempt to flip an edge with a null vertex." << std::endl;
	throw 0;
      }

      unsigned int check = 0;
      if (a->in(b)) check++;
      if (b->in(a)) check++;

      switch (check) {
      case 0: std::cerr << "Warning: Attempt to flip an edge between vertices that have no relation." << std::endl; break;
      case 1: std::cerr << "Warning: Attempt to flip an edge between vertices that have an assymetric relation." << std::endl; break;
      default: break;
      }
      
      typename V::VPtr x = a->next(b);
      typename V::VPtr y = a->prev(b);

      if (!x || !y) {
	std::cerr << "Error: Attempt to flip an edge to a neighbouring null vertex." << std::endl;
	throw 0;
      }

      a->remove(b);
      b->remove(a);
      x->spliceNext(a, y);
      y->splicePrev(a, x);
    }
  };
  template <class V> class Flip<V, false, false> {
    public:
    void operator()(typename V::VPtr& a, typename V::VPtr& b) {
      typename V::VPtr x = a->next(b);
      typename V::VPtr y = a->prev(b);

      a->remove(b);
      b->remove(a);
      x->spliceNext(a, y);
      y->splicePrev(a, x);
    }
  };

  /** @brief A function place a new vertex at the center of a polygon.

      This function is an implementation of bistellar move 2 from
      ``Simplicial moves on complexes and manifolds'' (Lickorish,
      1999).
  */
  template <class V, bool do_checks = true> class Centroid {
  public:
    /** @brief The centroid operation
	@param a The first vertex on the edge.
	@param b The second vertex on the edge.

	This function creates a new vertex at the center of a polygon
	defined such that a and b form an edge of a polygon in the
	counter clockwise direction.  If a and b do not form an edge, a
	warning is printed to the terminal and a null vertex is
	returned.
    */
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {
      // This code path should never occur
      std::cerr << "Error: Centroid operator() stub entered." << std::endl;
      throw 0;
    }
  };
  template <class V> class Centroid<V, true> {
  public:
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {
      if (!a || !b) {
	std::cerr << "Error: Attempt to find the centroid along an edge with a null vertex." << std::endl;
	throw 0;
      }

      unsigned int check = 0;
      if (a->in(b)) check++;
      if (b->in(a)) check++;

      switch (check) {
      case 0:
	std::cerr << "Warning: Attempt to create a centroid for vertices that have no relation." << std::endl;
	return typename V::VPtr();
	break;
      case 1:
	std::cerr << "Warning: Attempt to create a centroid for vertices that have an assymetric relation." << std::endl;
	return typename V::VPtr();
	break;
      default: break;
      }

      typename V::VPtr x = (new V())->vptr();
      typename V::Neighbourhood n;
      n.push_back(a);
      a->spliceNext(b, x);

      typename V::VPtr g = a;
      typename V::VPtr h = b;

      while (h != a) {
	if (!g || !h) {
	  std::cerr << "Error: Enconutered a null vertex when interpreting the polygon around the centrioid." << std::endl;
	  throw 0;
	}
	n.push_back(h);
	h->splicePrev(g, x);
	algebra::pathref<V>(false, h, g, algebra::opPrev, algebra::opPrev, algebra::opSwap, algebra::opStop);
      }

      x->nbAssign(n);
      return x;
    }
  };
  template <class V> class Centroid<V, false> {
  public:
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {
      typename V::VPtr x = (new V())->vptr();
      typename V::Neighbourhood n;
      n.push_back(a);
      a->spliceNext(b, x);

      typename V::VPtr g = a;
      typename V::VPtr h = b;

      while (h != a) {
	n.push_back(h);
	h->splicePrev(g, x);
	algebra::pathref<V>(false, h, g, algebra::opPrev, algebra::opPrev, algebra::opSwap, algebra::opStop);
      }

      x->nbAssign(n);
      return x;
    }
  };

  /** @brief An function object for the edge split operation. */
  template <class V, bool use_old_nb = false, bool do_checks = true>
  class EdgeSplit {
  public:
    /** @brief The edge split operation.
	@param a The first vertex on the edge to split
	@param b The second vertex on the edge to split
    */
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {
      // This code path should never occur
      std::cerr << "Error: Centroid operator() stub entered." << std::endl;
      throw 0;
    }
  };
  template <class V> class EdgeSplit<V, true, true> {
  public:
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {
      if (!a || !b) {
	std::cerr << "Error: Attempt to split an edge to a null vertex." << std::endl;
	throw 0;
      }

      if (!a->in(b) || !b->in(a)) {
	std::cerr << "Error: Attempt to split an edge of non-neighbouring vertices." << std::endl;
	throw 0;
      }

      typename V::VPtr x = a->getOld()->next(b);
      typename V::VPtr y = a->getOld()->prev(b);

      if (!x || !y) {
	std::cerr << "Error: Attempt to split an edge with a neighbouring nul vertex." << std::endl;
	throw 0;
      }

      typename V::VPtr v = (new V())->vptr();

      a->replace(b, v);
      b->replace(a, v);
      x->spliceNext(a, v);
      y->splicePrev(a, v);

      typename V::Neighbourhood n;
      n.push_back(a);
      n.push_back(y);
      n.push_back(b);
      n.push_back(x);
      v->nbAssign(n);

      return v;
    }    
  };
  template <class V> class EdgeSplit<V, false, true> {
  public:
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {
      if (!a || !b) {
	std::cerr << "Error: Attempt to split an edge to a null vertex." << std::endl;
	throw 0;
      }

      if (!a->in(b) || !b->in(a)) {
	std::cerr << "Error: Attempt to split an edge of non-neighbouring vertices." << std::endl;
	throw 0;
      }

      typename V::VPtr x = a->next(b);
      typename V::VPtr y = a->prev(b);

      if (!x || !y) {
	std::cerr << "Error: Attempt to split an edge with a neighbouring nul vertex." << std::endl;
	throw 0;
      }

      typename V::VPtr v = (new V())->vptr();

      a->replace(b, v);
      b->replace(a, v);
      x->spliceNext(a, v);
      y->splicePrev(a, v);

      typename V::Neighbourhood n;
      n.push_back(a);
      n.push_back(y);
      n.push_back(b);
      n.push_back(x);
      v->nbAssign(n);

      return v;
    }    
  };
  template <class V> class EdgeSplit<V, true, false> {
  public:
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {
      typename V::VPtr x = a->getOld()->next(b);
      typename V::VPtr y = a->getOld()->prev(b);

      typename V::VPtr v = (new V())->vptr();

      a->replace(b, v);
      b->replace(a, v);
      x->spliceNext(a, v);
      y->splicePrev(a, v);

      typename V::Neighbourhood n;
      n.push_back(a);
      n.push_back(y);
      n.push_back(b);
      n.push_back(x);
      v->nbAssign(n);

      return v;
    }    
  };
  template <class V> class EdgeSplit<V, false, false> {
  public:
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {
      typename V::VPtr x = a->next(b);
      typename V::VPtr y = a->prev(b);

      typename V::VPtr v = (new V())->vptr();

      a->replace(b, v);
      b->replace(a, v);
      x->spliceNext(a, v);
      y->splicePrev(a, v);

      typename V::Neighbourhood n;
      n.push_back(a);
      n.push_back(y);
      n.push_back(b);
      n.push_back(x);
      v->nbAssign(n);

      return v;
    }    
  };
}

#endif
