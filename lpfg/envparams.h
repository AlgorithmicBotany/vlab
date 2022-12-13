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



#ifndef __ENVPARAMS_H__
#define __ENVPARAMS_H__

#include <string>

#include "configfile.h"

class EnvironmentParams : public ConfigFile {
public:
  EnvironmentParams();
  void Default();
  void Load(const char *);
  enum eCommType { ctPipes, ctSockets, ctMemory, ctFiles };
  bool FollowingModule() const { return _IsFlagSet(flFollMod); }
  bool CmndLineSpecified() const { return !_cmndln.empty(); }
  const char *CmndLine() const { return _cmndln.c_str(); }
  const std::string &PosFmt() const { return _posFmt; }
  const std::string &HeadFmt() const { return _headFmt; }
  const std::string &LeftFmt() const { return _leftFmt; }
  const std::string &UpFmt() const { return _upFmt; }
  const std::string &WdthFmt() const { return _lineWidthFmt; }
  const std::string &ScaleFmt() const { return _scaleFmt; }

private:
  eCommType _commType;
  enum { eMaxCmndLine = 1023, eMaxFormatLength = 127 };

  std::string _cmndln;
  std::string _posFmt;
  std::string _headFmt;
  std::string _leftFmt;
  std::string _upFmt;
  std::string _lineWidthFmt;
  std::string _scaleFmt;

  enum eFlags { flVerbose = 1 << 0, flFollMod = 1 << 1 };

  void _ReadCommType(const char *);
  void _ReadExecutable(const char *, const char *);
  void _ReadTurtlePos(const char *);
  void _ReadTurtleHead(const char *);
  void _ReadTurtleLeft(const char *);
  void _ReadTurtleUp(const char *);
  void _ReadLineWidth(const char *);
  void _ReadScale(const char *);
  void _ReadInterpretedModules(const char *);

  enum eLabels {
    lCommType = 0,
    lExecutable,
    lTurtlePos,
    lTurtleHead,
    lTurtleLeft,
    lTurtleUp,
    lLineWidth,
    lScale,
    lInterpretedModules,
    lVerbose,
    lFollowingModule,

    elCount
  };
  static const char *_strLabels[];
};

extern EnvironmentParams envparams;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
