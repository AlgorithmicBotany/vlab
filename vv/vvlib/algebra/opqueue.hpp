#ifndef __ALGEBRA_OPQUEUE_HPP__
#define __ALGEBRA_OPQUEUE_HPP__

#include <cstdarg>
#include <algorithm>

#include <algebra/abstractvertex.hpp>

namespace algebra {
  /* unsigned ints are used so that the ellipsis declaration
     in path() can be used.
  */
  const unsigned int opStop = 0;
  const unsigned int opNext = 1;
  const unsigned int opPrev = 2;
  const unsigned int opSwap = 3;

  /** @brief Encapsulation of paths. */
  template <class V>
  class OpQueue {
  public:
    /** @brief Constructor
        @param use_old If true, the old states of the neighbourhoods are
                       used to execute the path.
    */
    OpQueue(bool use_old) : use_old(use_old) {};

    /** @brief Add an operation to the tail of the path.
        @param id One of opStop, opNext, opPrev or opSwap.

        If an unknown operation ID is used, no addition is made.
    */
    void push(unsigned int id) {
      switch(id) {
      case opNext:
      case opPrev:
      case opSwap:
	operations.push_back(id);
	break;
      default: break;
      }
    }

    /** @brief Execute the path on a pair of vertices.
        @param first The first vertex in the pair.
        @param second The second vertex in the pair.
    */
    typename V::VPtr exec(typename V::VPtr first, typename V::VPtr second) {
      if (use_old) {
	for (iter = operations.begin(); iter != operations.end(); ++iter) {
	  switch(*iter) {
	  case opNext: {
	    second = first->getOld()->next(second);
	    break;
	  }
	  case opPrev: {
	    second = first->getOld()->prev(second);
	    break;
	  }
	  case opSwap: {
	    typename V::VPtr t = first;
	    first = second;
	    second = t;
	    break;
	  }
	  case opStop:
	    return second;
	    break;
	  }
	}
      }
      else {
	for (iter = operations.begin(); iter != operations.end(); ++iter) {
	  switch(*iter) {
	  case opNext: {
	    second = first->next(second);
	    break;
	  }
	  case opPrev: {
	    second = first->prev(second);
	    break;
	  }
	  case opSwap: {
	    typename V::VPtr t = first;
	    first = second;
	    second = t;
	    break;
	  }
	  case opStop:
	    return second;
	    break;
	  }
	}
      }
      return second;
    }
  private:
    bool use_old;
    std::list<unsigned int> operations;
    std::list<unsigned int>::iterator iter;
  };

  /** @brief A wrapper function to declare and execute a path.
      @param old If true, the old neighbourhoods are used.
      @param a   The first vertex in the pair.
      @param b   The second vertex in the pair.
      @param ... The list of operation parameters.
  */
  template <class V>
  typename V::VPtr path(bool old, typename V::VPtr & a, typename V::VPtr& b, unsigned int op ...) {
    va_list ap;
    va_start(ap, op);

    OpQueue<V> opPath(old);
    opPath.push(op);

    unsigned int id = opStop;
    do {
      id = va_arg(ap, int);
      opPath.push(id);
    } while(id != opStop);

    va_end(ap);

    return opPath.exec(a, b);
  }

  /** @brief Encapsulation of referenced paths. */
  template <class V>
  class OpRefQueue {
  public:
    OpRefQueue(bool use_old) : use_old(use_old) {};

    void push(unsigned int id) {
      switch(id) {
      case opNext:
      case opPrev:
      case opSwap:
	operations.push_back(id);
	break;
      default: break;
      }
    }

    typename V::VPtr exec(typename V::VPtr& first, typename V::VPtr& second) {
      for (iter = operations.begin(); iter != operations.end(); ++iter) {
	switch(*iter) {
	case opNext: {
          if (use_old)
	    second = first->getOld()->next(second);
          else
	    second = first->next(second);
	  break;
	}
	case opPrev: {
          if (use_old)
	    second = first->getOld()->prev(second);
          else
	    second = first->prev(second);
	  break;
	}
	case opSwap: {
	  typename V::VPtr t = first;
	  first = second;
	  second = t;
	  break;
	}
	case opStop:
	  return second;
	  break;
	}
      }
      return second;
    }
  private:
    bool use_old;
    std::list<unsigned int> operations;
    std::list<unsigned int>::iterator iter;
  };

  /** @brief A wrapper function to declare and execute a referenced path.
      @param old If true, the old neighbourhoods are used.
      @param a   The first vertex in the pair.
      @param b   The second vertex in the pair.
      @param ... The list of operation parameters.
  */
  template <class V>
  typename V::VPtr pathref(bool old, typename V::VPtr& a, typename V::VPtr& b, unsigned int op ...) {
    va_list ap;
    va_start(ap, op);

    OpRefQueue<V> opPath(old);
    opPath.push(op);

    unsigned int id = opStop;
    do {
      id = va_arg(ap, int);
      opPath.push(id);
    } while(id != opStop);

    va_end(ap);

    return opPath.exec(a, b);
  }
}

#endif
