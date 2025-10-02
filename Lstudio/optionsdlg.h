#ifndef __OPTIONSDLG_H__
#define __OPTIONSDLG_H__


class LStudioOptions;

class OptionsDlg : public Dialog
{
public:
	OptionsDlg(const LStudioOptions&);
	void UpdateData(bool);
	void UpdateOptions(LStudioOptions&) const;
private:
	int _showCtl;
};


#else
	#error File already included
#endif
