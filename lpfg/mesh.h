#ifndef MESH_H
#define MESH_H

// Mostly wrapper for md2.h and OpenGL interplay
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cstddef>

#include "vector3d.h"
#include "volume.h"
#include "objout.h"
#include "utils.h"

struct MeshGroup {
  MeshGroup(std::string const &name);
  std::string m_name;
  int facesStart, facesEnd;
  int verticesStart, verticesEnd;
};

struct Tex2f {
  union {
    struct {
      float m_x, m_y;
    };
    float m_coord[2];
  };
  operator const float*() const	{ return &(m_coord[0]); }
};

class Mesh : public MeshGroup {
public:
  struct Mesh_Face {
    union {
      int vertIndex[3];
      struct {
        int vertA;
        int vertB;
        int vertC;
      };
    };
    union {
      int uvIndex[3];
      struct {
        int uvA;
        int uvB;
        int uvC;
      };
    };

    // So that only Mesh can delete it
  private:
    void operator delete(void *) {}
  };
  struct Mesh_Vertex {
    Vector3d m_pos;

    explicit Mesh_Vertex(Vector3d const &pos = Vector3d(0, 0, 0)) : m_pos(pos) {}

    float &operator[](int i) { return m_pos[i]; }
    float operator[](int i) const { return m_pos[i]; }
    Vector3d const &pos() const { return m_pos; }

    // So that only Mesh can delete it
  private:
    void operator delete(void *) {}
  };

  typedef std::vector<MeshGroup> GroupVec;

  typedef std::vector<Mesh_Vertex> VertexVec;
  typedef std::vector<Mesh_Face> FaceVec;
  typedef std::vector<Tex2f> UVVec;
  typedef std::vector<Vector3d> NormalVec;

  typedef VertexVec::iterator iterator;
  typedef VertexVec::const_iterator const_iterator;
  typedef FaceVec::iterator FaceIterator;
  typedef FaceVec::const_iterator const_FaceIterator;
  typedef UVVec::iterator UVIterator;
  typedef UVVec::const_iterator const_UVIterator;
  typedef NormalVec::iterator NormalIterator;
  typedef NormalVec::const_iterator const_NormalIterator;
  typedef GroupVec::iterator GroupIterator;
  typedef GroupVec::const_iterator const_GroupIterator;

public:
  explicit Mesh(size_t id = 0, Vector3d const &initScale = Vector3d(1, 1, 1),
                std::string const &name = std::string("Mesh"));
  virtual ~Mesh();

  bool load(const std::string &filepath);
  bool loadByName(const std::string &filename);

  size_t id() const;
  Vector3d const &initScale() const;
  Vector3d const &initContactPoint() const;
  int const &initContactVertId() const;

  void id(size_t idNumber);
  void initScale(Vector3d const &scale);
  void initContactPoint(Vector3d const &contactPoint);

  Mesh_Vertex const *vertsData() const;
  Mesh_Vertex const *vertsData(MeshGroup const &g) const;
  Mesh_Face const *trisData() const;
  Vector3d const *faceNormalData() const;
  Vector3d const *vertexNormalData() const;
  Tex2f const *uvData() const;

  Mesh_Face const &triangle(int triID) const;
  Mesh_Vertex const &vertex(int vertID) const;

  size_t vertsCount() const;
  size_t trisCount() const;
  size_t faceNormalsCount() const;
  size_t vertexNormalsCount() const;
  size_t groupCount() const;

  iterator begin();
  iterator end();

  const_iterator begin() const;
  const_iterator end() const;

  FaceIterator faceBegin();
  FaceIterator faceEnd();

  const_FaceIterator faceBegin() const;
  const_FaceIterator faceEnd() const;

  UVIterator uvBegin();
  UVIterator uvEnd();

  const_UVIterator uvBegin() const;
  const_UVIterator uvEnd() const;

  NormalIterator normalBegin();
  NormalIterator normalEnd();

  const_NormalIterator normalBegin() const;
  const_NormalIterator normalEnd() const;

  GroupIterator groupBegin();
  GroupIterator groupEnd();

  const_GroupIterator groupBegin() const;
  const_GroupIterator groupEnd() const;

  VertexVec const &vertexVector() const;
  NormalVec const &vertexNormalsVector() const;

  bool isLoaded() const;

  void addGroup(MeshGroup const &g);
  MeshGroup &currentGroup();
  bool hasGroup() const;

  void copyAnimatedMeshFromGroup(Mesh const &m, MeshGroup const &g);

