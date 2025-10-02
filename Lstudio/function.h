#ifndef __FUNCTION_H__
#define __FUNCTION_H__


namespace FunctionSpace
{
	enum DrawWhat
	{
		DrawPoints = 1,
			DrawSegments = 1 << 1,
			DrawCurve = 1 << 2,
	};
	const int DrawAll = DrawPoints | DrawSegments | DrawCurve;
};


class Function : public EditableObject
{
public:
	Function();
	~Function();
	void Copy(const EditableObject*);
	EditableObject* Clone() const;

	void Reset();
	void Generate(WriteTextFile&) const;
	void Import(ReadTextFile&);
	void DrawInGallery() const;
	void Draw(int drawWhat) const;
	int GetClosestPoint(WorldPointf) const;
	WorldPointf GetPoint(size_t) const;
	void MovePoint(int i, WorldPointf wp);
	int AddPoint(WorldPointf);
	void DeletePoint(int);
	const char* GetName() const
	{ return _name; }
	void SetName(const char* txt)
	{
		assert(strlen(txt)<MaxNameLength);
		strcpy(_name, txt);
	}
	bool IsNamed(const char*) const;
	void GetBoundingBox(BoundingBox& bb) const;
	bool CanDelete(int) const;
	int NumOfPoints() const
	{ return _arr.size(); }

	DWORD ClipboardSize() const;
	char* CopyToClipboard(char*) const;
	const char* LoadFromClipboard(const char*);
	void Flip();
	bool Flipped() const
	{ return _Flipped; }

	// version related functions
	bool ImplementsSamples() const
	{ return _version>=101; }

	int GetSamples() const
	{
		assert(ImplementsSamples());
		return _samples;
	}
	void SetSamples(int i)
	{
		assert(ImplementsSamples());
		assert(i>2);
		_samples = i;
	}
	int GetVersion() const
	{ return _version; }
	void SetVersion(int);

	bool operator!=(const EditableObject&) const;
	bool IsNamed() const
	{ return 0 != strcmp(_name, "unnamed"); }
private:
	void _DrawPoints() const;
	void _DrawSegments() const;
	void _DrawCurve() const;
	FunctionPoints _arr;
	bool _Flipped;
	int _samples;

	enum { MaxNameLength = 32 };
	char _name[MaxNameLength];

	WorldPointf _P(float) const;
	float _N(int, int, float) const;
	float _Nk1(int, float) const;
	float _Nkt(int, int, float) const;
	int _Uk(int) const;

	// _version = 100*majorver+minorver; so for example:
	// 101 means: major version 1, minor version 1 or: 1.01
	int _version;

	void _Generate0000(WriteTextFile&) const;
	void _Generate0101(WriteTextFile&) const;
	void _Import0000(ReadTextFile&);
	void _Import0101(ReadTextFile&);

	DECLARE_COUNTER;
};


#else
	#error File already included
#endif

