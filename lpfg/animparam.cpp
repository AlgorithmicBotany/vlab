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



#include "animparam.h"
#include "file.h"
#include "utils.h"

#include <string.h>

const char *AnimParam::_strLabels[] = {"first frame:",
                                       "last frame:",
                                       "swap interval:",
                                       "step:",
                                       "double buffer:",
                                       "clear between frames:",
                                       "hcenter between frames:",
                                       "scale between frames:",
                                       "new view between frames:",
                                       "display on request:",
                                       "frame numbers:",
                                       "frame dump path:"};

AnimParam animparam;

AnimParam::AnimParam() { Default(); }

void AnimParam::Default() {
  _firstframe = 0;
  _lastframe = -1;
  _timeout = 0;
  _step = 1;
  _FlagSet(flClearBF);
  _FlagSet(flDoubleBuffer);
  _Fn = nfConsecutive;
  _frameDumpPath = "";
}

void AnimParam::Load(const char *fname) {
  ReadTextFile src(fname);

  const int BfSize = 128;
  char line[BfSize];

  while (!src.Eof()) {
    src.ReadLine(line, BfSize);
    int cntn = -1;
    int lbl = _Label(line, cntn, _strLabels, elCount);
    switch (lbl) {
    case -1:
      Utils::Message("Unrecognized command: %s\n in %s line %d\n", line, fname,
                     src.Line() - 1);
      break;
    case lFirstFrame: {
      int n;
      int res = sscanf(line + cntn, "%d", &n);
      if (1 != res)
        Utils::Message("Error reading animate file %s at line %d\n", fname,
                       src.Line() - 1);
      else if (n < 0)
        Utils::Message("Invalid value for first frame in file %s at line %d\n",
                       fname, src.Line() - 1);
      else
        _firstframe = n;
    } break;
    case lLastFrame: {
      int n;
      int res = sscanf(line + cntn, "%d", &n);
      if (1 != res)
        _Error(line, src);
      else if (n >= 0)
        _lastframe = n;
    } break;
    case lTimeout: {
      int n;
      int res = sscanf(line + cntn, "%d", &n);
      if (1 != res)
        _Error(line, src);
      else
        _timeout = 10 * n;
    } break;
    case lStep: {
      int n;
      int res = sscanf(line + cntn, "%d", &n);
      if (1 != res)
        _Error(line, src);
      else if (_step < 1) {
        Utils::Message("Invalid value of step in file %s at line %d\n", fname,
                       src.Line() - 1);
        _step = 1;
      } else
        _step = n;
    } break;
    case lDoubleBuffer:
      if (!_ReadOnOff(line + cntn, flDoubleBuffer))
        _Error(line, src);
      break;
    case lClearBF:
      if (!_ReadOnOff(line + cntn, flClearBF))
        _Error(line, src);
      break;
    case lHCenterBF:
      if (!_ReadOnOff(line + cntn, flHCenterBF))
        _Error(line, src);
      break;
    case lScaleBF:
      if (!_ReadOnOff(line + cntn, flScaleBF))
        _Error(line, src);
      break;
    case lNewViewBF:
      if (!_ReadOnOff(line + cntn, flNewViewBF))
        _Error(line, src);
      break;
    case lDispOnReq:
      if (!_ReadOnOff(line + cntn, flDispOnReq))
        _Error(line, src);
      break;
    case lFrameNum: {
      const char *str = line + cntn;
      str = Utils::SkipBlanks(str);
      const char *consecutive = "consecutive";
      const char *stepno = "stepno";
      if (0 == strncmp(str, consecutive, strlen(consecutive)))
        _Fn = nfConsecutive;
      else if (0 == strncmp(str, stepno, strlen(stepno)))
        _Fn = nfStepNo;
      else
        _Error(line, src);
    } break;
    case lFrameDumpPath: {
      const char *str = line + cntn;
      _frameDumpPath.append(str);
      Utils::Message("Setting frame dump path to: %s", _frameDumpPath.c_str());
    } break;
    }
  }

  // MC - Nov. 2015 - ensure that if clear between frame is off, double
  // buffering is also off
  if (!_IsFlagSet(flClearBF)) {
    if (_IsFlagSet(flDoubleBuffer)) {
      Utils::Message("\nWarning! 'clear between frames' is off but 'double "
                     "buffer' is on.\n");
      Utils::Message("Setting 'double buffering' to off.\n");
      _FlagClear(flDoubleBuffer);
    }
  }
}
