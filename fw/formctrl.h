#ifndef __FORMCTRL_H__
#define __FORMCTRL_H__


class FormCtrl : public Ctrl
{
public:
	FormCtrl(HWND);
	~FormCtrl();
	void SetFocus(HWND, HWND);
	void KillFocus(HWND, HWND);
	Window GetDlgItem(int id) const
	{ return Window(::GetDlgItem(Hwnd(), id)); }
	static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

protected:
	class Creator
	{
	public:
		virtual FormCtrl* Create(HWND) = 0;
	};
	template <typename T>
		class FormCreator : public Creator
	{
		FormCtrl* Create(HWND hDlg)
		{ return new T(hDlg); }
	};
};


#endif
