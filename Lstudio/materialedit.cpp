#include <memory>

#include <fw.h>
#include <glfw.h>

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "glgallery.h"

#include "prjnotifysnk.h"
#include "contmodedit.h"

#include "resource.h"

#include "material.h"
#include "matparamcb.h"
#include "materialedit.h"
#include "matpreview.h"
#include "matgallery.h"


HWND MaterialEdit::Create(HWND hParent, HINSTANCE hInst, PrjNotifySink* pNotifySink)
{
	class MaterialEditCreator : public Creator
	{
	public:
		MaterialEditCreator(PrjNotifySink* pNotifySink) : _pNotifySink(pNotifySink)
		{}
		FormCtrl* Create(HWND hDlg)
		{ return new MaterialEdit(hDlg, _pNotifySink); }
	private:
		PrjNotifySink* _pNotifySink;
	};
	MaterialEditCreator creator(pNotifySink);

	return CreateDialogParam
		(
		hInst, 
		MAKEINTRESOURCE(IDD_MATERIAL),
		hParent,
		reinterpret_cast<DLGPROC>(FormCtrl::DlgProc),
		reinterpret_cast<LPARAM>(&creator)
		);
}


MaterialEdit::MaterialEdit(HWND hwnd, PrjNotifySink* pNotifySink) : 
ContModeEdit(hwnd, pNotifySink),
_AmbientCallback(this, MaterialParamCallback::eAmbient),
_DiffuseCallback(this, MaterialParamCallback::eDiffuse),
_EmissionCallback(this, MaterialParamCallback::eEmission),
_SpecularCallback(this, MaterialParamCallback::eSpecular),
_ShininessCallback(this, MaterialParamCallback::eShininess),
_TransparencyCallback(this, MaterialParamCallback::eTransparency)
#ifdef MATEDITRESIZEABLE
,
_ColormapBtn(GetDlgItem(IDC_COLORMAP)),
_Gallery(GetDlgItem(IDC_GALLERY)),
_MatNo(GetDlgItem(IDC_MATNO)),
_MatId(GetDlgItem(IDC_OBJID)),
_Transp(GetDlgItem(IDC_TRANSPARENCY)),
_TranspLbl(GetDlgItem(IDC_TRANSPLBL)),
_Shin(GetDlgItem(IDC_SHININESS)),
_ShinLbl(GetDlgItem(IDC_SHINLBL)),
_Emiss(GetDlgItem(IDC_EMISSION)),
_EmissLbl(GetDlgItem(IDC_EMISSLBL)),
_Spec(GetDlgItem(IDC_SPECULAR)),
_SpecLbl(GetDlgItem(IDC_SPECLBL)),
_Diffuse(GetDlgItem(IDC_DIFFUSE)),
_DiffLbl(GetDlgItem(IDC_DIFFLBL)),
_Amb(GetDlgItem(IDC_AMBIENT)),
_AmbLbl(GetDlgItem(IDC_AMBLBL)),
_Preview(GetDlgItem(IDC_MATPREVIEW))
#endif
{
	_Filename[0] = 0;
	_thePreview = reinterpret_cast<MaterialPreview*>(GetDlgItem(IDC_MATPREVIEW).GetPtr());
	_theGallery = reinterpret_cast<MaterialGallery*>(GetDlgItem(IDC_GALLERY).GetPtr());
	_pView = _thePreview;
	_pGallery = _theGallery;

	_pAmbient = reinterpret_cast<ColorSlider*>(GetDlgItem(IDC_AMBIENT).GetPtr());
	_pDiffuse = reinterpret_cast<ColorSlider*>(GetDlgItem(IDC_DIFFUSE).GetPtr());
	_pEmission = reinterpret_cast<ColorSlider*>(GetDlgItem(IDC_EMISSION).GetPtr());
	_pSpecular = reinterpret_cast<ColorSlider*>(GetDlgItem(IDC_SPECULAR).GetPtr());
	_pShininess = reinterpret_cast<ColorSlider*>(GetDlgItem(IDC_SHININESS).GetPtr());
	_pTransparency = reinterpret_cast<ColorSlider*>(GetDlgItem(IDC_TRANSPARENCY).GetPtr());

	_theGallery->SetMaterialEdit(this);
	_thePreview->SetMaterialEdit(this);

	_pAmbient->SetCallback(&_AmbientCallback);
	_pAmbient->SetSharedPickColor(&_SharedPickClr);
	_pDiffuse->SetCallback(&_DiffuseCallback);
	_pDiffuse->SetSharedPickColor(&_SharedPickClr);
	_pEmission->SetCallback(&_EmissionCallback);
	_pEmission->SetSharedPickColor(&_SharedPickClr);
	_pSpecular->SetCallback(&_SpecularCallback);
	_pSpecular->SetSharedPickColor(&_SharedPickClr);
	_pShininess->SetCallback(&_ShininessCallback);
	_pTransparency->SetCallback(&_TransparencyCallback);

	{
		//LongString lbl(64);
		ResString lbl(64, IDS_AMBIENT);
		//lbl.Load(IDS_AMBIENT);
		_pAmbient->SetLabel(lbl);
		lbl.Load(IDS_DIFFUSE);
		_pDiffuse->SetLabel(lbl);
		lbl.Load(IDS_SPECULAR);
		_pSpecular->SetLabel(lbl);
		lbl.Load(IDS_EMISSION);
		_pEmission->SetLabel(lbl);
	}

	_UpdateControls();	

#ifdef MATEDITRESIZEABLE
	// Set up constrains
	{
		RECT dlgrect;
		GetWindowRect(dlgrect);
		_ColormapBtn.SetLeft(dlgrect);
		_ColormapBtn.SetWidth();
		_ColormapBtn.SetBottom(dlgrect);
		_ColormapBtn.SetHeight();
		_ColormapBtn.SetMinTop(dlgrect);

		_Gallery.SetLeft(dlgrect);
		_Gallery.SetRight(dlgrect);
		_Gallery.SetBottom(dlgrect);
		_Gallery.SetHeight();
		_Gallery.SetMinWidth();
		_Gallery.SetMinTop(dlgrect);

		_MatNo.SetTop(dlgrect);
		_MatNo.SetRight(dlgrect);
		_MatNo.SetHeight();
		_MatNo.SetWidth();
		_MatNo.SetMinLeft(dlgrect);
		_MatId.SetTop(dlgrect);
		_MatId.SetRight(dlgrect);
		_MatId.SetHeight();
		_MatId.SetWidth();
		_MatId.SetMinLeft(dlgrect);
		_Transp.SetTop(dlgrect);
		_Transp.SetRight(dlgrect);
		_Transp.SetHeight();
		_Transp.SetWidth();
		_Transp.SetMinLeft(dlgrect);
		_TranspLbl.SetTop(dlgrect);
		_TranspLbl.SetRight(dlgrect);
		_TranspLbl.SetHeight();
		_TranspLbl.SetWidth();
		_TranspLbl.SetMinLeft(dlgrect);
		_Shin.SetTop(dlgrect);
		_Shin.SetRight(dlgrect);
		_Shin.SetHeight();
		_Shin.SetWidth();
		_Shin.SetMinLeft(dlgrect);
		_ShinLbl.SetTop(dlgrect);
		_ShinLbl.SetRight(dlgrect);
		_ShinLbl.SetHeight();
		_ShinLbl.SetWidth();
		_ShinLbl.SetMinLeft(dlgrect);
		_Emiss.SetTop(dlgrect);
		_Emiss.SetRight(dlgrect);
		_Emiss.SetHeight();
		_Emiss.SetWidth();
		_Emiss.SetMinLeft(dlgrect);
		_EmissLbl.SetTop(dlgrect);
		_EmissLbl.SetRight(dlgrect);
		_EmissLbl.SetHeight();
		_EmissLbl.SetWidth();
		_EmissLbl.SetMinLeft(dlgrect);
		_Spec.SetTop(dlgrect);
		_Spec.SetRight(dlgrect);
		_Spec.SetHeight();
		_Spec.SetWidth();
		_Spec.SetMinLeft(dlgrect);
		_SpecLbl.SetTop(dlgrect);
		_SpecLbl.SetRight(dlgrect);
		_SpecLbl.SetHeight();
		_SpecLbl.SetWidth();
		_SpecLbl.SetMinLeft(dlgrect);
		_Diffuse.SetTop(dlgrect);
		_Diffuse.SetRight(dlgrect);
		_Diffuse.SetHeight();
		_Diffuse.SetWidth();
		_Diffuse.SetMinLeft(dlgrect);
		_DiffLbl.SetTop(dlgrect);
		_DiffLbl.SetRight(dlgrect);
		_DiffLbl.SetHeight();
		_DiffLbl.SetWidth();
		_DiffLbl.SetMinLeft(dlgrect);
		_Amb.SetTop(dlgrect);
		_Amb.SetRight(dlgrect);
		_Amb.SetHeight();
		_Amb.SetWidth();
		_Amb.SetMinLeft(dlgrect);
		_AmbLbl.SetTop(dlgrect);
		_AmbLbl.SetRight(dlgrect);
		_AmbLbl.SetHeight();
		_AmbLbl.SetWidth();
		_AmbLbl.SetMinLeft(dlgrect);

		_Preview.SetLeft(dlgrect);
		_Preview.SetTop(dlgrect);
		_Preview.SetRight(dlgrect);
		_Preview.SetBottom(dlgrect);
		_Preview.SetMinWidth();
		_Preview.SetMinHeight();
	}		
#endif
}



