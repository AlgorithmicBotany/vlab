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



#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

class ReadTextFile;

class ConfigFile {
protected:
  ConfigFile() : _Flags(0) {}
  int _Label(const char *, int &, const char *lbls[], int) const;
  bool _ReadOnOff(const char *, unsigned int);
  void _Error(const char *, const ReadTextFile &) const;
  void _FlagSet(unsigned int f) { _Flags |= f; }
  void _FlagClear(unsigned int f) { _Flags &= ~f; }
  bool _IsFlagSet(unsigned int f) const { return f == (_Flags & f); }

private:
  unsigned int _Flags;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
