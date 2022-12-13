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



#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdlib>
#include <iostream>

namespace Utils {
unsigned int Execute(const char *, int envsize = 0);
void ExecuteDetached(const char *);
void Message(const char *, ...);
void Mark(const char *);
void Error(const char *);
void SetEnv(const char * /* command line path */);
void Log(const char *);
void ResetLog();
void Quitting();
bool IsWinNT();
void Sleep(int);

const char *ReadFloats(const char *, float *, int);
const char *SkipBlanks(const char *);
const char *SkipNonBlanks(const char *);
const char *ReadQuotedString(const char *, char *, int);
const char *FirstAlphabetic(const char *);

struct SEWrapper {
  SEWrapper(unsigned int e) : n(e) {}
  unsigned int n;
};

void RemoveFile(const char *);

class RawMemory {
public:
  RawMemory(size_t sz);
  ~RawMemory();
  operator char *() { return _mem; }
  void SetSizeAtLeast(size_t);
  size_t Size() const { return _size; }

private:
  size_t _size;
  char *_mem;
};

class WaitCursor {
public:
  WaitCursor();
  ~WaitCursor();
};

template <class Turtle> class PPName {
public:
  PPName() {}
  ~PPName() {}
  void Set(size_t) {}
};

class ProcessTimer {
#ifdef _WINDOWS
  long unsigned int startTime, endTime;
#endif
public:
  ProcessTimer();
  void Stop();
  long unsigned int msecElapsed() const;
};

class PostscriptPage {
public:
  PostscriptPage(std::ostream &trg) : _trg(trg) {
    _trg << "%!PS-Adobe" << std::endl;
    _trg << "%%Creator: Lpfg" << std::endl;
  }
  ~PostscriptPage() { _trg << "showpage" << std::endl; }

private:
  std::ostream &_trg;
};

} // namespace Utils

#ifdef Debug
#error Debug already defined
#else
#define Debug                                                                  \
  if (comlineparam.DebugMode())                                                \
  Utils::Message
#endif

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
