#ifndef __EDITOR_H__
#define __EDITOR_H__


class Editor
{
public:
	virtual void MoveWindow(RECT) = 0;
	virtual void SetFocus() = 0;
	virtual void Activated(bool) = 0;
	virtual void Show() = 0;
	virtual void Hide() = 0;
	virtual void ColorschemeModified() = 0;
	virtual void Generate() const = 0;
	virtual void Clear() = 0;
	virtual void Load(const char*) = 0;
	virtual void ExecuteMode() = 0;
	virtual void SetEditorFont(HFONT) = 0;
	virtual void Find() = 0;
	virtual void FindAgain() = 0;
	virtual const char* Name() const = 0;
	virtual HWND HEdit() = 0;
	virtual bool Modified() const = 0;
	virtual bool Import(const char*) = 0;
	virtual bool IsEmpty() const = 0;
	virtual void Enable(bool) = 0;
};

typedef Editor* PEditor;

#endif
