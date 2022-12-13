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

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "utils.h"
#include "exception.h"

#include "comlineparam.h"

#ifdef _WINDOWS
#include "utilsWin.imp"
#endif
#ifdef LINUX
#include "utilsLnx.imp"
#endif

#define NOMESSAGE 0

static FILE *fpLog = 0;

void Utils::Message(const char *format, ...) {
  if (!comlineparam.QuietMode()) {
    static char bf[10240];
    va_list args;
    va_start(args, format);
    vsprintf(bf, format, args);
    va_end(args);
    if (NOMESSAGE == 0)
      Log(bf);
  }
}

void Utils::Mark(const char *msg) {
  Log("\n\n*********  ");
  Log(msg);
  Log("\n");
}

void Utils::Error(const char *msg) {
  Log("\n\n*********\n");
  Log(msg);
  Log("\n*********\n");
}

void Utils::Log(const char *str) {
  if (0 == fpLog)
    fpLog = fopen("lpfg.log", "wt");
  fputs(str, stdout);
  if (0 != fpLog)
    fputs(str, fpLog);
}

void Utils::ResetLog() {
  if (0 != fpLog)
    fclose(fpLog);

  fpLog = fopen("lpfg.log", "wt");
}

void Utils::Quitting() {
  if (0 != fpLog)
    fclose(fpLog);
}

const char *Utils::ReadFloats(const char *txt, float *arr, int count) {
  txt = Utils::SkipBlanks(txt);
  if (0 == *txt)
    throw Exception("Error reading float\n");
  for (int i = 0; i < count; ++i) {
    int res = sscanf(txt, "%f", &(arr[i]));
    if (res != 1)
      throw Exception("Error reading float\n");
    txt = Utils::SkipNonBlanks(txt);
    if (i < count - 1) {
      if (0 == *txt)
        throw Exception("Error reading float\n");
      txt = Utils::SkipBlanks(txt);
    }
  }
  txt = Utils::SkipBlanks(txt);
  if (0 == *txt)
    return 0;
  else
    return txt;
}

const char *Utils::SkipBlanks(const char *txt) {
  while (isspace(*txt))
    ++txt;
  return txt;
}

const char *Utils::SkipNonBlanks(const char *txt) {
  while (!isspace(*txt) && 0 != *txt)
    ++txt;
  return txt;
}

const char *Utils::FirstAlphabetic(const char *txt) {
  static char alphas[55] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
  return txt + strcspn(txt, alphas);
}

const char *Utils::ReadQuotedString(const char *src, char *trg, int bflen) {
  if ('"' == src[0]) {
    ++src;
    int i = 0;
    while ((src[i] != '"') && (0 != src[i])) {
      if (i == bflen - 1) {
        trg[i] = 0;
        return src + i;
      }
      trg[i] = src[i];
      ++i;
    }
    trg[bflen - 1] = 0;
    return src + i + 1;
  } else {
    int i = 0;
    while (!isspace(src[i]) && 0 != (src[i])) {
      if (i == bflen - 1) {
        trg[i] = 0;
        return src + i;
      }
      trg[i] = src[i];
      ++i;
    }
    return src + i;
  }
}

Utils::RawMemory::RawMemory(size_t sz) {
  _size = sz;
  _mem = reinterpret_cast<char *>(malloc(sz));
  if (0 == _mem)
    throw Exception("Out of memory");
}

void Utils::RawMemory::SetSizeAtLeast(size_t sz) {
  if (sz > _size) {
    char *newmem = reinterpret_cast<char *>(realloc(_mem, sz));
    if (0 == newmem)
      throw Exception("Out of memory");
    _size = sz;
    _mem = newmem;
  }
}

Utils::RawMemory::~RawMemory() { free(_mem); }
