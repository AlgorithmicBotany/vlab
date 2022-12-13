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



#include "comlineparam.h"
#include "utils.h"
#include "glenv.h"
#include "exception.h"
#include "animparam.h"
#include "drawparam.h"
#include "contourarr.h"
#include "funcs.h"
#include "envparams.h"
#include <string.h>

const int eDefaultSize =
#ifdef _WINDOWS
    CW_USEDEFAULT
#else
    -1
#endif
    ;

#ifdef LINUX
#include <QDesktopWidget>
#endif

ComndLineParam comlineparam;

ComndLineParam::ComndLineParam() {
  _savingMode = OFF;
  _fMode = 0;
  _lsystemfile[0] = 0;
  _colorfile[0] = 0;
  _animfile[0] = 0;
  _fsetfile[0] = 0;
  _tfsetfile[0] = 0;
  _drawprmfile[0] = 0;
  _csetfile[0] = 0;
  _outfile[0] = 0;
  _outType = oUnknown;
  _envfile[0] = 0;
  _StartInAnimMode(false);
  _ColormapMode(true);
  _DebugMode(false);
  _BatchMode(false);
  _SilentMode(false);
  _VerboseMode(false);
  _CheckNumeric(false);
  _SetLDll(false);
  _SetModeFlag(false, moCompileOnly);
  _wndOptions = 0;
  _initRect.left = _initRect.right = _initRect.top = _initRect.bottom =
      eDefaultSize;
}

void ComndLineParam::Parse(int __argc, char **__argv) {
  for (int i = 1; i < __argc; ++i) {
    size_t l = strlen(__argv[i]);
    if ('-' == __argv[i][0])
      _ParseOption(__argv, i);
    else if (!(strcmp(__argv[i] + l - 2, ".l")))
      _SetLsystemfile(__argv[i]);
    else if (!(strcmp(__argv[i] + l - 4, ".map")))
      _SetColormapfile(__argv[i]);
    else if (!(strcmp(__argv[i] + l - 4, ".mat")))
      _SetMaterialfile(__argv[i]);
    else if (!(strcmp(__argv[i] + l - 2, ".a")))
      _SetAnimatefile(__argv[i]);
    else if (!(strcmp(__argv[i] + l - 5, ".fset")))
      _SetFunctionsfile(__argv[i]);
    else if (!(strcmp(__argv[i] + l - 5, ".tset")))
      _SetTFunctionsfile(__argv[i]);
    else if (!(strcmp(__argv[i] + l - 5, ".vset")))
      _SetVFunctionsfile(__argv[i]);    
    else if (!(strcmp(__argv[i] + l - 5, ".func")))
      _SetIndividualFunctionsfile(__argv[i]);
    else if (!(strcmp(__argv[i] + l - 3, ".dr"))){
      Utils::Message("Warning this command line option: %s is deprecated\n", __argv[i]);
      _SetDrawparamfile(__argv[i]);
    }
    else if (!(strcmp(__argv[i] + l - 2, ".v")))
      _SetDrawparamfile(__argv[i]);
    else if (!(strcmp(__argv[i] + l - 5, ".cset")))
      _SetContoursFile(__argv[i]);
    else if (!(strcmp(__argv[i] + l - 4, ".con")))
      _SetIndividualContoursfile(__argv[i]);
    else if (!(strcmp(__argv[i] + l - 2, ".e")))
      _SetEnvironmentFile(__argv[i]);
    else
      Utils::Message("Unrecognized command line option: %s\n", __argv[i]);
  }
}

void ComndLineParam::_SetLsystemfile(const char *lfile) {
  strncpy(_lsystemfile, lfile, MaxPath);
  _lsystemfile[MaxPath] = 0;
}

void ComndLineParam::_SetColormapfile(const char *mapfile) {
  strncpy(_colorfile, mapfile, MaxPath);
  _colorfile[MaxPath] = 0;
  _ColormapMode(true);
}

void ComndLineParam::_SetMaterialfile(const char *matfile) {
  strncpy(_colorfile, matfile, MaxPath);
  _colorfile[MaxPath] = 0;
  _ColormapMode(false);
}

void ComndLineParam::_SetAnimatefile(const char *animfile) {
  strncpy(_animfile, animfile, MaxPath);
  _animfile[MaxPath] = 0;
}

void ComndLineParam::_SetFunctionsfile(const char *fsetfile) {
  strncpy(_fsetfile, fsetfile, MaxPath);
  _fsetfile[MaxPath] = 0;
}
void ComndLineParam::_SetTFunctionsfile(const char *tfsetfile) {
  strncpy(_tfsetfile, tfsetfile, MaxPath);
  _tfsetfile[MaxPath] = 0;
}
void ComndLineParam::_SetVFunctionsfile(const char *vfsetfile) {
  strncpy(_vfsetfile, vfsetfile, MaxPath);
  _vfsetfile[MaxPath] = 0;
}

