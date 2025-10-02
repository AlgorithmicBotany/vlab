#ifndef __SHAREDSTRINGS_H__
#define __SHAREDSTRINGS_H__

namespace SharedStr
{
	const char* GetLibString(int);
	enum StrIds 
	{
		strZero = 0,
		strErrVlbGetExtensions,
		strErrFetchIcon,
		strErrWinSock,
		strErrCreateSock,
		strErrSocketWrite,
		strErrSocketRead,
		strErrConnectionClosed,
		strErrCannotResolveHost,
		strErrCannotConnect,
		strErrCannotLogin,
		strErrOpenRGBFile,
		strErrOnlyRGBFile,
		strErrOutOfMemory,
		strErrNoObjectSelect,
		strErrOpenObject,
		strYesNoNotAProject,
		strErrCreateExtension,
		strErrDelOofsRoot,
		strDeleteObject,
		strDeleteObjectAndChildren,
		strErrDelTree,
		strCopyPasteNotSupported,
		strErrPasteObject,
		strErrPasteObjects,
		strErrPasteSrcNotSet,
		strErrLocateObject,
		strErrCannotRenameObj,
		strHideExtensions,
		strShowExtensions,
		strHideIcon,
		strShowIcon,
		strErrOpenConfig,
		strObjectsStillOpen,
		strErrFindObject,
		strErrDeadHyperlink,
		strHypercopyNotSupported,
		strErrFetchId,
		strErrNonexistentId,
		strErrCouldNotCreateDestinationDirectory,
		strErrCouldNotCreateFile,
		strErrPastingObjectUnderHyperobject,
		strDeleteHyperobject,
		strDeleteHyperobjectAndChildren,
		strErrCouldNotCreateHyperobject,
		strErrHyperobjectLookupPath,
		strErrHyperPasteAcrossBrowsers,
		strCannotGetUUIDForObject,
		strErrCannotCreateObj,
		strFailedOrNotSupported,

		strLastString
	};
}


#endif
