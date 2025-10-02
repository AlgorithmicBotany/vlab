#ifndef terrain_inc
#define terrain_inc

#include "quadTree.h"
#include "TerrainPatch.h"

#include <iostream>
#include <fstream>

#define TERRAIN_DATA "small.patch"
#define TERRAIN_SCALE 128
#define NUM_LEVELS 4
#define DISTANCE_CONSTANT 0

using std::ifstream;

struct terrainData
{
	//public:
	void loadTerrain();
	bool surfaceHeightAt(V3f worldSpacePoint, V3f &surfaceIntersectionPoint);
	
	//private:
	void loadTerrain2(const char *fn);
	void calculateResolutions();
	void fillLeaves(QuadTree<TerrainPatch> *curr);
	void subDivide(QuadTree<TerrainPatch> *curr);
	void fillTerrainData(QuadTree<TerrainPatch> *curr, int rowOff, int colOff);
	bool distCheck(V3d pos, int level);
	Cracks checkForCracks(QuadTree<TerrainPatch> *curr);
	int drawGridLOD(QuadTree<TerrainPatch> *curr);
	void setDrawFlags(QuadTree<TerrainPatch> *curr);
	void markCracks(QuadTree<TerrainPatch> *curr);
	void setGlobalPointers(QuadTree<TerrainPatch> *curr);
	
	V3d cameraPos;
	QuadTree<TerrainPatch> *LODGrid;
	V3d **dataPoints[NUM_LEVELS], **vertexNormals[NUM_LEVELS];
	int numU[NUM_LEVELS];
	int numV[NUM_LEVELS];	
	
	double TERRAIN_WIDTH;
	bool showGrid;
};

// returns the Patch of terrain whos data covers that of the point in the xz plane
TerrainPatch* getNearestPatch(V3d point, QuadTree<TerrainPatch> *terrainTree)
{
	if (terrainTree->data->topLeft.z <= point.z)
	{
		if (terrainTree->data->bottomRight.z >= point.z)
		{
			if (terrainTree->data->topLeft.x <= point.x)
			{
				if (terrainTree->data->bottomRight.x >= point.x)
				{
					//Point is in the rectangle
					if (terrainTree->level == NUM_LEVELS-1)
					return terrainTree->data;
					else
					{
						TerrainPatch *NW,*NE,*SW,*SE;
						NW = getNearestPatch(point, terrainTree->NW);
						if (NW == NULL)
						{
							NE = getNearestPatch(point, terrainTree->NE);
							if (NE == NULL)
							{
								SW = getNearestPatch(point, terrainTree->SW);
								if (SW == NULL)
								{
									SE = getNearestPatch(point, terrainTree->SE);
									if (SE == NULL)
										return NULL;
									else
										return SE;
								}
								else
									return SW;
							}
							else
								return NE;
						}
						else
							return NW;
					}
				}
			}
		}
	}
	return NULL;
}

