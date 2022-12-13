#include "Connection3D.h"
#include "stdafx.h"

[module(name = "LStudioConnection3DInterfaceApp")];

[event_receiver(com)] class Connection3DImpl : public Connection3D {
private:
  CComPtr<ISensor> sensor;
  bool initted;
  float rotSensitivity, translationSensitivity;
  float scaleRotation, scaleTranslation;
  HWND hWnd;

public:
  Connection3DImpl();
  ~Connection3DImpl();
  bool init(HWND hw, bool poll = false);
  int getStatus(Connection3DStatus &status);

  void setRotationSensitivity(float s) { rotSensitivity = s; }
  void setTranslationSensitivity(float s) { translationSensitivity = s; }

  HRESULT OnSensorInput();
};

bool initCom() {
  HRESULT hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
  if (!SUCCEEDED(hr)) {
    char buffer[100];
    sprintf(buffer, "Error 0x%x\n", hr);
    ::MessageBox(NULL, buffer, _T("CoInitializeEx failed"),
                 MB_ICONERROR | MB_OK);
    return false;
  }
  return true;
}

Connection3DImpl::Connection3DImpl() {
  initted = false;
  scaleRotation = 1024;
  scaleTranslation = 512;
  rotSensitivity = 10.0f;
  translationSensitivity = 10.0f;
}

HRESULT Connection3DImpl::OnSensorInput() {
  OutputDebugString("From OnSensorInput\n");
  SendMessage(hWnd, LSTUDIO_3D_CONNECTION_MSG, 0, 0);
  return S_OK;
}

bool Connection3DImpl::init(HWND hw, bool poll) {
  initted = false;
  hWnd = hw;

  if (!initCom())
    return false;

  CComPtr<IUnknown> device;
  HRESULT hr = device.CoCreateInstance(__uuidof(Device));
  if (SUCCEEDED(hr)) {
    CComPtr<ISimpleDevice> simpleDevice;

    hr = device.QueryInterface(&simpleDevice);
    if (SUCCEEDED(hr)) {
      hr = simpleDevice->get_Sensor(&sensor);

      if (!poll) {
        hr = __hook(&_ISensorEvents::SensorInput, sensor,
                    &Connection3DImpl::OnSensorInput);
        if (FAILED(hr)) {
          ::MessageBox(NULL, "OnSensorInput Error", "", MB_OK);
          return false;
        }
      }

      // keyboard = simpleDevice->Keyboard;
      // simpleDevice->LoadPreferences(_T("")); // Associate a configuration
      // with this device

      simpleDevice->Connect(); // Connect to the driver
      initted = true;
    }
  }

  return initted;
}

Connection3DImpl::~Connection3DImpl() {
  if (initted) {
    initted = false;
    CComPtr<ISimpleDevice> device;

    // Release the sensor and keyboard interfaces
    if (sensor) {
      sensor->get_Device((IDispatch **)&device);
      sensor.Release();
    }

    // if (keyboard)
    //    keyboard.Release();

    if (device) {
      // Disconnect it from the driver
      device->Disconnect();
      device.Release();
    }
  }
}

/*
   retv:   0  -   Some error occured
           1  -   No data was obtained
           2  -   Some status is obtained
*/
int Connection3DImpl::getStatus(Connection3DStatus &status) {
  int retv = 1;
  if (sensor) {
    try {
      CComPtr<IAngleAxis> pRotation;
      CComPtr<IVector3D> pTranslation;
      sensor->get_Rotation(&pRotation);
      sensor->get_Translation(&pTranslation);

      double angle, length, tx, ty, tz, rx, ry, rz, period;
      pRotation->get_Angle(&angle);
      pTranslation->get_Length(&length);

      // Check if the cap is still displaced
      if (angle > 0. || length > 0.) {
        angle /= scaleRotation * rotSensitivity;
        length /= scaleTranslation * translationSensitivity;

        pTranslation->get_X(&tx);
        pTranslation->get_Y(&ty);
        pTranslation->get_Z(&tz);

        pRotation->get_X(&rx);
        pRotation->get_Y(&ry);
        pRotation->get_Z(&rz);
        period = sensor->get_Period(&period);

        status.tx = (float)tx;
        status.ty = (float)ty;
        status.tz = (float)tz;
        status.tlen = (float)length;

        status.rx = (float)rx;
        status.ry = (float)ry;
        status.rz = (float)rz;
        status.rangle = (float)angle;
        status.period = (float)period;
        retv = 2;
      }

      pRotation.Release();
      pTranslation.Release();
    } catch (...) {
      retv = 0;
    }
  }
  return retv;
}

extern "C" {
__declspec(dllexport) void *getConnection3DImpl() {
  return new Connection3DImpl();
}
}
