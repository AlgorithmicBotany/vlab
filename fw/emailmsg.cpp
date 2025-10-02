#include <string>
#include <cassert>

#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "lstring.h"
#include "emailmsg.h"


void EmailMessage::SetAddress(const std::string& txt)
{
	_address = "SMTP:";
	_address += txt;
}


void EmailMessage::SetTitle(const std::string& txt)
{
	_title = txt;
}


void EmailMessage::SetText(const std::string& txt)
{
	_txt = txt;
}

