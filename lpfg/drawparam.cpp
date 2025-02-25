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



#include <string>

#include "drawparam.h"
#include "exception.h"
#include "utils.h"
#include "glenv.h"
#include "surfarr.h"
#include "terrain.h"
#include "tropismarr.h"
#include "tropismdata.h"
#include "file.h"
#include "texturearr.h"
#include "bsurfarr.h"
#include "quaternion.h"
#include "mesharr.h" // MC - Dec. 2020 - added array for OBJ meshes and animated meshes

#ifndef WIN32
#include <QFont>
#endif

SurfaceArray surfaces;
TextureArray textures;
TropismArray Tropisms;
SimpleTropism STropism;
DrawParams drawparams;
MeshArray meshes;

const float DefaultZoom = 1.0f / 0.9f;
const float DefaultMinZoom = 0.0005f;
const float DefaultMaxZoom = 5000.0f;
const int DefaultContourDivisions = 16;

const char *DrawParams::_strLabels[] = {"#",
                                        "line style:",
                                        "z buffer:",
                                        "render mode:",
                                        "light:",
                                        "surface:",
                                        "texture:",
                                        "front distance:",
                                        "back distance:",
                                        "tropism:",
                                        "torque:",
                                        "projection:",
                                        "scale:",
                                        "scale factor:",
                                        "min zoom:",
                                        "max zoom:",
                                        "rotation:",
                                        "font:",
                                        "winfont:",
                                        "stropism:",
                                        "window:",
                                        "window border:",
                                        "contour sides:",
                                        "generate on view change:",
                                        "view:",
                                        "bsurface:",
                                        "box:",
                                        "corrected rotation:",
                                        "backface culling:",
                                        "terrain:",
                                        "concave polygons:",
                                        "gradient:",
                                        "function:",
                                        "gallery:",
                                        "shadow map:",
                                        "PS linecap:",
                                        "stationary lights:",
                                        "auto normals:",
                                        "tapered cylinders:",
                                        "antialiasing:",
                                        "mesh:",
                                        "animated mesh:",
                                        "capped cylinders:",
                                        "wireframe line width:"};

DrawParams::DrawParams() { Default(); }

void DrawParams::Default(
    bool ) // int is used to avoid resetting some default parameters
{
  _dparams._linestyle = DParams::lsPixel;
  _dparams._antialiasingSamples = 0;
  _dparams._rendermode = DParams::rmFilled;
  _vparams._projmode = DParams::pmParallel;
  _dparams._rov = DParams::rovOff;
  _dparams._concavePolygons = DParams::concaveOff;
  _dparams._cappedCylinders = DParams::cappedCylindersOff;
  _dparams._postscriptGradient = DParams::gradientOff;
  _dparams._postscriptGradientAmount = 0.0f;
  _dparams._wireframeLineWidth = 1.0f;
  _vparams._scale = DefaultZoom;
  _vparams._minzoom = DefaultMinZoom;
  _vparams._maxzoom = DefaultMaxZoom;
  _dparams._contourDivisions = DefaultContourDivisions;
  gl.ResetLights();
  textures.Clear();
  surfaces.Clear();
  Tropisms.Clear();
  STropism.Default();
  _vparams._Clipping.Clear();
  _dparams._font.Default();
  _dparams._winfont.Default();
  _vparams._aWP.clear();
  _FlagSet(flCorrectedRotation);
  _FlagClear(flBackfaceCulling);
  _shadowparams._size = 1024;
  _shadowparams._color[0] = 0.2f;
  _shadowparams._color[1] = 0.2f;
  _shadowparams._color[2] = 0.4f;
  _shadowparams._offset[0] = 5.f;
  _shadowparams._offset[1] = 10.f;
  _dparams._postscriptLinecap = 0;
  _FlagSet(flStationaryLights);
  _FlagClear(flAutomaticNormals);
  _FlagClear(flTaperedCylinders);
  meshes.Clear();
}