  void Draw(void) const;
  void DrawObj(OpenGLMatrix& mtrx, OpenGLMatrix& nrmx, 
		  ObjOutputStore& trg, int color, int texture) const;
  void DrawRayshade(OpenGLMatrix& mtrx, OpenGLMatrix& nrmx, 
		  std::ofstream& target, int color, int texture) const;
  
  Volume const &GetBoundingBox(void) const;

  bool IsTextured() const
  { return m_TextureId != -1; }
  int TextureId() const
  { return m_TextureId; }

  bool Reread();

private:
  bool loadFromFile(const std::string &filepath);

  void resetGeometry();

  void scaleByInitalScale();
  void translateByContactPoint();
  void normalizeScaling();
  void reloadFaceNormals();
  void reloadVertexNormals();
  void reloadBoundingBox();

private:
  bool m_loaded;
  size_t m_id;
  std::string m_filename;
  Vector3d m_initScale;

  VertexVec m_vertices;
  std::vector<Tex2f> m_uvs;
  FaceVec m_faces;

  std::vector<Vector3d> m_vertexNormals;
  std::vector<Vector3d> m_faceNormals;
  GroupVec m_groups;

  Volume m_boundingBox;
  int m_TextureId;
  Vector3d m_contactPoint;
  int m_contactVertId;
};

inline MeshGroup::MeshGroup(std::string const &name) : m_name(name) {}

inline Mesh::Mesh(size_t id, Vector3d const &initScale, std::string const &name)
    : MeshGroup(name), m_loaded(false), m_id(id), m_initScale(initScale), m_TextureId(-1), 
      m_contactPoint(0.f,0.f,0.f), m_contactVertId(0) {}

inline Mesh::~Mesh() {}

inline Mesh::iterator Mesh::begin() { return m_vertices.begin(); }

inline Mesh::iterator Mesh::end() { return m_vertices.end(); }

inline Mesh::const_iterator Mesh::begin() const { return m_vertices.begin(); }

inline Mesh::const_iterator Mesh::end() const { return m_vertices.end(); }

inline Mesh::FaceIterator Mesh::faceBegin() { return m_faces.begin(); }

inline Mesh::FaceIterator Mesh::faceEnd() { return m_faces.end(); }

inline Mesh::const_FaceIterator Mesh::faceBegin() const {
  return m_faces.begin();
}

inline Mesh::VertexVec const &Mesh::vertexVector() const { return m_vertices; }

inline Mesh::NormalVec const &Mesh::vertexNormalsVector() const { return m_vertexNormals; }

inline Mesh::const_FaceIterator Mesh::faceEnd() const { return m_faces.end(); }

inline Mesh::UVIterator Mesh::uvBegin() { return m_uvs.begin(); }

inline Mesh::UVIterator Mesh::uvEnd() { return m_uvs.end(); }

inline Mesh::const_UVIterator Mesh::uvBegin() const { return m_uvs.begin(); }

inline Mesh::const_UVIterator Mesh::uvEnd() const { return m_uvs.end(); }

inline Mesh::NormalIterator Mesh::normalBegin() { return m_vertexNormals.begin(); }

inline Mesh::NormalIterator Mesh::normalEnd() { return m_vertexNormals.end(); }

inline Mesh::const_NormalIterator Mesh::normalBegin() const { return m_vertexNormals.begin(); }

inline Mesh::const_NormalIterator Mesh::normalEnd() const { return m_vertexNormals.end(); }

inline Mesh::GroupIterator Mesh::groupBegin() { return m_groups.begin(); }

inline Mesh::GroupIterator Mesh::groupEnd() { return m_groups.end(); }

inline Mesh::const_GroupIterator Mesh::groupBegin() const {
  return m_groups.begin();
}

inline Mesh::const_GroupIterator Mesh::groupEnd() const {
  return m_groups.end();
}

inline Vector3d const &Mesh::initScale() const { return m_initScale; }

inline void Mesh::initScale(Vector3d const &scale) { m_initScale = scale; }

inline Vector3d const &Mesh::initContactPoint() const { return m_contactPoint; } 

inline int const &Mesh::initContactVertId() const { return m_contactVertId; } 

inline void Mesh::initContactPoint(Vector3d const &contactPoint) { m_contactPoint = contactPoint; }

