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



#ifndef __COMPACTOR_H__
#define __COMPACTOR_H__

/*
MeshCompactor  v 1.0
April 2002
Martin Fuhrer

Compact a mesh by welding together duplicate vertices.
*/

#include "compactmesh.h"

// coordinates with differences less than ERRORRADIUS are considered identical
const double ERRORRADIUS = 0.00001;

class HashVertex {
public:
  HashVertex(float x, float y, float z);
  ~HashVertex();
  int compareWith(float x, float y, float z);

  int initialIndex;
  HashVertex *next;

private:
  float vertex[3];
};

class HashElement {
public:
  HashElement();
  ~HashElement();
  HashVertex *findMatch(float v[3]);
  void addHashVertex(float v[3], int vIndex);

  HashVertex *hashVertexList; // first item in the list
  HashVertex *lastHashVertex; // last item in the list
  int count;                  // is this needed?
};

class MeshCompactor {
public:
  MeshCompactor(int triNum, int *triArray, int verNumber, float *verArray,
                float *norArray, float *texArray);
  ~MeshCompactor();

  unsigned int hashVertex(float v[3]);
  int compact();
  void setDecimalPlaces(unsigned int n);

private:
  void addVertex(float v[3], int vIndex);
  void roundVertex(float v[3]);

  HashElement **hashArray;
  int hashArraySize;
  unsigned char sbox[256];

  int triNum;
  int verNumber;
  int *triArray;
  float *verArray;
  float *norArray;
  float *texArray;

  int *reindexArray;
  int *repetitionArray;

  int uniqueVertices; // running count of number of unique vertices
  int decimalPlaces;  // number of decimal places to round to

  char format[10];
  char temp[40];
};

#endif
