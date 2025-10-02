#include <memory>

#include <fw.h>
#include <glfw.h>

#include "objfgvobject.h"

#include "material.h"

Material::~Material()
{}

void Material::Copy(const EditableObject* pObj)
{
	const Material* pSrc = dynamic_cast<const Material*>(pObj);
	_params = pSrc->_params;
}


EditableObject* Material::Clone() const
{
	std::unique_ptr<Material> pNew(new Material);
	pNew->Copy(this);
	return pNew.release();
}


void Material::Generate(WriteBinFile& trg) const
{
	matfiletype mat;
	mat.transparency = static_cast<unsigned char> (_params.GetTransparency());
	const GLfloat* arr = _params.GetAmbient();
	mat.ambient[0] = static_cast<unsigned char>(arr[0]*255.0+0.5);
	mat.ambient[1] = static_cast<unsigned char>(arr[1]*255.0+0.5);
	mat.ambient[2] = static_cast<unsigned char>(arr[2]*255.0+0.5);
	arr = _params.GetDiffuse();
	mat.diffuse[0] = static_cast<unsigned char>(arr[0]*255.0+0.5);
	mat.diffuse[1] = static_cast<unsigned char>(arr[1]*255.0+0.5);
	mat.diffuse[2] = static_cast<unsigned char>(arr[2]*255.0+0.5);
	arr = _params.GetEmission();
	mat.emissive[0] = static_cast<unsigned char>(arr[0]*255.0+0.5);
	mat.emissive[1] = static_cast<unsigned char>(arr[1]*255.0+0.5);
	mat.emissive[2] = static_cast<unsigned char>(arr[2]*255.0+0.5);
	arr = _params.GetSpecular();
	mat.specular[0] = static_cast<unsigned char>(arr[0]*255.0+0.5);
	mat.specular[1] = static_cast<unsigned char>(arr[1]*255.0+0.5);
	mat.specular[2] = static_cast<unsigned char>(arr[2]*255.0+0.5);
	mat.shininess = static_cast<unsigned char>(_params.GetShininess());
	trg.Write(&mat, sizeof(matfiletype), 1);
}


void Material::Load(ReadBinFile& src)
{
	matfiletype mat;
	src.Read(&mat, sizeof(matfiletype));
	{
		GLfloat* aAmbient = _params.GetAmbient();
		aAmbient[0] = mat.ambient[0]/255.0f;
		aAmbient[1] = mat.ambient[1]/255.0f;
		aAmbient[2] = mat.ambient[2]/255.0f;
	}
	{
		GLfloat* aDiffuse = _params.GetDiffuse();
		aDiffuse[0] = mat.diffuse[0]/255.0f;
		aDiffuse[1] = mat.diffuse[1]/255.0f;
		aDiffuse[2] = mat.diffuse[2]/255.0f;
	}
	{
		GLfloat* aSpecular = _params.GetSpecular();
		aSpecular[0] = mat.specular[0]/255.0f;
		aSpecular[1] = mat.specular[1]/255.0f;
		aSpecular[2] = mat.specular[2]/255.0f;
	}
	{
		GLfloat* aEmission = _params.GetEmission();
		aEmission[0] = mat.emissive[0]/255.0f;
		aEmission[1] = mat.emissive[1]/255.0f;
		aEmission[2] = mat.emissive[2]/255.0f;
	}
	_params.SetTransparency(mat.transparency/255.0f);
	if (mat.shininess>128)
		mat.shininess = 128;
	_params.SetShininess(mat.shininess);
}



void Material::Reset()
{
	_params.Reset();
}


void Material::DrawInGallery() const
{}


void Material::SetEmission(const GLfloat* aEmission)
{
	GLfloat* aE = _params.GetEmission();
	aE[0] = aEmission[0];
	aE[1] = aEmission[1];
	aE[2] = aEmission[2];
}


void Material::SetSpecular(const GLfloat* aSpecular)
{
	GLfloat* aE = _params.GetSpecular();
	aE[0] = aSpecular[0];
	aE[1] = aSpecular[1];
	aE[2] = aSpecular[2];
}


void Material::SetDiffuse(const GLfloat* aDiffuse)
{
	GLfloat* aE = _params.GetDiffuse();
	aE[0] = aDiffuse[0];
	aE[1] = aDiffuse[1];
	aE[2] = aDiffuse[2];
}



void Material::SetAmbient(const GLfloat* aAmbient)
{
	GLfloat* aE = _params.GetAmbient();
	aE[0] = aAmbient[0];
	aE[1] = aAmbient[1];
	aE[2] = aAmbient[2];
}


DWORD Material::ClipboardSize() const
{
	return sizeof(MaterialParams);
}

char* Material::CopyToClipboard(char* pCur) const
{
	ToClipboard<MaterialParams>(_params, pCur);
	return pCur;
}

const char* Material::LoadFromClipboard(const char* pCur)
{
	FromClipboard<MaterialParams>(_params, pCur);
	return pCur;
}


bool Material::operator!=(const EditableObject& r) const
{
	const Material* pR = dynamic_cast<const Material*>(&r);
	return _params != pR->_params;
}
