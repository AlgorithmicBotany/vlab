#ifndef __ALGEBRA__XMLMESH_HPP__
#define __ALGEBRA__XMLMESH_HPP__

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <list>
#include <map>

//#include <qxml.h>
//#include <qfile.h>
//#include <qstring.h>

#include <algebra/abstractmesh.hpp>
#include <algebra/abstractvertex.hpp>

namespace algebra {
template <class V> class XMLMesh;

/** @brief Abstract base class for parsing vvm files. */
template <class V>
class AbstractXMLMeshParser : { //public QXmlDefaultHandler {
  public:
    AbstractXMLMeshParser(XMLMesh<V>* pMesh);
    virtual ~AbstractXMLMeshParser();

    virtual bool startDocument();
    //virtual bool startElement(const QString&, const QString&, const QString&, const QXmlAttributes&);
    virtual bool startElement(const std::string&, const std::string&, const std::string&);
    virtual bool endDocument();

  protected:
    virtual void processMeshAttribute(XMLMesh<V>*, std::string name, std::istringstream& is) = 0;
    virtual void processVertexAttribute(typename AbstractMesh<V>::VPtr& v, std::string name, std::istringstream& is) = 0;
    virtual void processEdgeAttribute(typename V::edge_type& e, std::string name, std::istringstream& is) = 0;
    XMLMesh<V>*  pMesh;

    std::map<unsigned int, typename AbstractMesh<V>::VPtr> newVertices;
    std::map<unsigned int, std::list<unsigned int> >       newNeighbourhoods;

    struct Edge {
      Edge(unsigned int f, unsigned int s, bool sym, typename V::edge_type e) :
        first(f),
        second(s),
        symmetric(sym),
        e(e)
      {}

      unsigned int          first;
      unsigned int          second;
      bool                  symmetric;
      typename V::edge_type e;
    };

    std::list<Edge> newEdges;

    bool         reset_vbase;
    unsigned int vbase;
};

/** @brief A function to write vertex neighbourhoods to vvm files. */
template <class V>
class AbstractXMLWriteNeighbourhood : public V::NFunc {
  public:
    void operator()(typename V::VPtr v, typename V::VPtr n) {
      *os << " " << n->getLabel();
    }
    std::ostream* os;
};

/** @brief A function to write the edges incident to a vertex */
template <class V>
class AbstractXMLWriteEdges : public V::NFunc {
  public:
    void operator()(typename V::VPtr v, typename V::VPtr n) = 0;
    std::ostream* os;
};

/** @brief A function to write vvm files */
template <class V>
class AbstractXMLMeshWriter : public AbstractMesh<V>::VFunc {
  public:
    virtual ~AbstractXMLMeshWriter() {}
    virtual void operator()(typename AbstractMesh<V>::VPtr v) = 0;
    AbstractXMLWriteNeighbourhood<V> write_neighbourhood;
    std::ostream* os;
};

/** @brief A function to write the edges in vvm files */
template <class V>
class AbstractXMLMeshEdgeWriter : public AbstractMesh<V>::VFunc {
  public:
    virtual ~AbstractXMLMeshEdgeWriter() {}
    virtual void operator()(typename AbstractMesh<V>::VPtr v) {
      v->forEachNeighbour(*write_edges);
    }
    AbstractXMLWriteEdges<V>* write_edges;
};

/** @brief A vertex set class that reads and writes to vvm files. */
template <class V>
class XMLMesh : public AbstractMesh<V> {
  public:
    XMLMesh();
    virtual ~XMLMesh();

    void readXMLFile(std::string filename);
    void writeXMLFile(std::string filename, bool write_edges = false);
    void printXMLFile(bool write_edges = false);

    virtual void writeMeshAttributes(std::ostream& os) = 0;

    void setParser(AbstractXMLMeshParser<V>* pParser);
    void setWriter(typename algebra::XMLMesh<V>::VFunc * pWriter);
    void setEWriter(typename algebra::XMLMesh<V>::VFunc * pEWriter);

  protected:
    AbstractXMLMeshParser<V>*     pParser;
    AbstractXMLMeshWriter<V>*     pWriter;
    AbstractXMLMeshEdgeWriter<V>* pEWriter;
};
}

/** @breif Constructor */
template <class V>
algebra::AbstractXMLMeshParser<V>::AbstractXMLMeshParser(XMLMesh<V>* pMesh) :
  pMesh(pMesh),
  reset_vbase(true),
  vbase(0)
{}

/** @brief Destructor */
template <class V>
algebra::AbstractXMLMeshParser<V>::~AbstractXMLMeshParser() {}

/** @brief Clears the mesh when a new vvm file is read. */
template <class V>
bool algebra::AbstractXMLMeshParser<V>::startDocument() {
  if (pMesh) {
    reset_vbase = true;
    pMesh->clear();
    return true;
  }
  return false;
}

/** @brief Handle the data for each element in the vvm file.

  For elements name 'mesh' and 'v', the attributes are handled by
  the inherited attribute processing functions.  For the 'v'
  element, a special attribute 'nb' is specially handled here to
  read the vertex neighbourhoods.  Elements that are not named
  'mesh' or 'v' are ignored.
  */
