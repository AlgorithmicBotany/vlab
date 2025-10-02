#ifndef __ANIMATEEDIT_H__
#define __ANIMATEEDIT_H__


class AnimateEdit : public FormCtrl
{
public:
	AnimateEdit(HWND);
	~AnimateEdit();

	static HWND Create(HWND, HINSTANCE);

	// Message handlers
	bool Command(int, Window, UINT);

	void Import(const TCHAR*);
	void Export() const;
	void Clear();

	void Generate() const;
	const TCHAR* Name() const;
private:

	void _UpdateControls() const;
	void _InterpretLine(const char*);
	EditLine _Last;

	AnimData _data;

	TCHAR _Filename[_MAX_PATH+1];

	DECLARE_COUNTER;
};


#else
	#error File already included
#endif
