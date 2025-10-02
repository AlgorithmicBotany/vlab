#include <fw.h>

#include "resource.h"

#include "lstudioptns.h"
#include "params.h"
#include "prjnotifysnk.h"
#include "tedit.h"
#include "lprjctrl.h"
#include "objfgvedit.h"
#include "contmodedit.h"
#include "funcedit.h"

void LProjectCtrl::_DoMagic()
{
	LSystemEdit* pLsysEdit = _editors.LsysEdit();
	std::string sel;
	pLsysEdit->GetSelection(sel);
	pLsysEdit->SelectAfterSelection();
	FuncEdit* pFuncEdit = _editors.FunctionEditor();
	string_buffer fnames;
	const int n = pFuncEdit->Items();
	int i;
	for (i=0; i<n; ++i)
		fnames.add(pFuncEdit->GetObjectName(i));
	for (i=0; i<n; ++i)
	{
		const char* nm = pFuncEdit->GetObjectName(i);
		std::string::size_type p = sel.find(nm);
		if (std::string::npos != p)
		{
			std::string::size_type pe = p+strlen(nm);
			if (!isalnum(sel[p-1]) && !isalnum(sel[pe]) && sel[p-1] != '_' && sel[pe]!= '_')
				_FuncNameFound(sel, nm, pFuncEdit, i, fnames);
		}
	}
	pLsysEdit->ReplaceSelection(sel.c_str());
}

static void _BuildNewName(std::string& nm, const string_buffer& fnms)
{
	std::string trynm(nm);
	std::string::iterator it = trynm.end();
	--it;
	char n = *it;
	char rplcc = '0';
	if (isdigit(n) && n!='9')
	{
		while (rplcc <= '9')
		{
			*it = rplcc;
			if (!fnms.contains(trynm.c_str()))
			{
				nm = trynm;
				return;
			}
			++rplcc;
		}
	}
	rplcc = '0';
	trynm = nm;
	trynm.append(1, rplcc);
	_BuildNewName(trynm, fnms);
	nm = trynm;
}


static bool _is_ident(const std::string& str, std::string::size_type l, std::string::size_type p)
{
	// What is before?
	if (p>0)
	{
		--p;
		if (isalnum(str[p]) || '_' == str[p])
			return false;
		++p;
	}
	// What is after?
	p += l;
	if (p<str.length())
	{
		if (isalnum(str[p]) || '_' == str[p])
			return false;
	}
	return true;
}

static void replace(std::string& str, std::string::size_type ln, const std::string& sbs, std::string::size_type p)
{
	for (std::string::const_iterator it = sbs.begin(); it != sbs.end(); ++it)
	{
		if (ln>0)
		{
			str[p] = *it;
		}
		else
		{
			str.insert(p, 1, *it);
		}
		--ln;
		++p;
	}
}

void LProjectCtrl::_FuncNameFound(std::string& str, const char* fnm, FuncEdit* pFEdit, int i, string_buffer& fnms)
{
	// Build new name
	std::string newname(fnm);
	_BuildNewName(newname, fnms);
	// Duplicate function in the gallery
	pFEdit->Duplicate(i, newname.c_str());
	fnms.add(newname.c_str());

	// Replace all occurences of the old name in the buffer
	std::string::size_type p = str.find(fnm);
	while (std::string::npos != p)
	{
		std::string ofnm(fnm);
		if (_is_ident(str, ofnm.length(), p))
			replace(str, ofnm.length(), newname, p);
		p = str.find(fnm, ++p);
	}
}