template <class V>
bool algebra::AbstractXMLMeshParser<V>::startElement(
                                                     const std::string& namespaceURI,
                                                     const std::string& localName,
                                                     const std::string& qName,
                                                    )//const QXmlAttributes& atts)
{
  if (!pMesh) return false;
  if (qName == "mesh") {
    //for (int i = 0; i < atts.length(); i++) {
    //  std::istringstream is(atts.value(i).toStdString());
    //  processMeshAttribute(pMesh, atts.qName(i).toStdString(), is);
    //}
  }
  else if (qName == "e") {
    typename V::edge_type e;
    unsigned int first = 0;
    unsigned int second = 0;
    bool         symmetric = false;
    // for (int i = 0; i < atts.length(); i++) {
    //   std::istringstream is(atts.value(i).toStdString());
    //   if (atts.qName(i) == "first") {
    //     is >> std::ws >> first >> std::ws;
    //   }
    //   else if (atts.qName(i) == "second") {
    //     is >> std::ws >> second >> std::ws;
    //   }
    //   else if (atts.qName(i) == "symmetric") {
    //     is >> std::ws >> symmetric >> std::ws;
    //   }
    //   else {
    //     processEdgeAttribute(e, atts.qName(i).toStdString(), is);
    //   }
    // }
    // newEdges.push_back(Edge(first + vbase, second + vbase, symmetric, e));
  }
  else if (qName == "v") {
    typename XMLMesh<V>::VPtr v = pMesh->createVertex();
    if (reset_vbase) {
      vbase = v->getLabel();
      reset_vbase = false;
    }
    newVertices[v->getLabel()] = v;
    // for (int i = 0; i < atts.length(); i++) {
    //   std::istringstream is(atts.value(i).toStdString());
    //   if (atts.qName(i) == "nb") {
    //     std::list<unsigned int> neighbours;

    //     while (!is.eof()) {
    //       unsigned int label;
    //       is >> std::ws >> label >> std::ws;
    //       neighbours.push_back(label + vbase);
    //     }

    //     newNeighbourhoods[v->getLabel()] = neighbours;
    //   }
    //   else {
    //     processVertexAttribute(v, atts.qName(i).toStdString(), is);
    //   }
    // }
  }
  return true;
}

/** @brief Finishes the data processing at the end of the vvm file. */
template <class V>
bool algebra::AbstractXMLMeshParser<V>::endDocument() {
  using namespace std;

  int count = int(newVertices.size());
  for (typename map<unsigned int, typename AbstractMesh<V>::VPtr >::iterator i = newVertices.begin(); i != newVertices.end(); i++) {
    if (!count) break;
    count--;

    list<unsigned int> l = newNeighbourhoods[i->first];
    typename AbstractMesh<V>::VPtr prev;

    for (list<unsigned int>::iterator index = l.begin(); index != l.end(); index++) {
      i->second->spliceNext(prev, newVertices[*index]);
      prev = newVertices[*index];
    }
  }

  for (typename list<Edge>::iterator i = newEdges.begin(); i != newEdges.end(); ++i) {
    if (i->symmetric)
      (newVertices[i->first] | newVertices[i->second]) = i->e;
    else
      (newVertices[i->first] ^ newVertices[i->second]) = i->e;
  }

  return true;
}

/** @brief Constructor. */
template <class V>
algebra::XMLMesh<V>::XMLMesh() :
  AbstractMesh<V>(),
  pParser(0),
  pWriter(0),
  pEWriter(0)
{}

/** @brief Destructor. */
template <class V> algebra::XMLMesh<V>::~XMLMesh() {}

/** @brief Read a vvm file.
  @param filename The vvm file to read.
  */
template <class V>
void algebra::XMLMesh<V>::readXMLFile(std::string filename) {
  if (!pParser) return;
  std::cerr << "readXMLFile is not implemented yet.\n";
  // QXmlSimpleReader reader;
  // reader.setContentHandler(pParser);
  // QFile* file = new QFile(filename.c_str());
  // reader.parse(QXmlInputSource(file));
}

/** @brief A function to relabel the vertices.

  This function should only be used internally by the library.
  */
template <class V>
class Relabel : public algebra::AbstractMesh<V>::VFunc {
  unsigned int nextlabel;
  public:
  Relabel(unsigned int start = 0) : nextlabel(start) {}
  void operator()(typename algebra::AbstractMesh<V>::VPtr v) {
    v->relabel(nextlabel++);
  }
};

/** @brief A function to write vvm files.
  @param filename The name of the file to write to.
  */
template <class V>
void algebra::XMLMesh<V>::writeXMLFile(std::string filename, bool write_edges) {
  using namespace std;

  if (!pWriter) return;

  ofstream out(filename.c_str());
  pWriter->os = &out;
  pWriter->write_neighbourhood.os = &out;
  pEWriter->write_edges->os = &out;

  Relabel<V> r;
  this->forEachVertex(r);

  out << "<mesh";
  writeMeshAttributes(out);
  out << ">" << endl;
  this->forEachVertex(*pWriter);
  if (write_edges) this->forEachVertex(*pEWriter);
  out << "</mesh>" << endl;
}

/** @brief Print the vvm file to the terminal. */
template <class V>
void algebra::XMLMesh<V>::printXMLFile(bool write_edges) {
  using namespace std;

  if (!pWriter) return;

  pWriter->os = &std::cerr;
  pWriter->write_neighbourhood.os = &std::cerr;
  pEWriter->write_edges->os = &std::cerr;

  Relabel<V> r;
  this->forEachVertex(r);

  std::cerr << "<mesh ";
  writeMeshAttributes(std::cerr);
  std::cerr << ">" << endl;
  this->forEachVertex(*pWriter);
  if (write_edges) this->forEachVertex(*pEWriter);
  std::cerr << "</mesh>" << endl;
}

/** @brief Sets the parser object to use. */
template <class V>
void algebra::XMLMesh<V>::setParser(algebra::AbstractXMLMeshParser<V>* pParser) {
  this->pParser = pParser;
}

/** @brief Sets the writer object to use. */
template <class V>
void algebra::XMLMesh<V>::setWriter(typename algebra::XMLMesh<V>::VFunc * pWriter) {
  this->pWriter = pWriter;
}

#endif
