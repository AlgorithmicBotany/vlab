/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#ifndef __LPFG_H__
#define __LPFG_H__

#ifdef LINUX
#include <unistd.h>
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QTimer>

#include <QtPlugin>

#endif

#ifdef _WINDOWS
#include "WinTablet\WinTablet.h"
#include "Connection3D\Connection3D.h"
#endif

#ifdef LINUX
#include "menu.h"
#include "xutils.h"
#endif

#include "lengine.h"
#include "usermenu.h"

#ifdef _WINDOWS
class View;
typedef View *PView;
#else
class View;
class GLWidget;
typedef GLWidget *PView;
#endif

class LPFG {
public:
#ifdef _WINDOWS
  LPFG(HINSTANCE);
  static void Initialize(HWND, const CREATESTRUCT *);
  void DoInitialize(HWND, const CREATESTRUCT *);
  void Create();
  void Destroy();
  void ConsoleClosed();
  void Register();
  HWND MdiClient() const { return _hMDIClient; }
#endif

  LPFG();
#ifdef LINUX
  bool CreateWindow();
  bool Initialize();
  void helpCB() const;
  void pdfHelpCB() const;
  void about_lpfg_cb();
  GLWidget *_glview;
#endif
  ~LPFG();

  void Repaint() const;

  void ContextMenu(int, int);
  static void CommandCb(int);
  void Command(int);
  void Timer();
  void Size(int, int);
  void InitMenu();
  void Recording();
  bool IsRecording() const { return _recording; }

  void RecordingForPovray();
  bool IsRecordingForPovray() const { return _recordingForPovray; }

  bool parametersNeedUpdating();
  float setOrGetParameterf(const char *name, float defVal);
  int setOrGetParameteri(const char *name, int defVal);
  float GetParameterf(const char *name);
  int GetParameteri(const char *name);
  void SetParameterf(const char *name, float val);
  void SetParameteri(const char *name, int val);
  void DelayWrite();
  void Write();

#ifdef _WINDOWS
  bool IsMDI() const;
  HINSTANCE GetInstance() const { return _hInstance; }
  int Loop();
  void HandleMsg(MSG &);

#endif

  void PanicExit();

#ifdef _WINDOWS
  DWORD
#else
  int
#endif
  Execute(char *) const;

  int RunBatchMode();
  int CompileOnly();
#ifdef _WINDOWS
  int Run();
#else
  int Run(QApplication &);
#endif
  void DumpString(const char *) const;
  bool Animating() const { return _mAMode != amStill; }
  const LEngine &GetLEngine() const { return _lengine; }
  LEngine &GetLEngine() { return _lengine; }

  void UseView(int);
  void CloseView(int);

  void ViewClosing(View *, int);
  void GenerateString();

  // Commands
  void Step();
  void Rewind();
  void Clear();
  void RunSim() {
    // This is called only if RunSimulation() specified
#ifdef LINUX
    if (_mainWindow) _mainWindow->RunSimulation();
#else
    _Run();
#endif
  }
  void Stop() { _Stop(); }
  void Forever() { _Forever(); }
  void GenerateStatic() { _GenerateStatic(); }
  void Exit();
  void OutputString(void) const;
  void OutputString(const std::string &fname) const;
  void OutputString(std::ostream &out) const {
    return _lengine.OutputString(out);
  }
  void LoadString(void);
  void LoadString(const std::string &fname);
  void LoadString(std::istream &in) {
    _lengine.LoadString(in);
    Repaint();
  }
  void SaveFrame(int) const;

  bool ViewExists(int) const;
  bool ViewExist() const;
  bool Running() const { return _mAMode != amStill; }
  void DisplayFrame() { _displayFrame = true; }
  void SetOutputFrame(const char *name)
  { _outputFrame = true; _outputFrameName = std::string(name); }
  bool IsOutputFrame() { return _outputFrame; } 
  void OutputFrame();
  float vvXmin(int) const;
  float vvYmin(int) const;
  float vvZmin(int) const;
  float vvXmax(int) const;
  float vvYmax(int) const;
  float vvZmax(int) const;
  float vvScale(int) const;
  void StopFunction();
  double GillespieTime() const { return _lengine.GetGillespieTime(); }
  void SeedGillespie(long seed) { return _lengine.SGRand(seed); }
  void ResetGillespie() { _lengine.ResetGillespie(); }

  MouseStatus GetMouseStatus(void);
  TabletStatus GetTabletStatus(void);

  UserMenu _userMenu;
  int _userMenuChoice;
  void UserMenuClear(void) {
    _userMenu.clear();
    _userMenuChoice = -1;
  }
  int UserMenuChoice(void) {
    int x = _userMenuChoice;
    _userMenuChoice = -1;
    return x;
  }

#ifdef _WINDOWS
  Connection3D *getC3D(void) { return _c3d; }
#endif
  PView GetView(unsigned int x)

