#ifndef __SIZESTATE_H__
#define __SIZESTATE_H__


class SizeState
{
public:
	explicit SizeState(WPARAM st) : _st(static_cast<UINT>(st)) {}
	UINT Value() const
	{ return _st; }
	bool Minimized() const
	{ return _st == SIZE_MINIMIZED; }
private:
	UINT _st;
};

#endif
