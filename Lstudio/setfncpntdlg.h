#ifndef __SETFNCPNTDLG_H__
#define __SETFNCPNTDLG_H__


class SetFuncPntDlg : public Dialog
{
public:
	SetFuncPntDlg(WorldPointf, float, float);
	bool DoInit();
	void UpdateData(bool);
	float X() const
	{ return _x; }
	float Y() const
	{ return _y; }
private:
	bool _Check();
	float _x, _y;
	const float _minv, _maxv;
};

#else
	#error File already included
#endif
