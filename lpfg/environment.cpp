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



#ifdef LINUX
#include <unistd.h>
#endif

#include <sstream>

#include "environment.h"
#include "envparams.h"
#include "exception.h"
#include "utils.h"
#include "lstriter.h"
#include "vector3d.h"
#include "turtle.h"

Environment::Environment(const EnvironmentParams &params) : _params(params) {
  const int semId =
#ifdef WIN32
      GetCurrentProcessId()
#else
      getpid()
#endif
      * 4;
  _sem.Create(semId, false, true);

  if (!params.CmndLineSpecified())
    throw Exception("Environment command line not specified");
  else {
    std::stringstream optns;
    optns << " -k " << semId << " -ext " << semId << ".0";
    _cmndln = params.CmndLine();
    std::string::size_type spc = _cmndln.find_first_of(" \t");
    _cmndln.insert(spc, optns.str());
  }

  {
    std::stringstream tfn;
    tfn << ".to_field" << semId << ".0";
    _outfnm = tfn.str();
  }

  {
    std::stringstream ffn;
    ffn << ".from_field" << semId << ".0";
    _infnm = ffn.str();
  }
  _fInFile = 0;
  _fOutFile = 0;
}

Environment::~Environment() { Stop(); }

void Environment::Stop() {
  _SendEOT();
  _server.WaitAndClose(1000);
  ASSERT(0 == _fInFile);
  ASSERT(0 == _fOutFile);
  Utils::RemoveFile(_outfnm.c_str());
  Utils::RemoveFile(_infnm.c_str());
}

void Environment::OpenOutputFile() {
  ASSERT(0 == _fOutFile);
  _fOutFile = fopen(_outfnm.c_str(), "wt");
}

void Environment::CloseOutputFile(bool ReleaseSem) {
  ASSERT(0 != _fOutFile);
  fputs("Control: 5 0\n", _fOutFile);
  fclose(_fOutFile);
  _fOutFile = 0;
  if (ReleaseSem)
    _sem.Release(0);
}

void Environment::Start() {
  try {
    _server.Start(_cmndln.c_str());
  } catch (Exception e) {
    Utils::Message(e.Msg());
  }
  if (!_server.IsRunning()) {
    std::string errMsg("Environmental program ");
    errMsg += _cmndln.substr(0, _cmndln.find_first_of(" \t"));
    errMsg += " did not start.";
    Utils::Error(errMsg.c_str());
  }
  Utils::Sleep(1);
}

void Environment::_SendEOT() {
  Output output(this);
  ASSERT(0 != _fOutFile);
  fputs("Control: 9 0\n", _fOutFile);
}

void Environment::SendFollowingModule(const LstringIterator &iterator) {
  LstringIterator i(iterator);
  ++i;
  if (!i.AtEnd()) {
    __lc_ModuleIdType mid = i.GetModuleId();
    size_t sz = GetSizeOfParams(mid);
    const char *nm = GetNameOf(mid);
    fputs(nm, _fOutFile);
#ifdef NO_MEMCOPY
    const char *pF = i.Ptr() + sizeof(__lc_ModuleIdType);
#else
    // Mik 02/2013 - There is no guarantee the struct will not be padded with
    // extra bytes so we can (1) add 2 bytes to the pointer (assuming
    // sizeof(__lc_ModuleIdType)==2): const char* pF =
    // i.Ptr()+sizeof(__lc_ModuleIdType) + 2; or (2) get a pointer to the first
    // parameter of the module's parameters. Note: getting a pointer to the
    // first member: moduleId; does not help because there may be padding
    // between moduleId and the first parameter. We take a temporary struct
    // defined by L2C and assume it is padded the same way: struct
    // __lc_FollowingModuleStruct { struct Data { __lc_ModuleIdType moduleId;
    // float Param0;} data; };

    const __lc_FollowingModuleStruct *pFollowingModule =
        reinterpret_cast<const __lc_FollowingModuleStruct *>(i.Ptr());

    // now, we can set a pointer to the first floating point parameter
    const char *pF =
        reinterpret_cast<const char *>(&pFollowingModule->data.Param0);

    // using reinterpret_cast in this way is probably very compiler dependent,
    // so it would be better to add a function/pointer in l2c/lpfg that returns
    // a pointer to the first parameter. Wouldn't that cause a lot of extra
    // overhead, as each module would have a pointer to its first parameter?
#endif
    if (sz >= sizeof(float))
      fputc('(', _fOutFile);
    while (sz >= sizeof(float)) {
      float v;
      memcpy(&v, pF, sizeof(float));
      fprintf(_fOutFile, "%f", v);
      sz -= sizeof(float);
      pF += sizeof(float);
      if (sz >= sizeof(float))
        fputc(',', _fOutFile);
      else
        fputc(')', _fOutFile);
    }
  }
}

