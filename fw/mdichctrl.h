/**************************************************************************

  File:		mdichctrl.h
  Created:	30-Jan-98


  Declaration of class MDIChildCtrl


**************************************************************************/


#ifndef __MDICHCTRL_H__
#define __MDICHCTRL_H__


class MDIChildCtrl : public Ctrl
{
public:
	MDIChildCtrl(HWND, const CREATESTRUCT*, int);
	~MDIChildCtrl();

	// Message handlers
	virtual bool MDIActivate(bool, HWND);

	// operations
	void Activate();
	bool IsActive() const
	{ return _active; }
protected:
	virtual HMENU DocumentMenu() const;
	virtual HMENU DocumentWindowMenu() const;
	void SetDocumentMenu();
	void SetDefaultMenu();
private:
	const int _DocTypeID;
	bool _active;
};


#endif
