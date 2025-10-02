/**************************************************************************

  File:		button.h
  Created:	03-Feb-98


  Declaration of class Button 


**************************************************************************/


#ifndef __BUTTON_H__
#define __BUTTON_H__

class Icon;

class Button : public Window
{
public:
	Button(Window w) : Window(w) {}
	void SetCheck(bool state = true)
	{ Button_SetCheck(Hwnd(), state ? BST_CHECKED : BST_UNCHECKED); }
	bool IsChecked() const
	{ return BST_CHECKED == Button_GetCheck(Hwnd()); }
	void SetIcon(const Icon&);
};


#endif
