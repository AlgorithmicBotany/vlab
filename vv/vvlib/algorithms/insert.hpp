#ifndef __ALGORITHMS_INSERT_HPP__
#define __ALGORITHMS_INSERT_HPP__

#include <utility>
#include <algebra/abstractvertex.hpp>

namespace algorithms {

  /** @brief Insert a new vertex on an edge.
      @param a an existing vertex
      @param b an existing vertex

      This function creates a new vertex and inserts it between two existing
      vertices by replacing it into the existing neighbourhoods.  If the vertices
      have no relation or an assymmetric relation, a warning is printed to stderr.
   */
  template <class V, bool do_checks = true> class Insert {
  public:
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {
      std::cerr << "Error: Insert stub operator entered." << std::endl;
      throw 0;
    }
  };
  template <class V> class Insert<V, true> {
  public:
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {    
      if (!a || !b) {
	std::cerr << "Error: Attempt to insert next to a null vertex." << std::endl;
	throw 0;
      }

      unsigned int check = 0;
      if (a->in(b)) check++;
      if (b->in(a)) check++;

      switch (check) {
      case 0: std::cerr << "Warning: Attempt to insert a vertex between vertices that have no relation." << std::endl;
	return typename V::VPtr();
	break;
      case 1: std::cerr << "Warning: Attempt to insert a vertex between vertices that have an assymetric relation." << std::endl;
	return typename V::VPtr();
	break;
      default: break;
      }

      typename V::VPtr x = (new V())->vptr();

      a->replace(b, x);
      b->replace(a, x);

      typename V::Neighbourhood nb;
      nb.push_back(std::make_pair(a, typename V::edge_type()));
      nb.push_back(std::make_pair(b, typename V::edge_type()));
      x->nbAssign(nb);

      return x;
    }
  };
  template <class V> class Insert<V, false> {
  public:
    typename V::VPtr operator()(typename V::VPtr& a, typename V::VPtr& b) {    
      typename V::VPtr x = (new V())->vptr();

      a->replace(b, x);
      b->replace(a, x);

      typename V::Neighbourhood nb;
      nb.push_back(std::make_pair(a, typename V::edge_type()));
      nb.push_back(std::make_pair(b, typename V::edge_type()));
      x->nbAssign(nb);

      return x;
    }
  };
}

#endif
