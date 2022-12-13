#ifndef LSTUDIO_CONNECTION3D_H_
#define LSTUDIO_CONNECTION3D_H_

const DWORD LSTUDIO_3D_CONNECTION_MSG = WM_USER + 0x437;

struct Connection3DStatus
{
    float rx, ry, rz, rangle;
    float tx, ty, tz, tlen;
    float period;

    Connection3DStatus()
    {
        rx = ry = rz = rangle = tx = ty = tz = tlen = period = 0;
    }
};

class Connection3D
{
public:

    virtual ~Connection3D() {}
    virtual bool init(HWND hw, bool poll = false) = 0;
    virtual int getStatus(Connection3DStatus& status) = 0;
    virtual void setRotationSensitivity(float s) = 0;
    virtual void setTranslationSensitivity(float s) = 0;
};


inline void disposeConnection3D(Connection3D* p)
{
    delete p;
}

inline Connection3D* getConnection3D(HWND hWnd, bool poll = false)
{
    UINT pmode = SetErrorMode(SEM_FAILCRITICALERRORS);
    HMODULE h = LoadLibrary("Connection3D.dll");
    if ( NULL == h )
    {
        SetErrorMode(pmode);
        return 0;
    }
    
    typedef void* (*funcp)();
    void* f = GetProcAddress(h, "getConnection3DImpl");
    if ( !f )
    {
        SetErrorMode(pmode);
        return 0;
    }
    SetErrorMode(pmode);

    funcp func = (funcp)f;

    void* p = func();
    Connection3D* c3dp = (Connection3D*)p;
    if ( !c3dp->init(hWnd, poll) )
    {
        disposeConnection3D(c3dp);
        return 0;
    }

    return c3dp;
}

#endif
