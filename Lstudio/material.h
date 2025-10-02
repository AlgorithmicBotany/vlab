#ifndef __MATERIAL_H__
#define __MATERIAL_H__


class Material : public EditableObject
{
public:
	~Material();
	const GLfloat* GetAmbient() const
	{ return _params.GetAmbient(); }
	void SetAmbient(const GLfloat*);
	const GLfloat* GetDiffuse() const
	{ return _params.GetDiffuse(); }
	void SetDiffuse(const GLfloat*);
	const GLfloat* GetEmission() const
	{ return _params.GetEmission(); }
	void SetEmission(const GLfloat*);
	const GLfloat* GetSpecular() const
	{ return _params.GetSpecular(); }
	void SetSpecular(const GLfloat*);
	int GetShininess() const
	{ return _params.GetShininess(); }
	void SetShininess(int shine)
	{ _params.SetShininess(shine); }
	int GetTransparency() const
	{ return _params.GetTransparency(); }
	void SetTransparency(GLfloat transp)
	{ _params.SetTransparency(transp); }

	void Copy(const EditableObject*);
	EditableObject* Clone() const;
	void Load(ReadBinFile&);
	void Generate(WriteBinFile&) const;
	void Reset();
	void DrawInGallery() const;
	const MaterialParams& GetParams() const
	{ return _params; }
	void Apply() const
	{ _params.Apply(); }


	DWORD ClipboardSize() const;
	char* CopyToClipboard(char*) const;
	const char* LoadFromClipboard(const char*);

	bool operator!=(const EditableObject&) const;
private:
	// Good old C structure (i w tym momencie paw)
	struct matfiletype
	{ 
		unsigned char transparency;
		unsigned char ambient[3];
		unsigned char diffuse[3];
		unsigned char emissive[3];
		unsigned char specular[3];
		unsigned char shininess;
	};

	MaterialParams _params;
};


#else
	#error File already included
#endif
