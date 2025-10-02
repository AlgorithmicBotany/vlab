/**************************************************************************

  File:		tabctrl.h
  Created:	02-Feb-98


  Declaration of class TabCtrl


**************************************************************************/


#ifndef __TABCTRL_H__
#define __TABCTRL_H__



class TabCtrl : public Window
{
public:
	TabCtrl(const Window*, const UINT*, int, int);
	void AdjustRect(bool flag, RECT* pR)
	{ TabCtrl_AdjustRect(Hwnd(), flag, pR); }
	int GetCurSel()
	{ return TabCtrl_GetCurSel(Hwnd()); }
	void GetItemRect(int i, RECT& r)
	{ TabCtrl_GetItemRect(Hwnd(), i, &r); }
	void SetCurSel(int n)
	{ TabCtrl_SetCurSel(Hwnd(), n); }
	void SetImageList(HIMAGELIST hList)
	{ TabCtrl_SetImageList(Hwnd(), hList); }
	void SetTabText(const UINT*, int);
	void SetTabText(int, const std::string&);
	void SetTabIcons(int);
	void SetTabTextAndIcons(const UINT*, int);
	void SetTabTextAndIcon(int, const std::string&);
};


#endif
