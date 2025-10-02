#ifndef __MATERIALEDITOR_H__
#define __MATERIALEDITOR_H__


class MaterialPreview;
class MaterialGallery;
class PrjNotifySink;

#define MATEDITRESIZEABLE



class MaterialEdit : public ContModeEdit
{
public:
	static HWND Create(HWND, HINSTANCE, PrjNotifySink*);

	bool Command(int, Window, UINT);
#ifdef MATEDITRESIZEABLE
	bool Size(SizeState, int, int);
#endif

	MaterialEdit(HWND, PrjNotifySink*);
	~MaterialEdit()
	{
	}
	const MaterialParams& GetPreviewParams() const;
	void SetPreviewParams(const MaterialParams&);
	void ApplyMaterial(const MaterialParams&);
	const MaterialParams& GetParamsFromGallery() const;

	void UpdateAmbient(COLORREF, bool);
	void UpdateDiffuse(COLORREF, bool);
	void UpdateSpecular(COLORREF, bool);
	void UpdateEmission(COLORREF, bool);
	void UpdateShininess(COLORREF, bool);
	void UpdateTransparency(COLORREF, bool);

	void Import(const TCHAR* fname);
	void Export() const;
	bool IsNamed() const
	{ return 0 != _Filename[0]; }
	void SetName(const TCHAR* fname)
	{ _tcscpy(_Filename, fname); }

	void Generate() const;
	const TCHAR* Name() const;

	void ApplyNow();
	bool SelectionChanged(int, int);

	void Activated(bool act)
	{ _SharedPickClr.Show(act); }
protected:
	bool _ObjectModified(bool) const;
private:

	void _UpdateControls();
	void _UpdateView();


	MaterialPreview* _thePreview;
	MaterialGallery* _theGallery;

	MaterialParamCallback _AmbientCallback;
	MaterialParamCallback _DiffuseCallback;
	MaterialParamCallback _EmissionCallback;
	MaterialParamCallback _SpecularCallback;
	MaterialParamCallback _ShininessCallback;
	MaterialParamCallback _TransparencyCallback;
	ColorSlider* _pAmbient;
	ColorSlider* _pDiffuse;
	ColorSlider* _pEmission;
	ColorSlider* _pSpecular;
	ColorSlider* _pShininess;
	ColorSlider* _pTransparency;

	TCHAR _Filename[_MAX_PATH+1];

	SharedPickColor _SharedPickClr;

#ifdef MATEDITRESIZEABLE
	GeometryConstrain _ColormapBtn;
	GeometryConstrain _Gallery;
	GeometryConstrain _MatNo;
	GeometryConstrain _MatId;
	GeometryConstrain _Transp;
	GeometryConstrain _TranspLbl;
	GeometryConstrain _Shin;
	GeometryConstrain _ShinLbl;
	GeometryConstrain _Emiss;
	GeometryConstrain _EmissLbl;
	GeometryConstrain _Spec;
	GeometryConstrain _SpecLbl;
	GeometryConstrain _Diffuse;
	GeometryConstrain _DiffLbl;
	GeometryConstrain _Amb;
	GeometryConstrain _AmbLbl;
	GeometryConstrain _Preview;
#endif
};


#else
	#error File already included
#endif
