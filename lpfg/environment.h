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



#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include <string>
#include <vector>

#include <cstdio>

#include "asrt.h"
#include "semaphore.h"
#include "process.h"

class LstringIterator;
class Turtle;
class EnvironmentParams;

class EnvironmentReply {
public:
  EnvironmentReply() : _hasData(false), _more(false) {}
  void Set(const char *);
  bool DataPresent() const { return _hasData; }
  int Position() const {
    ASSERT(DataPresent());
    return _pos;
  }
  size_t Size() const { return _data.size(); }
  bool ExpectingMore() const { return _more; }
  float GetParameter(size_t item) const { return _data[item]; }

private:
  enum flags { flFirstChunk = 1, flLastChunk = 4, flExit = 8 };
  bool _hasData;
  bool _more;
  int _pos;
  std::vector<float> _data;
};

class Environment {
public:
  Environment(const EnvironmentParams &);
  ~Environment();

  void Start();
  void Stop();
  void SendData(float, const LstringIterator &, const Turtle *);
  void SendData(float, float, const LstringIterator &, const Turtle *);
  void SendData(int, const float *, const LstringIterator &, const Turtle *);
  void WaitForReply();
  void GetReply(EnvironmentReply &);

  class Output {
  public:
    Output(Environment *pEnv) : _pEnv(pEnv), _ignore(false) {
      if (0 != _pEnv)
        _pEnv->OpenOutputFile();
    }
    ~Output() {
      if (0 != _pEnv)
        _pEnv->CloseOutputFile(!_ignore);
    }
    void Ignore() { _ignore = true; }

  private:
    Environment *_pEnv;
    bool _ignore;
  };

  void OpenOutputFile();
  void CloseOutputFile(bool /* sendToEnv */);
  bool IsRunning() const { return _server.IsRunning(); }

private:
  void SendFollowingModule(const LstringIterator &);
  FILE *_OpenInput();
  void _SendEOT();
  void _SendTurtleData(const Turtle *) const;

  const EnvironmentParams &_params;
  SemaphorePair _sem;
  Process _server;
  std::string _cmndln;
  std::string _outfnm;
  std::string _infnm;
  FILE *_fOutFile;
  FILE *_fInFile;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
