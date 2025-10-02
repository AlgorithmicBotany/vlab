#ifndef __MESSAGES_H__
#define __MESSAGES_H__


namespace VLB
{
	namespace Commands
	{
		enum CommIds
		{
			ObjectStarted = 35000,
			ObjectClosing,
			ObjectCloseRequest,
			ObjectSaveRequest,
			ObjectMakeExtensionRequest,
			BrowserGenDiffRequest
		};
		const char* FromFrame = ".fromframe";
		const char* ToFrame = ".toframe";
		const char* IgnoredFiles = ".ignored";
		const char* DiffFile = ".diffile";
		const char* NameFile = ".name";
	}
}


#endif
