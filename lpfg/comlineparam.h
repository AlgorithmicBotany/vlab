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



#ifndef __COMLINEPARAM_H__
#define __COMLINEPARAM_H__

#include <string>
#include <vector>

#include "asrt.h"
#include "maxpath.h"
#include "rect.h"

enum OutType { oText, oBinary, oUnknown, oOBJ, oImage, oPostscript };

typedef enum { CONTINUOUS, TRIGGERED, OFF } SavingMode;

class ComndLineParam {
public:
  ComndLineParam();
  void Parse(int __argc, char** __argv);
  const char *Lsystemfile() const { return _lsystemfile; }
  const char *Colormapfile() const {
    ASSERT(ColormapMode());
    return _colorfile;
  }
  bool ColormapfileSpecified() const { return _colorfile[0] != 0; }
  bool FunctionsSpecified() const { return 0 != _fsetfile[0]; }
  bool TFunctionsSpecified() const { return 0 != _tfsetfile[0]; }
  bool VFunctionsSpecified() const { return 0 != _vfsetfile[0]; }
  const char *FunctionsFile() const { return _fsetfile; }
  const char *TFunctionsFile() const { return _tfsetfile; }
  const char *VFunctionsFile() const { return _vfsetfile; }
  // Pascal
  bool IndividualFunctionsSpecified() const {
    return 0 != _fIndividualFiles.size();
  }
  const std::vector<std::string> IndividualFunctionsFile() const {
    return _fIndividualFiles;
  }

  const char *Materialfile() const {
    ASSERT(!ColormapMode());
    return _colorfile;
  }
  const char *DrawparamFile() const { return _drawprmfile; }
  const std::vector<std::string> TextureFiles() const { return _textureFile; }
  void clearTextureFiles() { _textureFile.clear(); }
  bool DrawparamFileSpecified() const { return _drawprmfile[0] != 0; }
  const char *AnimparamFile() const { return _animfile; }
  bool AnimparamFileSpecified() const { return _animfile[0] != 0; }
  const char *Outputfile() const { return _outfile; }
  bool OutputfileSpecified() const { return _outfile[0] != 0; }
  OutType OutputType() const { return _outType; }
  const char *EnvironmentFile() const { return _envfile; }
  bool EnvFileSpecified() const { return _envfile[0] != 0; }
  const char *ContoursFile() const { return _csetfile; }
  bool ContoursSpecified() const { return _csetfile[0] != 0; }
  // Pascal
  bool IndividualContoursSpecified() const {
    return 0 != _cIndividualFiles.size();
  }
  const std::vector<std::string> IndividualContoursFile() const {
    return _cIndividualFiles;
  }

  const char *Path() const { return _path.c_str(); }
  bool PathSpecified() const { return !_path.empty(); }

  void Apply();
  bool ColormapMode() const { return _IsModeFlagSet(moColormap); }
  bool StartInAnimMode() const { return _IsModeFlagSet(moStartInAnim); }
  bool DebugMode() const { return _IsModeFlagSet(moDebug); }
  bool BatchMode() const { return _IsModeFlagSet(moBatch); }
  bool SilentMode() const { return _IsModeFlagSet(moSilent); }
  bool TimerMode() const { return _IsModeFlagSet(moTimer); }
  bool VerboseMode() const { return _IsModeFlagSet(moVerbose); }
  bool CheckNumeric() const { return _IsModeFlagSet(moCheckNumeric); }
  bool HasConsole() const { return 0 == (_wndOptions & woNoMsgLog); }
  bool NoBorder() const { return 0 != (_wndOptions & woNoBorder); }
  bool WindowSpecified() const {
    return 0 != (_wndOptions & (woPosSpecified | woSizeSpecified));
  }
  bool PosSpecified() const { return 0 != (_wndOptions & woPosSpecified); }
  bool SizeSpecified() const { return 0 != (_wndOptions & woSizeSpecified); }
  bool DimRelative() const { return 0 != (_wndOptions & woDimRelative); }
  Rect WindowRect() const { return _initRect; }
  void SetExpired() { _SetModeFlag(true, moExpired); }
  bool Expired() const { return _IsModeFlagSet(moExpired); }
  bool UseDll() const { return _IsModeFlagSet(moLDll); }
  const std::string &Dll() const { return _dll; }
  bool CompileOnly() const { return _IsModeFlagSet(moCompileOnly); }
  bool QuietMode() const { return _IsModeFlagSet(moQuiet); }
  bool TabletMode() const { return _IsModeFlagSet(moTablet); }
  bool TimedMode() const { return _IsModeFlagSet(moTimed); }
  bool DumpString() const { return _IsModeFlagSet(moDumpString); }
  bool InterpretToFile() const { return _IsModeFlagSet(moInterpretToFile); }
  bool CleanEA20() const { return _IsModeFlagSet(moCleanEA20); }
  bool ToStringEveryStep() const { return _IsModeFlagSet(moToStringEveryStep); }

