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



#ifndef __VIEW_H__
#define __VIEW_H__

#include <GL/glew.h>
#include <windows.h>
#include <GL/gl.h>

#include "Connection3D\Connection3D.h"

#include "volume.h"
#include "projection.h"
#include "rect.h"
#include "menu.h"
#include "viewtask.h"

// we need to include lintrfc for MouseStatus,
// and lparams for lintrfc
#include "include/lparams.h"
#include "include/lintrfc.h"

class LPFG;
class Clipping;
class WindowParams;
class DirectoryWatcher;

class View {
public:
  struct CrSt {
    CrSt(LPFG *pLpfg, int id) : _pLpfg(pLpfg), _id(id) {}
    LPFG *_pLpfg;
    int _id;
  };

  View(HWND, const CREATESTRUCT *);
  ~View();

  static View *Create(HINSTANCE, const char *, HWND, const RECT &, LPFG *, int);
  static void Register(HINSTANCE);
  HINSTANCE GetInstance() const { return _hInstance; }
  void ResetView();
  void ResetOpenGL();
  void PanBy(int, int);
  void ZoomBy(int);
  void FrustumBy(int);
  void RollBy(int);
  void RotateBy(int, int);
  void DrawStep();

  void Repaint();
  void Resize(int, int);
  void UserContextMenu(int, int);
  void ContextMenu(int, int);
  void Command(int);

  void ShowAxis(bool);
  void ResetRotation();
  void ResetZoom();
  void ResetPan();
  void ResetRoll();

  bool IsVisible() const;
  void Show();
  void Showing(bool);
  void Show(const WindowParams &);
  void ShowMaximized();
  const Projection &GetProjection() const { return _projection; }
  void HCenter();
  void Scale();
  void Clear();
  void DrawParamsChanged();

  void makeCurrent(void) const;

  void InsertX(int, int);
  void InsertPosX(int, int);
  void StartPanning(int, int);
  void StartZooming(int, int);
  void StartRotating(int, int);
  void StartFrustum(int, int);
  void StartRoll(int, int);
  void StartIdle();
  void MouseMove(int, int);
  static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
  void FillMouseStatus(MouseStatus &) const;
  CameraPosition GetCameraPosition(void) const;

  static bool IsClean() { return 0 == _counter; }
  void Destroy();

  void Resize(int, int, int, int);
  void GetRelSize(const Rect &, float *) const;
  void SetIcon(HICON hI) {
    SendMessage(_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hI);
  }

  float vvXmin() const;
  float vvYmin() const;
  float vvZmin() const;
  float vvXmax() const;
  float vvYmax() const;
  float vvZmax() const;
  float vvScale() const;

  HWND getWindowHandle(void) { return _hWnd; }
  void handleC3D(const Connection3DStatus &);

private:
  void _DrawExpired() const;
  HMENU

  _BuildViewListMenu();
  void _MakeFont();
  void _SaveFrame(int) const;
  void _DrawAxis() const;
  void _SwapBuffers() const;
  Projection _projection;
  MouseStatus _mouseStatus;

  HWND _hWnd;
  HINSTANCE _hInstance;
  bool _shown;

  ViewTask *_pTask;
  ViewIdleTask _IdleTask;
  ViewRotateTask _RotateTask;
  ViewZoomTask _ZoomTask;
  ViewFrustumTask _FrustumTask;
  ViewRollTask _RollTask;
  ViewPanTask _PanTask;
  bool _hasMoved;
  bool _dontPaint;

public:
  LPFG *_pLpfg;
  bool dontPaint() { return _dontPaint; }
  void setDontPaint(bool b) { _dontPaint = b; }

private:
  int _id;
  int _fileNameid;

  HDC _hdc;
  HGLRC _hrc;

  void SetTitle();

  bool _AxisOn;
  bool _RecordingOn;

  unsigned int _flist;
  unsigned int _oflist;
  void *_pQ;

  class CurrentContext {
  public:
    CurrentContext(const View *pView) {
      wglMakeCurrent(pView->_hdc, pView->_hrc);
    }
    ~CurrentContext() { wglMakeCurrent(0, 0); }
  };
  friend class View::CurrentContext;

  class GLRestore {
  public:
    GLRestore(GLenum cap) : _cap(cap), _enable(glIsEnabled(_cap)) {}
    ~GLRestore() {
      if (_enable)
        glEnable(_cap);
      else
        glDisable(_cap);
    }

  private:
    const GLenum _cap;
    const GLboolean _enable;
  };

  void ResetFrustum();
  void ShowAxis();
  void OutputBmp();
  void OutputRGB() const;
  void _SaveAsBmp(const char *) const;
  void _DoInsertX(int, int);
  void _DoInsertPosX(int, int);

public:
  void Recording();
  void RecordingForPovray();
  void OutputPovray(int frameNumber) const; // Used when recording
  void OutputPovray(const char *FolderName) const;
  void PovrayCameraParallel(std::ostream &) const;
  void PovrayCameraPerspective(std::ostream &) const;

  void OutputObj() const;
  void OutputFBX() const;
  void OutputRayshade() const;
  void OutputPostscript();
  void OutputView() const;

private:
  void OutputPovray() const;
  static int _counter;

  // MC - July 2016 - added private members for GLSL shaders and methods for
  // generating shadow map
  bool shadowMapFBO_supported;
  GLuint shadowMapTexture, shadowMapResolution, shadowMapFBO;
  GLuint lightMatrixLocation, shadowMapLocation, shadowColorLocation;
  GLuint textureLocation, textureShadowLocation, numLightsLocation;

  void initShadowMap(void);
  void setShadowMapFBO_supported(bool b) { shadowMapFBO_supported = b; };
  void resizeShadowMap(void);
  void freeShadowMap(void);
  void beginShadowMap(void);
  void endShadowMap(void);
  void beginMainShader(void);
  void endMainShader(void);

  GLuint mainShaderProgramID, shadowShaderProgramID;
  GLuint initShaders(const char *vertex_file_path,
                     const char *fragment_file_path);
  OpenGLMatrix lightMatrix; // the projection-model-view matrix for rendering
                            // from the light source
};

inline void View::makeCurrent() const { wglMakeCurrent(_hdc, _hrc); }

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
