/**************************************************************************

  File:		logfont.h
  Created:	08-Jan-98


  Declaration of class LogFont


**************************************************************************/


#ifndef __LOGFONT_H__
#define __LOGFONT_H__

#include <string>

class LogFont : public LOGFONT
{
public:
	LogFont(const LOGFONT&);
	LogFont(int, const std::string&);
};


#endif
