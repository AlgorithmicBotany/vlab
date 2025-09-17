#ifndef __ALGEBRA__ABSTRACTVERTEX_HPP__
#define __ALGEBRA__ABSTRACTVERTEX_HPP__

#include <iostream>
#include <utility>
#include <list>
#include <set>
#include <map>
#include <algorithm>

namespace algebra
{
  /** @brief Class for the abstract vertex.
    @param A type that encapsulates the properties of the vertex that
    are not relevant to the vv-algebra.

    This class provides the algebraic functionality for the abstract
    vertex.  Vertex properties not relevant to the algebra are
    contained in the PosVertex templated type.  Memory management of
    the vertices is handled via the smart pointers provided.
    */
  template <class PosVertex, class Edge>
   class AbstractVertex
     {
   public:
    typedef PosVertex position_type;
    typedef Edge      edge_type;

    // public types
    /** @brief A memory management container.

      The VOwner class is a container that abstract vertices add
      themselves to when they are created.  The class is used to
      ensure that all vertices are destroyed when they need to be.
      This will prevent memory leaks when vertices in a shared
      object are dynamicaly unloaded from memory.  This class is
      staticaly allocated to give a simple singleton functionality.
      */
    class VOwner
      {
      std::set<AbstractVertex<PosVertex, Edge>*> vertices;
      bool isActive;
    public:
      /** @brief Constructor */
      VOwner() : isActive(true) {}

      /** @brief Destructor. */
      ~VOwner()
        {
        clear();
        }

      /** @brief Add a vertex. */
      void add(AbstractVertex<PosVertex, Edge>* v)
        {
        vertices.insert(v);
        }

      /** @brief Remove a vertex. */
      void remove(AbstractVertex<PosVertex, Edge>* v)
        {
        vertices.erase(v);
        }

      /** @brief Destroy all vertices. */
      void clear()
        {
        isActive = false;
        for(typename std::set<AbstractVertex<PosVertex, Edge>*>::iterator i = vertices.begin(); i != vertices.end(); ++i)
          {
          delete *i;
          }
        }

      /** @brief Check if the memory pool is active. */
      bool active() {return isActive;}
      };
    static VOwner vowner;

   public:

    /** @brief A smart pointer for abstract vertices.

      The following smart pointer uses reference counting to destroy
      vertices as soon as they are no longer referenced.
      */
    class VPtr
      {
      mutable AbstractVertex<PosVertex, Edge>* ptr;

    public:
      /** @brief Default constructor */
      VPtr() : ptr(0) {}

      /** @brief Copy constructor from raw pointer. */
      VPtr(AbstractVertex<PosVertex, Edge>* p) : ptr(p)
        {
        if (ptr) ++(ptr->refcount);
        }

      /** @brief Copy constructor from another smart pointer. */
      VPtr(const VPtr& p) : ptr(p.ptr)
        {
        if (ptr) ++(ptr->refcount);
        }

      /** @brief Destructor */
      ~VPtr()
        {
        if (vowner.active())
          {
          if (ptr && --(ptr->refcount) == 0)
            {
            ptr->vowner.remove(ptr);
            delete ptr;
            }
          }
        }

      void loopStart();
      bool loopNotDone();
      void loopNext();
      VPtr getCurrent();

      /** @brief Pointer dereference */
      inline AbstractVertex<PosVertex, Edge>& operator*() const
        {
        return *ptr;
        }

      /** @brief Member dereference */
      inline AbstractVertex<PosVertex, Edge>* operator->() const
        {
        if (!ptr)
          {
          std::cerr << "Fatal Error: Dereference of null pointer attempted." << std::endl;
          throw 0;
          }
        return ptr;
        }

      /** @brief Return a raw pointer */
      inline AbstractVertex<PosVertex, Edge>* raw() const
        {
        return ptr;
        }

      /** @brief Conversion to bool to check if the pointer is not null. */
      inline operator bool()
        {
        return (reinterpret_cast<void*>(ptr) != 0);
        }

