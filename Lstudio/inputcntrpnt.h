#ifndef __INPUTCONTOURPOINT_H__
#define __INPUTCONTOURPOINT_H__


class SetCntrPntDlg : public Dialog
{
public:
	SetCntrPntDlg(const WorldPointf& p) : 
	  Dialog(IDD_INPUTCNTRPOINT),
	  _x(p.X()), _y(p.Y())
	{}
	void UpdateData(bool);
	float GetX() const
	{ return _x; }
	float GetY() const
	{ return _y; }
private:
	float _x, _y;
};

#endif