bool MaterialEdit::Command(int id, Window, UINT)
{
	switch (id)
	{
	case IDC_COLORMAP :
		assert(0 != _pNotifySink);
		_pNotifySink->UseColormap();
		break;
	}
	return true;
}


void MaterialEdit::UpdateAmbient(COLORREF clr, bool final)
{
	GLfloat ambient[3];
	ambient[0] = GetRValue(clr)/255.0f;
	ambient[1] = GetGValue(clr)/255.0f;
	ambient[2] = GetBValue(clr)/255.0f;
	_thePreview->SetAmbient(ambient);
	_thePreview->ApplyAndRedraw();
	Modified(final);
}


void MaterialEdit::UpdateDiffuse(COLORREF clr, bool final)
{
	GLfloat diffuse[3];
	diffuse[0] = GetRValue(clr)/255.0f;
	diffuse[1] = GetGValue(clr)/255.0f;
	diffuse[2] = GetBValue(clr)/255.0f;
	_thePreview->SetDiffuse(diffuse);
	_thePreview->ApplyAndRedraw();
	Modified(final);
}


void MaterialEdit::UpdateSpecular(COLORREF clr, bool final)
{
	GLfloat specular[3];
	specular[0] = GetRValue(clr)/255.0f;
	specular[1] = GetGValue(clr)/255.0f;
	specular[2] = GetBValue(clr)/255.0f;
	_thePreview->SetSpecular(specular);
	_thePreview->ApplyAndRedraw();
	Modified(final);
}