      /** @brief Check if the pointer is null. */
      inline bool operator!() const
        {
        return (reinterpret_cast<void*>(ptr) == 0);
        }

      /** @brief Check if two smart pointers refer to the same vertex. */
      inline bool operator==(const VPtr& v) const
        {
        if (!ptr || !v)
          {
          std::cerr << "Fatal Error: Dereference of null pointer attempted." << std::endl;
          throw 0;
          }
        return (ptr == v.ptr);
        }

      /** @brief Check if two smart pointers do not refer to the same vertex. */
      inline bool operator!=(const VPtr& v) const
        {
        if (!ptr || !v)
          {
          std::cerr << "Fatal Error: Dereference of null pointer attempted." << std::endl;
          throw 0;
          }
        return (ptr != v.ptr);
        }

      /** @brief Compare the order of two vertices */
      inline bool operator< (const VPtr& v) const
        {
        if (!ptr || !v)
          {
          std::cerr << "Fatal Error: Dereference of null pointer attempted." << std::endl;
          throw 0;
          }
        return (ptr < v.ptr);
        }

      /** @brief Compare the order of two vertices */
      inline bool operator> (const VPtr& v) const
        {
        if (!ptr || !v)
          {
          std::cerr << "Fatal Error: Dereference of null pointer attempted." << std::endl;
          throw 0;
          }
        return (ptr > v.ptr);
        }

      /** @brief Compare the order of two vertices */
      inline bool operator<= (const VPtr& v) const
        {
        if (!ptr || !v)
          {
          std::cerr << "Fatal Error: Dereference of null pointer attempted." << std::endl;
          throw 0;
          }
        return (ptr <= v.ptr);
        }

      /** @brief Compare the order of two vertices */
      inline bool operator>= (const VPtr& v) const
        {
        if (!ptr || !v)
          {
          std::cerr << "Fatal Error: Dereference of null pointer attempted." << std::endl;
          throw 0;
          }
        return (ptr >= v.ptr);
        }

      /** @brief Assign a vertex from one smart pointer to another. */
      VPtr& operator= (const VPtr& v)
        {
        if (ptr) --(ptr->refcount);
        ptr = v.ptr;
        if (!ptr) ptr = 0;
        else ++(ptr->refcount);
        return *this;
        }

      /** @brief Assymmetric edge access */
      Edge& operator^(VPtr& v)
        {
        ptr->synchEdges();
        return ptr->getEdge(v);
        }

      /** @brief Symmetric edge access */
      Edge& operator|(VPtr& v)
        {
        ptr->synchEdges();
        v->addSynchEdge(*this);
        return ptr->getEdge(v);
        }
      };

    friend class VPtr;

    /** @brief Neighbourhood function.

      A base class to iterate a function over a vertex neighbourhood.
      */
    class NFunc
      {
    public:
     NFunc() {}
     virtual ~NFunc() {}
     virtual void operator()(VPtr v, VPtr neighbour) = 0;
      };

    // Neighbourhood types
    typedef typename std::list<std::pair<VPtr, Edge> > Neighbourhood;
    typedef typename Neighbourhood::iterator           NIter;

    // Construction and Deruction
    AbstractVertex();
    AbstractVertex(unsigned int label);
    virtual ~AbstractVertex();

    // Positional operations
    PosVertex&   getPosition();

    // Edge operations
    Edge& getEdge(const VPtr& v);
    bool  isNullEdge(const VPtr& v);
    void  addSynchEdge(VPtr& v);
    void  synchEdges();

    // Vertex Queries
    unsigned int getLabel();
    bool         operator==(const AbstractVertex& v) const;
    bool         operator< (const AbstractVertex& v) const;
    VPtr         vptr();

