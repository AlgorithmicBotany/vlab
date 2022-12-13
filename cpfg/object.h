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



#ifndef _CPFG_OBJECT_
#define _CPFG_OBJECT_

turtleDrawDispatcher *objSetDispatcher(DRAWPARAM *, VIEWPARAM *);

void objCreateTriMesh(int maxTriangles);
void objExpandTriMesh(int addTriangles);
void objExpandTriMeshV(int addTriangles, int addVertices);
void objWriteTriMesh();
void objCreateSurfaceMesh(int maxTriangles, char surfaceIdentifier);

typedef struct {
  int verMax, triMax, verNum, triNum, verElements, triElements, texElements, id;
  float *meshVertices, *meshTexture, *meshNormals, *meshSurface;
  int *meshTriangles;
} MeshStruct;

#endif /* _CPFG_OBJECT_ */
