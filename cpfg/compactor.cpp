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



#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#ifdef WIN32
#include "warningset.h"
#endif

#include "compactor.h"

HashVertex::HashVertex(float x, float y, float z) {
  vertex[0] = x;
  vertex[1] = y;
  vertex[2] = z;
  initialIndex = 0;
  next = 0;
}

HashVertex::~HashVertex() {}

int HashVertex::compareWith(float x, float y, float z) {
  if ((fabs(vertex[0] - x) < ERRORRADIUS) &&
      (fabs(vertex[1] - y) < ERRORRADIUS) &&
      (fabs(vertex[2] - z) < ERRORRADIUS))
    return 1;
  return 0;
}

HashElement::HashElement() {
  hashVertexList = 0;
  lastHashVertex = 0;
  count = 0;
}

HashElement::~HashElement() {
  HashVertex *curHashVertex = hashVertexList;
  HashVertex *nextHashVertex;

  while (curHashVertex != 0) {
    nextHashVertex = curHashVertex->next;
    delete curHashVertex;
    curHashVertex = nextHashVertex;
  }
}

HashVertex *HashElement::findMatch(float v[3]) {
  HashVertex *curHashVertex = hashVertexList;
  HashVertex *foundHashVertex = 0;

  while (foundHashVertex == 0) {
    if (curHashVertex->compareWith(v[0], v[1], v[2]))
      return curHashVertex;
    else {
      curHashVertex = curHashVertex->next;
      if (curHashVertex == 0) // reached end of list
        return 0;
    }
  }
  return 0;
}

void HashElement::addHashVertex(float v[3], int vIndex) {
  HashVertex *newHashVertex = new HashVertex(v[0], v[1], v[2]);
  if (hashVertexList == 0)
    hashVertexList = newHashVertex;
  else
    lastHashVertex->next = newHashVertex;
  lastHashVertex = newHashVertex;
  count++;
  newHashVertex->initialIndex = vIndex;
}

MeshCompactor::MeshCompactor(int triNum, int *triArray, int verNumber,
                             float *verArray, float *norArray,
                             float *texArray) {
  int x;

  this->triNum = triNum;
  this->triArray = triArray;
  this->verNumber = verNumber;
  this->verArray = verArray;
  this->norArray = norArray;
  this->texArray = texArray;
  ;

  uniqueVertices = 0;
  hashArraySize = verNumber;
  hashArray = new HashElement *[hashArraySize];

  for (x = 0; x < hashArraySize; x++)
    hashArray[x] = 0;

  reindexArray = new int[verNumber];
  repetitionArray = new int[verNumber];
  for (x = 0; x < verNumber; x++) {
    reindexArray[x] = 0;
    repetitionArray[x] = 0;
  }

  // construct S-box
  unsigned char i, j, tmp;

  for (i = 0; i < 255; i++)
    sbox[i] = i;

  for (i = 0; i < 255; i++) {
    j = (unsigned char)(rand() % 256);
    tmp = sbox[i];
    sbox[i] = sbox[j];
    sbox[j] = tmp;
  }

  setDecimalPlaces(3);

  float dummy[3] = {-0.687052f, 1.000000f, -0.687052f};
  roundVertex(dummy);
}

MeshCompactor::~MeshCompactor() {
  for (int x = 0; x < hashArraySize; x++)
    delete hashArray[x];
  delete[] hashArray;
  delete[] reindexArray;
  delete[] repetitionArray;
}

/* S-box hashing function suitable for hashing vertices, based on
Ray Tracing News, Volume 9, Number 1 */

unsigned int MeshCompactor::hashVertex(float v[3]) {
  unsigned char *byte;
  unsigned long hash;

  byte = (unsigned char *)v;
  hash = 127 * sbox[byte[0] ^ 37] + 179 * sbox[byte[1] ^ 79] +
         233 * sbox[byte[2] ^ 41] + 283 * sbox[byte[3] ^ 83] +
         353 * sbox[byte[4] ^ 43] + 419 * sbox[byte[5] ^ 89] +
         467 * sbox[byte[6] ^ 47] + 547 * sbox[byte[7] ^ 97] +
         607 * sbox[byte[8] ^ 53] + 661 * sbox[byte[9] ^ 59] +
         739 * sbox[byte[10] ^ 61] + 811 * sbox[byte[11] ^ 67];

  return hash % hashArraySize;
}

