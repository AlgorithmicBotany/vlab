#ifndef LSTUDIO_WINTABLET_H_
#define LSTUDIO_WINTABLET_H_

#include <windows.h>

struct WinTabletOrientation
{
    float azimuth;  // clockwise rotation of the cursor about the
                    // z-axis through a full circular range (in degrees)

    float altitude; // angle with the x-y plane through a signed 
                    // semi-circular range (in degrees). Negative sign
                    // indicates inverted pen
};

struct WinTabletStatus
{
    int sx, sy; // screen coordinates
    int x, y;   // client coordinates
    WinTabletOrientation ori;
    double pressure; // between 0 and 1
    unsigned int cursorT;
    unsigned int buttonState;
};

class WinTablet
{
public:

    virtual ~WinTablet() {}
    virtual bool init(HWND hwnd, bool useSysFlag = true, bool poll = true) = 0;
    virtual bool hasOrientation() = 0;
    virtual bool hasPressure() = 0;
    virtual bool getStatus(WinTabletStatus& ts) = 0;
};

inline WinTablet* getWinTablet()
{
    int oldMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    HMODULE h = LoadLibrary("WinTablet.dll");
    SetErrorMode(oldMode);
    if ( NULL == h )
        return 0;
    
    typedef void* (*funcp)();
    void* f = GetProcAddress(h, "getWinTabletImpl");
    if ( !f )
        return 0;

    funcp func = (funcp)f;

    void* p = func();
    WinTablet* tabletp = (WinTablet*)p;
    return tabletp;
}

inline void disposeTablet(WinTablet* p)
{
    delete p;
}

#endif
