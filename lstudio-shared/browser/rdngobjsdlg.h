#ifndef __READINGOBJSDLG_H__
#define __READINGOBJSDLG_H__


namespace VLB
{

class ReadingObjsDlg : public Dialog
{
public:
	ReadingObjsDlg(Canvas&, Connection*, Node*, WORD dlgid, int path_id);

	bool DoInit();
	bool Command(int, Window, UINT);
private:

	void _DoThread();
	static void _ThreadFunc(void* pV)
	{ 
		ReadingObjsDlg* pSelf = reinterpret_cast<ReadingObjsDlg*>(pV);
		pSelf->_DoThread();
	}

	const int _path_id;
	Canvas& _cnv;
	Connection* _pConnection;
	Node* _pParent;
	Window _Text;
	LONG _abort;
};

}


#else
	#error File already included
#endif

