#include "mesh.h"
#include "texturearr.h"
#include "glutils.h"

#include <iterator>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <array>

// ========== FILE I/O ========== //
std::ostream &operator<<(std::ostream &out, Tex2f const &vec) {
  return out << vec.m_x << " " << vec.m_y;
}

std::istream &operator>>(std::istream &in, Tex2f &vec) {
  return in >> vec.m_x >> vec.m_y;
}

std::ostream &operator<<(std::ostream &out, Vector3d const &vec) {
  return out << vec.X() << " " << vec.Y() << " " << vec.Z();
}

std::istream &operator>>(std::istream &in, Vector3d &vec) {
  return in >> vec[0] >> vec[1] >> vec[2];
}

bool Mesh::loadFromFile(const std::string &filepath) {
  std::ifstream in(filepath.c_str());
  if (!in.is_open()) {
    std::string error_str ("Could not open mesh file: ");
    error_str += filepath;
    Utils::Error(error_str.c_str());
    return false;
  }

  // save file name (like in surface.cpp)
  m_filename = filepath;
  size_t dot = m_filename.find('.');
  if (std::string::npos != dot)
    m_filename.erase(dot);

  std::string s;

  Vector3d tmp3f;
  Tex2f tmp2f;
  std::string tmpStr;

  MeshGroup gNew(filepath + std::string("base"));
  gNew.facesStart = 0;
  gNew.verticesStart = 0;
  addGroup(gNew); // default group

  while (getline(in, s)) {
    std::stringstream line(s);
    std::string token;

    line >> token;

    if (token == "v") {
      line >> tmp3f;
      m_vertices.push_back(Mesh_Vertex(tmp3f));
    } else if (token == "vt") {
      line >> tmp2f;
      m_uvs.push_back(tmp2f);
      // currently ignore;
    } else if (token == "g") // push group
    {
      line >> tmpStr;
      MeshGroup gNew(tmpStr);

      if (hasGroup()) {
        MeshGroup &gOld = currentGroup();
        gOld.facesEnd = m_faces.size();
	    gOld.verticesEnd = m_vertices.size();
      }

      gNew.facesStart = m_faces.size();
      gNew.verticesStart = m_vertices.size();
      addGroup(gNew);
    } else if (token == "o") // obj name
    {
      line >> tmpStr;
      m_name = tmpStr;
    } else if (token == "f") {
      typedef std::array<int, 3> Indices;
      Indices indices = {-1, -1, -1}; // init

      // hopefully CCW winding
      std::vector<Indices> verts;
      while (line >> s) {
        std::stringstream item(s);

        // reset
        indices[0] = -1;
        indices[1] = -1;
        indices[2] = -1;

        for (int j = 0; getline(item, s, '/') && j < 3; ++j) {
          std::stringstream index(s);
          index >> indices[j];
        }

        // Indices are 0 based, so minus by 1
        if (indices[0] != -1) {
          indices[0] -= 1;
        }

        if (indices[1] != -1) {
          indices[1] -= 1;
        }

        if (indices[2] != -1) {
          indices[2] -= 1;
        }

        verts.push_back(indices); // polygon vert
      }

      if (verts.size() == 3) // triangle
      {
        Mesh_Face triangle;
        for (int i = 0; i < 3; ++i) {
          triangle.vertIndex[i] = verts[i][0];
          triangle.uvIndex[i] = verts[i][1];
          // triangle.normal[i] = verts[i][2];
        }
        m_faces.push_back(triangle);
      } else if (verts.size() == 4) // Quad
      {
        Mesh_Face triangle;
        for (int i = 0; i < 3; ++i) {
          triangle.vertIndex[i] = verts[i][0];
          triangle.uvIndex[i] = verts[i][1];
          // triangle.normal[i] = verts[i][2];
        }
        m_faces.push_back(triangle);

        int j = 2;
        for (int i = 0; i < 3; ++i) {
          triangle.vertIndex[i] = verts[i][0];
          triangle.uvIndex[i] = verts[i][1];
          // triangle.normal[i] = verts[j][2];

          j = (j + 1) % 4;
        }
        m_faces.push_back(triangle);
      } else {
        std::stringstream errorMsg;
        errorMsg << "(ERROR) Invalid face in " << filepath << ":" << s;

	Utils::Message(errorMsg.str().c_str());
        throw std::runtime_error(errorMsg.str());
      }
    }
  }

  if (hasGroup()) {
    MeshGroup &gOld = currentGroup();
    gOld.facesEnd = m_faces.size();
    gOld.verticesEnd = m_vertices.size();
  }

  return true;
}

