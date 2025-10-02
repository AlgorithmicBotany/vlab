#ifndef __GENCTRL_H__
#define __GENCTRL_H__


namespace GenCtrl
{

const char ClassName[] = "GeneralCtrlClssNm";

class AbstractMaker
{
public:
	virtual Ctrl* Create(HWND, const CREATESTRUCT*) = 0;
};

bool OnCreate(HWND hWnd, const CREATESTRUCT* pCS);


LRESULT CALLBACK Proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void Register(HINSTANCE);
void Unregister(HINSTANCE);

class GWinMaker : public WinMaker
{
public:
	GWinMaker(HINSTANCE hInst) : WinMaker(ClassName, hInst) {}
};

class CreateData
{
public:
	CreateData(const CREATESTRUCT* pCS, void* pV) : _pCS(pCS), _pV(pV) {}
	template<typename Init>
	Init* GetInit() const
	{ return reinterpret_cast<Init*>(_pV); }
	int GetX() const
	{ return _pCS->x; }
	int GetY() const
	{ return _pCS->y; }
	int GetWidth() const
	{ return _pCS->cx; }
	int GetHeight() const
	{ return _pCS->cy; }
	HINSTANCE GetInstance() const
	{ return _pCS->hInstance; }
	HWND GetParent() const
	{ return _pCS->hwndParent; }
	const CREATESTRUCT* pCS() const
	{ return _pCS; }
private:
	const CREATESTRUCT* _pCS;
	void* _pV;
};

template<class Ct, typename Init>
class CtrlMaker : public AbstractMaker
{
public:
	CtrlMaker(Init* pInit) : _pInit(pInit) {}
	Ctrl* Create(HWND hWnd, const CREATESTRUCT* pCS)
	{
		CreateData cd(pCS, _pInit);
		return new Ct(hWnd, &cd); 
	}
private:
	Init* _pInit;
};

template<class Ct, typename Init>
Ct* CreateParam(GWinMaker& wm, Init* pInit)
{
	CtrlMaker<Ct, Init> cm(pInit);
	wm.lpData(&cm);
	Window w(wm.Create());
	Ct* pRes = reinterpret_cast<Ct*>(w.GetPtr());
	return pRes;
}

template<class Ct>
Ct* Create(GWinMaker& wm)
{
	return CreateParam<Ct, char>(wm, 0);
}


}

#endif