void MaterialEdit::UpdateEmission(COLORREF clr, bool final)
{
	GLfloat emission[3];
	emission[0] = GetRValue(clr)/255.0f;
	emission[1] = GetGValue(clr)/255.0f;
	emission[2] = GetBValue(clr)/255.0f;
	_thePreview->SetEmission(emission);
	_thePreview->ApplyAndRedraw();
	Modified(final);
}


void MaterialEdit::UpdateShininess(COLORREF clr, bool final)
{
	_thePreview->SetShininess(GetRValue(clr)/2);
	_thePreview->ApplyAndRedraw();
	Modified(final);
}

void MaterialEdit::UpdateTransparency(COLORREF clr, bool final)
{
	GLfloat transparency = GetRValue(clr)/255.0f;
	_thePreview->SetTransparency(transparency);
	_thePreview->ApplyAndRedraw();
	Modified(final);
}

void MaterialEdit::_UpdateView()
{
}


void MaterialEdit::_UpdateControls()
{
	const MaterialParams& Src = _thePreview->GetMaterialParams();
	_pAmbient->SetColor(Src.GetAmbient());
	_pDiffuse->SetColor(Src.GetDiffuse());
	_pEmission->SetColor(Src.GetEmission());
	_pSpecular->SetColor(Src.GetSpecular());
	{
		int shn = Src.GetShininess();
		if (128==shn)
			shn=255;
		else
			shn*=2;
		_pShininess->SetColor(shn);
	}
	_pTransparency->SetColor(Src.GetTransparency());
	_pAmbient->Invalidate();
	_pDiffuse->Invalidate();
	_pEmission->Invalidate();
	_pSpecular->Invalidate();
	_pShininess->Invalidate();
	_pTransparency->Invalidate();
	SetDlgItemInt(Hwnd(), IDC_OBJID, _theGallery->GetCurrent(), true);
}


const TCHAR* MaterialEdit::Name() const
{
	if (0 != _Filename[0])
		return _Filename;
	else
		return __TEXT("material.mat");
}


void MaterialEdit::Generate() const
{
	WriteBinFile trg(Name());
	_theGallery->Generate(trg);
}


void MaterialEdit::Import(const TCHAR* fname)
{
	_theGallery->Import(fname);
	_tcscpy(_Filename, fname);
	Retrieve();
}


#ifdef MATEDITRESIZEABLE
bool MaterialEdit::Size(SizeState, int w, int h)
{
	_ColormapBtn.Adjust(w, h);
	_Gallery.Adjust(w, h);
	_MatNo.Adjust(w, h);
	_MatId.Adjust(w, h);
	_Transp.Adjust(w, h);
	_TranspLbl.Adjust(w, h);
	_Shin.Adjust(w, h);
	_ShinLbl.Adjust(w, h);
	_Emiss.Adjust(w, h);
	_EmissLbl.Adjust(w, h);
	_Spec.Adjust(w, h);
	_SpecLbl.Adjust(w, h);
	_Diffuse.Adjust(w, h);
	_DiffLbl.Adjust(w, h);
	_Amb.Adjust(w, h);
	_AmbLbl.Adjust(w, h);
	_Preview.Adjust(w, h);
	return true;
}
#endif

void MaterialEdit::Export() const
{
	Generate();
}

bool MaterialEdit::_ObjectModified(bool final) const
{
	return _pNotifySink->MaterialModified(final);
}

void MaterialEdit::ApplyNow()
{
	ObjectEdit::ApplyNow();
	_pNotifySink->MaterialModified(true);
}


bool MaterialEdit::SelectionChanged(int cur, int newsel)
{
	const bool res = ObjectEdit::SelectionChanged(cur, newsel);
	if (res)
		_pNotifySink->MaterialModified(true);
	return res;
}