void Mesh::copyAnimatedMeshFromGroup(Mesh const &m_src, MeshGroup const &g) {

	addGroup(g);

	m_loaded = m_src.isLoaded();
	m_id = m_src.id();
	m_initScale = m_src.initScale();
	m_contactPoint = m_src.initContactPoint();
	m_contactVertId = m_src.initContactVertId();

	const_iterator start = m_src.begin() + g.verticesStart;
	const_iterator end = m_src.begin() + g.verticesEnd;
	m_vertices = VertexVec(start,end);
		
	const_UVIterator uvStart = m_src.uvBegin();
	const_UVIterator uvEnd = m_src.uvEnd();
	m_uvs = UVVec(uvStart,uvEnd);
	
	// assuming triangles are same for all groups
	const_FaceIterator faceStart = m_src.faceBegin();
	const_FaceIterator faceEnd = m_src.faceEnd();
	m_faces = FaceVec(faceStart,faceEnd);
	
	// since vertices have changed, recompute the normals	
	scaleByInitalScale();
	translateByContactPoint();
	reloadFaceNormals();
	reloadVertexNormals();
	reloadBoundingBox();
}

void Mesh::Draw(void) const
{
  typedef Mesh::Mesh_Face Face;

  for (auto iter = groupBegin(); iter != groupEnd(); ++iter) {
    auto const &g = *iter;

    glBegin(GL_TRIANGLES);
    for (int fID = g.facesStart; fID < g.facesEnd; ++fID) {
      Face const &f = m_faces[fID];
		
      size_t aID = f.vertIndex[0];
      size_t bID = f.vertIndex[1];
      size_t cID = f.vertIndex[2];

      glTexCoord2fv(m_uvs[f.uvIndex[0]]);
      glNormal3fv(m_vertexNormals[aID]);
      glVertex3fv(m_vertices[aID].m_pos);
      glTexCoord2fv(m_uvs[f.uvIndex[1]]);
      glNormal3fv(m_vertexNormals[bID]);
      glVertex3fv(m_vertices[bID].m_pos);
      glTexCoord2fv(m_uvs[f.uvIndex[2]]);
      glNormal3fv(m_vertexNormals[cID]);
      glVertex3fv(m_vertices[cID].m_pos);

    }
    glEnd();
  }
}

void Mesh::DrawObj(OpenGLMatrix& mtrx, OpenGLMatrix& nrmx,
		ObjOutputStore& trg, int color, int texture) const
{
  typedef Mesh::Mesh_Face Face;

  for (auto iter = groupBegin(); iter != groupEnd(); ++iter) {
    auto const &g = *iter;

    // some groups in the Mesh object have no faces, so they are ignorned for now
    if (g.facesStart == g.facesEnd)
      continue;

    // start new OBJ group
    trg.NewGroup();
    trg.PrintMaterialUse(color,texture);

    // write the vertex, normal and texture coordinates and indicies for all the triangles
    for (int fID = g.facesStart; fID < g.facesEnd; ++fID) {
      Face const &f = m_faces[fID];
		
      size_t aID = f.vertIndex[0];
      size_t bID = f.vertIndex[1];
      size_t cID = f.vertIndex[2];

      size_t aTexID = f.uvIndex[0];
      size_t bTexID = f.uvIndex[1];
      size_t cTexID = f.uvIndex[2];

      // possibly write 3 vertices, with texture coordinates, and 3 normals
      // duplicates are ignored by the trg Object
      Vector3d v, t, n;

      v = m_vertices[aID].m_pos;
      v.Transform(mtrx);
      t.Set(m_uvs[aTexID].m_x,m_uvs[aTexID].m_y,0.);
      n = m_vertexNormals[aID];
      n.Transform(nrmx);
      std::pair<size_t, size_t> vx1 = trg.VertexTexCoord(v, t);
      size_t nx1 = trg.Normal(n);

      v = m_vertices[bID].m_pos;
      v.Transform(mtrx);
      t.Set(m_uvs[bTexID].m_x,m_uvs[bTexID].m_y,0.);
      n = m_vertexNormals[bID];
      n.Transform(nrmx);
      std::pair<size_t, size_t> vx2 = trg.VertexTexCoord(v, t);
      size_t nx2 = trg.Normal(n);

      v = m_vertices[cID].m_pos;
      v.Transform(mtrx);
      t.Set(m_uvs[cTexID].m_x,m_uvs[cTexID].m_y,0.);
      n = m_vertexNormals[cID];
      n.Transform(nrmx);
      std::pair<size_t, size_t> vx3 = trg.VertexTexCoord(v, t);
      size_t nx3 = trg.Normal(n);

      if (vx1 != vx2 && vx1 != vx3 && vx2 != vx3) {
        trg.Triangle(vx1.first, nx1, vx1.second, vx2.first, nx2, vx2.second,
                     vx3.first, nx3, vx3.second, color, texture);
      }
    }
  }
}

