#ifndef __FWUSER_H__
#define __FWUSER_H__

class FWUser
{
public:
	FWUser(HINSTANCE hInst);
	~FWUser();
	static void Register(HINSTANCE);
	static void Unregister(HINSTANCE);
private:
	static bool _exists;
	HINSTANCE _hInst;
};


int Main(HINSTANCE, Window::sw);



#endif
