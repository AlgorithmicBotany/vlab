#ifndef __PATCH_H__
#define __PATCH_H__


namespace PatchSpace
{
	enum DrawWhat
	{
		DrawMesh = 1,
			// More than one Draw[Dense | VeryDense]Wireframe parameter can be specified 
			// in call to Patch::Draw though it is waste of CPU time
			DrawWireframe = 1 << 1,
			DrawDenseWireframe = 1 << 2,
			DrawVeryDenseWireframe = 1 << 3,
			DrawShaded = 1 << 4,
			DrawKnotNumbers = 1 << 5,
			DrawKnots = 1 << 6
	};
};


class Patch
{
public:
	Patch();
	void Draw(int drawWhat, int activepoint, unsigned int) const;
	const WorldPointf& GetPoint(int ix) const
	{
		assert(ix>=0);
		assert(ix<16);
		return _pts[ix];
	}
	void SetPoint(int, WorldPointf);
	void SetPoint(int ix, WorldPointf p, WorldPointf, WorldPointf, WorldPointf);
	void SetX(int, float, WorldPointf, WorldPointf, WorldPointf);
	void SetY(int, float, WorldPointf, WorldPointf, WorldPointf);
	void SetZ(int, float, WorldPointf, WorldPointf, WorldPointf);
	void operator=(const Patch&);
	void Write(WriteTextFile&) const;
	void Load(ReadTextFile&);
	void Reset();
	void SetUsed()
	{ _isUsed = true; }
	bool YZSymmetric() const
	{ return _YZSymmetric; }
	void ToggleYZSymmetric() 
	{ _YZSymmetric = !_YZSymmetric; }

	bool ConnectedLeft() const
	{ return _cntLeft; }
	bool ConnectedRight() const
	{ return _cntRight; }
	bool ConnectedUp() const
	{ return _cntUp; }
	bool ConnectedDown() const
	{ return _cntDown; }

	void ConnectDown(const Patch&);
	void SetColorInfo(const PatchColorInfo& src)
	{ _ColorInfo = src; }
	const PatchColorInfo& GetColorInfo() const
	{ return _ColorInfo; }
	void GetBoundingBox(BoundingBox&) const;


	DWORD ClipboardSize() const;
	char* CopyToClipboard(char*) const;
	const char* LoadFromClipboard(const char*);

	bool IsEqual(const Patch*) const;
private:
	float _SymmetricX(float, WorldPointf cp, WorldPointf h, WorldPointf up);
	float _SymmetricY(float, WorldPointf cp, WorldPointf h, WorldPointf up);
	float _SymmetricZ(float, WorldPointf cp, WorldPointf h, WorldPointf up);
	WorldPointf _Symmetric(WorldPointf, WorldPointf cp, WorldPointf h, WorldPointf up);
	WorldPointf _pts[16];
	PatchColorInfo _ColorInfo;
	bool _YZSymmetric;
	bool _isUsed;
	bool _cntDown, _cntUp, _cntLeft, _cntRight;
};


#else
	#error File already included
#endif