    // Neighbourhood Queries;
    unsigned int getNeighbourCount();
    bool         in(VPtr& v);
    VPtr         next(VPtr& v);
    VPtr         next_flagged();
    VPtr         prev(VPtr& v);
    VPtr         prev_flagged();
    VPtr         next(VPtr& v, unsigned int k);
    VPtr         next_flagged(unsigned int k);
    VPtr         prev(VPtr& v, unsigned int k);
    VPtr         prev_flagged(unsigned int k);
    VPtr         select();
    VPtr         flagged();

    // Neighbourhood Operations
    void remove(VPtr& target);
    void remove_flagged();
    void replace(VPtr& target, VPtr& v);
    void replace_flagged(VPtr& v);
    void spliceNext(VPtr& target, VPtr& v);
    void spliceNext_flagged(VPtr& v);
    void splicePrev(VPtr& target, VPtr& v);
    void splicePrev_flagged(VPtr& v);
    void flag(VPtr& target);

    void forEachNeighbour(NFunc& f);
    void recurseForEachNeighbour(NFunc& f);

    /** @brief Reset the label counter.

      This function is only used internally by the library.  It
      should not be generaly used.
      */
    static void resetLabels()
      {
      nextlabel = 0;
      }

    /** @brief Reassign a vertex label.

      This function is only used internally by the library.  It
      should not be generaly used.
      */
    void relabel(unsigned int l) {label = l;}

    // state synch
    void synchronise();
    VPtr getOld();
    void restore();

    // neighbourhood building
    void nbClear();
    void nbAssign(Neighbourhood& nba);

    // cloning
    VPtr clone();

   private:
    unsigned int        label;
    static unsigned int nextlabel;
    unsigned int        refcount;

    Neighbourhood       neighbours;
    Neighbourhood       old_neighbours;
    Neighbourhood*      nb;

    PosVertex           position;
    PosVertex           old_position;

    NIter f_iter;
    NIter old_f_iter;

    bool old;

    NIter current;
    bool  looping_old;

    Edge            nulledge;
    std::set<VPtr>  esynch;
     };
}

template <class PosVertex, class Edge>
unsigned int algebra::AbstractVertex<PosVertex, Edge>::nextlabel = 0;

/** @brief Constructor. */
template <class PosVertex, class Edge>
algebra::AbstractVertex<PosVertex, Edge>::AbstractVertex() :
label(nextlabel),
 refcount(0),
 neighbours(),
 old_neighbours(),
 position(),
 old_position(),
 f_iter(),
 old_f_iter(),
 old(false),
 looping_old(false)
{
  vowner.add(this);
  nextlabel++;
  f_iter = neighbours.end();
  nb = &neighbours;
}

/** @brief Destructor. */
template <class PosVertex, class Edge>
algebra::AbstractVertex<PosVertex, Edge>::~AbstractVertex() {}

/** @brief Return the vertex properties. */
 template <class PosVertex, class Edge>
PosVertex& algebra::AbstractVertex<PosVertex, Edge>::getPosition()
{
  if (old)
    {
    restore();
    return old_position;
    }
  else return position;
}

/** @brief Edge access. */
 template <class PosVertex, class Edge>
Edge& algebra::AbstractVertex<PosVertex, Edge>::getEdge(const VPtr& v)
{
  Edge* ret = &nulledge;
  for (NIter i = nb->begin(); i != nb->end(); ++i)
    {
    if (i->first == v)
      {
      ret = &(i->second);
      break;
      }
    }
  restore();
  if (ret == &nulledge)
    {
    std::cerr << "Fatal error: Null edge was accessed." << std::endl;
    throw 0;
    }
  return *ret;
}

/** @brief Test for a null edge. */
 template <class PosVertex, class Edge>
bool algebra::AbstractVertex<PosVertex, Edge>::isNullEdge(const VPtr& v)
{
  Edge* ret = &nulledge;
  for (NIter i = nb->begin(); i != nb->end(); ++i)
    {
    if (i->first == v)
      {
      ret = &(i->second);
      break;
      }
    }
  restore();
  return (ret == &nulledge);
}

/** @brief Add an edge to the synchronisation list */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::addSynchEdge(VPtr& v)
{
  esynch.insert(v);
}

