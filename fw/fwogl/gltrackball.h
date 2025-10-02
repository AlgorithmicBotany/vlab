#ifndef __GLTRACKBALL_H__
#define __GLTRACKBALL_H__



class GLTrackball : public OpenGLWindow
{
public:
	GLTrackball(HWND, const CREATESTRUCT*);
	~GLTrackball();

	bool LButtonDown(KeyState, int, int);
	bool MouseMove(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool CaptureChanged();

	virtual void MapScreenToWorld(int, int, WorldPointf&) const;
	void RotateBy(int, int);
	void ResetRotation();
protected:

	void _DoSize();
	void _DoOther();
	virtual void _SetViewbox() = 0;

	GLRotateTask _RotateTask;
	GLTask* _pTask;

	float _rotX, _rotY;
	ViewBox _viewbox;
	float _upp;
};


#endif
