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



#ifndef __PIPEPAIR_H__
#define __PIPEPAIR_H__

class PipePair {
#ifdef _WINDOWS
public:
  PipePair(SECURITY_ATTRIBUTES *);
  ~PipePair();
  HANDLE ReadEnd() const { return _hRead; }
  HANDLE WriteEnd() const { return _hWrite; }
  bool GetLine(char *, size_t);
  bool GetLine(char *, size_t, DWORD, DWORD);
  void Send(const char *);

private:
  HANDLE _hRead;
  HANDLE _hWrite;
#endif
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