/** @brief Synchronise the edges */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::synchEdges()
{
  typename std::set<VPtr>::iterator i;
  for (i = esynch.begin(); i != esynch.end(); i++)
    {
    VPtr v = this->vptr();
    if ((*i)->in(v))
      getEdge(*i) = (*i)->getEdge(this->vptr());
    }
  esynch.clear();
}

/** @brief Get the vertex label. */
 template <class PosVertex, class Edge>
unsigned int algebra::AbstractVertex<PosVertex, Edge>::getLabel()
{
  return label;
}

/** @brief Check for equality */
template <class PosVertex, class Edge>
bool algebra::AbstractVertex<PosVertex, Edge>::operator==(const algebra::AbstractVertex<PosVertex, Edge>& v) const
{
  return label == v.label;
}

/** @brief Compare vertex order. */
template <class PosVertex, class Edge>
bool algebra::AbstractVertex<PosVertex, Edge>::operator<(const algebra::AbstractVertex<PosVertex, Edge>& v) const
{
  return label < v.label;
}

/** @brief Get a smart pointer to the vertex. */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::vptr()
{
  return VPtr(this);
}

/** @brief Get the number of vertices in the neighourhood. */
 template <class PosVertex, class Edge>
unsigned int algebra::AbstractVertex<PosVertex, Edge>::getNeighbourCount()
{
  int i = int(nb->size());
  restore();
  return i;
}

/** @brief Check if a vertex exists in the neighbourhood.
  @param v The vertex to search for.
  */
 template <class PosVertex, class Edge>
bool algebra::AbstractVertex<PosVertex, Edge>::in(VPtr& v)
{
  bool ret = false;
  for (NIter i = nb->begin(); i != nb->end(); i++)
    if (i->first == v)
      {
      ret = true;
      break;
      }
  restore();
  return ret;
}

/**
 * @brief Get the vertex after the target in the neighbourhood.
 * @param v The vertex to search for.
 */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::next(VPtr& v)
{
  NIter i;

  for (i = nb->begin() ; i != nb->end() ; ++i)
    if (i->first == v)
      {
      VPtr ret;
      if (++i == nb->end())
        ret = nb->front().first;
      else
        ret = i->first;

      restore();
      return ret;
      }

  restore();
  return VPtr();
}

/** @brief return the next after the flagged vertex. */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::next_flagged()
{
  if (f_iter == nb->end())
    return VPtr();
  NIter i = f_iter;
  i++;
  if (i == nb->end())
    i = nb->begin();
  restore();
  return i->first;
}

/** @brief Get the vertex before the target in the neighbourhood.
  @param v The target to search for.
  */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::prev(VPtr& v)
{
  for (NIter i = nb->begin(); i != nb->end(); ++i)
    if (i->first == v)
      {
      VPtr ret;
      if (i == nb->begin())
        ret = nb->back().first;
      else
        ret = (--i)->first;
      restore();
      return ret;
      }
  restore();
  return VPtr();
}

/** @brief return the previous to the flagged vertex. */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::prev_flagged()
{
  if (f_iter == nb->end())
    return VPtr();
  NIter i = f_iter;
  if (i == nb->begin())
    i = nb->end();
  i--;
  restore();
  return i->first;
}

/** @brief Get the k-th vertex after the target in the neighbourhood.
  @param v The vertex to search for.
  @param k Number of vertices to skip after v
  */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::next(VPtr& v, unsigned int k)
{
  for (NIter i = nb->begin(); i != nb->end(); ++i)
    if (i->first == v)
      {
      for (unsigned int j = 0; j < k; ++j)
        {
        if (++i == nb->end())
          i = nb->begin();
        }
      VPtr ret = i->first;
      restore();
      return ret;
      }
  restore();
  return VPtr();
}

