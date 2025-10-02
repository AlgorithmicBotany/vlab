#ifndef __CONTOUR_H__
#define __CONTOUR_H__


class Matrix;
class Matrix4x3;
class CtrlPoint;

class Contour : public EditableObject
{
public:
	Contour();
	~Contour();
	void Draw(int) const;
	void DrawInGallery() const;
	void Copy(const EditableObject*);
	EditableObject* Clone() const;
	int AddPoint(WorldPointf);
	void DeletePoint(int);
	void SetClosed(bool set)
	{ _closed = set; }
	bool IsClosed() const
	{ return _closed; }
	int NumOfPoints() const
	{ return _items; }

	enum DrawWhat
	{
		DrawPoints = 1,
		DrawSegments = 1 << 1,
		DrawCurve = 1 << 2
	};

	enum ContourType
	{
		btRegular,
		btEndPoint
	};

	ContourType Type() const
	{ return _type; }
	void SetType(ContourType);

	bool IsValidSamples(int i) const
	{
		return (i>=MinSamples && i<=MaxSamples);
	}

	int GetSamples() const
	{ return _samples; }
	void SetSamples(int);

	WorldPointf GetPoint(int i) const;
	int GetPointMultiplicity(int i) const;
	void MovePoint(int i, WorldPointf p);
	void IncMultiplicity(int);
	int GetClosestPoint(WorldPointf) const;
	void Reset();
	const char* GetName() const
	{ return _Name; }
	void SetName(const char* name)
	{
		assert(strlen(name)<MaxNameLength);
		strcpy(_Name, name);
	}
	bool IsNamed() const;
	bool IsNamed(const char*) const;
	void GetBoundingBox(BoundingBox&) const;
	void Import(ReadTextFile&);
	void Generate(WriteTextFile&) const;
	bool CanDelete(int) const;
	bool CanIncMultiplicity(int) const;
	bool IsCurve() const
	{ return 10 == _version; }

	// Copies data from memory. Returned pointer
	// points to the memory immediately after the retrieved data
	const char* LoadFromClipboard(const char*); 
	DWORD ClipboardSize() const;
	char* CopyToClipboard(char*) const;

	bool operator!=(const EditableObject&) const;

	void SetVersion(int);
	int GetVersion() const
	{ return _version; }
private:
	enum
	{
		DefaultVersion = 103,
		MaxNameLength = 32,
		DefaultSamples = 16,
		MinSamples = 4,
		MaxSamples = 128
	};
	static const char* _DefaultName() 
	{ return "unnamed"; }

	CtrlPoint* _arr;
	int _pts; // number of control points (includes
              // multiplicity information)
	int _size;
	int _items;
	bool _closed;
	int _samples; 

	int _steps; // determines precision of the drawn curve (drawing only)

	ContourType _type;

	void _SetSize(int); // Clears contents
	void _DrawRegularCurve() const;
	void _DrawEndPtCurve() const;
	static WorldPointf _P(float, const std::vector<WorldPointf>&);
	static float _N(int, int, float, int);
	static float _Nk1(int, float, int);
	static float _Nkt(int, int, float, int);
	static int _Uk(int, int);

	void _DrawCurve() const;
	void _DrawCurve(const Matrix4x3&) const;
	static WorldPointf _BSpline(const Matrix&, const Matrix4x3&);

	void _Grow();
	void _Insert(int, WorldPointf);
	int _InsertSmart(WorldPointf);

	char _Name[MaxNameLength];

	int _version;

	void _Import0000(const char*, ReadTextFile&);
	void _Import0010(ReadTextFile&);
	void _Import0101(ReadTextFile&);
	void _Import0102(ReadTextFile&);
	void _Import0103(ReadTextFile&);
	void _Generate0000(WriteTextFile&) const;
	void _Generate0010(WriteTextFile&) const;
	void _Generate0101(WriteTextFile&) const;
	void _Generate0102(WriteTextFile&) const;
	void _Generate0103(WriteTextFile&) const;

	DECLARE_COUNTER;
};


#else
	#error File already included
#endif
