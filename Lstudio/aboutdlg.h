#ifndef __ABOUTDLG_H__
#define __ABOUTDLG_H__


class AboutDlg : public Dialog
{
public:
	AboutDlg();
	bool DoInit();
};

#else
	#error File already included
#endif