/** @brief return the kth vertex after the flagged vertex.
  @param k Number of vertices to skip after the flagged
  */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::next_flagged(unsigned int k)
{
  if (!(*f_iter)) return VPtr();
  NIter i = f_iter;
  for (unsigned int j = 0; j < k; ++j)
    {
    ++i;
    if (i == nb->end())
      i = nb->begin();
    }
  restore();
  return i->first;
}

/** @brief Get the k-th vertex before the target in the neighbourhood.
  @param v The vertex to search for.
  @param k Number of vertices to skip after v
  */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::prev(VPtr& v, unsigned int k)
{
  for (NIter i = nb->begin(); i != nb->end(); ++i)
    if (i->first == v)
      {
      for (unsigned int j = 0; j < k; ++j)
        {
        if (i == nb->begin())
          i = nb->end();
        --i;
        }
      VPtr ret = i->first;
      restore();
      return ret;
      }

  restore();
  return VPtr();
}

/** @brief return the kth vertex after the flagged vertex.
  @param k Number of vertices to skip after the flagged
  */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::prev_flagged(unsigned int k)
{
  if (!(*f_iter)) return VPtr();
  NIter i = f_iter;
  for (unsigned int j = 0; j < k; ++j)
    {
    if (i == nb->begin())
      i = nb->end();
    --i;
    }
  restore();
  return i->first;
}

/** @brief Returns a vertex from the neighbourhood.
*/
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::select()
{
  VPtr ret;
  if (!nb->empty())
    ret = nb->front().first;
  restore();
  return ret;
}

/** @brief Returns the flagged vertex. */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::flagged()
{
  if (f_iter != nb->end())
    return f_iter->first;
  else
    return VPtr();
}

/** @brief Removes a vertex from the neighbourhood.
  @param target The vertex to be removed.
  */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::remove(VPtr& target)
{
  restore();
  synchEdges();
  for (NIter i = neighbours.begin(); i != neighbours.end(); ++i)
    {
    if (i->first == target)
      {
      bool delete_current = false;
      if (i == f_iter)
        f_iter = neighbours.end();
      if (i == current)
        delete_current = true;
      NIter next = neighbours.erase(i);
      if( delete_current )
        current = next;
      break;
      }
    }
}

/** @brief Remove the flagged vertex. */
 template  <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::remove_flagged()
{
  restore();
  synchEdges();
  if (f_iter == neighbours.end()) return;
  remove(f_iter->first);
  f_iter = neighbours.end();
}

/** @brief Replace a vertex in the neighbourhood.
  @param target The vertex to be replaced.
  @param v The new vertex that replace the target.
  */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::replace(VPtr& target, VPtr& v)
{
  restore();
  synchEdges();
  for (NIter i = neighbours.begin(); i != neighbours.end(); ++i)
    {
    if (i->first == target)
      {
      i->first = v;
      break;
      }
    }
}

/** @brief replaced the flagged vertex with v.
  @param v The new vertex that replace the target.
  */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::replace_flagged(VPtr& v)
{
  restore();
  synchEdges();
  if (f_iter != neighbours.end())
    f_iter->first = v;
}

/** @brief Insert a vertex after another in the neighbourhood.
  @param target The vertex before the place where the new vertex is inserted.
  @param v The vertex to be inserted.
  */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::spliceNext(VPtr& target, VPtr& v)
{
  restore();
  for (NIter i = neighbours.begin(); i != neighbours.end(); ++i)
    {
    if (i->first == target)
      {
      neighbours.insert(++i, std::make_pair(v, Edge()));
      return;
      }
    }
  neighbours.push_back(std::make_pair(v, Edge()));
}

/** @brief Insert a vertex after the flagged vertex
  @param v The vertex to be inserted.
  */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::spliceNext_flagged(VPtr& v)
{
  restore();
  if (f_iter != neighbours.end())
    {
    NIter i = f_iter;
    neighbours.insert(++i, std::make_pair(v, Edge()));
    }
}