  // Pascal
  void SetTexturefile(const char *f) { _SetTexturefile(f); }

private:
  void _ColormapMode(bool f) { _SetModeFlag(f, moColormap); }
  void _StartInAnimMode(bool f) { _SetModeFlag(f, moStartInAnim); }
  void _DebugMode(bool f) { _SetModeFlag(f, moDebug); }
  void _BatchMode(bool f) { _SetModeFlag(f, moBatch); }
  void _SilentMode(bool f) { _SetModeFlag(f, moSilent); }
  void _VerboseMode(bool f) { _SetModeFlag(f, moVerbose); }
  void _CheckNumeric(bool f) { _SetModeFlag(f, moCheckNumeric); }
  void _SetLDll(bool f) { _SetModeFlag(f, moLDll); }
  void _SetTimed(bool f) { _SetModeFlag(f, moTimed); }
  void _DumpString(bool f) { _SetModeFlag(f, moDumpString); }
  void _InterpretToFile(bool f) { _SetModeFlag(f, moInterpretToFile); }
  void _CleanEA20(bool f) { _SetModeFlag(f, moCleanEA20); }
  void _InterpretToFileEveryStep(bool f) {
    _SetModeFlag(f, moToStringEveryStep);
  }

  enum ModeOptions {
    moStartInAnim = 1 << 0,
    moColormap = 1 << 1,
    moDebug = 1 << 2,
    moBatch = 1 << 3,
    moSilent = 1 << 4,
    moVerbose = 1 << 5,
    moCheckNumeric = 1 << 6,
    moExpired = 1 << 7,
    moLDll = 1 << 8,
    moCompileOnly = 1 << 9,
    moQuiet = 1 << 10,
    moTablet = 1 << 11,
    moTimer = 1 << 12,
    moTimed = 1 << 13,
    moDumpString = 1 << 14,
    moInterpretToFile = 1 << 15,
    moCleanEA20 = 1 << 16,
    moToStringEveryStep = 1 << 17,
  };
  unsigned int _fMode;

  void _SetModeFlag(bool f, ModeOptions m) {
    if (f)
      _fMode |= m;
    else
      _fMode &= ~m;
  }
  bool _IsModeFlagSet(ModeOptions m) const { return 0 != (_fMode & m); }
  void _ParseOption(char **, int &);
  void _SetLsystemfile(const char *);
  char _lsystemfile[MaxPath + 1];
  void _SetColormapfile(const char *);
  void _SetMaterialfile(const char *);
  char _colorfile[MaxPath + 1];
  void _SetAnimatefile(const char *);
  char _animfile[MaxPath + 1];
  // Pascal
  void _SetIndividualFunctionsfile(const char *);
  std::vector<std::string> _fIndividualFiles;
  void _SetTexturefile(const char *);
  std::vector<std::string> _textureFile;
  ////////
  void _SetFunctionsfile(const char *);
  void _SetTFunctionsfile(const char *);
  void _SetVFunctionsfile(const char *);
  char _fsetfile[MaxPath + 1];
  char _tfsetfile[MaxPath + 1];
  char _vfsetfile[MaxPath + 1];
  void _SetDrawparamfile(const char *);
  char _drawprmfile[MaxPath + 1];
  // Pascal
  void _SetIndividualContoursfile(const char *);
  std::vector<std::string> _cIndividualFiles;
  ////////

  void _SetContoursFile(const char *);
  char _csetfile[MaxPath + 1];
  void _SetOutputFile(int &, char **);
  char _outfile[MaxPath + 1];
  OutType _outType;
  void _SetEnvironmentFile(const char *);
  char _envfile[MaxPath + 1];
  void _SetPath(int &, char **);
  std::string _path;
  void _SetDll(int &, char **);
  std::string _dll;

  void _SetWindowRelSize(int &, char **);
  void _SetWindowRelPosition(int &, char **);
  void _SetWindowPosition(int &, char **);
  void _SetWindowSize(int &, char **);

  Rect _initRect;

  enum WindowOptions {
    woNoBorder = 1 << 0,
    woNoMsgLog = 1 << 1,
    woPosSpecified = 1 << 2,
    woSizeSpecified = 1 << 3,
    woDimRelative = 1 << 4
  };
  unsigned int _wndOptions;

private:
  SavingMode _savingMode;

public:
  SavingMode savingMode() { return _savingMode; };
};

extern ComndLineParam comlineparam;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