inline bool Mesh::load(const std::string &filepath) {
  if (m_loaded)
    resetGeometry();

  // defaults
  m_TextureId = -1;
  m_contactPoint = Vector3d(0.f,0.f,0.f);
  m_initScale = Vector3d(1.f,1.f,1.f);

  // first parameter should be the file name
  const char *token = Utils::SkipBlanks(filepath.c_str());
  char fnm[80];
  if (sscanf(filepath.c_str(), "%79s", fnm) != 1) {
    Utils::Error("Invalid Mesh specification command: file name was not specified.\n"
                 "Usage - mesh: filename S: scale T: textureId C: contact_x contact_y contact_z\n"
		         "Defaults: scale = 1; contact = 0,0,0; no texture.\n");

  }
  // check for other parameters
  token = Utils::SkipNonBlanks(token); // skip over the filename
  token = Utils::SkipBlanks(token);

  float arr[3];
  while (NULL != token) {
    switch (*token) {
    case 'S':
      token = Utils::ReadFloats(token + 2, arr, 1);
      m_initScale *= arr[0];
      break;
    case 'T':
      token = Utils::ReadFloats(token + 2, arr, 1);
      m_TextureId = int(arr[0]);
      break;
    case 'C':
      token = Utils::ReadFloats(token + 2, arr, 3);
      m_contactPoint = Vector3d(arr[0],arr[1],arr[2]);
      break;
    default:
      Utils::Error("Invalid Mesh specification command.\n"
		    "Usage - mesh: filename S: scale T: textureId C: contact_x contact_y contact_z\n"
		    "Defaults: scale = 1; contact = 0,0,0; no texture.\n");
      token = NULL;
      break;
    }
  }

  m_loaded = loadFromFile(fnm);

  if (m_loaded) {
    scaleByInitalScale();
    translateByContactPoint();
    reloadFaceNormals();
    reloadVertexNormals();
    reloadBoundingBox();
  }

  return m_loaded;
}

inline bool Mesh::loadByName(const std::string &filename) {
  if (m_loaded)
    resetGeometry();

  m_loaded = loadFromFile(filename);

  if (m_loaded) {
    scaleByInitalScale();
    translateByContactPoint();
    reloadFaceNormals();
    reloadVertexNormals();
    reloadBoundingBox();
  }

  return m_loaded;

}

inline void Mesh::resetGeometry() {
  m_loaded = false;

  m_vertices.clear();
  m_uvs.clear();
  m_faces.clear();

  m_vertexNormals.clear();
  m_faceNormals.clear();
  m_groups.clear();
}

inline Mesh::Mesh_Face const &Mesh::triangle(int triID) const {
  return m_faces[triID];
}

inline Mesh::Mesh_Vertex const &Mesh::vertex(int vertID) const {
  return m_vertices[vertID];
}

inline void Mesh::reloadFaceNormals() {
  m_faceNormals.clear();
  m_faceNormals.reserve(trisCount());

  for (size_t triIdx = 0; triIdx < trisCount(); ++triIdx) {
    auto const &t = m_faces[triIdx];

    auto const &a = m_vertices[t.vertIndex[0]].m_pos;
    auto const &b = m_vertices[t.vertIndex[1]].m_pos;
    auto const &c = m_vertices[t.vertIndex[2]].m_pos;

    //Vec3f norm = (b - a).crossProduct(c - a);
    Vector3d norm = (b - a) % (c - a); 
    norm.Normalize();

    m_faceNormals.push_back(norm); // place in vec is ID with tris
  }
}

inline void Mesh::reloadVertexNormals() {
  if (m_faceNormals.size() != trisCount())
    reloadFaceNormals();

  m_vertexNormals.clear();
  m_vertexNormals.resize(vertsCount(), Vector3d(0, 0, 0));

  // Accumulate normals, and count verts
  for (size_t triIdx = 0; triIdx < trisCount(); ++triIdx) {
    auto const &t = trisData()[triIdx];
    auto const &faceNorm = faceNormalData()[triIdx];

    for (int i = 0; i < 3; ++i) {
      auto const &vertIdx = t.vertIndex[i];
      m_vertexNormals[vertIdx] += faceNorm;
    }
  }

  // Take average
  for (size_t vertIdx = 0; vertIdx < vertsCount(); ++vertIdx) {
    auto &norm = m_vertexNormals[vertIdx];
    norm.Normalize();
  }
}

inline void Mesh::reloadBoundingBox() {
  Vector3d maxXYZ, minXYZ;

  if (vertsCount() > 1) {
    auto const &v = m_vertices[0];
    maxXYZ = Vector3d(v[0], v[1], v[2]);
    minXYZ = maxXYZ;
  }

  for (size_t vertIdx = 1; vertIdx < vertsCount(); ++vertIdx) {
    auto const &v = m_vertices[vertIdx];

    if (v[0] > maxXYZ.X()) // Builds bounding box
      maxXYZ.X(v[0]);
    if (v[1] > maxXYZ.Y())
      maxXYZ.Y(v[1]);
    if (v[2] > maxXYZ.Z())
      maxXYZ.Z(v[2]);

    if (v[0] < minXYZ.X())
      minXYZ.X(v[0]);
    if (v[1] < minXYZ.Y())
      minXYZ.Y(v[1]);
    if (v[2] < minXYZ.Z())
      minXYZ.Z(v[2]);
  }

  m_boundingBox.Set(minXYZ.X(),maxXYZ.X(),minXYZ.Y(),maxXYZ.Y(),minXYZ.Z(),maxXYZ.Z());
}

