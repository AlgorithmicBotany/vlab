#include <string>
#include <cassert>

#include <windows.h>
#include <mapi.h>
#include <tchar.h>

#include "warningset.h"

#include "lstring.h"
#include "dynlib.h"
#include "emailmsg.h"
#include "email.h"
#include "exception.h"


Email::Email(HWND hWnd) : _dll("MAPI32.DLL")
{
	_MAPILogon = reinterpret_cast<LPMAPILOGON>(_dll.GetProc(__TEXT("MAPILogon")));
	_MAPILogoff = reinterpret_cast<LPMAPILOGOFF>(_dll.GetProc(__TEXT("MAPILogoff")));
	_MAPISendMail = reinterpret_cast<LPMAPISENDMAIL>(_dll.GetProc(__TEXT("MAPISendMail")));
	ULONG res = _MAPILogon
		(
		reinterpret_cast<ULONG>(hWnd), 
		0, 0, 
		MAPI_FORCE_DOWNLOAD | MAPI_LOGON_UI, 0, 
		&_hSMAPI
		);
	if (res != SUCCESS_SUCCESS)
		throw Exception("Cannot connect to e-mail client");
}


Email::~Email()
{
	_MAPILogoff(_hSMAPI, 0, 0, 0);
}

/*
void Email::Send(EmailMessage& msg)
{
	MapiMessage mmsg;
	MapiRecipDesc Sender;
	{
		Sender.ulReserved = 0;
		Sender.ulRecipClass = MAPI_ORIG;
		Sender.lpszName = "";
		Sender.lpszAddress = "";
		Sender.ulEIDSize = 0;
		Sender.lpEntryID = 0;
	}

	MapiRecipDesc Recips[1];
	{
		Recips[0].ulReserved = 0;
		Recips[0].ulRecipClass = MAPI_TO;
		Recips[0].lpszName = "";
		Recips[0].lpszAddress = msg.Address();
		Recips[0].ulEIDSize = 0;
		Recips[0].lpEntryID = 0;
	}

	{
		mmsg.ulReserved = 0;
		mmsg.lpszSubject = msg.Title();
		mmsg.lpszNoteText = msg.Text();
		mmsg.lpszMessageType = 0;
		mmsg.lpszDateReceived = "";
		mmsg.lpszConversationID = "";
		mmsg.flFlags = 0;
		mmsg.lpOriginator = &Sender;
		mmsg.nRecipCount = 1;
		mmsg.lpRecips = Recips;
		mmsg.nFileCount = 0;
		mmsg.lpFiles = 0;
	}

	ULONG res = _MAPISendMail(_hSMAPI, 0, &mmsg, 0, 0);
	if (SUCCESS_SUCCESS != res)
		throw Exception("Error sending mail");
}
*/
