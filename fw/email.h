#ifndef __EMAIL_H__
#define __EMAIL_H__


class Email
{
public:
	Email(HWND);
	~Email();
	void Send(EmailMessage&);
private:
	DynLib _dll;
	LHANDLE _hSMAPI;

	LPMAPILOGON _MAPILogon;
	LPMAPILOGOFF _MAPILogoff;
	LPMAPISENDMAIL _MAPISendMail;
};



#endif
