#ifndef __TRADITIONAL_H__
#define __TRADITIONAL_H__

class TraditionalPatch
{
public:
	TraditionalPatch(ReadTextFile&);
	TraditionalPatch(const char**); // Create from clipboard data
	TraditionalPatch(const TraditionalPatch& src)
	{
		memcpy(_Name, src._Name, MaxNameLength);
		_ColorInfo = src._ColorInfo;
		memcpy(_ANeighbours, src._ANeighbours, MaxNeighboursLength);
		memcpy(_LNeighbours, src._LNeighbours, MaxNeighboursLength);
		memcpy(_BNeighbours, src._BNeighbours, MaxNeighboursLength);
		for (int i=0; i<16; ++i)
			_pts[i] = src._pts[i];
		INC_COUNTER;
	}
	~TraditionalPatch();
	WorldPointf GetPoint(int i) const
	{
		assert(i>=0);
		assert(i<16);
		return _pts[i];
	}
	void Generate(WriteTextFile& trg) const;
	const PatchColorInfo& GetColorInfo() const
	{ return _ColorInfo; }

	DWORD ClipboardSize() const;
	char* CopyToClipboard(char*) const;
	void Draw() const;
private:
	enum { MaxNameLength = 64 };
	char _Name[MaxNameLength];
	PatchColorInfo _ColorInfo;
	enum { MaxNeighboursLength = 64 };
	char _ANeighbours[MaxNeighboursLength];
	char _LNeighbours[MaxNeighboursLength];
	char _BNeighbours[MaxNeighboursLength];
	WorldPointf _pts[16];

	DECLARE_COUNTER;
};


class TraditionalSurface
{
public:
	TraditionalSurface(ReadTextFile&, int num=-1);
	TraditionalSurface(const char**); // Create from clipboard data
	~TraditionalSurface();
	int Patches() const
	{ return _aPatches.size(); }
	WorldPointf ContactPoint() const
	{ return _cp; }
	WorldPointf EndPoint() const
	{ return _ep; }
	const TraditionalPatch& AccessFirst() const
	{ 
		assert(!_aPatches.empty());
		return *(_aPatches.begin()); 
	}
	void Generate(WriteTextFile&) const;
	const ViewBox& GetBoundingBox() const
	{ return _bb; }
	float Size() const
	{ return _size; }
	const WorldPointf& Heading() const
	{ return _heading; }
	const WorldPointf& Up() const
	{ return _up; }

	DWORD ClipboardSize() const;
	char* CopyToClipboard(char*) const;
	void Draw() const;
private:
	ViewBox _bb;
	int _precisionT, _precisionS;
	WorldPointf _cp;
	WorldPointf _ep;
	WorldPointf _heading;
	WorldPointf _up;
	float _size;
	typedef std::vector<TraditionalPatch>::const_iterator cit;
	std::vector<TraditionalPatch> _aPatches;

	DECLARE_COUNTER;
};



#else
	#error File already included
#endif
