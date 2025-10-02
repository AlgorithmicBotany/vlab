/**************************************************************************

  File:		selectidlg.h
  Created:	04-Dec-97


  Declaration of class SelectIdDlg


**************************************************************************/



#ifndef __SELECTIDDLG_H__
#define __SELECTIDDLG_H__


class SelectIdDlg : public Dialog
{
public:
	SelectIdDlg();
	int GetId();
protected:
	int _id;
};

#else
	#error File already included
#endif
