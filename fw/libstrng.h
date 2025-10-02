#ifndef __LIBSTRINGS_H__
#define __LIBSTRINGS_H__




namespace FWStr
{
	const char* GetLibString(int);
	enum StrIds
	{
		strZero = 0,
		LoadingBitmap,
		InvalidBMPFile,
		LoadingIcon,
		UnsupportedBMPFormat,
		AccessClipboard,
		MemForClipboard,
		ErrorCaption,
		DoubleExpected,
		IntegerExpected,
		CreatingSynchObject,
		OpenFile,
		ReadFromFile,
		WriteToFile,
		UnexpectedEOF,
		LineTooLong,
		CreateFont,
		LoadingMenu,
		Timeout,
		ManipulatingKey,
		OpenSemaphore,
		CreatingThread,
		ChDir,
		ErrOpenFile,
		Message,
		YesNo,
		Error,
		CreateWindowClss,
		ErrCreateTmpFile,

		nmLastString
	};
}

#endif