// Casts a ray from the worldSpacePoint In the Y Direction and returns the point of intersection with the terrain
// Algorithm from http://pages.cpsc.ucalgary.ca/~samavati/cpsc453/pdfs/week13.pdf
// and   http://www.devmaster.net/wiki/Ray-triangle_intersection
bool terrainData::surfaceHeightAt(V3f worldSpacePoint, V3f &surfaceIntersectionPoint)
{
	
	TerrainPatch *patch = getNearestPatch(worldSpacePoint, LODGrid);
	if (patch == NULL)
	return false;
	
	V3d N,A,B,C,P,O,D;
	double dist, alpha, beta, gamma, S, S1, S2;
	
	worldSpacePoint.y = patch->maxHeight;
	O = worldSpacePoint; // Origin of the ray
	D = V3d(0.0,-1.0,0.0); // Direction of the ray
	
	for (int u=0; u<patch->_numU-1; ++u)
	{
		for (int v=0; v<patch->_numV-1; ++v)
		{
			// BOTTOM TRIANGLE
			N = (patch->_vertexNormals[u][v] + patch->_vertexNormals[u][v+1] + patch->_vertexNormals[u+1][v+1]) / 3.0; // The normal of the triangle
			A = patch->_dataPoints[u][v]; // A point on the triangle
			B = patch->_dataPoints[u][v+1];
			C = patch->_dataPoints[u+1][v+1];
			
			dist = -((O-A)*N)/(D*N);
			P = O + dist*D; // Point in the plane of the triangle. 
			
			// Now test weather this point its inside the triangle with barycentric coordinates
			S = 0.5 * CrossProduct((A-B),(A-C)).Length();
			S1 = 0.5 * CrossProduct((B-P),(C-P)).Length();
			S2 = 0.5 * CrossProduct((P-C),(P-A)).Length();
			
			alpha = S1/S;
			beta  = S2/S;
			gamma = 1 - alpha - beta;
			
			if ( alpha >= 0.0 && alpha <= 1.0)
			{
				if (beta >= 0.0 && beta <= 1.0)
				{
					if (gamma >= 0.0 && gamma <= 1.0)
					{
						surfaceIntersectionPoint = P;
						return true;
					}
				}
			}
			
			// TOP TRIANGLE
			N = (patch->_vertexNormals[u][v] + patch->_vertexNormals[u+1][v] + patch->_vertexNormals[u+1][v+1]) / 3.0; // The normal of the triangle
			B = patch->_dataPoints[u+1][v];
			
			// Now test weather this point its inside the triangle with barycentric coordinates
			S = 0.5 * CrossProduct((A-B),(A-C)).Length();
			S1 = 0.5 * CrossProduct((B-P),(C-P)).Length();
			
			alpha = S1/S;
			beta  = S2/S;
			gamma = 1 - alpha - beta;
			
			if ( alpha >= 0.0 && alpha <= 1.0)
			{
				if (beta >= 0.0 && beta <= 1.0)
				{
					if (gamma >= 0.0 && gamma <= 1.0)
					{
						surfaceIntersectionPoint = P;
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool terrainData::distCheck(V3d pos, int level)
{
	pos.y = 0; 
	double dist = (cameraPos - pos).Length();
	
	double RAD_PER_LEVEL = sqrt(2.0) * (TERRAIN_WIDTH/pow(2.0,(double)level)) + TERRAIN_WIDTH/NUM_LEVELS;
	if (dist <= (NUM_LEVELS - level)*RAD_PER_LEVEL + DISTANCE_CONSTANT)
		return true;
	else
		return false;
}

Cracks terrainData::checkForCracks(QuadTree<TerrainPatch> *curr)
{
	Cracks cracks;
	cracks.N = false; cracks.E = false; cracks.S = false; cracks.W = false;

	if (curr->level == 0 || curr->level == 1)
		return cracks;
	
	if (curr->isFrom == NW_QUAD)
	{
		if (curr->N != NULL)
		{
			if (curr->N->parent->data->toBeDrawn)
				cracks.N = true;	
		}
		if (curr->W != NULL)
		{
			if (curr->W->parent->data->toBeDrawn)
				cracks.W = true;
		}
	}
	else if (curr->isFrom == NE_QUAD)
	{
		if (curr->N != NULL)
		{	
			if (curr->N->parent->data->toBeDrawn)
				cracks.N = true;
		}
		if (curr->E != NULL)
		{
			if (curr->E->parent->data->toBeDrawn)
				cracks.E = true;
		}
	}
	else if (curr->isFrom == SE_QUAD)
	{
		if (curr->S != NULL)
		{
			if (curr->S->parent->data->toBeDrawn)
				cracks.S = true;	
		}
		if (curr->E != NULL)
		{
			if (curr->E->parent->data->toBeDrawn)
				cracks.E = true;
		}
	}
	else if (curr->isFrom == SW_QUAD)
	{
		if (curr->S != NULL)
		{
			if (curr->S->parent->data->toBeDrawn)
				cracks.S = true;
		}
		if (curr->W != NULL)
		{
			if (curr->W->parent->data->toBeDrawn)
				cracks.W = true;
		}
	}
	
	return cracks;
}

void terrainData::setDrawFlags(QuadTree<TerrainPatch> *curr)
{
	bool NE,NW,SE,SW;
	NE = NW = SE = SW = false;

	if ( curr->getNE() != NULL &&  curr->getNW() != NULL &&  curr->getSW() != NULL &&  curr->getSE() != NULL )
	{
		NE = distCheck(curr->getNE()->data->midPoint, curr->getNE()->level );

		NW = distCheck(curr->getNW()->data->midPoint, curr->getNW()->level );

		SE = distCheck(curr->getSE()->data->midPoint, curr->getSE()->level );

		SW = distCheck(curr->getSW()->data->midPoint, curr->getSW()->level );
	}

	if (curr->level < NUM_LEVELS-1)
	{
		if ( NE || NW || SE || SW )
		{
			if (NE)
				setDrawFlags(curr->getNE());
			else
				curr->getNE()->data->toBeDrawn = true;

			if (NW)
				setDrawFlags(curr->getNW());
			else
				curr->getNW()->data->toBeDrawn = true;

			if (SE)
				setDrawFlags(curr->getSE());
			else
				curr->getSE()->data->toBeDrawn = true;

			if (SW)
				setDrawFlags(curr->getSW());
			else
				curr->getSW()->data->toBeDrawn = true;
		}
		else
			curr->data->toBeDrawn = true;
	}
	else
		curr->data->toBeDrawn = true;
}

void terrainData::fillTerrainData(QuadTree<TerrainPatch> *curr, int rowOff, int colOff)
{
	int numRows = pow(2.0,(double)curr->level);
	
	if (curr->level < NUM_LEVELS)
	{
		curr->data->loadPatchFrom(dataPoints[curr->level], vertexNormals[curr->level], numU[curr->level], numV[curr->level], numRows, numRows, rowOff, colOff);
		fillTerrainData(curr->getNW(), rowOff*2, colOff*2);
		fillTerrainData(curr->getNE(), rowOff*2, (colOff*2)+1);
		fillTerrainData(curr->getSW(), (rowOff*2)+1, colOff*2);
		fillTerrainData(curr->getSE(), (rowOff*2)+1, (colOff*2)+1);
	}
}

void terrainData::subDivide(QuadTree<TerrainPatch> *curr)
{
	curr->spawnChildren();
	
	curr->getNW()->data = new TerrainPatch();
	curr->getNE()->data = new TerrainPatch();
	curr->getSW()->data = new TerrainPatch();
	curr->getSE()->data = new TerrainPatch();
}

void terrainData::fillLeaves(QuadTree<TerrainPatch> *curr)
{	
	if (curr->getNW() == NULL && curr->getNE() == NULL && curr->getSW() == NULL && curr->getSE() == NULL)
	subDivide(curr);
	else
	{
		fillLeaves(curr->getNW());
		fillLeaves(curr->getNE());
		fillLeaves(curr->getSW());
		fillLeaves(curr->getSE());
	}
}

void terrainData::setGlobalPointers(QuadTree<TerrainPatch> *curr)
{
	curr->setGlobalPointers();
	
	if (curr->level < NUM_LEVELS -1 )
	{
		setGlobalPointers(curr->getNW());
		setGlobalPointers(curr->getNE());
		setGlobalPointers(curr->getSW());
		setGlobalPointers(curr->getSE());
	}
}

void terrainData::calculateResolutions()
{
	for (int i=NUM_LEVELS-1; i>0; i--)
	{
		for (int u=0; u<numU[i-1]; ++u)
		{
			for (int v=0; v<numV[i-1]; ++v)
			{
				dataPoints[i-1][u][v] = dataPoints[i][u*2][v*2];
			}
		}
	}
}

void terrainData::loadTerrain()
{
	loadTerrain2(TERRAIN_DATA);
	calculateResolutions();
	
	TERRAIN_WIDTH *= TERRAIN_SCALE;
	
	LODGrid = new QuadTree<TerrainPatch>(0,0,NONE);
	LODGrid->data = new TerrainPatch();
	
	for (int i=0; i<NUM_LEVELS; ++i)
	fillLeaves(LODGrid);

	setGlobalPointers(LODGrid);
	fillTerrainData(LODGrid, 0,0);
	
	cameraPos = V3d(0,0,0);
	showGrid = false;
}

void terrainData::loadTerrain2(const char *fn)
{
	ifstream File;
	File.open(fn);
	if (File)
	{	
		File >> TERRAIN_WIDTH;
		for (int l=0; l<NUM_LEVELS; ++l)
		{
			
			File >> numU[l] >> numV[l];

			// Allocate the memory for the data points
			dataPoints[l] = new V3d* [numU[l]];
			for (int i=0; i<numU[l]; ++i)
			dataPoints[l][i] = new V3d[numV[l]];
			
			// Allocate the memory for the normals
			vertexNormals[l] = new V3d* [numU[l]];
			for (int i=0; i<numU[l]; ++i)
			vertexNormals[l][i] = new V3d[numV[l]];
			
			//read in the data points
			for (int i=0; i<numU[l]; ++i)
			{
				for (int j=0; j<numV[l]; ++j)
				{
					File >> dataPoints[l][i][j].x >> dataPoints[l][i][j].y >> dataPoints[l][i][j].z;
					File >> vertexNormals[l][i][j].x >> vertexNormals[l][i][j].y >> vertexNormals[l][i][j].z;
					
					dataPoints[l][i][j]*= TERRAIN_SCALE;
				}
			}
		}
		File.close();
	}
	else 
		printf("Cant open File");
}

void terrainData::markCracks(QuadTree<TerrainPatch> *curr)
{
	if (curr->data->toBeDrawn)
	curr->data->cracks = checkForCracks(curr);
	else
	{
		markCracks(curr->getNE());
		markCracks(curr->getNW());
		markCracks(curr->getSE());
		markCracks(curr->getSW());
	}
}

int terrainData::drawGridLOD(QuadTree<TerrainPatch> *curr)
{
	int triCount = 0;
	if (curr->data->toBeDrawn)
	triCount += curr->data->draw(true, showGrid);
	else
	{
		triCount += drawGridLOD(curr->getNE());
		triCount += drawGridLOD(curr->getNW());
		triCount += drawGridLOD(curr->getSE());
		triCount += drawGridLOD(curr->getSW());
	}

	return triCount;
}

terrainData terrain;
module drawTerrain(terrainData);

interpretation:

drawTerrain(td):
{
	CameraPosition realCameraPos = GetCameraPosition(0);
	double scale = td.TERRAIN_WIDTH/2.0 * vvScale(0);
	
	td.cameraPos = realCameraPos.position;
	td.cameraPos = td.cameraPos + scale*V3d(realCameraPos.head.x,realCameraPos.head.y,realCameraPos.head.z);
	
	td.setDrawFlags(td.LODGrid);
	td.markCracks(td.LODGrid);

	nproduce SB SetColor(2);
	td.drawGridLOD(td.LODGrid);
	nproduce EB;

	produce;
}

#endif