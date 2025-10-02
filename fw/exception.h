/**************************************************************************

  File:		exception.h
  Created:	24-Nov-97


  Declaration of classes Exception


**************************************************************************/


#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

const int ExceptionMessageLength = 2048;

class Exception
{
public:
	Exception(const TCHAR*, ...);
	Exception(UINT, ...);

	Exception(HINSTANCE, UINT, ...);

	void AddMsg(HINSTANCE, UINT, ...);
	void AddMsg(UINT, ...);
	const TCHAR* Msg() const
	{ return _msg; }
protected:
	static TCHAR _msg[ExceptionMessageLength];
	const DWORD _errval;
};


#endif