void ComndLineParam::_SetIndividualFunctionsfile(const char *functionFile) {
  std::string fFile;
  fFile = std::string(functionFile);
  _fIndividualFiles.push_back(fFile);
}

void ComndLineParam::_SetDrawparamfile(const char *drpfile) {
  //std::cerr<<"Use of draw param file is deprecated: "<<drpfile<<std::endl;
  strncpy(_drawprmfile, drpfile, MaxPath);
  _drawprmfile[MaxPath] = 0;
}

void ComndLineParam::_SetContoursFile(const char *csetfile) {
  strncpy(_csetfile, csetfile, MaxPath);
  _csetfile[MaxPath] = 0;
}

void ComndLineParam::_SetTexturefile(const char *ctfile) {
  std::string tfile = std::string(ctfile);
  bool found = false;
  for (size_t i = 0; (i < _textureFile.size()) && (!found); ++i) {
    if (tfile.compare(_textureFile[i]) == 0)
      found = true;
  }
  if (!found)
    _textureFile.push_back(ctfile);
}

void ComndLineParam::_SetIndividualContoursfile(const char *contourFile) {
  std::string cFile;
  cFile = std::string(contourFile);
  _cIndividualFiles.push_back(cFile);
}

void ComndLineParam::_ParseOption(char **argv, int &i) {
  const char *opt = argv[i];
  if (!(strcmp("-a", opt)))
    _StartInAnimMode(true);
  else if (!(strcmp("-d", opt))) {
    Utils::Message("Debug mode on\n");
    _DebugMode(true);
  } else if (!(strcmp("-rmode", opt))) {
    const char *opt1 = argv[i + 1];
    if ((strcmp(opt1, "expl") == 0) || (strcmp(opt1, "explicit") == 0))
      _savingMode = OFF;
    if ((strcmp(opt1, "cont") == 0) || (strcmp(opt1, "continuous") == 0))
      _savingMode = CONTINUOUS;
    if ((strcmp(opt1, "trig") == 0) || (strcmp(opt1, "triggered") == 0))
      _savingMode = TRIGGERED;
    ++i;
  } else if (!(strcmp("-b", opt)))
    _BatchMode(true);
  else if (!(strcmp("-out", opt)))
    _SetOutputFile(i, argv);
  else if (!(strcmp("-wnb", opt)))
    _wndOptions |= woNoBorder | woNoMsgLog;
  else if (!(strcmp("-wnm", opt)))
    _wndOptions |= woNoMsgLog;
  else if (!(strcmp("-wr", opt)))
    _SetWindowRelSize(i, argv);
  else if (!(strcmp("-wpr", opt)))
    _SetWindowRelPosition(i, argv);
  else if (!(strcmp("-wp", opt)))
    _SetWindowPosition(i, argv);
  else if (!(strcmp("-w", opt)))
    _SetWindowSize(i, argv);
  //  else if (!(strcmp("-s", opt)))
  //  _SilentMode(true);
  else if (!(strcmp("-v", opt)))
    _VerboseMode(true);
  else if (!(strcmp("-cn", opt)))
    _CheckNumeric(true);
  else if (!(strcmp("-lp", opt)))
    _SetPath(i, argv);
  else if (!(strcmp("-dll", opt)))
    _SetDll(i, argv);
  else if (!(strcmp("-o", opt))) // synonym of dll but most explicit for mac os and linux
    _SetDll(i, argv);
  else if (!(strcmp("-c", opt)))
    _SetModeFlag(true, moCompileOnly);
  else if (!(strcmp("-q", opt)))
#ifdef LINUX
    _SetModeFlag(true, moQuiet);
#else
   _SilentMode(true);
#endif
  else if (!(strcmp("-tablet", opt)))
    _SetModeFlag(true, moTablet);
  else if (!(strcmp("-timer", opt)))
    _SetModeFlag(true, moTimer);
  else if (!(strcmp("-timed", opt)))
    _SetModeFlag(true, moTimed);
  else if (!(strcmp("-ds", opt)))
    _SetModeFlag(true, moDumpString);
  else if (!(strcmp("-dtf", opt)))
    _SetModeFlag(true, moInterpretToFile);
  else if (!(strcmp("-cleanEA20", opt)))
    _SetModeFlag(true, moCleanEA20);
  else if (!(strcmp("-dtfes", opt)))
    _SetModeFlag(true, moToStringEveryStep);
  else
    Utils::Message("Unrecognized command line option: %s\n", argv[i]);
}