  {
    if (x < _aView.size())
      return _aView[x];
    else
      return NULL;
  }

#ifdef LINUX
  void ResetRotation() { _ResetRotation(); }
  void ResetZoom() { _ResetZoom(); }
  void ResetPan() { _ResetPan(); }
  void ResetRoll() { _ResetRoll(); }
  void ResetView() { _ResetView(); }
  void Run() { _Run(); }
  void Rerun() { _Rerun(); }
  void NewLsystem() { _NewLsystem(); }
  void NewModel() { _NewModel(); }
  void RereadColors() { _RereadColors(); }
  bool RereadDrawParams() { return _RereadDrawParams(); }
  void RereadAnimParams() { _RereadAnimParams(); }
  void RereadSurfaces() { _RereadSurfaces(); }
  void RereadSurfacesNorepaint() { _RereadSurfacesNorepaint(); }
  void RereadTexturesNorepaint() { _RereadTexturesNorepaint(); }
  void RereadContours() { _RereadContours(); }
  void RereadContoursNoRerun() { _RereadContoursNoRerun(); }
  void RereadFunctions() { _RereadFunctionsRerun(); }
  void RereadFunctionsNoRerun() { _RereadFunctions(); }
  void OutputRGB();
  void OutputRayshade();
  void OutputPOVRay();
  void OutputPostscript();
  void OutputObj() { _mainWindow->OutputObj(); }
  void OutputView();
  void CreateView(int id){_CreateView(id);}
  void initializeMultiView();
  void HandleTabletEvent(QTabletEvent *event);
  void clearGLWidget(){
    _aView.clear();
  }
#endif

private:
  bool _LoadWindowPlacement();
  void _SaveWindowPlacement();
  void _NewModel();
  void _NewLsystem();

  void _ResetView(bool sync = true);
  void _ResetViewNoSync();
  void _ToggleShowAxis();
  void _ResetRotation();
  void _ResetZoom();
  void _ResetPan();
  void _ResetRoll();
  void SaveViewArrangement();

  void _Step();
  void _Rewind();
  void _Run();
  void _Stop();
  void _Forever();
  void _Exit();

  void StartTimer();
  void StopTimer();

  void Cascade();
  void TileH();
  void TileV();
  void DefaultArrangement();

  void _GenerateStatic();

  void _RereadColors(bool sync = true);
  bool _RereadDrawParams(bool sync = true);
  void _RereadAnimParams(bool sync = true);
  void _RereadEnvironment(bool sync = false);
  void _RereadSurfaces();
  void _RereadSurfacesNorepaint();
  void _RereadTexturesNorepaint();
  void _RereadCurveXYZRerun();
  void _RereadContours();
  void _RereadFunctionsRerun();
  void _RereadContoursNoRerun();
  void _RereadFunctions();
  void _Rerun();
  void _NewAnimate();
  void _NewHomomorphism();
  void _NewProjection();
  void _NewRender();
  void _Scale();

  bool _ValidLsystem() const;

  void _SetEnv();
  void _HandshakeLstudio();
#ifdef _WINDOWS
  HMENU
#else
  Menu
#endif
  _BuildViewListMenu();

  void _Sleep(int mscnds) {
    if (mscnds > 0)
#ifdef _WINDOWS
      ::Sleep(mscnds);
#endif
#ifdef LINUX
    usleep(1000 * mscnds);
#endif
  }

#ifdef USE_GLUT
  void _BuildMenu();
  static void _KeybdCB(unsigned char, int, int);
  void _Keybd(unsigned char);
#endif
  void _SyncMaster() const;
#ifdef _WINDOWS
  static const char *ClassName() { return "LPFGMV"; }
  const char *WinPlaceRegName() const { return "MVWindowPlacement"; }
  HACCEL Accelerator() const { return _hAccel; }
  void CreateClientWindow();
  HINSTANCE _hInstance;
  HANDLE _hLSemaphore;
  HWND _hLProject;
  HWND _hWnd;
  HWND _hMDIClient;
  HACCEL _hAccel;
  HMENU _hMenu;
  HMENU _hWinMenu;
  SIZE _size;

  WinTablet *_tablet;
  Connection3D *_c3d;
#endif
#ifdef LINUX
  QTabletEvent *_lastTabletEvent;
#endif
  bool _show;
  bool _recording;
  bool _recordingForPovray;
  int _frameNo;
  bool _BatchMode;

  void _CreateView(int);
  void _CloseView(int);
#ifdef _WINDOWS
  std::vector<PView> _aView;
  typedef std::vector<PView>::const_iterator Viter;
#endif
#ifdef LINUX
  View *_mainWindow;
  std::vector<GLWidget *> _aView; //[Pascal] openglWidget, trying to stay
                                  // compatible with Windows implementation
  typedef std::vector<GLWidget *>::const_iterator Viter;
#endif
#ifdef LINUX
  //	LPFGMainWindow* _pWindow;
#endif
  LEngine _lengine;

#ifdef _WINDOWS
  HICON _hMainIcon;
  HICON _hDocIcon;
#endif
  bool _RecordingOn;
  bool _displayFrame;
  bool _outputFrame;
  std::string _outputFrameName;

#ifdef _WINDOWS
  WINDOWPLACEMENT _wp;
#endif

  enum AnimateMode { amStill, amRun, amForever };

  AnimateMode _mAMode;
};

#endif
