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



#ifndef __ANIMPARAM_H__
#define __ANIMPARAM_H__

#include "configfile.h"
#include <string>
#include <iostream>

class AnimParam : public ConfigFile {
public:
  AnimParam();
  void Default();
  int Timeout() const { return _timeout; }
  int FirstFrame() const { return _firstframe; }
  int LastFrame() const { return _lastframe; }
  int Step() const { return _step; }
  void Load(const char *);
  bool DoubleBuffer() const { return _IsFlagSet(flDoubleBuffer); }
  bool ClearBetweenFrames() const { return _IsFlagSet(flClearBF); }
  bool HCenterBetweenFrames() const { return _IsFlagSet(flHCenterBF); }
  bool ScaleBetweenFrames() const { return _IsFlagSet(flScaleBF); }
  bool NewViewBetweenFrames() const { return _IsFlagSet(flNewViewBF); }
  bool DisplayOnRequest() const { return _IsFlagSet(flDispOnReq); }
  std::string FrameDumpPath() const { return _frameDumpPath; }
  enum nf { nfConsecutive, nfStepNo };
  nf FrameNumbers() const { return _Fn; }

private:
  int _firstframe;
  int _lastframe;
  int _timeout;
  int _step;
  nf _Fn;
  std::string _frameDumpPath;

  enum eFlags {
    flDoubleBuffer = 1 << 0,
    flClearBF = 1 << 1,
    flHCenterBF = 1 << 2,
    flScaleBF = 1 << 3,
    flDispOnReq = 1 << 4,
    flNewViewBF = 1 << 8
  };

  enum eLabels {
    lFirstFrame = 0,
    lLastFrame,
    lTimeout,
    lStep,
    lDoubleBuffer,
    lClearBF,
    lHCenterBF,
    lScaleBF,
    lNewViewBF,
    lDispOnReq,
    lFrameNum,
    lFrameDumpPath,

    elCount
  };
  static const char *_strLabels[];
};

extern AnimParam animparam;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
