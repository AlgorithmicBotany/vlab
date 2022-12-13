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



#ifdef _WINDOWS

#include <windows.h>

#include "pipepair.h"

PipePair::PipePair(SECURITY_ATTRIBUTES *pSA) {
  CreatePipe(&_hRead, &_hWrite, pSA, 0);
}

PipePair::~PipePair() {
  CloseHandle(_hRead);
  CloseHandle(_hWrite);
}

bool PipePair::GetLine(char *line, size_t maxlen) {
  DWORD avail;
  PeekNamedPipe(_hRead, 0, 0, 0, &avail, 0);
  if (0 == avail)
    return false;
  size_t ix = 0;
  for (;;) {
    PeekNamedPipe(_hRead, 0, 0, 0, &avail, 0);
    if (avail > 0) {
      DWORD read;
      ReadFile(_hRead, line + ix, 1, &read, 0);
      if (10 == line[ix]) {
        line[ix + 1] = 0;
        break;
      }
      ++ix;
      if (ix == maxlen - 1) {
        line[ix] = 0;
        break;
      }
    } else {
      line[ix] = 0;
      break;
    }
  }
  return true;
}

bool PipePair::GetLine(char *line, size_t maxlen, DWORD timeout,
                       DWORD waitint) {
  line[0] = 0;
  for (;;) {
    DWORD read, avail, left;
    PeekNamedPipe(_hRead, line, static_cast<DWORD>(maxlen), &read, &avail,
                  &left);
    if (read > 0) {
      char *lf = strchr(line, '\n');
      if (0 != lf) {
        ReadFile(_hRead, line, static_cast<DWORD>(lf - line) + 1, &read, 0);
        if (lf > line) {
          if (lf[-1] == '\r') {
            lf[-1] = '\n';
            lf[0] = 0;
          } else
            lf[1] = 0;
        }
        return true;
      } else {
        ReadFile(_hRead, line, read, &read, 0);
        maxlen -= read;
        line += read;
      }
    }
    if (timeout == 0) {
      line[0] = 0;
      return false;
    }
    if (timeout < waitint)
      waitint = timeout;
    Sleep(waitint);
    timeout -= waitint;
  }
}

void PipePair::Send(const char *str) {
  const char cr = '\r';
  while (0 != str[0]) {
    DWORD written;
    if (str[0] == '\n')
      WriteFile(_hWrite, &cr, 1, &written, 0);
    WriteFile(_hWrite, str, 1, &written, 0);
    ++str;
  }
}

#endif
