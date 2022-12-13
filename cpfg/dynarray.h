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



#ifndef __DYNARRAY_H__
#define __DYNARRAY_H__

#include <cassert>

template <class T> class DynArray {
public:
  DynArray(int n) {
    assert(n > 0);
    _size = n;
    _arr = new T[_size];
    _items = 0;
  }
  ~DynArray() { delete[] _arr; }
  int Count() const { return _items; }
  T &operator[](int i) {
    assert(i >= 0);
    assert(i < _items);
    return _arr[i];
  }
  void Add(const T &item) {
    if (_size == _items)
      _Grow();
    _arr[_items] = item;
    _items++;
  }

protected:
  T *_arr;
  int _size;
  int _items;
  void _Grow() {
    T *pNew = new T[_size * 2];
    _size *= 2;
    for (int i = 0; i < _items; i++)
      pNew[i] = _arr[i];
    delete[] _arr;
    _arr = pNew;
  }
};

#else
#error File already included
#endif
