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



#include <sstream>

#include "envparams.h"
#include "utils.h"
#include "file.h"

const char *EnvironmentParams::_strLabels[] = {
    "communication type:",  "executable:",
    "turtle position:",     "turtle heading:",
    "turtle left:",         "turtle up:",
    "turtle line width:",   "turtle scale factor:",
    "interpreted modules:", "verbose:",
    "following module:"};

EnvironmentParams envparams;

EnvironmentParams::EnvironmentParams() { Default(); }

void EnvironmentParams::Default() {
  _commType = ctFiles;
  _cmndln = "";
  _posFmt = "%f %f %f";
  _headFmt = "";
  _leftFmt = "";
  _upFmt = "";
  _lineWidthFmt = "";
  _scaleFmt = "";
  _FlagClear(flVerbose);
  _FlagClear(flFollMod);
}

void EnvironmentParams::Load(const char *fname) {
  ReadTextFile src(fname);

  const int BfSize = 128;
  char line[BfSize];

  while (!src.Eof()) {
    src.ReadLine(line, BfSize);
    int cntn = -1;
    int lbl = _Label(line, cntn, _strLabels, elCount);
    switch (lbl) {
    case -1:
      Utils::Message("Unrecognized command: %s\n in %s at line %d\n", line,
                     fname, src.Line());
      break;
    case lCommType:
      _ReadCommType(line + cntn);
      break;
    case lExecutable:
      _ReadExecutable(line + cntn, fname);
      break;
    case lTurtlePos:
      _ReadTurtlePos(line + cntn);
      break;
    case lTurtleHead:
      _ReadTurtleHead(line + cntn);
      break;
    case lTurtleLeft:
      _ReadTurtleLeft(line + cntn);
      break;
    case lTurtleUp:
      _ReadTurtleUp(line + cntn);
      break;
    case lLineWidth:
      _ReadLineWidth(line + cntn);
      break;
    case lScale:
      _ReadScale(line + cntn);
      break;
    case lInterpretedModules:
      Utils::Message(
          "Command: %s\n in %s at line %d is not supported. Command ignored.\n",
          line, fname, src.Line());
      break;
    case lVerbose:
      if (!_ReadOnOff(line + cntn, flVerbose))
        _Error(line, src);
      break;
    case lFollowingModule:
      if (!_ReadOnOff(line + cntn, flFollMod))
        _Error(line, src);
      break;
    }
  }
}

void EnvironmentParams::_ReadCommType(const char *) {}

void EnvironmentParams::_ReadExecutable(const char *ln, const char *fnm) {
  ln = Utils::SkipBlanks(ln);
  _cmndln = ln;
  size_t dashe = _cmndln.find(" -e ");
  if (std::string::npos == dashe) {
    size_t space = _cmndln.find(' ');
    if (std::string::npos == space)
      space = _cmndln.length();
    std::stringstream app;
    app << " -e " << fnm;
    _cmndln.insert(space, app.str());
  }
}

void EnvironmentParams::_ReadTurtlePos(const char *ln) { _posFmt = ln; }

void EnvironmentParams::_ReadTurtleHead(const char *ln) { _headFmt = ln; }

void EnvironmentParams::_ReadTurtleLeft(const char *ln) { _leftFmt = ln; }

void EnvironmentParams::_ReadTurtleUp(const char *ln) { _upFmt = ln; }

void EnvironmentParams::_ReadLineWidth(const char *ln) {
  ln = Utils::SkipBlanks(ln);
  _lineWidthFmt = ln;
}

void EnvironmentParams::_ReadScale(const char *ln) {
  ln = Utils::SkipBlanks(ln);
  _scaleFmt = ln;
}

void EnvironmentParams::_ReadInterpretedModules(const char *) {}
