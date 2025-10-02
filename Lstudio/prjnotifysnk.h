#ifndef __PRJNOTIFYSINK_H__
#define __PRJNOTIFYSINK_H__

class PrjNotifySink
{
public:
	virtual void UseMaterials() = 0;
	virtual void UseColormap() = 0;
	virtual void AdvancedSurfaceMode() = 0;
	virtual void SimpleSurfaceMode() = 0;
	virtual bool ColormapModified(bool) = 0;
	virtual bool MaterialModified(bool) = 0;
	virtual bool FunctionModified(bool) = 0;
	virtual bool ContourCurveModified(bool) = 0;
	virtual bool ContourModified(bool) = 0;
	virtual bool SurfaceModified(bool) = 0;
	virtual bool CurveXYZModified(bool) = 0;
	virtual bool LsystemModified(bool) = 0;
	virtual bool ViewFileModified(bool) = 0;
	virtual bool ExternalFileModified(bool) = 0;
	virtual bool NewModel(bool) = 0;
	virtual bool ContinuousMode() const = 0;
	virtual void ContinuousMode(bool) = 0;
	virtual void ReadLSspecifications() = 0;
	virtual void ReadSpecifications() = 0;
	virtual HWND GetViewEditWnd() const = 0;
	virtual HWND GetLsystemEditWnd() const = 0;
	virtual const std::string& GetLabTable() const = 0;
};


#else
	#error File already included
#endif
