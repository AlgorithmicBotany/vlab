#ifndef __CONTMODEDIT_H__
#define __CONTMODEDIT_H__



// ContModeEdit -- Editor ready to work in the continuous modeling mode


class ContModeEdit : public FormCtrl, public ObjectEdit
{
public:
	ContModeEdit(HWND, PrjNotifySink*);
	bool Timer(UINT);

	virtual void Modified(bool);
protected:
	virtual bool _ObjectModified(bool) const = 0;
	PrjNotifySink* const _pNotifySink;
private:
	bool _InSync;
	FW::Timer _timer;
	enum 
	{ 
		tTimerId = 101,
			SyncTimerTimeout = 100
	};
};


#else
	#error File already included
#endif