void ComndLineParam::_SetOutputFile(int &i, char **argv) {
  ++i;
  strncpy(_outfile, argv[i], MaxPath);
  _outfile[MaxPath] = 0;
  const char *pDot = strrchr(_outfile, '.');
  if (0 != pDot) {
    ++pDot;
    if (!(strcmp("str", pDot)) || !(strcmp("txt", pDot)))
      _outType = oText;
    else if (!(strcmp("strb", pDot)) || !(strcmp("bin", pDot)))
      _outType = oBinary;
    else if (!(strcmp("obj", pDot)))
      _outType = oOBJ;
    else if (!(strcmp("png", pDot)) || (!(strcmp("jpg", pDot))) ||
             (!(strcmp("bmp", pDot))) || (!(strcmp("gif", pDot))) ||
             (!(strcmp("pdf", pDot))) || (!(strcmp("tiff", pDot))))
      _outType = oImage;
    else if (!(strcmp("ps", pDot)))
      _outType = oPostscript;
  }
}

void ComndLineParam::_SetEnvironmentFile(const char *efile) {
  strncpy(_envfile, efile, MaxPath);
  _envfile[MaxPath] = 0;
}

void ComndLineParam::_SetPath(int &i, char **argv) {
  ++i;
  _path = argv[i];
}

void ComndLineParam::_SetWindowPosition(int &i, char **argv) {
  ++i;
  _initRect.left = atoi(argv[i]);
  ++i;
  _initRect.top = atoi(argv[i]);
  _wndOptions |= woPosSpecified;
}

void ComndLineParam::_SetWindowRelPosition(int &i, char **argv) {
  ++i;
#ifdef _WINDOWS
  _initRect.left =
      static_cast<int>(atof(argv[i]) * GetSystemMetrics(SM_CXSCREEN));
#endif
#ifdef LINUX
  QDesktopWidget widget;
  QRect mainScreenSize = widget.availableGeometry(widget.primaryScreen());
  _initRect.left = static_cast<int>(atof(argv[i]) * mainScreenSize.width());
#endif

  ++i;
#ifdef _WINDOWS
  _initRect.top =
      static_cast<int>(atof(argv[i]) * GetSystemMetrics(SM_CYSCREEN));
#endif
#ifdef LINUX
  _initRect.top = static_cast<int>(atof(argv[i]) * mainScreenSize.height());
#endif

  _wndOptions |= woPosSpecified | woDimRelative;
}

void ComndLineParam::_SetWindowSize(int &i, char **argv) {
  ++i;
  _initRect.right = atoi(argv[i]);
  ++i;
  _initRect.bottom = atoi(argv[i]);
  _wndOptions |= woSizeSpecified;
}

void ComndLineParam::_SetWindowRelSize(int &i, char **argv) {
  ++i;
#ifdef _WINDOWS
  _initRect.right =
      static_cast<int>(atof(argv[i]) * GetSystemMetrics(SM_CXSCREEN));
#endif
#ifdef LINUX
  QDesktopWidget widget;
  QRect mainScreenSize = widget.availableGeometry(widget.primaryScreen());
  _initRect.right = static_cast<int>(atof(argv[i]) * mainScreenSize.width());
#endif
  ++i;

#ifdef _WINDOWS
  _initRect.bottom =
      static_cast<int>(atof(argv[i]) * GetSystemMetrics(SM_CYSCREEN));
#endif
#ifdef LINUX
  _initRect.bottom = static_cast<int>(atof(argv[i]) * mainScreenSize.height());
#endif

  _wndOptions |= woSizeSpecified | woDimRelative;
}

void ComndLineParam::_SetDll(int &i, char **argv) {
  ++i;
  _dll = argv[i];
  _SetLDll(true);
}

void ComndLineParam::Apply() {
  if (ColormapMode() && ColormapfileSpecified()) {
    try {
      gl.LoadColormap(_colorfile);
    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  } else if (!ColormapMode()) {
    try {
      gl.LoadMaterials(_colorfile);
    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  }

  if (AnimparamFileSpecified()) {
    try {
      animparam.Load(_animfile);
    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  }

  if (FunctionsSpecified()) {
    try {
      functions.Load(_fsetfile);
    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  }
  if (TFunctionsSpecified()) {
    try {
      functions.Load(_tfsetfile);
    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  }
  if (VFunctionsSpecified()) {
    try {
      functions.Load(_vfsetfile);
    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  }

  if (IndividualFunctionsSpecified()) {
    try {
      functions.LoadIndividualFunctions(_fIndividualFiles);
    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  }

  if (DrawparamFileSpecified()) {
    try {
      drawparams.Load(_drawprmfile);

    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  }

  if (ContoursSpecified()) {
    try {
      contours.Load(_csetfile);
    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  }
  if (IndividualContoursSpecified()) {
    try {
      contours.LoadIndividualContours(_cIndividualFiles);
    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  }

  if (EnvFileSpecified()) {
    try {
      envparams.Load(_envfile);
    } catch (Exception e) {
      Utils::Message(e.Msg());
    }
  }
}