void Mesh::DrawRayshade(OpenGLMatrix& mtrx, OpenGLMatrix& nrmx, 
		  std::ofstream& target, int color, int texture) const
// this is a copy of the OBJ output more or less
// probably could be changed to output triangle for different cases
// instead of repeating the code
{
  typedef Mesh::Mesh_Face Face;

  bool textured = texture >= 0 ? true : false;

  for (auto iter = groupBegin(); iter != groupEnd(); ++iter) {
    auto const &g = *iter;

    // some groups in the Mesh object have no faces, so they are ignorned for now
    if (g.facesStart == g.facesEnd)
      continue;

    // write the vertex, normal and texture coordinates and indicies for all the triangles
    for (int fID = g.facesStart; fID < g.facesEnd; ++fID) {
      Face const &f = m_faces[fID];
		
      size_t aID = f.vertIndex[0];
      size_t bID = f.vertIndex[1];
      size_t cID = f.vertIndex[2];

      size_t aTexID = f.uvIndex[0];
      size_t bTexID = f.uvIndex[1];
      size_t cTexID = f.uvIndex[2];

      // write 3 vertices, with texture coordinates, and 3 normals
      // duplicates are ignored
      Vector3d v1, t1, n1;
      v1 = m_vertices[aID].m_pos;
      v1.Transform(mtrx);
      t1.Set(m_uvs[aTexID].m_x,m_uvs[aTexID].m_y,0.);
      n1 = m_vertexNormals[aID];
      n1.Transform(nrmx);

      Vector3d v2, t2, n2;
      v2 = m_vertices[bID].m_pos;
      v2.Transform(mtrx);
      t2.Set(m_uvs[bTexID].m_x,m_uvs[bTexID].m_y,0.);
      n2 = m_vertexNormals[bID];
      n2.Transform(nrmx);

      Vector3d v3, t3, n3;
      v3 = m_vertices[cID].m_pos;
      v3.Transform(mtrx);
      t3.Set(m_uvs[cTexID].m_x,m_uvs[cTexID].m_y,0.);
      n3 = m_vertexNormals[cID];
      n3.Transform(nrmx);

      if (v1 != v2 && v1 != v3 && v2 != v3) {

          target << "triangle";
          if (textured)
            target << "uv";
          target << "\n";

          target << v1[0] << " " << v1[1] << " " << v1[2]
                 << " "
                 << n1[0] << " " << n1[1] << " " << n1[2];
          if (textured)
            target << " " << t1[0] << " " << t1[1];
          target << "\n";

          target << v2[0] << " " << v2[1] << " " << v2[2]
                 << " "
                 << n2[0] << " " << n2[1] << " " << n2[2];
          if (textured)
            target << " " << t2[0] << " " << t2[1];
          target << "\n";

          target << v3[0] << " " << v3[1] << " " << v3[2]
                 << " "
                 << n3[0] << " " << n3[1] << " " << n3[2];
          if (textured)
            target << " " << t3[0] << " " << t3[1];
          target << "\n";

          if (textured)
            target << "texture image " << textures.getFilename(texture) << "\nmap uv\n";

      }
    }
  }

}


bool Mesh::Reread() {
  std::string fname = m_filename + ".obj";
  bool success = false;
#ifndef WIN32
  int cpt = 0;
  while (!success && cpt < 100) {
    success = loadByName(fname.c_str());
    cpt++;
  }
#else
  success = loadByName(fname.c_str());
#endif
  return success;
}

