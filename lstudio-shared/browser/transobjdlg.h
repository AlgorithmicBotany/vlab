#ifndef __TRANSFEROBJSDLG_H__
#define __TRANSFEROBJSDLG_H__


namespace VLB
{


class TransferObjsDlg : public Dialog
{
public:
	TransferObjsDlg(RemoteAccess* pSrc, RemoteAccess* pTrg, const std::string& srcpth, const std::string& trgpth);
private:
	RemoteAccess* _pSrc;
	RemoteAccess* _pTrg;
	const std::string& _srcpth;
	const std::string& _trgpth;
};


}


#endif