void Environment::SendData(float v, const LstringIterator &iterator,
                           const Turtle *pTurtle) {
  ASSERT(0 != _fOutFile);
  fprintf(_fOutFile, "%zu E(%f)", iterator.Position(), v);
  if (envparams.FollowingModule())
    SendFollowingModule(iterator);
  fputc('\n', _fOutFile);
  _SendTurtleData(pTurtle);
}

void Environment::SendData(float v1, float v2, const LstringIterator &iterator,
                           const Turtle *pTurtle) {
  ASSERT(0 != _fOutFile);
  fprintf(_fOutFile, "%zu E(%f,%f)", iterator.Position(), v1, v2);
  if (envparams.FollowingModule())
    SendFollowingModule(iterator);
  fputc('\n', _fOutFile);
  _SendTurtleData(pTurtle);
}

void Environment::SendData(int numParams, const float *vs,
                           const LstringIterator &iterator,
                           const Turtle *pTurtle) {
  ASSERT(0 != _fOutFile);
  fprintf(_fOutFile, "%zu E(", iterator.Position());
  for (int i = 0; i < numParams; i++) {
    if (i != 0)
      fprintf(_fOutFile, ",");
    fprintf(_fOutFile, "%f", vs[i]);
  }
  fprintf(_fOutFile, ")");
  if (envparams.FollowingModule())
    SendFollowingModule(iterator);
  fputc('\n', _fOutFile);
  _SendTurtleData(pTurtle);
}

void Environment::_SendTurtleData(const Turtle *pTurtle) const {
  if (!(_params.PosFmt().empty())) {
    Vector3d p = pTurtle->GetPosition();
    fprintf(_fOutFile, _params.PosFmt().c_str(), p.X(), p.Y(), p.Z());
    fputc('\n', _fOutFile);
  }
  if (!(_params.HeadFmt().empty())) {
    Vector3d h = pTurtle->GetHeading();
    fprintf(_fOutFile, _params.HeadFmt().c_str(), h.X(), h.Y(), h.Z());
    fputc('\n', _fOutFile);
  }
  if (!(_params.LeftFmt().empty())) {
    Vector3d l = pTurtle->GetLeft();
    fprintf(_fOutFile, _params.LeftFmt().c_str(), l.X(), l.Y(), l.Z());
    fputc('\n', _fOutFile);
  }
  if (!(_params.UpFmt().empty())) {
    Vector3d u = pTurtle->GetUp();
    fprintf(_fOutFile, _params.UpFmt().c_str(), u.X(), u.Y(), u.Z());
    fputc('\n', _fOutFile);
  }
}

void Environment::WaitForReply() { _sem.Wait(1); }

FILE *Environment::_OpenInput() {
  FILE *res = 0;
  res = fopen(_infnm.c_str(), "rt");
  return res;
}

void Environment::GetReply(EnvironmentReply &reply) {
  do {
    if (0 == _fInFile)
      _fInFile = _OpenInput();

    if (0 == _fInFile) {
      Utils::Message("Error opening response file\n");
      return;
    }
    ASSERT(0 != _fInFile);
    char bf[256];
    bf[0] = 0;
    if (0 == fgets(bf, 256, _fInFile)) {
      Utils::Message("Problem reading response\n");
      if (feof(_fInFile))
        Utils::Message("End of file\n");
      else if (ferror(_fInFile))
        perror("error: ");
      strcpy(bf, "Control: 5");
    }
    reply.Set(bf);
    if (!reply.DataPresent()) {
      fclose(_fInFile);
      _fInFile = 0;
    }
    if (reply.ExpectingMore()) {
      _sem.Release(0);
      WaitForReply();
    }
  } while (reply.ExpectingMore());
}

void EnvironmentReply::Set(const char *input) {
  std::string str(input);
  if (0 == strncmp("Control:", str.c_str(), 7)) {
    _hasData = false;
    int r;
    sscanf(str.c_str(), "Control: %d", &r);
    if (r & flLastChunk)
      _more = false;
    else
      _more = true;
  } else {
    _data.clear();
    _hasData = true;
    _more = false;
    // extract position
    _pos = atoi(str.c_str());
    // find beginning of E's parameters
    size_t token = str.find_first_of('(');
    do {
      str.erase(0, token + 1);
      float v = static_cast<float>(atof(str.c_str()));
      _data.push_back(v);
      token = str.find_first_of(",)");
    } while (str.at(token) != ')');
  }
}
