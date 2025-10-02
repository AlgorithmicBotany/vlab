#ifndef __SURFACE_H__
#define __SURFACE_H__


enum 
{
	ContactPointId = 16,
		InvalidControlPointId = -1
};

class TraditionalSurface;


class Surface : public EditableObject
{
public:
	Surface();
	~Surface();
	const WorldPointf& GetPoint(int n) const;
	void SetPoint(int i, WorldPointf p);
	const WorldPointf& GetCP() const
	{ return _cp; }
	bool YZSymmetric() const
	{ return _pPatch->YZSymmetric(); }
	void ToggleYZSymmetric()
	{ _pPatch->ToggleYZSymmetric(); }

	void SetPointX(int i, float x)
	{ 
		if (ContactPointId == i)
			_cp.X(x);
		else
			_pPatch->SetX(i, x, _cp, _heading, _up); 
	}
	void SetPointY(int i, float y)
	{ 
		if (ContactPointId == i)
			_cp.Y(y);
		else
			_pPatch->SetY(i, y, _cp, _heading, _up); 
	}
	void SetPointZ(int i, float z)
	{ 
		if (ContactPointId == i)
			_cp.Z(z);
		else
			_pPatch->SetZ(i, z, _cp, _heading, _up); 
	}

	void Draw(int drawwhat, int activepoint, unsigned int) const;
	void DrawInGallery() const;
	void Copy(const EditableObject*);
	EditableObject* Clone() const;
	void Import(ReadTextFile&);
	void Reset();
	void Generate(WriteTextFile&) const;
	const char* GetName() const
	{ return _Name; }
	void SetName(const char* name)
	{
		assert(strlen(name)<MaxNameLength);
		strcpy(_Name, name);
	}
	void GetBoundingBox(BoundingBox&) const;

	DWORD ClipboardSize() const;
	char* CopyToClipboard(char*) const;
	const char* LoadFromClipboard(const char*);

	bool operator!=(const EditableObject&) const;
	bool Editable() const
	{ return _editable; }
	bool IsNamed(const char*) const;
private:
	enum
	{
		MaxNameLength = 32
	};

	void _DrawAxis(bool ActiveCP, unsigned int) const;
	void _GenerateEditable(WriteTextFile&) const;
	void _GetBoundingBox(BoundingBox&) const;

	char _Name[MaxNameLength];
	bool _editable;
	Patch* _pPatch;
	WorldPointf _cp;
	WorldPointf _ep;
	WorldPointf _heading;
	WorldPointf _up;
	float _size;


	void operator=(const Surface&);

	TraditionalSurface* _pTraditional;
};


#else
	#error File already included
#endif
