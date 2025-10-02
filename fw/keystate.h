#ifndef __KEYSTATE_H__
#define __KEYSTATE_H__

class KeyState
{
public:
	explicit KeyState(WPARAM state) : _state(static_cast<UINT>(state)) 
	{}
	bool IsShift() const
	{ return (MK_SHIFT & _state) != 0; }
	bool IsCtrl() const
	{ return (MK_CONTROL & _state) != 0; }
	bool IsLButton() const
	{ return (MK_LBUTTON & _state) != 0; }

	enum eState
	{
		fShift = MK_SHIFT,
		fCtrl = MK_CONTROL
	};

	void SetState(eState st)
	{ _state = st; }
private:
	UINT _state;
};


#endif
