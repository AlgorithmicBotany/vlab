#define WIN32_LEAN_AND_MEAN
#include "WinTabletImpl.h"
#include <windows.h>

#include <stdio.h>

inline float fix32ToDouble(FIX32 x) {
  float f = (float)(INT(x)) + ((float)FRAC(x) / float(65536));
  return f;
}

WinTabletImpl::WinTabletImpl() {
  hctx = NULL;
  hWnd = NULL;
  ori = false;
  pressure = false;
}

WinTabletImpl::~WinTabletImpl() { WTClose(hctx); }

bool WinTabletImpl::init(HWND handleWnd, bool useSysFlag, bool poll) {
  hWnd = handleWnd;
  if (!WTInfo(0, 0, NULL)) // check if wintab is available
    return false;

  initOrientationInfo();
  initPressureInfo();

  LOGCONTEXT lc;
  AXIS TabletX, TabletY;          // tablet size
  WTInfo(WTI_DEFCONTEXT, 0, &lc); // get default region

  if (!poll)
    lc.lcOptions |= CXO_MESSAGES; // send WT_PACKET message to the owner window

  lc.lcPktData = PACKETDATA;
  lc.lcPktMode = PACKETMODE;
  lc.lcMoveMask = PACKETDATA;
  lc.lcBtnUpMask = lc.lcBtnDnMask;
  if (useSysFlag)
    lc.lcOptions |= CXO_SYSTEM;

  // Set the entire tablet as active
  WTInfo(WTI_DEVICES, DVC_X, &TabletX);
  WTInfo(WTI_DEVICES, DVC_Y, &TabletY);
  lc.lcInOrgX = 0;
  lc.lcInOrgY = 0;
  lc.lcInExtX = TabletX.axMax;
  lc.lcInExtY = TabletY.axMax;

  // output data in screen coords
  lc.lcOutOrgX = lc.lcOutOrgY = 0;
  lc.lcOutExtX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  lc.lcOutExtY =
      -GetSystemMetrics(SM_CYVIRTUALSCREEN); // move origin to upper left

  hctx = WTOpen(hWnd, &lc, TRUE); // open the region
  return hctx != NULL;
}

void WinTabletImpl::initOrientationInfo() {
  AXIS oinfo[3];
  ori = WTInfo(WTI_DEVICES, DVC_ORIENTATION, &oinfo) != 0;

  // if orientation is supported and azimuth and altitude are supported
  if (ori && oinfo[0].axResolution && oinfo[1].axResolution) {
    // assume for now that the info returned is in degrees
    azimuthFactor = 360.0f / float(oinfo[0].axMax);
    altitudeFactor = 90.0f / float(oinfo[1].axMax);
  } else {
    ori = false;
  }
}

void WinTabletImpl::initPressureInfo() {
  AXIS pc;
  int i = WTInfo(WTI_DEVICES, DVC_NPRESSURE, &pc);
  pressure = false;
  if (i > 0) {
    pressure = true;
    pressureFactor = double(1) / pc.axMax;
  }
}

bool WinTabletImpl::hasOrientation() { return ori; }

bool WinTabletImpl::hasPressure() { return pressure; }

bool WinTabletImpl::getStatus(WinTabletStatus &ts) {
  PACKET pkt;
  if (0 == WTPacketsGet(hctx, 1, &pkt))
    return false;

  ts.sx = pkt.pkX;
  ts.sy = pkt.pkY;

  POINT pt;
  pt.x = pkt.pkX;
  pt.y = pkt.pkY;
  BOOL b = ::ScreenToClient(hWnd, &pt);
  if (b == FALSE) {
    throw 0;
  }
  ts.x = pt.x;
  ts.y = pt.y;

  ts.pressure = pkt.pkNormalPressure * pressureFactor;
  if (ori) {
    ts.ori.azimuth = pkt.pkOrientation.orAzimuth * azimuthFactor;
    ts.ori.altitude = pkt.pkOrientation.orAltitude * altitudeFactor;
  }
  ts.cursorT = pkt.pkCursor;
  ts.buttonState = pkt.pkButtons;

  WTPacketsGet(hctx, 1024, 0); // flush packet queue

  return true;
}

extern "C" {
__declspec(dllexport) void *getWinTabletImpl() { return new WinTabletImpl(); }
}
