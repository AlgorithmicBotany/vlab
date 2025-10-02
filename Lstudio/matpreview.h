#ifndef __MATERIALPREVIEW_H__
#define __MATERIALPREVIEW_H__


class Material;
class MaterialEdit;

class MaterialPreview : public OpenGLWindow, public ObjectView
{
public:
	static void Register(HINSTANCE);

	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);

	MaterialPreview(HWND, const CREATESTRUCT*);
	~MaterialPreview();

	
	const MaterialParams& GetMaterialParams() const;
	void SetTransparency(GLfloat);
	void SetShininess(int);
	void SetEmission(const GLfloat*);
	void SetSpecular(const GLfloat*);
	void SetDiffuse(const GLfloat*);
	void SetAmbient(const GLfloat*);
	void ApplyAndRedraw();
	

	void SetMaterialEdit(MaterialEdit* pEdit);

	void ForceRepaint()
	{ 
		ApplyAndRedraw();
	}

private:

	void _DoInit();
	void _DoExit();
	void _DoPaint() const;
	void _DoSize();


	Material* _GetMaterial();
	const Material* _GetMaterial() const;

	GLUquadricObj* _pQobj;

	MaterialEdit* _theEdit;

	static const TCHAR* _ClassName()
	{ return __TEXT("MaterialPreview"); }

	DECLARE_COUNTER;
};


#else
	#error File already included
#endif
