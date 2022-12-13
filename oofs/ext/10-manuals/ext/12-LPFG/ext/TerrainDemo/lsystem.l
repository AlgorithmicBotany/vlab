#include "lpfgall.h"

VisibilityMode terrainVisibilityMode;
CameraPosition cameraPos;

derivation length: 100000;

Axiom: Terrain(cameraPos);

Start:
{
  terrainVisibilityMode = Shaded;
  terrainVisibilityAll(terrainVisibilityMode);
}

production:

Terrain(t):
{
	cameraPos=GetCameraPosition(0);
	produce Terrain(cameraPos);
}

	