inline void Mesh::scaleByInitalScale() {
  for (Mesh_Vertex &v : m_vertices) {
    v.m_pos.Scale(m_initScale.X(),m_initScale.Y(),m_initScale.Z());
  }
}

inline void Mesh::translateByContactPoint() {
  // translate the mesh by the contact point,
  // and save the index of the vertex nearest to the contact point
  // (this is needed in the TransformAttachedParticle function,
  // when the Mesh is attached to a rigid body)
  float minLenSq = 1e11;
  int vertId = 0;
  for (Mesh_Vertex &v : m_vertices) {
    v.m_pos -= m_contactPoint;
    float lenSq = (v.m_pos - m_contactPoint).LengthSquared();
    if (lenSq < minLenSq) {
      minLenSq = lenSq;
      m_contactVertId = vertId;
    }
    ++vertId;
  }
}

inline void Mesh::normalizeScaling() {
  Vector3d maxXYZ, minXYZ;

  if (vertsCount() > 1) {
    auto const &v = m_vertices[0];
    maxXYZ = Vector3d(v[0], v[1], v[2]);
    minXYZ = maxXYZ;
  }

  for (size_t vertIdx = 1; vertIdx < vertsCount(); ++vertIdx) {
    auto const &v = m_vertices[vertIdx];

    if (v[0] > maxXYZ.X()) // Builds bounding box
      maxXYZ.X(v[0]);
    if (v[1] > maxXYZ.Y())
      maxXYZ.Y(v[1]);
    if (v[2] > maxXYZ.Z())
      maxXYZ.Z(v[2]);

    if (v[0] < minXYZ.X())
      minXYZ.X(v[0]);
    if (v[1] < minXYZ.Y())
      minXYZ.Y(v[1]);
    if (v[2] < minXYZ.Z())
      minXYZ.Z(v[2]);
  }

  Vector3d boxDim = (maxXYZ - minXYZ).Abs();
  Vector3d offset = -maxXYZ + boxDim * 0.5;

  float maxLen = 0;

  // Offsets Model to center and builds bounding sphere radius
  for (size_t vertIdx = 0; vertIdx < vertsCount(); ++vertIdx) {
    auto &v = m_vertices[vertIdx];

    v[0] += offset.X();
    v[1] += offset.Y();
    v[2] += offset.Z();

    // Sqaured Length actually...
    float currLen = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    // Builds Bounding Sphere Radius
    if (currLen > maxLen) {
      maxLen = currLen;
    }
  }

  // TODO check divBy0
  maxLen = 1.0f / std::sqrt(maxLen);

  for (size_t vertIdx = 0; vertIdx < vertsCount(); ++vertIdx) {
    auto &v = m_vertices[vertIdx];

    v[0] *= maxLen;
    v[1] *= maxLen;
    v[2] *= maxLen;
  }
}

inline size_t Mesh::vertsCount() const { return m_vertices.size(); }

inline size_t Mesh::trisCount() const { return m_faces.size(); }

inline size_t Mesh::faceNormalsCount() const { return m_faceNormals.size(); }

inline size_t Mesh::vertexNormalsCount() const {
  return m_vertexNormals.size();
}

inline size_t Mesh::groupCount() const { return m_groups.size(); }

inline bool Mesh::isLoaded() const { return m_loaded; }

inline Mesh::Mesh_Vertex const *Mesh::vertsData() const {
  return m_vertices.data();
}

inline Mesh::Mesh_Face const *Mesh::trisData() const { return m_faces.data(); }

inline Vector3d const *Mesh::faceNormalData() const {
  return m_faceNormals.data();
}

inline Vector3d const *Mesh::vertexNormalData() const {
  return m_vertexNormals.data();
}

inline Tex2f const *Mesh::uvData() const {
  return m_uvs.data();
}

inline size_t Mesh::id() const { return m_id; }

inline void Mesh::id(size_t idNumber) { m_id = idNumber; }

inline void Mesh::addGroup(MeshGroup const &g) { m_groups.push_back(g); }

inline MeshGroup &Mesh::currentGroup() { return m_groups.back(); }

inline bool Mesh::hasGroup() const { return !m_groups.empty(); }

inline Volume const &Mesh::GetBoundingBox() const { return m_boundingBox; }


#endif // MESH_H