int MeshCompactor::compact() {
  int x, y;

  for (x = 0; x < verNumber; x++)
    addVertex(verArray + x * 3, x);

  for (x = 0; x < triNum * 3; x++)
    triArray[x] = reindexArray[triArray[x]];

  for (x = 0; x < verNumber; x++) {
    if (norArray) {
      if (repetitionArray[x] > 1) {
        for (y = 0; y < 3; y++)
          norArray[x * 3 + y] /= repetitionArray[x];
      }
    }
  }

  int curIndex = 0;
  x = 0;

  while (curIndex == reindexArray[x]) {
    curIndex++;
    x++;
  }

  for (; x < verNumber; x++) {
    if (curIndex != reindexArray[x])
      continue;
    else {
      verArray[curIndex * 3 + 0] = verArray[x * 3 + 0];
      verArray[curIndex * 3 + 1] = verArray[x * 3 + 1];
      verArray[curIndex * 3 + 2] = verArray[x * 3 + 2];

      if (norArray) {
        norArray[curIndex * 3 + 0] = norArray[x * 3 + 0];
        norArray[curIndex * 3 + 1] = norArray[x * 3 + 1];
        norArray[curIndex * 3 + 2] = norArray[x * 3 + 2];
      }

      if (texArray) {
        texArray[curIndex * 2 + 0] = texArray[x * 2 + 0];
        texArray[curIndex * 2 + 1] = texArray[x * 2 + 1];
      }

      curIndex++;
    }
  }

  return curIndex;
}

void MeshCompactor::setDecimalPlaces(unsigned int n) {
  if (n > 16)
    n = 16;
  decimalPlaces = n;

  format[0] = '%';
  format[1] = '\0';
  sprintf(temp, ".%df", decimalPlaces);
  strcat(format, temp);

  for (unsigned int x = 0; x < sizeof(temp); x++)
    temp[x] = '\0';
}

void MeshCompactor::addVertex(float v[3], int vIndex) {
  int hashIndex;
  roundVertex(v);
  hashIndex = hashVertex(v);

  HashElement *curHashElement = hashArray[hashIndex];

  if (curHashElement == 0) // first time a vertex has been hashed here
  {                        // so create a new HashVertex object
    curHashElement = new HashElement();
    hashArray[hashIndex] = curHashElement;
    curHashElement->addHashVertex(v, vIndex);

    reindexArray[vIndex] = uniqueVertices;
    uniqueVertices++;
    repetitionArray[vIndex] = 1;
  } else // at least one vertex has already been hashed here, so check if the
  {      // the new vertex matches any existing ones
    HashVertex *matchingHashVertex = curHashElement->findMatch(v);
    if (matchingHashVertex == 0) // new vertex is unique
    {
      curHashElement->addHashVertex(v, vIndex);

      reindexArray[vIndex] = uniqueVertices;
      uniqueVertices++;
      repetitionArray[vIndex] = 1;
    } else // new vertex matches an existing one
    {
      int originalVertexIndex = matchingHashVertex->initialIndex;
      reindexArray[vIndex] = reindexArray[originalVertexIndex];
      repetitionArray[originalVertexIndex]++;
      if (norArray)
        for (int x = 0; x < 3; x++)
          norArray[originalVertexIndex * 3 + x] += norArray[vIndex * 3 + x];
      // printf("  vertex already exists\n");
    }
  }
}

void MeshCompactor::roundVertex(float v[3]) {
  sprintf(temp, format, v[0]);
  sscanf(temp, "%f", &(v[0]));
  sprintf(temp, format, v[1]);
  sscanf(temp, "%f", &(v[1]));
  sprintf(temp, format, v[2]);
  sscanf(temp, "%f", &(v[2]));

  // remove the sign bit if float value is 0 (-0 and +0 hash differently)
  if (v[0] == 0.0f)
    v[0] = fabsf(v[0]);
  if (v[1] == 0.0f)
    v[1] = fabsf(v[1]);
  if (v[2] == 0.0f)
    v[2] = fabsf(v[2]);
}

// Wrapper for C++ code, to be called from C source files

int compactMesh(int triNum, int *triArray, int verNumber, float *verArray,
                float *norArray, float *texArray) {
  MeshCompactor meshCompactor(triNum, triArray, verNumber, verArray, norArray,
                              texArray);

  int newVerNumber = meshCompactor.compact();

  return newVerNumber;
}
