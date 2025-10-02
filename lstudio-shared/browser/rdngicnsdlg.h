#ifndef __READINGICNSDLG_H__
#define __READINGICNSDLG_H__

namespace VLB
{

class ReadingIcnsDlg : public Dialog
{
public:
	ReadingIcnsDlg(Canvas&, Connection*, Node*, int, WORD, int);
	bool DoInit();
	bool Command(int, Window, UINT);
private:

	void _DoThread();
	static void _ThreadFunc(void* pV)
	{ 
		ReadingIcnsDlg* pSelf = reinterpret_cast<ReadingIcnsDlg*>(pV);
		pSelf->_DoThread();
	}

	const int _progress_id;
	Canvas& _cnv;
	Connection* _pConnection;
	Node* _pParent;
	ProgressBar _Progress;
	LONG _abort;
	const int _width;

};


}


#else
	#error File already included
#endif


