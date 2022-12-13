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



#ifndef __SPLINEFUN_H__
#define __SPLINEFUN_H__

class WorldPoint;
class ReadTextFile;
class WriteTextFile;

class SplineFunction {
public:
  SplineFunction();
  SplineFunction(const char *, int);
  SplineFunction(ReadTextFile &, int = 100);
  ~SplineFunction();
  double LowerLimit() const { return _LowLimit; }
  double UpperLimit() const { return _UpLimit; }
  void operator=(const SplineFunction &);
  double GetValue(double) const;
  bool Reread();
  bool Reread(ReadTextFile &);
  void Dump(WriteTextFile &) const;
  const char *GetName() const { return _Fname; }

private:
  bool _Load0000(ReadTextFile &, int /* samples */);
  bool _Load0101(ReadTextFile &);
  bool _Reread0000(ReadTextFile &);
  bool _Reread0101(ReadTextFile &);
  void _Precompute(int);
  void _Default();
  double _Value(double) const;
  int waitOpenFile(const char *fname);
  double _LowLimit;
  double _UpLimit;
  size_t _size;
  int _samples;
  WorldPoint *_arr;
  double *_vals;
  char _Fname[MaxPath + 1];

  WorldPoint P(double) const;
  double N(int, int, double) const;
  double Nk1(int, double) const;
  double Nkt(int, int, double) const;
  int Uk(int) const;
};

#else
#error File already included
#endif
