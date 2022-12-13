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



#ifndef __SPLINEARRAY_H__
#define __SPLINEARRAY_H__

class SplineFunctionArray : public DynArray<SplineFunction> {
public:
  SplineFunctionArray() : DynArray<SplineFunction>(16) { _galleryname[0] = 0; }
  bool Read(const char *, int);
  bool ReadGallery(const char *);
  void Reset();
  bool Reread();
  const char *GetName(int) const;
  const char *GetGalleryName() const { return _galleryname; }

private:
  bool _RereadGallery();
  bool _Read0101(ReadTextFile &);
  bool _Reread0101(ReadTextFile &);
  int waitOpenFile(const char *fname);
  char _galleryname[MaxPath + 1];
};

#else
#error File already included
#endif
