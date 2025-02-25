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



#ifndef __DRAWPARAM_H__
#define __DRAWPARAM_H__

#include <vector>
#include <string>
#include <sstream>

#ifndef WIN32
#include <QFont>
#endif

#include "clipping.h"
#include "winparams.h"
#include "configfile.h"

namespace DParams {
enum LineStyle { lsPixel, lsPolygon, lsCylinder };
enum RenderMode {
  rmFilled,
  rmWireframe,
  rmShaded,
  rmShadows // MC - Oct. 2015 - added 'shadows' render mode
};
enum ProjectionMode { pmParallel, pmPerspective };
enum RedrawOnViewChange { rovOn, rovOff, rovTriggered };
enum ConcavePolygons { concaveOff, concaveOn };
enum PostscriptGradient {
  gradientOff,
  gradientLeftToRight,
  gradientBottomToTop
};
enum CappedCylinders { cappedCylindersOn, cappedCylindersOff };
} // namespace DParams

class DrawParams : public ConfigFile {
public:
  DrawParams();
  bool Load(const char *);
  DParams::LineStyle LineStyle() const { return _dparams._linestyle; }
  DParams::RenderMode RenderMode() const { return _dparams._rendermode; }
  int antialiasing() const { return _dparams._antialiasingSamples;}
  void SetRenderMode(DParams::RenderMode newMode) {
    _dparams._rendermode = newMode;
  }
  DParams::ProjectionMode ProjectionMode() const { return _vparams._projmode; }
  DParams::RedrawOnViewChange RedrawOnViewChange() const {
    return _dparams._rov;
  }
  DParams::ConcavePolygons ConcavePolygons() const {
    return _dparams._concavePolygons;
  }
  DParams::PostscriptGradient PostscriptGradient() const {
    return _dparams._postscriptGradient;
  }
  float PostscriptGradientAmount() const {
    return _dparams._postscriptGradientAmount;
  }
  int PostscriptLinecap() const { return _dparams._postscriptLinecap; }
  DParams::CappedCylinders CappedCylinders() const {
    return _dparams._cappedCylinders;
  }
  float WireframeLineWidth() const {
    return _dparams._wireframeLineWidth;
  }

  void NoTrigger() { _dparams._rov = DParams::rovOff; }
  bool ZBuffer() const { return _IsFlagSet(flZBuffer); }
  void Default(bool init = true);
  float Scale() const { return _vparams._scale; }
  float MinZoom() const { return _vparams._minzoom; }
  float MaxZoom() const { return _vparams._maxzoom; }
  int ContourDivisions() const { return _dparams._contourDivisions; }
  bool MultiViewOn() const;
  bool CorrectedRotation() const { return _IsFlagSet(flCorrectedRotation); }
  bool BackfaceCulling() const { return _IsFlagSet(flBackfaceCulling); }
  bool StationaryLights() const { return _IsFlagSet(flStationaryLights); }
  bool AutomaticNormals() const { return _IsFlagSet(flAutomaticNormals); }
  bool TaperedCylinders() const { return _IsFlagSet(flTaperedCylinders); }

  bool isOpenGL_2() { return _openGL_2; }
  void setOpenGL_2(bool b) { _openGL_2 = b; }

  // MC - Oct. 2015 - shadow map commands
  int ShadowMapSize() const { return _shadowparams._size; }
  float ShadowMapOffsetFactor() const { return _shadowparams._offset[0]; }
  float ShadowMapOffsetUnits() const { return _shadowparams._offset[1]; }
  float ShadowMapRed() const { return _shadowparams._color[0]; }
  float ShadowMapGreen() const { return _shadowparams._color[1]; }
  float ShadowMapBlue() const { return _shadowparams._color[2]; }

  const Clipping &Clip() const { return _vparams._Clipping; }
  bool FontSpecified() const {
#ifdef _WINDOWS
    return _dparams._winfont.Specified();
#else
    return _dparams._font.Specified();
#endif
  }
  class WinFont {
  public:
    WinFont();
    bool Read(const char *);
    bool Specified() const { return 0 != _name[0]; }
    void Default();
    int Size() const { return _size; }
    bool Bold() const { return _bold; }
    bool Italic() const { return _italic; }
    const char *Name() const { return _name; }

  private:
    char _name[80];
    int _size;
    bool _italic;
    bool _bold;
#ifndef WIN32
    QFont _qfont;

  public:
    QFont qfont() { return _qfont; }
#endif
  };

  class Font {
  public:
    Font();
    bool Read(const char *);
    bool ReadWinFont(const char *);

    bool Specified() const { return 0 != _name[0]; }
    void Default();
    int Size() const { return _size; }
    std::string Bold() const { return _bold; }
    bool Italic() const { return _italic; }
    const std::string Name() const { return _name; }

  private:
    std::string _name;
    int _size;
    bool _italic;
    std::string _bold;
#ifndef WIN32
    QFont _qfont;

  public:
    const QFont qfont() const { return _qfont; }
#endif
  };

  const WinFont &GetWinFont() const { return _dparams._winfont; }

