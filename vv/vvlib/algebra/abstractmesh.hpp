#ifndef __ALGEBRA__ABSTRACTMESH_HPP__
#define __ALGEBRA__ABSTRACTMESH_HPP__

#include <set>

namespace algebra {
  template <class V>

  /** @brief The base class for handling vertex sets.

      This class is used as the base class for vertex sets in the vv
      algebra.  It provides all the basic functionality required of
      the sets to implement the algebra.  However, the inherited class
      XMLMesh is more useful in programs as it adds file I/O
      capabilities.
  */
  class AbstractMesh {
  public:
    typedef typename V::VPtr VPtr;

    /** @brief A base class for functions.

        Functions inherited from VFunc can be applied to each vertex
        in a particular set.
    */
    class VFunc {
    public:
      virtual void operator()(VPtr v) = 0;
    };

    AbstractMesh();
    virtual ~AbstractMesh();

    VPtr createVertex();
    void addVertex(const VPtr& v);
    void removeVertex(const VPtr& v);
    void clear();
    void merge(AbstractMesh& m);

    void forEachVertex(VFunc& f);

    unsigned int vertexCount() ;
    bool         in(const VPtr& v);

    VPtr select();

    void synchronise();

    void loopStart();
    bool loopNotDone();
    void loopNext();
    VPtr getCurrent();

    virtual AbstractMesh<V>& operator=(const AbstractMesh<V>& m);

  private:
    typedef typename std::set<VPtr>       VSet;
    typedef typename VSet::iterator       VIter;

    VSet  vertices;
    VIter current;
    bool  removed_first;
    bool  looping;
  };
}

/** @brief Constructor */
template <class V> algebra::AbstractMesh<V>::AbstractMesh() :
  removed_first(false),
  looping(false)
{
  current = vertices.begin();
}

/** @brief Destructor */
template <class V> algebra::AbstractMesh<V>::~AbstractMesh() {}

/** @brief Allocate a new vertex and add it to the set. */
template <class V>
typename algebra::AbstractMesh<V>::VPtr algebra::AbstractMesh<V>::createVertex() {
  V* vp = new V();
  VPtr v = vp->vptr();

  vertices.insert(v);
  return v;
}

/** @brief Add a vertex to the set.
    @param v The vertex to be inserted.

    If v points to null or already exists in the set, then nothing
    happens.
*/
template <class V>
void algebra::AbstractMesh<V>::addVertex(const VPtr& v) {
  if (!v) return;
  vertices.insert(v);
}

/** @brief Remove a specified vertex from the set.
    @param v The vertex to be removed.

    If v does not exist in the set, nothing happens.
*/
template <class V>
void algebra::AbstractMesh<V>::removeVertex(const VPtr& v) {
  if (looping) {
    if (!v) return;
    if (v == *current) {
      if (current == vertices.begin())
	removed_first = true;
      else
	--current;
    }
  }
  vertices.erase(v);
}

/** @brief Remove all vertices from the set. */
template <class V>
void algebra::AbstractMesh<V>::clear() {
  vertices.clear();
}

/** @brief Add all the vertices from another set into this one.
    @param m The supplied set.

    After this function, all the vertices in m and the current set
    exist exactly once in the current set.  No changer are made to m.
*/
template <class V>
void algebra::AbstractMesh<V>::merge(algebra::AbstractMesh<V>& m) {
  vertices.insert(m.vertices.begin(), m.vertices.end());
}

/** @brief Execute the f on each vertex in the mesh.
    @param f A function object inherited from VFunc.
*/
template <class V>
void algebra::AbstractMesh<V>::forEachVertex(VFunc& f) {
  for (VIter i = vertices.begin(); i != vertices.end(); ++i)
    f(*i);
}

/** @brief Return the number of vertices in the set. */
template <class V>
unsigned int algebra::AbstractMesh<V>::vertexCount()  {
  typedef unsigned int ui;
  return ui(vertices.size());
}

/** @brief Check if a vertex is contained in the set. */
template <class V>
bool algebra::AbstractMesh<V>::in(const VPtr& v) {
  return (vertices.find(v) != vertices.end());
}

template <class V>
typename V::VPtr algebra::AbstractMesh<V>::select() {
  if (vertexCount())
    return *(vertices.begin());
  else
    return typename V::VPtr();
}

/** @brief Record the current state of each vertex in the set. */
template <class V>
void algebra::AbstractMesh<V>::synchronise() {
  for (VIter i = vertices.begin(); i != vertices.end(); ++i)
    (*i)->synchronise();
}

/** @brief Start the iteration.

    This function exists for providing iteration in the generated code.
*/
template <class V>
void algebra::AbstractMesh<V>::loopStart() {
  current = vertices.begin();
  looping = true;
}

/** @brief Get the current vertex in the iteration.

    This function exists for providing iteration in the generated code.
*/
template <class V>
typename algebra::AbstractMesh<V>::VPtr algebra::AbstractMesh<V>::getCurrent() {
  return *current;
}

/** @brief Check if there is more vertices to iterate over.

    This function exists for providing iteration in the generated code.
*/
template <class V>
bool algebra::AbstractMesh<V>::loopNotDone() {
  bool done = (current == vertices.end());
  if (done) looping = false;
  return !done;
}

/** @brief Advance the iteration.

    This function exists for providing iteration in the generated code.
*/
template <class V>
void algebra::AbstractMesh<V>::loopNext() {
  if (removed_first) {
    loopStart();
    removed_first = false;
  }
  else
    ++current;
}

/** @brief Copy a vertex set.
    @param m The supplied vertex.

    This function copies the contents of the vertex set of m to the
    current one.  The current set is overwritten.
*/
template <class V>
algebra::AbstractMesh<V>& algebra::AbstractMesh<V>::operator=(const algebra::AbstractMesh<V>& m) {
  vertices.clear();
  vertices = m.vertices;
  current = vertices.begin();
  return *this;
}

#endif
