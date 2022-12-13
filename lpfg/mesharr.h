#ifndef __MESHARR_H__
#define __MESHARR_H__

#include "mesh.h"
//#include "meshconstraintanimation.h"

class MeshArray 
{
public:
	MeshArray();
	bool AddMesh(const char*);
	bool AddAnimatedMesh(const char*);
	void Clear();
	void DrawMesh(size_t id, float sx, float sy, float sz);

	bool ValidMeshId(size_t id) const
	{ return (id < _meshes.size()); }
	//bool ValidAnimatedMeshId(size_t id) const
	//{ return (id < _animatedMeshes.size()); }
	
	bool IsMeshTextured(size_t id) const
	{ ASSERT(ValidMeshId(id)); return _meshes[id].IsTextured(); }
	int MeshTextureId(size_t id) const
	{ ASSERT(ValidMeshId(id)); return _meshes[id].TextureId(); }
	//bool IsAnimatedMeshTextured(size_t id) const
	//{ ASSERT(ValidAnimatedMeshId(id)); return _animatedMeshes[id].IsTextured(); }
	//int AnimatedMeshTextureId(size_t id) const
	//{ ASSERT(ValidAnimatedMeshId(id)); return _animatedMeshes[id].TextureId(); }

	const Mesh& GetMesh(size_t id) const
	{ ASSERT(ValidMeshId(id)); return _meshes[id]; }
	//const MeshConstraintAnimation& GetAnimatedMesh(size_t id) const
	//{ ASSERT(ValidAnimatedMeshId(id)); return _animatedMeshes[id]; }
	//bool Reread();
private:
	std::vector<std::string> _meshFiles;
	std::vector<Mesh> _meshes;
	//std::vector<MeshConstraintAnimation> _animatedMeshes;
};


extern MeshArray meshes;


#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
