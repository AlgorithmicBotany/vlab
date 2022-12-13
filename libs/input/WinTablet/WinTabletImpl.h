#ifndef LSTUDIO_WINTABLETIMPL_H_
#define LSTUDIO_WINTABLETIMPL_H_

#include <wintab.h>
#define PACKETDATA (PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | \
					PK_ORIENTATION | PK_CURSOR)
#define PACKETMODE  0
#include <pktdef.h>
#include "WinTablet.h"

class WinTabletImpl : public WinTablet
{
private:

    HCTX hctx;
    HWND hWnd;

    // orientation
    bool ori;
    float azimuthFactor, altitudeFactor;

    bool pressure;
    double pressureFactor;

public:

    WinTabletImpl();
    ~WinTabletImpl();
    bool init(HWND hwnd, bool useSysFlag = true, bool poll = true);
    bool hasOrientation();
    bool hasPressure();
    bool getStatus(WinTabletStatus& ts);

private:

    void initOrientationInfo();
    void initPressureInfo();
};

#endif
