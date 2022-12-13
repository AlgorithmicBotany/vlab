#include "mesharr.h"
#include "glutils.h"
#include "utils.h"


MeshArray::MeshArray() {}

bool MeshArray::AddMesh(const char* cmnd)
{
	// check that the mesh hasn't already been allocated in array
	// if yes, then re-read it
	std::string tfile = std::string(cmnd);
	for (unsigned int i = 0; i < _meshFiles.size(); ++i) {
	  if (tfile.compare(_meshFiles[i]) == 0){
	    return _meshes[i].Reread();
	  }
	}
	Mesh mesh(_meshes.size(), Vector3d(1.f,1.f,1.f));
	if (mesh.load(cmnd)){
	  _meshes.push_back(mesh);
	  _meshFiles.push_back(cmnd);
	}
	return true;
}
/*
bool MeshArray::AddAnimatedMesh(const char* cmnd)
{
	std::string tfile = std::string(cmnd);
	for (int i = 0; i < _meshFiles.size(); ++i) {
	  if (tfile.compare(_meshFiles[i]) == 0){
	    Utils::Message ("AddAnimatedMesh: Reread is not implemented\n");
	    return false;
	    // QQQ return operator[](i).Reread();
	  }
	}
	MeshConstraintAnimation animatedMesh(_animatedMeshes.size(), Vector3d(1.f,1.f,1.f));
	if (animatedMesh.load(cmnd)) {
	  _animatedMeshes.push_back(animatedMesh);
	  _meshFiles.push_back(cmnd);
	}
	
	return true;
}
*/
void MeshArray::Clear()
{
	_meshFiles.clear();
	_meshes.clear();
//QQQ	_animatedMeshes.clear();
}

void MeshArray::DrawMesh(size_t id, float sx, float sy, float sz)
{
	ASSERT(ValidMeshId(id));
	glScalef(sx,sy,sz);
	_meshes[id].Draw();
}
/*
void MeshArray::DrawAnimatedMesh(size_t id, float scale, float t)
{
	ASSERT(ValidAnimatedMeshId(id));
	glScalef(scale,scale,scale);
	_animatedMeshes[id].Draw(t);
}
*/
/*
bool MeshArray::Reread()
{
  for (iterator it = begin(); it != end(); ++it){
    bool success = it->Reread();
    if (!success)
      return false;
  }
  return true;
}
*/