bool DrawParams::Load(const char *fname) {
  //std::cerr << "Loading parameters\n" << std::endl;
  static const char *ifile = "drawparams.i.tmp";
  static char cmnd[256];

  std::ofstream clopt("clopt", std::ios::out);
  _FlagClear(flAutomaticNormals);
  _FlagClear(flTaperedCylinders);

  sprintf(cmnd, "%s %s %s", LPFGParams::PreprocScript, fname, ifile);
  const int EnvSize = 4096;
  Utils::Execute(cmnd, EnvSize);
  Utils::RemoveFile("clopt");
  {
    ReadTextFile src(ifile);
    const int BfSize = 256;
    char line[BfSize];

    bool correctedRotationPresent = false;
    Tropisms.Reset();
    while (!src.Eof()) {

      src.ReadLine(line, BfSize);
      int cntn = -1;
      int lbl = _Label(line, cntn, _strLabels, elCount);
      switch (lbl) {
      case -1:
        Utils::Message("Unrecognized command: %s\n in %s line %d\n", line,
                       src.Filename(), src.Line());
        break;
      case lPreproc:
        break;
      case lLineStyle:
        if (!_ReadLineStyle(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lAntialiasing:
        if (!_ReadAntialiasing(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lZBuffer:
        if (!_ReadOnOff(line + cntn, flZBuffer)) {
          _Error(line, src);
        }
        break;
      case lRenderMode:
        if (!_ReadRenderMode(line + cntn)) {
          _Error(line, src);
        }

        break;
      case lLight:
        if (!_ReadLight(line + cntn)) {
          _Error(line, src);
        }

        break;
      case lSurface:
        if (!_ReadSurface(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lTexture:
        if (!_ReadTexture(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lFrontDist:
        if (!_ReadFrontDistance(line + cntn)) {
          _Error(line, src);
        }

        break;
      case lBackDist:
        if (!_ReadBackDistance(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lTropism:
        if (!_ReadTropism(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lTorque:
        if (!_ReadTorque(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lProjection:
        if (!_ReadProjection(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lScale:
      case lScaleFactor:
        if (!_ReadScale(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lMinZoom:
        if (!_ReadMinZoom(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lMaxZoom:
        if (!_ReadMaxZoom(line + cntn)) {
          _Error(line, src);
        }
        break;

      case lFont:
        if (!_ReadFont(line + cntn)) {
          _Error(line, src);
        }
        break;

      case lWinFont:
        if (!_ReadWinFont(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lSTropism:
        if (!_ReadSTropism(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lWindow:
        if (!_ReadWindow(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lBorderWindow:
        if (!_ReadBorderWindow(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lContourDivisions:
        if (!_ReadContourDivisions(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lGenerateOnViewChange:
        if (!_ReadGenerateOnViewChange(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lViewModif:
        if (!_ReadViewModifier(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lBSurface:
        if (!_ReadBSurface(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lBBox:
        if (!_ReadBBox(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lCorrectedRotation:
        if (!_ReadOnOff(line + cntn, flCorrectedRotation)) {
          _Error(line, src);
        }
        correctedRotationPresent = true;
        break;
      case lBackfaceCulling:
        if (!_ReadOnOff(line + cntn, flBackfaceCulling)) {
          _Error(line, src);
        }
        break;
      case lTerrain:
        if (!_ReadTerrain(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lConcavePolygons:
        if (!_ReadConcavePolygons(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lPostscriptGradient:
        if (!_ReadPostscriptGradient(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lPostscriptLinecap:
        if (!_ReadPostscriptLinecap(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lWireframeLineWidth:
        if (!_ReadWireframeLineWidth(line + cntn)) {
          _Error(line, src);
        }
        break;
      case lFunction:
      case lGallery:
        break;
      case lShadowMap: // MC - Oct. 2015 - Read shadow map
        if (!_ReadShadowMap(line + cntn)) {
          _Error(line, src);
        }
        break;

      case lStationaryLights:
        if (!_ReadOnOff(line + cntn, flStationaryLights)) {
          _Error(line, src);
        }
        break;
      case lAutomaticNormals:
        if (!_ReadOnOff(line + cntn, flAutomaticNormals)) {
          _Error(line, src);
        }
        break;
      case lTaperedCylinders:
        if (!_ReadOnOff(line + cntn, flTaperedCylinders)) {
          _Error(line, src);
        }
        break;
      case lMesh: // MC - Dec. 2020 - Read OBJ mesh and Animated Mesh
        if (!_ReadMesh(line+cntn)){
          _Error(line, src);
        }
        break;
      case lAnimatedMesh:
        if (!_ReadAnimatedMesh(line+cntn)){
          _Error(line, src);
        }
        break;
      case lCappedCylinders:
        if (!_ReadCappedCylinders(line + cntn)) {
          _Error(line, src);
        }
        break;
      }
    }

    if (!correctedRotationPresent)
      Utils::Error(
          "corrected rotation: command not found.\n"
          "If rotations are different than in previous versions of lpfg, "
          "include the line\n"
          "corrected rotation: off\n"
          "in the view file. To eliminate this warning message, put the line\n"
          "corrected rotation: on\n"
          "in the view file. See the entry on corrected rotation: in the "
          "documentation,\n"
          "section 3.2: \"Draw/view parameters file\" for an explanation.\n");
  }
  Utils::RemoveFile(ifile);
  return true;
}

bool DrawParams::_ReadTerrain(const char *line) const {
  try {
    line = Utils::SkipBlanks(line);
    if (terrainData != NULL)
      delete terrainData;

    terrainData = new TerrainData();
    terrainData->loadTerrain(line);
    return true;
  } catch (Exception e) {
    Utils::Error(e.Msg());
  }
  return false;
}

bool DrawParams::_ReadSurface(const char *line) const {
  try {
    line = Utils::SkipBlanks(line);
    return surfaces.AddSurface(line);
  } catch (Exception e) {
    Utils::Error(e.Msg());
  }
  return false;
}

bool DrawParams::_ReadBSurface(const char *line) const {
  try {
    line = Utils::SkipBlanks(line);
    bsurfaces.AddSurface(line);
    return true;
  } catch (Exception e) {
    Utils::Error(e.Msg());
  }
  return false;
}

bool DrawParams::_ReadTexture(const char *line) const {
  try {
    const char *fname = Utils::SkipBlanks(line);
    textures.Add(fname);
    return true;
  } catch (Exception e) {
    Utils::Error(e.Msg());
  }
  return false;
}

bool DrawParams::_ReadLight(const char *line) const {
  try {
    LightSource light(line);
    light.TurnOn();
    gl.AddLight(light);
    return true;
  } catch (Exception e) {
    Utils::Error(e.Msg());
  }
  return false;
}

bool DrawParams::_ReadBackDistance(const char *line) {
  float back;
  int res = sscanf(line, "%f", &back);
  if (1 != res)
    return false;
  _vparams._Clipping.SetBack(back);
  return true;
}

bool DrawParams::_ReadFrontDistance(const char *line) {
  float front;
  int res = sscanf(line, "%f", &front);
  if (1 != res)
    return false;
  _vparams._Clipping.SetFront(front);
  return true;
}

bool DrawParams::_ReadTropism(const char *line) {
  try {
    Tropism tropism(line, Tropism::trTropism);
    //[PASCAL] 2019-08-30 The following doesn't make any sense that's why is
    // commented out Tropisms.Clear();
    Tropisms.Add(tropism);
    return true;
  } catch (Exception e) {
    Utils::Message(e.Msg());
  }
  return false;
}

bool DrawParams::_ReadTorque(const char *line) {
  try {
    Tropism torque(line, Tropism::trTorque);
    //[PASCAL] 2019-08-30 The following doesn't make any sense that's why is
    // commented out Tropisms.Clear();
    Tropisms.Add(torque);
    return true;
  } catch (Exception e) {
    Utils::Message(e.Msg());
  }
  return false;
}

bool DrawParams::_ReadProjection(const char *line) {
  line = Utils::SkipBlanks(line);
  if (0 == strncmp(line, "parallel", 8))
    _vparams._projmode = DParams::pmParallel;
  else if (0 == strncmp(line, "perspective", 11))
    _vparams._projmode = DParams::pmPerspective;
  else {
    Utils::Message("Invalid projection type: %s\n", line);
    return false;
  }
  return true;
}

bool DrawParams::_ReadLineStyle(const char *line) {
  line = Utils::SkipBlanks(line);
  if (0 == strcmp(line, "pixel"))
    _dparams._linestyle = DParams::lsPixel;
  else if (0 == strcmp(line, "polygon"))
    _dparams._linestyle = DParams::lsPolygon;
  else if (0 == strcmp(line, "cylinder"))
    _dparams._linestyle = DParams::lsCylinder;
  else {
    Utils::Message("Invalid line style: %s\n", line);
    return false;
  }
  return true;
}

bool DrawParams::_ReadAntialiasing(const char *line) {
  line = Utils::SkipBlanks(line);
  int samples;
  int res = sscanf(line, "%d", &samples);
  if (res != 1) {
    Utils::Message("Invalid number of parameters in the antialiasing command\n");
    return false;
  }
  if (samples > 10 || samples < 0) {
    Utils::Message("Antialiasing samples must be an integer between 0 and 10\n");
    return false;
  }

  _dparams._antialiasingSamples = samples;
  return true;
}

bool DrawParams::_ReadRenderMode(const char *line) {
  line = Utils::SkipBlanks(line);
  if (0 == strcmp(line, "filled"))
    _dparams._rendermode = DParams::rmFilled;
  else if (0 == strcmp(line, "wireframe"))
    _dparams._rendermode = DParams::rmWireframe;
  else if (0 == strcmp(line, "shaded"))
    _dparams._rendermode = DParams::rmShaded;
  else if (0 ==
           strcmp(
               line,
               "shadows")) // MC - Oct. 2015 - added 'shadows' rendering option
    _dparams._rendermode = DParams::rmShadows;
  else {
    Utils::Message("Invalid render mode: %s\n", line);
    return false;
  }

  // So we dont have to worry about where the terrain module is in the view file
  if (terrainData != NULL) {
    if (_dparams._rendermode == DParams::rmWireframe)
      terrainData->terrainVisibilityAll(Wireframe);
    else
      terrainData->terrainVisibilityAll(Shaded);
  }

  return true;
}

bool DrawParams::_ReadScale(const char *line) {
  const float epsilon = 0.0001f;
  float scale;
  int res = sscanf(line, "%f", &scale);
  if (1 != res)
    return false;
  if (fabs(scale) < epsilon) {
    Utils::Message("Invalid scale value: %f. Cannot be less than %f\n", scale,
                   epsilon);
    return false;
  }
  _vparams._scale = 1.0f / scale;
  return true;
}

bool DrawParams::_ReadMaxZoom(const char *line) {
  float v;
  int res = sscanf(line, "%f", &v);
  if (1 != res)
    return false;
  _vparams._maxzoom = v;
  return true;
}

bool DrawParams::_ReadMinZoom(const char *line) {
  float v;
  int res = sscanf(line, "%f", &v);
  if (1 != res)
    return false;
  _vparams._minzoom = v;
  return true;
}

DrawParams::WinFont::WinFont() { Default(); }

void DrawParams::WinFont::Default() {
  strcpy(_name, "Arial");
  _size = 14;
  _bold = _italic = false;
#ifndef WIN32
  int w = QFont::Normal;
  _qfont = QFont(_name, _size, w, _italic);
#endif
}

bool DrawParams::WinFont::Read(const char *line) {
  Default();
  if (0 == line[0])
    return false;
  line = Utils::ReadQuotedString(line, _name, 80);
  if (0 == line[0]) {
    Default();
    Utils::Message("Error reading font name\n");
    return false;
  }
  line = Utils::SkipBlanks(line);
  if (0 == line[0]) {
    Utils::Message("Error reading font size\n");
    return false;
  }
  _size = atoi(line);
  line = Utils::SkipNonBlanks(line);
  if (0 == line[0])
    return true;
  line = Utils::SkipBlanks(line);
  if (0 == line[0])
    return true;
  if ('I' == toupper(line[0]) || 'I' == toupper(line[1]))
    _italic = true;
  if ('B' == toupper(line[0]) || 'B' == toupper(line[1]))
    _bold = true;

#ifndef WIN32
  int w = QFont::Normal;
  if (_bold)
    w = QFont::Bold;
  _qfont = QFont(_name, _size, w, _italic);
#endif

  return true;
}

DrawParams::Font::Font() { Default(); }

void DrawParams::Font::Default() {
  _name = "Times";
  _size = 12;
  _bold = "normal";
  _italic = false;
#ifndef WIN32
  int w = QFont::Normal;
  if (!_bold.compare("light"))
    w = QFont::Light;
  else if (!_bold.compare("normal"))
    w = QFont::Normal;
  else if (!_bold.compare("medium"))
    w = QFont::DemiBold;
  else if (!_bold.compare("bold"))
    w = QFont::Bold;
  else if (!_bold.compare("black"))
    w = QFont::Black;

  _qfont = QFont(_name.c_str(), _size, w, _italic);
#endif
}

bool DrawParams::Font::Read(const char *line) {

  char font_spec[100];

  strcpy(font_spec, line);

  std::vector<std::string> vector_font;

  char delim = '-';
  std::string work = std::string(font_spec);
  std::string buf = "";
  size_t i = 0;
  while (i < work.length()) {
    if (work[i] != delim)
      buf += work[i];
    else {
      if (buf.length() > 0) {
        vector_font.push_back(buf);
        buf = "";
      } else {
        buf += "*";
        vector_font.push_back(buf);
        buf = "";
      }
    }
    i++;
  }
  if (!buf.empty())
    vector_font.push_back(buf);

  if (vector_font.size() >= 7) {
    std::istringstream buffer(vector_font[7]);
    int font_size;
    buffer >> font_size; // value = 45

    if ((font_size > 100) || (font_size <= 0)) {
      // try to read the size at a different position
      std::istringstream buffer(vector_font[6]);
      buffer >> font_size; // value = 45
    }

    std::string weight = vector_font[3];
    std::string slant = vector_font[4];

    bool italic = false;
    if (!slant.compare("italic"))
      italic = true;

    delim = '_';
    work = vector_font[2];
    buf = "";
    i = 0;
    std::vector<std::string> vector_name_font;
    while (i < work.length()) {
      if (work[i] != delim)
        buf += work[i];
      else if (buf.length() > 0) {
        vector_name_font.push_back(buf);
        buf = "";
      }
      i++;
    }
    if (!buf.empty())
      vector_name_font.push_back(buf);

    std::string name_font = "";
    for (size_t i = 0; i < vector_name_font.size() - 1; i++)
      name_font += vector_name_font[i] + " ";

    name_font += vector_name_font[vector_name_font.size() - 1];

    _name = name_font;
    _size = font_size;
    _bold = weight;

    _italic = italic;
  }
#ifndef WIN32
  int w = QFont::Normal;
  if (!_bold.compare("light"))
    w = QFont::Light;
  else if (!_bold.compare("normal"))
    w = QFont::Normal;
  else if (!_bold.compare("medium"))
    w = QFont::DemiBold;
  else if (!_bold.compare("bold"))
    w = QFont::Bold;
  else if (!_bold.compare("black"))
    w = QFont::Black;
  _qfont = QFont(_name.c_str(), _size, w, _italic);

#endif

  return true;
}

bool DrawParams::Font::ReadWinFont(const char *line) {
  // MC June 2014 - copied the code from WinFont::Read because it works and is
  // clean.
  char name[80];
  line = Utils::SkipBlanks(line);
  if (0 == line[0])
    return false;
  line = Utils::ReadQuotedString(line, name, 80);
  _name = std::string(name);
  if (0 == line[0]) {
    Utils::Message("Error reading font name\n");
    return false;
  }
  line = Utils::SkipBlanks(line);
  if (0 == line[0]) {
    Utils::Message("Error reading font size\n");
    return false;
  }
  _size = atoi(line);
  line = Utils::SkipNonBlanks(line);
  if (0 == line[0])
    return true;
  line = Utils::SkipBlanks(line);
  if (0 == line[0])
    return true;
  if ('I' == toupper(line[0]) || 'I' == toupper(line[1]))
    _italic = true;
  if ('B' == toupper(line[0]) || 'B' == toupper(line[1]))
    _bold = "bold";
#ifndef WIN32
  int w = QFont::Normal;
  if (!_bold.compare("normal"))
    w = QFont::Bold;
  _qfont = QFont(_name.c_str(), _size, w, _italic);

#endif
  return true;
}

bool DrawParams::_ReadFont(const char *line) {
#ifdef _WINDOWS
  return _dparams._winfont.Read(line);
#else
  return _dparams._font.Read(line);
#endif
}

bool DrawParams::_ReadWinFont(const char *line) {
#ifdef _WINDOWS
  return _dparams._winfont.Read(line);
#else
  return _dparams._font.ReadWinFont(line);
#endif
}

bool DrawParams::_ReadSTropism(const char *line) {
  if (!STropism.Read(line))
    return false;
  else
    return true;
}

bool DrawParams::_ReadWindow(const char *line) {
  WindowParams wp;
  if (wp.Read(line)) {
    _vparams._aWP.push_back(wp);
    return true;
  } else
    return false;
}

bool DrawParams::_ReadBorderWindow(const char *line) {
  WindowBorderParams wp;
  if (wp.Read(line)) {
    _vparams._WBP = wp;
    return true;
  } else
    return false;
}

bool DrawParams::_ReadContourDivisions(const char *line) {
  int v;
  if (1 != sscanf(line, "%d", &v))
    return false;
  if (v < LPFGParams::MinContourDivisions) {
    Utils::Message("Invalid value for contour divisions: %d. "
                   "Assuming minimum allowed value (%d)\n",
                   v, LPFGParams::MinContourDivisions);
    v = LPFGParams::MinContourDivisions;
  }
  if (v > LPFGParams::MaxContourDivisions) {
    Utils::Message("Invalid value for contour divisions: %d. "
                   "Assuming maximum allowed value (%d)\n",
                   v, LPFGParams::MaxContourDivisions);
    v = LPFGParams::MaxContourDivisions;
  }
  // We have to set _divisions to v+1 because glTriangleStip draws
  // 2*(_divisions-1) triangles if we give it 2*_divisions points.
  //	_dparams._contourDivisions = v + 1;
  //[PASCAL] it looks this is true only for the general cylinder, I have ported
  // this to gencylturtle only
  _dparams._contourDivisions = v;
  return true;
}

bool DrawParams::_ReadGenerateOnViewChange(const char *line) {
  line = Utils::SkipBlanks(line);
  if (0 == strcmp(line, "on"))
    _dparams._rov = DParams::rovOn;
  else if (0 == strcmp(line, "off"))
    _dparams._rov = DParams::rovOff;
  else if (0 == strcmp(line, "triggered"))
    _dparams._rov = DParams::rovTriggered;
  else {
    Utils::Message("Invalid \"Generate on view change\" value");
    return false;
  }
  return true;
}

const char *_viewLabels[] = {"dir:", "pan:", "up:", "fov:", "shift:", "scale:"};
enum _viewIdentifiers {
  viDir = 0,
  viPan,
  viUp,
  viFOV,
  viShift,
  viScale,
  viLast
};

bool DrawParams::_ReadViewModifier(const char *line) {
  unsigned int id;

  float rx, ry, rz;
  float scale;
  float px, py, pz;
  int res = sscanf(line, "%u %f %f %f %f %f %f %f", &id, &rx, &ry, &rz, &scale,
                   &px, &py, &pz);
  if (res == 0) {
    Utils::Message("Could not find the view id in the view: command.\n");
    return false;
  }
  if (!IsValidViewId(id) && (id != 0)) {
    Utils::Message("Invalid view id = %u\n", id);
    return false;
  }

  if (_vparams._aWP.empty()) {
    ASSERT(id == 0);
    WindowParams wp;
    _vparams._aWP.push_back(wp);
    WindowBorderParams wbp;
    _vparams._WBP = wbp;
  }

  if (res == 8) {
    Utils::Message("It looks like you are using an old-style \
view: command in the view file. I'll parse it the best I can, but you should \
replace it with a new-style command.\n");
    _vparams._aWP[id].SetModifierScale(scale);
    _vparams._aWP[id].SetModifierPan(Vector3d(px, py, pz));

    // Old-style command doesn't set ZShift; set it here to zero
    // flags Projection::SetModifiers to recalculate it.
    _vparams._aWP[id].SetModifierZShift(0);

    float angle = 0.5f * Deg2Rad(rx);
    Vector3d axis(1, 0, 0);
    Quaternion rot(cos(angle), sin(angle) * axis);
    angle = 0.5f * Deg2Rad(ry);
    axis = Vector3d(0, 1, 0);
    rot *= Quaternion(cos(angle), sin(angle) * axis);
    angle = 0.5f * Deg2Rad(rz);
    axis = Vector3d(0, 0, 1);
    rot *= Quaternion(cos(angle), sin(angle) * axis);

    Vector3d dir = _vparams._aWP[id].GetDir();
    dir.RotateBy(rot);
    _vparams._aWP[id].SetModifierDir(dir);

    Vector3d up = _vparams._aWP[id].GetUp();
    up.RotateBy(rot);
    _vparams._aWP[id].SetModifierUp(up);
  } else {
    line = Utils::FirstAlphabetic(line);
    while (line[0] != '\0') {
      int cont = -1;
      int labelID = _Label(line, cont, _viewLabels, viLast);
      if (labelID >= 0)
        line += cont;

      float x, y, z;

      switch (labelID) {
      case -1:
        Utils::Message("Unrecognized view command: %s\n", line);
        return false;
      case viDir:
        res = sscanf(line, "%f %f %f", &x, &y, &z);
        if (res != 3) {
          Utils::Message("Could not parse dir directive in view: command\n");
          return false;
        }
        _vparams._aWP[id].SetModifierDir(Vector3d(x, y, z));
        break;

      case viPan:
        res = sscanf(line, "%f %f %f", &x, &y, &z);
        if (res != 3) {
          Utils::Message("Could not parse pan directive in view: command\n");
          return false;
        }
        _vparams._aWP[id].SetModifierPan(Vector3d(x, y, z));
        break;

      case viUp:
        res = sscanf(line, "%f %f %f", &x, &y, &z);
        if (res != 3) {
          Utils::Message("Could not parse up directive in view: command\n");
          return false;
        }
        _vparams._aWP[id].SetModifierUp(Vector3d(x, y, z));
        break;

      case viFOV:
        res = sscanf(line, "%f", &x);
        if (res != 1) {
          Utils::Message("Could not parse fov directive in view: command\n");
          return false;
        }
        _vparams._aWP[id].SetModifierFOV(x);
        break;

      case viShift:
        res = sscanf(line, "%f", &x);
        if (res != 1) {
          Utils::Message("Could not parse shift directive in view: command\n");
          return false;
        }
        _vparams._aWP[id].SetModifierZShift(x);
        break;

      case viScale:
        res = sscanf(line, "%f", &x);
        if (res != 1) {
          Utils::Message("Could not parse scale directive in view: command\n");
          return false;
        }
        // To be consistent with the separate "scale" command,
        // read in the scale from the "view" command as a reciprocal number.
        // See the _ReadScale function.
        _vparams._aWP[id].SetModifierScale(1.f/x);
        break;
      }

      // Move to the next subcommand ... the "while" is so
      // we don't stop on numbers like 1.2e-009.
      do {
        line = Utils::FirstAlphabetic(line);
      } while (Utils::FirstAlphabetic(&(line[1])) != &(line[1]) && line++);
    }
  }

  return true;
}

bool DrawParams::_ReadBBox(const char *line) {
  unsigned int id;
  float x1, x2, y1, y2, z1, z2;
  int res;
  res = sscanf(line, "%u %f %f %f %f %f %f", &id, &x1, &x2, &y1, &y2, &z1, &z2);
  if (res != 7) {
    Utils::Message("Invalid number of parameters in the box command\n");
    return false;
  }
  if (!IsValidViewId(id) && (id != 0)) {
    Utils::Message("Invalid view id = %u\n", id);
    return false;
  }
  if (_vparams._aWP.empty()) {
    ASSERT(id == 0);
    WindowParams wp;
    _vparams._aWP.push_back(wp);
  }
  if (x1 >= x2) {
    Utils::Message("Error in box: command: xmin>=xmax\n");
    return false;
  }
  if (y1 >= y2) {
    Utils::Message("Error in box: command: ymin>=ymax\n");
    return false;
  }
  if (z1 >= z2) {
    Utils::Message("Error in box: command: zmin>=zmax\n");
    return false;
  }
  Volume v;
  v.Set(x1, x2, y1, y2, z1, z2);
  _vparams._aWP[id].SetBoundingBox(v);
  return true;
}

bool DrawParams::_ReadConcavePolygons(const char *line) {
  line = Utils::SkipBlanks(line);
  if (0 == strcmp(line, "on"))
    _dparams._concavePolygons = DParams::concaveOn;
  else if (0 == strcmp(line, "off"))
    _dparams._concavePolygons = DParams::concaveOff;
  else {
    Utils::Message("Error in concave polygons: command: must be concave on or "
                   "concave off");
    return false;
  }
  return true;
}

bool DrawParams::_ReadCappedCylinders(const char *line) {
  line = Utils::SkipBlanks(line);
  if (0 == strcmp(line, "on"))
    _dparams._cappedCylinders = DParams::cappedCylindersOn;
  else if (0 == strcmp(line, "off"))
    _dparams._cappedCylinders = DParams::cappedCylindersOff;
  else {
    Utils::Message("Error in capped cylinders: command: must be on or off");
    return false;
  }
  return true;
}

bool DrawParams::_ReadPostscriptGradient(const char *line) {
  int type;
  float amount;
  int res;
  res = sscanf(line, "%d %f", &type, &amount);
  if (res != 2) {
    Utils::Message("Invalid number of parameters in the gradient command\n");
    return false;
  }
  if (type > 2 || type < 0) {
    Utils::Message("Gradient type must be 0, 1, or 2\n");
    return false;
  }
  _dparams._postscriptGradient = (DParams::PostscriptGradient)type;
  _dparams._postscriptGradientAmount = amount;
  return true;
}
bool DrawParams::_ReadPostscriptLinecap(const char *line) {
  int type;
  int res = sscanf(line, "%d", &type);
  if (res != 1) {
    Utils::Message("Invalid number of parameters in the PS linecap command\n");
    return false;
  }
  if (type > 2 || type < 0) {
    Utils::Message("Line cap type must be 0, 1, or 2\n");
    return false;
  }
  _dparams._postscriptLinecap = type;
  return true;
}

bool DrawParams::_ReadShadowMap(
    const char *line) // MC - OCt. 2015 - for reading shadow map command
{
  const char *_shadowLabels[] = {"size:", "color:", "offset:"};
  enum _shadowIdentifiers { siSize, siColor, siOffset, siLast };

  int size, res;
  line = Utils::FirstAlphabetic(line);
  while (line[0] != '\0') {
    int cont = -1;
    int labelID = _Label(line, cont, _shadowLabels, siLast);
    if (labelID >= 0)
      line += cont;

    float x, y, z;

    switch (labelID) {
    case -1:
      Utils::Message("Unrecognized shadow map command: %s\n", line);
      return false;

    case siSize:
      res = sscanf(line, "%d", &size);
      if (res != 1) {
        Utils::Message(
            "Could not parse size directive in shadow map: command\n");
        return false;
      }
      if (size % 2 != 0) {
        Utils::Message("Shadow map size should not be an odd number.\n");
        size += 1;
      }
      _shadowparams._size = size;
      break;

    case siColor:
      res = sscanf(line, "%f %f %f", &x, &y, &z);
      if (res != 3) {
        Utils::Message(
            "Could not parse color directive in shadow map: command\n");
        return false;
      }
      if (x < 0.0)
        x = 0.0;
      if (x > 1.0)
        x = 1.0;
      _shadowparams._color[0] = x;

      if (y < 0.0)
        y = 0.0;
      if (y > 1.0)
        y = 1.0;
      _shadowparams._color[1] = y;

      if (z < 0.0)
        z = 0.0;
      if (z > 1.0)
        z = 1.0;
      _shadowparams._color[2] = z;

      break;

    case siOffset:
      res = sscanf(line, "%f %f", &x, &y);
      if (res != 2) {
        Utils::Message(
            "Could not parse offset directive in shadow map: command\n");
        return false;
      }
      if (x < 0.0)
        x = 0.0;
      _shadowparams._offset[0] = x;
      if (y < 0.0)
        y = 0.0;
      _shadowparams._offset[1] = y;
      break;
    }
    // Move to the next subcommand ... the "while" is so
    // we don't stop on numbers like 1.2e-009.
    do {
      line = Utils::FirstAlphabetic(line);
    } while (Utils::FirstAlphabetic(&(line[1])) != &(line[1]) && line++);
  }

  return true;
}

bool DrawParams::IsValidViewId(size_t id) const {
  return (id < _vparams._aWP.size());
}

const std::string &DrawParams::ViewName(size_t id) const {
  assert(IsValidViewId(id));
  return _vparams._aWP[id].Name();
}

bool DrawParams::_ReadMesh(const char* line) const {
	try
	{
		line = Utils::SkipBlanks(line);
		return meshes.AddMesh(line);
	}
	catch (Exception e)
	{

		Utils::Error(e.Msg());
	}
	return false;
}

bool DrawParams::_ReadAnimatedMesh(const char* line) const {
	try
	{
		line = Utils::SkipBlanks(line);
		return false;//meshes.AddAnimatedMesh(line);
	}
	catch (Exception e)
	{

		Utils::Error(e.Msg());
	}
	return false;
}

bool DrawParams::_ReadWireframeLineWidth(const char *line) {
  const float epsilon = 0.0001f;
  float lineWidth;
  int res = sscanf(line, "%f", &lineWidth);
  if (1 != res)
    return false;
  if (fabs(lineWidth) < epsilon) {
    Utils::Message("Invalid line width value: %f. Cannot be less than %f\n",
                   lineWidth, epsilon);
    return false;
  }
  _dparams._wireframeLineWidth = lineWidth;
  return true;
}
