#ifndef __MATERIALPARAMS_H__
#define __MATERIALPARAMS_H__


class MaterialParams
{
public:
	MaterialParams();
	void Reset();
	void Apply() const;
	float* GetAmbient()
	{ return _ambient; }
	float* GetDiffuse()
	{ return _diffuse; }
	float* GetSpecular()
	{ return _specular; }
	float* GetEmission()
	{ return _emission; }
	const float* GetAmbient() const
	{ return _ambient; }
	const float* GetDiffuse() const
	{ return _diffuse; }
	const float* GetSpecular() const
	{ return _specular; }
	const float* GetEmission() const
	{ return _emission; }
	void SetShininess(int s)
	{
		assert(s>=0);
		assert(s<=128);
		_shininess = s;
	}
	int GetShininess() const
	{ return _shininess; }
	void SetTransparency(float t)
	{
		assert(t>=0);
		assert(t<=1);
		_ambient[3] = 1.0f-t;
		_diffuse[3] = 1.0f-t;
		_specular[3] = 1.0f-t;
		_emission[3] = 1.0f-t;
	}
	int GetTransparency() const
	{ return int(255.0f*(1.0f-_diffuse[3])+0.5f); }
	void operator=(const MaterialParams& src)
	{
		_ambient[0] = src._ambient[0];
		_ambient[1] = src._ambient[1];
		_ambient[2] = src._ambient[2];
		_ambient[3] = src._ambient[3];
		_diffuse[0] = src._diffuse[0];
		_diffuse[1] = src._diffuse[1];
		_diffuse[2] = src._diffuse[2];
		_diffuse[3] = src._diffuse[3];
		_specular[0] = src._specular[0];
		_specular[1] = src._specular[1];
		_specular[2] = src._specular[2];
		_specular[3] = src._specular[3];
		_emission[0] = src._emission[0];
		_emission[1] = src._emission[1];
		_emission[2] = src._emission[2];
		_emission[3] = src._emission[3];
		_shininess = src._shininess;
	}
	void Write(WriteBinFile&) const;
	void Write(WriteTextFile&) const;
	void Load(ReadBinFile&);
	void Load(ReadTextFile&);

	bool operator!=(const MaterialParams&) const;
	enum RgbaComponents
	{
		cRed   = 0,
		cGreen = 1,
		cBlue  = 2,
		cAlpha = 3,
		cCount = 4
	};

private:
	float _ambient[cCount];
	float _diffuse[cCount];
	float _specular[cCount];
	float _emission[cCount];
	int _shininess;
};

#endif