  const Font &GetFont() const { return _dparams._font; }

#ifndef WIN32
  const QFont GetQFont() const {
    QFont f =  _dparams._font.qfont();
    return f;
  }
#endif
  int multiViewSize() const { return _vparams._aWP.size();}
  bool IsMultiView() const { return (_vparams._aWP.size() > 1); }
  bool IsValidViewId(size_t) const;
  const std::string &ViewName(size_t) const;
  const WindowParams &GetView(size_t id) const {
    ASSERT(IsValidViewId(id));
    return _vparams._aWP[id];
  }
  const WindowBorderParams &GetWindowBorderParams() const {
    return _vparams._WBP;
  }
  bool ViewModifiersSet(size_t id) const {
    if (!IsValidViewId(id))
      return false;
    return _vparams._aWP[id].ModifiersSet();
  }
  bool IsBoundingBoxSet(size_t id) const {
    if (!IsValidViewId(id))
      return false;
    return _vparams._aWP[id].IsBoundingBoxSet();
  }
  const Volume &BoundingBox(size_t id) const {
    ASSERT(IsBoundingBoxSet(id));
    return _vparams._aWP[id].GetBoundingBox();
  }
  const WindowParams::ViewModifiers &Modifiers(size_t id) const {
    ASSERT(ViewModifiersSet(id));
    return _vparams._aWP[id].Modifiers();
  }

private:
  bool _ReadLineStyle(const char *);
  bool _ReadRenderMode(const char *);
  bool _ReadSurface(const char *) const;
  bool _ReadLight(const char *) const;
  bool _ReadTexture(const char *) const;
  bool _ReadFrontDistance(const char *);
  bool _ReadBackDistance(const char *);
  bool _ReadTropism(const char *);
  bool _ReadTorque(const char *);
  bool _ReadProjection(const char *);
  bool _ReadScale(const char *);
  bool _ReadMinZoom(const char *);
  bool _ReadMaxZoom(const char *);
  bool _ReadRotation(const char *);
  bool _ReadFont(const char *);
  bool _ReadWinFont(const char *);
  bool _ReadSTropism(const char *);
  bool _ReadWindow(const char *);
  bool _ReadBorderWindow(const char *);
  bool _ReadContourDivisions(const char *);
  bool _ReadGenerateOnViewChange(const char *);
  bool _ReadViewModifier(const char *);
  bool _ReadBSurface(const char *) const;
  bool _ReadBBox(const char *);
  bool _ReadTerrain(const char *) const;
  bool _ReadConcavePolygons(const char *);
  bool _ReadCappedCylinders(const char *);
  bool _ReadPostscriptGradient(const char *);
  bool _ReadShadowMap(const char *); // MC - Oct. 2015 - read shadow map command
  bool _ReadPostscriptLinecap(const char *);
  bool _ReadAntialiasing(const char *);
  bool _ReadMesh(const char*) const; // MC - Dec. 2020 - read OBJ mesh files
  bool _ReadAnimatedMesh(const char*) const;
  bool _ReadWireframeLineWidth(const char*);

  struct DrawingParams {
    DParams::LineStyle _linestyle;
    DParams::RenderMode _rendermode;
    DParams::RedrawOnViewChange _rov;
    DParams::ConcavePolygons _concavePolygons;
    DParams::CappedCylinders _cappedCylinders;
    DParams::PostscriptGradient _postscriptGradient;
    float _postscriptGradientAmount;
    int _postscriptLinecap;
    int _contourDivisions;
    WinFont _winfont;
    Font _font;
    int _antialiasingSamples;
    float _wireframeLineWidth;
  };
  DrawingParams _dparams;

  struct ViewingParams {
    DParams::ProjectionMode _projmode;
    float _scale;
    float _minzoom;
    float _maxzoom;
    Clipping _Clipping;
    std::vector<WindowParams> _aWP;
    WindowBorderParams _WBP;
  };
  ViewingParams _vparams;

  // MC - Oct. 2015 - shadow map parameters
  struct ShadowMapParams {
    int _size;
    float _color[3];
    float _offset[2];
  };
  ShadowMapParams _shadowparams;

  enum eLabels {
    lPreproc = 0,
    lLineStyle,
    lZBuffer,
    lRenderMode,
    lLight,
    lSurface,
    lTexture,
    lFrontDist,
    lBackDist,
    lTropism,
    lTorque,
    lProjection,
    lScale,
    lScaleFactor,
    lMinZoom,
    lMaxZoom,
    lRotation,
    lFont,
    lWinFont,
    lSTropism,
    lWindow,
    lBorderWindow,
    lContourDivisions,
    lGenerateOnViewChange,
    lViewModif,
    lBSurface,
    lBBox,
    lCorrectedRotation,
    lBackfaceCulling,
    lTerrain,
    lConcavePolygons,
    lPostscriptGradient,
    lFunction,
    lGallery,
    lShadowMap,
    lPostscriptLinecap,
    lStationaryLights,
    lAutomaticNormals,
    lTaperedCylinders,
    lAntialiasing,
    lMesh,
    lAnimatedMesh,
    lCappedCylinders,
    lWireframeLineWidth,
    elCount
  };

  enum eFlags {
    flZBuffer = 1 << 0,
    flCorrectedRotation = 1 << 1,
    flBackfaceCulling = 1 << 2,
    flStationaryLights = 1 << 3,
    flAutomaticNormals = 1 << 4,
    flTaperedCylinders = 1 << 5
  };
  static const char *_strLabels[];
  bool _openGL_2;
};

extern DrawParams drawparams;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