/** @brief Insert a vertex before another in the neighbourhood.
  @param target The vertex after the place where the new vertex is inserted.
  @param v The vertex to be inserted.
  */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::splicePrev(VPtr& target, VPtr& v)
{
  restore();
  for (NIter i = neighbours.begin(); i != neighbours.end(); ++i)
    {
    if (i->first == target)
      {
      neighbours.insert(i, std::make_pair(v, Edge()));
      return;
      }
    }
  neighbours.push_back(std::make_pair(v, Edge()));
}

/** @brief Insert a vertex after the flagged vertex
  @param v The vertex to be inserted.
  */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::splicePrev_flagged(VPtr& v)
{
  restore();
  if (f_iter != neighbours.end())
    neighbours.insert(f_iter, std::make_pair(v, Edge()));
}

/** @brief Sets the flagged vertex.
  @param target The vertex to flag.

  If target does not exist in the neighbourhood or targe is null, then
  the function has no effect.
  */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::flag(VPtr& target)
{
  restore();
  for (NIter i = neighbours.begin(); i != neighbours.end(); ++i)
    {
    if (i->first == target)
      {
      f_iter = i;
      break;
      }
    }
}

/** @brief Application of a function to each vertex in the neighbourhood.
  @param f A function inherited from NFunc.
  */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::forEachNeighbour(NFunc& f)
{
  for (NIter i = neighbours.begin(); i != neighbours.end(); ++i)
    f(vptr(), i->first);
}

/** @brief Recursive application of a function to each vertex in the neighbourhood.
  @param f A function inherited from NFunc.
  */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::recurseForEachNeighbour(NFunc& f)
{
  for (NIter i = nb->begin(); i != nb->end(); ++i)
    i->first.forEachNeighbour(f);
}

/**
 * @brief Record the state of the vertex.
 */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::synchronise()
{
  restore();
  old_neighbours.clear();
  old_neighbours = neighbours;
  old_position = position;
  old_f_iter = f_iter;
}

/** @brief Restore the current state of the vertex. */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::restore()
{
  if (old)
    {
    nb = &neighbours;
    std::swap(position, old_position);
    std::swap(f_iter, old_f_iter);

    old = false;
    }
}

/** @brief Activate the old version of the vertex state. */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::getOld()
{
  if (!old)
    {
    nb = &old_neighbours;
    std::swap(position, old_position);
    std::swap(f_iter, old_f_iter);

    old = true;
    }
  return vptr();
}

/** @brief Remove all vertices from the neighbourhood. */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::nbClear()
{
  restore();
  old_neighbours.clear();
  neighbours.clear();
}

/** @brief Assign a new neighbourhood to the vertex. */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::nbAssign(Neighbourhood& nba)
{
  restore();
  neighbours.clear();
  neighbours = nba;
}

/** @brief Start the neighbourhood iteration. */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::VPtr::loopStart()
{
  ptr->current = ptr->nb->begin();
  ptr->looping_old = ptr->old;
  ptr->restore();
}

/** @brief Check if the neighbourhood iteration is not complete. */
 template <class PosVertex, class Edge>
bool algebra::AbstractVertex<PosVertex, Edge>::VPtr::loopNotDone()
{
  bool ret = (ptr->current != ptr->nb->end());
  ptr->restore();
  return ret;
}

/** @brief Advance the neighbourhood iteration. */
 template <class PosVertex, class Edge>
void algebra::AbstractVertex<PosVertex, Edge>::VPtr::loopNext()
{
  ++(ptr->current);
}

/** @brief Get the current vertex of neighbourhood iteration. */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::VPtr::getCurrent()
{
  return ptr->current->first;
}

/** @Brief Clone the vertex. */
 template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VPtr algebra::AbstractVertex<PosVertex, Edge>::clone()
{
  VPtr v(new AbstractVertex<PosVertex, Edge>());
  v->neighbours = neighbours;
  v->position = position;
  return v;
}

template <class PosVertex, class Edge>
typename algebra::AbstractVertex<PosVertex, Edge>::VOwner algebra::AbstractVertex<PosVertex, Edge>::vowner;

#endif
