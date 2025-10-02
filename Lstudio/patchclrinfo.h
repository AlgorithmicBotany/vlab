#ifndef __PATCHCOLORINFO_H__
#define __PATCHCOLORINFO_H__


class PatchColorInfo
{
public:
	PatchColorInfo()
	{
		_TopColor = _BottomColor = 0;
		_TopDiffuse = _BottomDiffuse = 0.0; 
	}
	void SetTopColor(int tc)
	{ _TopColor = tc; }
	int GetTopColor() const
	{ return _TopColor; }
	void SetTopDiffuse(float tdiff)
	{ _TopDiffuse = tdiff; }
	float GetTopDiffuse() const
	{ return _TopDiffuse; }

	void SetBottomColor(int bc)
	{ _BottomColor = bc; }
	int GetBottomColor() const
	{ return _BottomColor; }
	void SetBottomDiffuse(float bdiff)
	{ _BottomDiffuse = bdiff; }
	float GetBottomDiffuse() const
	{ return _BottomDiffuse; }
	void Load(ReadTextFile&);
	void Write(WriteTextFile&) const;


	DWORD ClipboardSize() const;
	char* CopyToClipboard(char*) const;
	const char* LoadFromClipboard(const char*);
private:
	static const char* _StoreFormat()
	{ return "TC %d TD %f BC %d BD %f\n"; }
	int _TopColor;
	float _TopDiffuse;
	int _BottomColor;
	float _BottomDiffuse;
};



#else
	#error File already included
#endif
