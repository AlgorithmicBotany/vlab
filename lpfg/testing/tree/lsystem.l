#include <algorithm>
using std::max;
using std::min;
using std::sort;

#include <cstdio>

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>

#include "lpfgall.h"
#include "helper.cpp"
//#include "asmlib.h"

#define VOXEL_DENSITY 196
#define MAX_UNITS 450000
#define MAX_INTERNODES 900000

#define MAX_INTERNODESDROOP	1
#define MAX_UNITSDROOP	1
#define MAX_CHILDREN 69
#define MAX_PLANTS 1
#define MAX_SPECIES 3

#define MAX_GRASS 250000

#define MAX_UNITS_SMALL 20000
#define MAX_INTERNODES_SMALL 70000
#define MAX_PLANTS_SMALL 50
//extern "C" uint64 ReadTSC (void);

//#define HERE Printf("Here: %d\n", __LINE__)
#define HERE void(0)

class Branch;
struct InternodeData;
struct ApexDroopData;
struct Plant;

void queryVoxel(float x, float y, float z, /*results->*/ int* a, int* b, int* c);


enum BudMode
{
	wait,
	flower,
	shortNode,
	longNode,
	longShoot
};

struct InternodeData
{
	int id;
	int age;
	float iLength;
	float iWidth;
	float iRadialGrowth;
	bool isLast;

	Branch* branch;
	InternodeData* lastInternode;
};
typedef InternodeData* InternodePointer;

struct InternodeDroopData
{
	float position[3];
	int id;
	int age;
	float iLength;
	float iWidth;
	float iRadialGrowth;
	bool isLast;

	Branch* branch;
	InternodeDroopData* nextInternode;
};
typedef InternodeDroopData* InternodeDroopPointer;

struct ApexData
{
	
	float budAngle;
	float lastBudAngle;
	float light;
	int age;
	int order;
	int lastBudId;
	int id;
	int parentId;
	int num_childBuds;
	int lastChildNumber;

	int VoxelPosition[3];
	//float growthSpeed;
	//float branchProb;

	//bool indeterminate;

	float vigor;
	float rank;

	float headY, upY;	

	float apicalDominanceFactor;

	InternodeData* lastInternode;

	ApexDroopData* apexDroop;

	Plant* plant;
};
typedef ApexData* ApexPointer;

struct ApexDroopData
{
	float position[3];
	float budAngle;
	float lastBudAngle;
	float light;
	int age;
	int order;
	int lastBudId;
	int id;
	int parentId;
	int num_childBuds;
	int lastChildNumber;

	int VoxelPosition[3];
	//float growthSpeed;
	//float branchProb;

	//bool indeterminate;

	float vigor;
	float rank;

	float apicalDominanceFactor;

	InternodeDroopData* lastInternode;

	Plant* plant;
};
typedef ApexDroopData* ApexDroopPointer;

struct BudData
{
	float direction;
	int position;
	float positionAngle;
	BudMode budmode;
	int age;
	int order;
	int id;
	int parentId;
	int childNumber;

	int sleepCycles;

	InternodeData* parentInternode;
	//ApexData* parentApex;

	Plant* plant;
};

struct Voxel
{
	float light;
};

struct BranchPointData
{
	//int age;
	//float iLength;
	//float iWidth;
	//float iRadialGrowth;
	//int branchNumber;

	ApexPointer branchApex;
	bool cut;
	int cutDelay;

	//float energy;
	//float upkeep;
	//float upkeepEnergyRatio;

	Branch* branch;
	InternodeData* internode;
	InternodeData* parentInternode;
	BranchPointData* previousBranchPoint;
};
typedef BranchPointData* BranchPointer;

class Branch;

typedef Branch* ChildBranch;
class Branch
{
public:
	//Branch* childList[512];

	int VoxelPosition[3];

	int id;
	int lastChildId;
	int num_childBranches;			//how many daughter and granddaughter branches are from this branch
	
	int lastChildNumber;
	int parentId;

	int childNumber;				//when was this branch spawned by its parent
	ApexPointer parentApex;
	ApexPointer branchApex;
	float energy;
	float directEnergy;

	BudMode branchmode;

	BranchPointer branchPoint;

	int children;

	int order;

	float radialWidth;

	float rank;
	float parentVigor;
	float energyBudRatio;

	float vigor;
	float oldVigor;

	int childId;

	float apicalDominanceFactor;

	float leafDownAngle;

	float vigorCap;

	int lastChild; //childs of this branch
	ChildBranch childBranches[MAX_CHILDREN];

	Plant *plant;

	//friend bool operator<(const branch& a, const branch& b);
	Branch& operator=(const Branch& a);
};

struct ActiveParams
{
	//active parameters
	float cutThreshold;
	float plantDomThreshold[2];		//1: dominance 2: sapling
	float plantSaplingStrategy[2];
	float plantVigorousStrategy[2];
	float plantMatureStrategy[4];			//1: SP up (0,1,0) 2: SP down 3: SP: horizontal 4: up all
	float apicalDomThreshold[2];	//1: dominance 2: !dominance
	float direction[3];				//1: 
	float distEnergyParams[2];		//1: functionBreakpoint 2: best y
	float inheritOrderProb[2];		//1: dominance 2: !dominance
	int minCutHeight;				//the minimum height from base that is free of branches
	int minAge;
	float internodeLength; float internodeLengthTrunk;
	float branchProb;
	char leafMode;
};


struct BigPlant
{
	Branch branches[MAX_UNITS];
	ApexData ap[MAX_UNITS];
	BranchPointData bpd[MAX_UNITS];
	InternodeData internodes[MAX_INTERNODES];
	InternodeDroopData internodesDroop[MAX_INTERNODESDROOP];
	ApexDroopData apDroop[MAX_UNITSDROOP];
};

struct SmallPlant
{
	Branch branches[MAX_UNITS_SMALL];
	ApexData ap[MAX_UNITS_SMALL];
	BranchPointData bpd[MAX_UNITS_SMALL];
	InternodeData internodes[MAX_INTERNODES_SMALL];
	InternodeDroopData internodesDroop[MAX_INTERNODESDROOP];
	ApexDroopData apDroop[MAX_UNITSDROOP];
};

struct Plant
{
	Branch* branches;
	ApexData* ap;
	BranchPointData* bpd;
	InternodeData* internodes;
	InternodeDroopData* internodesDroop;
	ApexDroopData* apDroop;

	//Arrays* arraysBig;

	int freeIds[4096];
	int lastFreeId;
	int oldFreeId;

	int voxelPos[3];

	float branchGrowthSpeed;
	bool dominance;
	bool updateBranchGrowthSpeed;
	bool sapling;
	
	float radialGrowth;
	int MAX_Y, MAX_CHILD;

	
	ActiveParams params;

	int apId , interId , interDroopId, apDroopId ;
	
	void createPlant(InternodeData *root, ApexData *apex, BranchPointData *branchpoint, float startAngle, ActiveParams _params);

	int getBranchDistance(int branchId);

	void PropagateEnergyToChildren(float energy, int parentId);

	int generateId();

	int generateDroopId();

	int generateInternodeId();

	int generateInternodeDroopId();

	int createInternode(ApexPointer ap, bool place);

	int createInternodeDroop(ApexDroopPointer ap, bool place);

	void createBud(ApexPointer ap, BudData &bu);
};

struct World
{
	V3f grass[MAX_GRASS];
	Voxel voxels[VOXEL_DENSITY][VOXEL_DENSITY][VOXEL_DENSITY];
	float shadowThreshold;
	float shadowRedux;

	bool isActive;

	int seedCount;
	int grassCount;

	bool spawn;
	bool showLeaves;
	bool showVoxelBounds;
	bool randomSeed;
	bool hideThinBranches;
	bool seedGrass;

	char activeSpecies;
	char bigPlantsCount;
	char smallPlantsCount;

	void updateVoxels(int start_x, int start_y, int start_z, float strength);
};

struct SeedData
{
	V3tf position;
	
};

MouseStatus ms;

void queryLookupDirection(float x_head, float y_head, float z_head, float* x, float* y, float* z);

float photo(float light, float optimum, float maximum);



bool operator<(const Branch& a, const Branch& b)
{
	return a.directEnergy < b.directEnergy;
}

bool cmp(const Branch *a, const Branch *b)
{
	//Printf("TEST a %f b %f\n", a->directEnergy, b->directEnergy);
	//return a->directEnergy > b->directEnergy;
	return a->energyBudRatio > b->energyBudRatio;
}


Branch& Branch::operator=(const Branch& a)
{
	
	id = a.id;
	//lastChildId = a.lastChildId;
	directEnergy = a.directEnergy;
	energy = a.energy;
	order = a.order;
	energyBudRatio = a.energyBudRatio;
	
	//Printf("operat = id %i directEnergy %f\n", a.id, directEnergy);

	return *this;
}


module Internode(InternodePointer);
module Apex(ApexPointer);
module Bud(BudData);
module BranchPoint(BranchPointer);
module Temp(ApexPointer);
module BranchPointNext(BranchPointer);
module Seed(SeedData);
module Spawn();
module VoxelBounds();
module VoxelCut(bool);
module Grass();

module ApexDroop(ApexDroopPointer);
module InternodeDroop(InternodeDroopPointer);


InternodeData root[MAX_PLANTS + MAX_PLANTS_SMALL];
InternodePointer rootPointer[MAX_PLANTS + MAX_PLANTS_SMALL] ;//= &root, rootPointer1 = &root1, rootPointer2 = &root2;
ApexData apex[MAX_PLANTS + MAX_PLANTS_SMALL];//, apex1, apex2;
ApexPointer apexPointer[MAX_PLANTS + MAX_PLANTS_SMALL] ;//= &apex, apexPointer1 = &apex1, apexPointer2 = &apex2;
BranchPointData branchpoint[MAX_PLANTS + MAX_PLANTS_SMALL];//, branchpoint1, branchpoint2;
BranchPointer branchPointer[MAX_PLANTS + MAX_PLANTS_SMALL] ;//= &branchpoint, branchPointer1 = &branchpoint1, branchPointer2 = &branchpoint2;
SeedData seed[MAX_PLANTS + MAX_PLANTS_SMALL];

BigPlant bigPlants[MAX_PLANTS];
SmallPlant smallPlants[MAX_PLANTS_SMALL];


ActiveParams params[MAX_SPECIES];

int steps, derivationCounter;
float X=0.0f, Y=0.0f, Z=0.0f, DX=0.0f, DY=0.0f, DZ=0.0f, UX=0.0f, UY=0.0f, UZ=0.0f, LX=0.0f, LY=0.0f, LZ=0.0f,
h1=0.0f, h2=1.0f, h3=0.0f, u1=1.0f, u2=0.0f, u3=0.0f;
CameraPosition cameraPos;


World world;
Plant plant[MAX_PLANTS + MAX_PLANTS_SMALL];//, plant1[8];

/*long long rdtsc()
{
    __asm
    {
        RDTSC
    }
}*/
__declspec(naked)
unsigned __int64 __cdecl rdtsc(void)
{
   __asm
   {
      rdtsc
      ret       ;
   }
}


void Plant::createPlant(InternodeData *root, ApexData *apex, BranchPointData *branchpoint, float startAngle, ActiveParams _params)
{
	radialGrowth = 0.3f;
	MAX_Y, MAX_CHILD = 0;

	apId = 0;
	interId = 0;
	interDroopId = 0;
	apDroopId = 0;

	if( _params.leafMode == 0)
	{
		branches = &bigPlants[world.bigPlantsCount].branches[0];
		ap = &bigPlants[world.bigPlantsCount].ap[0];
		bpd = &bigPlants[world.bigPlantsCount].bpd[0];
		internodes = &bigPlants[world.bigPlantsCount].internodes[0];
		internodesDroop = &bigPlants[world.bigPlantsCount].internodesDroop[0];
		apDroop = &bigPlants[world.bigPlantsCount++].apDroop[0];
	}
	else
	{
		branches = &smallPlants[world.smallPlantsCount].branches[0];
		ap = &smallPlants[world.smallPlantsCount].ap[0];
		bpd = &smallPlants[world.smallPlantsCount].bpd[0];
		internodes = &smallPlants[world.smallPlantsCount].internodes[0];
		internodesDroop = &smallPlants[world.smallPlantsCount].internodesDroop[0];
		apDroop = &smallPlants[world.smallPlantsCount++].apDroop[0];
	}


	params.cutThreshold = _params.cutThreshold;
	params.plantDomThreshold[0] = _params.plantDomThreshold[0]; params.plantDomThreshold[1] = _params.plantDomThreshold[1];
	params.plantSaplingStrategy[0] = _params.plantSaplingStrategy[0]; params.plantSaplingStrategy[1] = _params.plantSaplingStrategy[1];
	params.plantVigorousStrategy[0] = _params.plantVigorousStrategy[0]; params.plantVigorousStrategy[1] = _params.plantVigorousStrategy[1];
	params.plantMatureStrategy[0] = _params.plantMatureStrategy[0];	params.plantMatureStrategy[1] = _params.plantMatureStrategy[1];
	params.plantMatureStrategy[2] = _params.plantMatureStrategy[2];	params.plantMatureStrategy[3] = _params.plantMatureStrategy[3];		//1: SP up (0,1,0) 2: SP down 3: SP: horizontal 4: up all
	params.apicalDomThreshold[0] = _params.apicalDomThreshold[0];	params.apicalDomThreshold[1] = _params.apicalDomThreshold[1];		//1: dominace 2: !dominance
	params.direction[0] = _params.direction[0];				//1: 
	params.distEnergyParams[0] = _params.distEnergyParams[0]; params.distEnergyParams[1] = _params.distEnergyParams[1];		//1: functionBreakpoint 2: best y
	params.inheritOrderProb[0] = _params.inheritOrderProb[0]; params.inheritOrderProb[1] = _params.inheritOrderProb[1];
	params.minCutHeight = _params.minCutHeight;
	params.minAge = _params.minAge;
	params.internodeLength = _params.internodeLength; params.internodeLengthTrunk = _params.internodeLengthTrunk;
	params.branchProb = _params.branchProb;
	params.leafMode = _params.leafMode;


	root->age = 1;
	root->iLength = 10.0f;
	root->iWidth = 0.9f;
	root->iRadialGrowth = radialGrowth;
	root->id=0;
	root->isLast = false;

	apex->age = 0;
	apex->budAngle = 138.5f;
	apex->lastBudAngle = startAngle;
	apex->order = 0;
	apex->lastBudId=0;
	apex->id=0;
	apex->num_childBuds=0;
	apex->lastChildNumber=0;
	//apex.growthSpeed = 1.5f;
	//apex.branchProb = 1.0f;
	//apex.indeterminate = true;
	apex->lastInternode = root;
	apex->apicalDominanceFactor = 0.0f;

	apex->plant = this;

	//branchpoint.age = 1;
	//branchpoint.iLength = root.iLength;
	//branchpoint.iWidth = root.iWidth;
	//branchpoint.iRadialGrowth = root.iRadialGrowth;
	//branchpoint.branchNumber=0;
	branchpoint->cut=false;
	branchpoint->branchApex = apex;
	branchpoint->branch = &branches[0];

	branches[0].id=0;
	branches[0].parentId = -1;
	branches[0].num_childBranches=0;
	branches[0].lastChildNumber=0;
	branches[0].branchApex = apex;
	branches[0].energy = 0.0f;
	dominance = true;
	sapling = true;
	updateBranchGrowthSpeed = false;
	branchGrowthSpeed = 0.75f;
	branches[0].radialWidth = root->iWidth;
	branches[0].vigor = 2.0f;
	branches[0].oldVigor = 0.0f;
	branches[0].vigorCap = 3.4f;
	branches[0].rank = 2.0f;
	branches[0].childBranches[0] = &branches[0];
	branches[0].lastChild = 1;
	branches[0].branchPoint = branchpoint;
	branches[0].apicalDominanceFactor = params.plantDomThreshold[0];

	branches[0].leafDownAngle = ran(36.0f);

	branches[0].plant = this;
	
	world.spawn = true;

	root->branch = &branches[0];

	
}

void blackOutCube(V3f point1, float lengthX, float lengthZ, float lengthY)
{
		V3f point2(point1), point3(point1), point4(point1);
		int voxel1[3], voxel2[3], voxel3[3], voxel4[3], x, y, z;

		point2.x = point1.x + lengthX;
		point3.z = point1.z + lengthZ;
		point4.y = point1.y - lengthY;


		queryVoxel(point1.x, point1.y, point1.z, &voxel1[0], &voxel1[1], &voxel1[2]);

		queryVoxel(point2.x, point2.y, point2.z, &voxel2[0], &voxel2[1], &voxel2[2]);

		queryVoxel(point3.x, point3.y, point3.z, &voxel3[0], &voxel3[1], &voxel3[2]);

		queryVoxel(point4.x, point4.y, point4.z, &voxel4[0], &voxel4[1], &voxel4[2]);

		for(y = voxel1[1]; y > voxel4[1]; y--)
			for(x = voxel1[0]; x < voxel2[0]; x++)
				for(z = voxel1[2]; z < voxel3[2]; z++)
					world.voxels[x][y][z].light = 10000.0f;


}

/*void blackOutTrapez(V3f point1, float lengthX, float lengthY)
{
	V3f point2(point1), point3(point1);
	int voxel1[3], voxel2[3]], voxel3[3];

	point2.x = point1.x + lengthX;
	point3.z = point1.z + lengthY;

	queryVoxel(point1.x, point1.y, point1.z, &voxel1[0], &voxel1[1], &voxel1[2]);

	queryVoxel(point2.x, point2.y, point2.z, &voxel2[0], &voxel2[1], &voxel2[2]);

	queryVoxel(point3.x, point3.y, point3.z, &voxel3[0], &voxel3[1], &voxel3[2]);

}*/

void makeShadowAreaHouse(V3f house1, V3f house2, float depth, float strength/*, V3f house3, V3f house4*/)
{
	int i,j,maxLength, voxelX, voxelY, voxelZ, voxel2[3], d,Z;
	float x,y,z, dx, dy, dz;

	dx = x = house1.x;
	dy = y = house1.y;
	dz = z = house1.z;

	//maxLength = house1.z - house2.z;
	

	queryVoxel(x, y, z, &voxelX, &voxelY, &voxelZ);
	Printf("begin P1 %i %i %i real %f %f %f \n", voxelX, voxelY, voxelZ, x, y, z);

	queryVoxel(house2.x, house2.y, house2.z, &voxel2[0], &voxel2[1], &voxel2[2]);
	Printf("begin P2 %i %i %i real %f %f %f \n", voxel2[0], voxel2[1], voxel2[2], house2.x, house2.y, house2.z);
	Z = voxel2[2];

	maxLength = Abs(voxelZ - voxel2[2]);
	Printf("maxLength %i \n", maxLength);

	dx += depth;

	queryVoxel(dx, dy, dz, &voxel2[0], &voxel2[1], &voxel2[2]);
	Printf("voxel2 0 %i \n", voxel2[0]);

	if(voxelX < voxel2[0]){d = voxelX; voxelX = voxel2[0]; voxel2[0] = d;}
	//while(Abs(depth) > 5.0f)
	//{
	for(j = voxelX; j > voxel2[0]; j--){
		for(i = voxelZ; i < Z; i++)
		{
			//voxelZ += 1;
			
			world.updateVoxels(voxelX, voxelY, i, strength);
			Printf("update %i %i %i\n", voxelX, voxelY, voxelZ);
		}
		/*if(depth > 0.0f)
		{
			depth -= 20.0f;
			voxelX -= 1;
		}
		else 
		{
			depth += 20.0f;
			voxelX += 1;
		}*/
		voxelX -= 1;
	}
	Printf("end %i %i %i\n", voxelX, voxelY, voxelZ);
	//}
}

float houseX = -50.0f/*700.0f*/, houseY = 500.0f, houseZ = 370.0f;
float house2X = 470.0f, house2Y = 800.0f, house2Z = 370.0f;
float house3X = -300.0f, house3Y = 800.0f, house3Z = 370.0f;
float house4X = 470.0f, house4Y = 840.0f, house4Z = 100.0f;
float house5X = -150.0f, house5Y = 880.0f, house5Z = -300.0f;

Start:
{	
	int i,j,k;
	float x_head = 0.3f, y_head=-0.3f, z_head=-0.8f; 
	float x,y,z;
	double temp, temp1;

	Printf("Start\n");
	//Backward();
	sran(1);

	world.shadowThreshold = 0.25f;
	world.shadowRedux = 1.4f;

	//Printf("clock cycles %f \n", temp=(double)rdtsc());

	//Printf("radius %f \n", calcNextRadius(0.1f, 2.0f * PI * 0.01f + 0.01f));
	//Printf("clock cycles %f \n", temp1=(double)rdtsc()-temp);
	for(i = 0; i<VOXEL_DENSITY; i++)
		for(j=1; j<VOXEL_DENSITY; j++)
			for(k=0; k<VOXEL_DENSITY; k++)
				world.voxels[i][j][k].light = ((float)(VOXEL_DENSITY-j))*0.075f + ran(0.075f);
	
	for(i = 0; i<VOXEL_DENSITY; i++)
		for(k=0; k<VOXEL_DENSITY; k++)
			world.voxels[i][0][k].light=100000.0f;

	for(i = 0; i<VOXEL_DENSITY; i++)
		for(k=0; k<VOXEL_DENSITY; k++)
			world.voxels[i][VOXEL_DENSITY-1][k].light=100000.0f;

	V3f house1(houseX ,houseY, -houseZ), house2(houseX ,houseY, houseZ);
	/*makeShadowAreaHouse(house1, house2, 200.0f, 16.0f);

	house1.x = house2X; house1.y = house2Y; house1.z = -house2Z;
	house2.x = house2X; house2.y = house2Y; house2.z = house2Z;
	makeShadowAreaHouse(house1, house2, -370.0f, 20.0f);

	//house1.x = house3X; house1.y = house3Y; house1.z = -house3Z;
	//house2.x = house3X; house2.y = house3Y; house2.z = house3Z;
	//makeShadowAreaHouse(house1, house2, -300.0f, 19.0f);

	world.shadowThreshold = 0.25f;
	world.shadowRedux = 1.1f;

	house1.x = house3X ; house1.y = house3Y; house1.z = -house3Z;
	house2.x = house3X ; house2.y = house3Y; house2.z = house3Z;
	//makeShadowAreaHouse(house1, house2, -20.0f, 2.0f);

	world.shadowThreshold = 0.25f;
	world.shadowRedux = 1.4f;

	blackOutCube(house1, 770.0f, 740.0f, 727.0f);

	

	world.shadowThreshold = 1000.25f;
	world.shadowRedux = 1.26f;

	house1.x = house4X; house1.y = house4Y; house1.z = -house4Z;
	house2.x = house4X; house2.y = house4Y; house2.z = house4Z;
	//makeShadowAreaHouse(house1, house2, -770.0f, 10018.0f);

	
	world.shadowThreshold = 0.25f;
	world.shadowRedux = 1.4f;


	house1.x = house5X; house1.y = house5Y; house1.z = house5Z;
	//blackOutCube(house1, 960.0f, 900.0f, 500.0f);

	Printf("voxel %f \n", world.voxels[98][66][98].light);
*/
	world.shadowThreshold = 0.25f;
	world.shadowRedux = 2.0f;

	//Printf("clock cycles %f \n", temp=(double)rdtsc()-temp);

	
	
	world.spawn = false;
	world.showLeaves = false;
	world.showVoxelBounds = false;
	world.randomSeed = true;
	world.isActive = true;
	world.hideThinBranches = false;
	world.seedGrass = false;

	world.activeSpecies = 0;
	world.grassCount = 0;
	world.bigPlantsCount = 0;
	world.smallPlantsCount = 0;

	UserMenuItem("Seed",0);
	UserMenuItem("Show leaves",1);
	UserMenuItem("Show voxel bounds",2);
	UserMenuItem("Random seed",3);
	UserMenuItem("Hide thin branches",4);
	UserMenuItem("Species one",5);
	UserMenuItem("Species two",6);
	UserMenuItem("Species three",7);
	UserMenuItem("Grass",8);

	for(i = 0; i< MAX_PLANTS + MAX_PLANTS_SMALL; i++)
	 {
		rootPointer[i] = &root[i];
		apexPointer[i] = &apex[i];
		branchPointer[i] = &branchpoint[i];
	 }
	world.seedCount = 0;



	steps = 0;
	derivationCounter = 0;

	//for(int i = 0; i< MAX_SPECIES; i++)
	//{
		params[0].cutThreshold = 0.55f;
		params[0].plantDomThreshold[0] = 25.0f; params[0].plantDomThreshold[1] = 24.0f;		//1: dominance 2: sapling
		/*params[0].plantSaplingStrategy[0] = 0.25f; params[0].plantSaplingStrategy[1] = -3.0f;
		params[0].plantVigorousStrategy[0] = 0.15f; params[0].plantVigorousStrategy[1] = 2.5f;
		params[0].plantMatureStrategy[0] = 0.15f;	params[0].plantMatureStrategy[1] = 4.5f;
		params[0].plantMatureStrategy[2] = 0.0;	params[0].plantMatureStrategy[3] = 0.0f;		//1: SP up (0,1,0) 2: SP down 3: SP: horizontal 4: up all
		*/params[0].plantSaplingStrategy[0] = 0.25f; params[0].plantSaplingStrategy[1] = 0.0f;
		params[0].plantVigorousStrategy[0] = 0.0f;//params[0].plantSaplingStrategy[0] ;
		params[0].plantVigorousStrategy[1] = 2.25f  ;
		params[0].plantMatureStrategy[0] = 0.0f;//params[0].plantSaplingStrategy[0] ;
		params[0].plantMatureStrategy[1] = 2.25f ;
		params[0].plantMatureStrategy[2] = 0.0f;	params[0].plantMatureStrategy[3] = 0.0f;//*/
		params[0].apicalDomThreshold[0] = 7.0f;	params[0].apicalDomThreshold[1] = 16.0f;		//1: dominace 2: !dominance
		params[0].direction[0] = 36.0f;				//1: 
		params[0].distEnergyParams[0] = 0.5f; params[0].distEnergyParams[1] = 2.0f;		//1: functionBreakpoint 2: best y
		params[0].inheritOrderProb[0] = 1.0f; params[0].inheritOrderProb[1] = 1.0f;
		params[0].minCutHeight = 7;
		params[0].minAge = 7;
		params[0].internodeLength = 2.4f; params[0].internodeLengthTrunk = 2.5f;
		params[0].branchProb = 0.37f;
		params[0].leafMode = 0;

		params[1].cutThreshold = 0.52f;
		params[1].plantDomThreshold[0] = 30.0f; params[1].plantDomThreshold[1] = 5.0f;		//1: dominance 2: sapling
		params[1].plantSaplingStrategy[0] = 1.25f; params[1].plantSaplingStrategy[1] = 0.0f;
		params[1].plantVigorousStrategy[0] = 1.5f;//params[0].plantSaplingStrategy[0] ;
		params[1].plantVigorousStrategy[1] = 0.5  ;
		params[1].plantMatureStrategy[0] = 0.25f;//params[0].plantSaplingStrategy[0] ;
		params[1].plantMatureStrategy[1] = 6.21f ;
		params[1].plantMatureStrategy[2] = 0.0f;	params[1].plantMatureStrategy[3] = 0.0f;//*/
		params[1].apicalDomThreshold[0] = 16.0f;	params[1].apicalDomThreshold[1] = 10.0f;		//1: dominace 2: !dominance
		params[1].direction[0] = 36.0f;				//1: 
		params[1].distEnergyParams[0] = 0.5f; params[1].distEnergyParams[1] = 2.0f;		//1: functionBreakpoint 2: best y
		params[1].inheritOrderProb[0] = 1.0f; params[1].inheritOrderProb[1] = 1.0f;
		params[1].minCutHeight = 14;
		params[1].minAge = 14;
		params[1].internodeLength = 2.6f; params[1].internodeLengthTrunk = 11.5f;
		params[1].branchProb = 0.0f;
		params[1].leafMode = 0;
		/*params[1].cutThreshold = 0.3f;
		params[1].plantDomThreshold[0] = 7.0f; params[1].plantDomThreshold[1] = 7.0f;		//1: dominance 2: sapling
		params[1].plantSaplingStrategy[0] = 0.25f; params[1].plantSaplingStrategy[1] = -3.0f;
		params[1].plantVigorousStrategy[0] = 1.15f; params[1].plantVigorousStrategy[1] = -2.5f;
		params[1].plantMatureStrategy[0] = 0.15f;	params[1].plantMatureStrategy[1] = -20.5f;
		params[1].plantMatureStrategy[2] = -2.5f;	params[1].plantMatureStrategy[3] = -5.0f;		//1: SP up (0,1,0) 2: SP down 3: SP: horizontal 4: up all
		params[1].apicalDomThreshold[0] = 12.0f;	params[1].apicalDomThreshold[1] = 7.0f;		//1: dominace 2: !dominance
		params[1].direction[0] = 36.0f;				//1: 
		params[1].distEnergyParams[0] = 0.5f; params[1].distEnergyParams[1] = 2.0f;		//1: functionBreakpoint 2: best y
		params[1].inheritOrderProb[0] = 1.0f; params[1].inheritOrderProb[1] = 1.0f;
		params[1].minCutHeight = 0;
		params[1].minAge = 0;
		params[1].internodeLength = 0.75f; params[1].internodeLengthTrunk = 5.0f;
		params[1].branchProb = 0.25f;
		params[1].leafMode = 1;*/

		params[2].cutThreshold = 0.5f;
		params[2].plantDomThreshold[0] = 7.0f; params[2].plantDomThreshold[1] = 7.0f;		//1: dominance 2: sapling
		params[2].plantSaplingStrategy[0] = 0.25f; params[2].plantSaplingStrategy[1] = -3.0f;
		params[2].plantVigorousStrategy[0] = 100.15f; params[2].plantVigorousStrategy[1] = -2.5f;
		params[2].plantMatureStrategy[0] = 10.15f;	params[2].plantMatureStrategy[1] = -20.5f;
		params[2].plantMatureStrategy[2] = -2.5f;	params[2].plantMatureStrategy[3] = -5.0f;		//1: SP up (0,1,0) 2: SP down 3: SP: horizontal 4: up all
		params[2].apicalDomThreshold[0] = 7.0f;	params[2].apicalDomThreshold[1] = 7.0f;		//1: dominace 2: !dominance
		params[2].direction[0] = 36.0f;				//1: 
		params[2].distEnergyParams[0] = 0.5f; params[2].distEnergyParams[1] = 2.0f;		//1: functionBreakpoint 2: best y
		params[2].inheritOrderProb[0] = 1.0f; params[2].inheritOrderProb[1] = 1.0f;
		params[2].minCutHeight = 0;
		params[2].minAge = 0;
		params[2].internodeLength = 0.7f; params[2].internodeLengthTrunk = 10.0f;
		params[2].branchProb = 0.9f;
		params[2].leafMode = 2;
	//}

	//plant[0].createPlant(&root[0], &apex[0], &branchpoint[0], 0.0f);

	//plant[1].createPlant(&root1, &apex1, &branchpoint1);

	//plant[2].createPlant(&root2, &apex2, &branchpoint2);


}

int Plant::getBranchDistance(int branchId)
{
	if(!branchId) return 0;

	//Printf("branch with id %i parent id %i has childs %i \n", 
	//	branchId, plant.branches[branchId].parentApex->id, plant.branches[branchId].parentApex->num_childBuds);
	


	//if(plant.branches[branchId].parentApex->order > 0)
	//return plant.branches[plant.branches[branchId].parentId].num_childBranches - plant.branches[branchId].childNumber;
	return branches[branchId].parentApex->num_childBuds - branches[branchId].childNumber;

	//else
	//	return branches[0].num_childBranches - branches[branchId].childNumber;
}


bool buttonDown, seedMode = true;
V3f mousePos(0,0,0), seedPos(0,0,0);

void seedPlant(V3f position, ActiveParams params)
{
	if(params.leafMode > 0)position.y -= 70.0f;
	else position.y -= 30.0f;

	queryVoxel(position.x, position.y, position.z, &plant[world.seedCount].voxelPos[0], &plant[world.seedCount].voxelPos[1], &plant[world.seedCount].voxelPos[2]);

	

	plant[world.seedCount].createPlant(&root[world.seedCount], &apex[world.seedCount], &branchpoint[world.seedCount], 138.5f * (float)world.seedCount, params);
	seed[world.seedCount].position = position;
}


StartEach:
{
	int i,j,k,p,start, temp, MAX_i=1, MAX_dist, it, startParent;
	//bool success = false;
	//Printf("start each \n");
	int menuChoice = UserMenuChoice();

	if(menuChoice == 0) seedMode = !seedMode;
	if(menuChoice == 1) world.showLeaves = !world.showLeaves;
	if(menuChoice == 2) world.showVoxelBounds = !world.showVoxelBounds;
	if(menuChoice == 3) world.randomSeed = !world.randomSeed;
	if(menuChoice == 4) world.hideThinBranches = !world.hideThinBranches;
	if(menuChoice == 5) world.activeSpecies = 0;
	if(menuChoice == 6) world.activeSpecies = 1;
	if(menuChoice == 7) world.activeSpecies = 2;
	if(menuChoice == 8) world.seedGrass = !world.seedGrass;

	if(world.seedGrass)
	{
		V3f position(0,0,0), seedPos(0,0,0);
		//world.seedGrass = false;

		
		
		for(i=0; i< MAX_GRASS; i++){
			position.x = ran(2.0f*1920.0f) - 1920.0f;
			position.y = 2.0f*1920.0f;
			position.z = ran(2.0f*1920.0f) - 1920.0f;

			if(terrainHeightAt(position, seedPos))
				if(/*seedPos.y < 160.0f&&*/ seedPos.y > 6.0f)world.grass[world.grassCount++] = seedPos;
		}
	}
	if(!world.isActive)
	{
		//int menuChoice = UserMenuChoice();
		

		ms = GetMouseStatus();
		buttonDown=ms.lbDown;


		mousePos = (V3f)ms.atMiddle;
 
		if(buttonDown && seedMode){
			
			if(terrainHeightAt(mousePos, seedPos))seedPlant(seedPos, params[world.activeSpecies]);
			//Printf("pos x %f y %f z %f, intersects: %i ", mousePos.x, mousePos.y, mousePos.z, terrainHeightAt(mousePos, seedPos));
			Printf(" seed Pos : x %f, y %f z %f\n", seedPos.x, seedPos.y, seedPos.z);

			//Sleep(500);
		}

		Forward();
		UseGroup(2);

		return;
	}


	if(!(steps%2)) 
	{
		UseGroup(1); 
		Backward();
		Printf("use group: 1\n\n");
	}
	else { UseGroup(0); Forward();

	Printf("use group: 0\n \n");

	//plant.cutThreshold = 0.43f;

	for(p = 0 ; p < world.seedCount; p++)
	if(plant[p].branches[0].branchApex!=NULL)
		plant[p].branches[0].energyBudRatio = plant[p].branches[0].directEnergy = plant[p].branches[0].energy = photo(plant[p].branches[0].branchApex->light, 2.5f, 2*50.0f);
	//Printf("Debug\n ");
	for(p = 0 ; p < world.seedCount; p++){

		float MAX = 10000.0f;
	float energy = 0.0f, q=0.0f;
	//Printf("Debug loop\n ");
	for(k=8; k>0; k--){

		//if((steps%2))break;

		start=0;
		startParent=0;
		//memset(startChild,0, 8192*4);
		MAX_i = 1;
		MAX_dist = 0;

		for(i=plant[p].apId; i>=0/*plant[p].branches[0].branchApex->num_childBuds && i<10*/; i--)
		{
			//if(i> MAX_UNITS/2 && plant[p].branches[i].branchApex!=NULL)Printf(" apex %i of order %i\n", i, plant[p].branches[i].order);
			//if(plant[p].branches[i].branchApex!=NULL /*&& plant[p].branches[i].branchApex->order == k*/)Printf("child %i k %i plant %i order %i\n", i,k, p, plant[p].branches[i].branchApex->order);
			if(plant[p].branches[i].branchApex!=NULL && plant[p].branches[i].branchApex->order == k){
				//Printf("Hallo\n");
				//world.updateVoxels( plant[p].branches[i].branchApex->VoxelPosition[0], plant[p].branches[i].branchApex->VoxelPosition[1],
				//	plant[p].branches[i].branchApex->VoxelPosition[2], 8.0f);

				plant[p].branches[i].id = i;

				if(plant[p].updateBranchGrowthSpeed)  {
					/*plant[p].branches[i].branchApex->growthSpeed = 1.0f;
					if(plant[p].branches[i].branchApex->order > 1)plant[p].branches[i].branchApex->branchProb = 0.5f;
					else plant[p].branches[i].branchApex->branchProb = 0.75f;*/
				}

				plant[p].branches[i].directEnergy = photo(plant[p].branches[i].branchApex->light, 2.5f, 50.0f) -
					plant[p].branches[i].parentApex->apicalDominanceFactor;
				
				plant[p].branches[i].energy += plant[p].branches[i].directEnergy;

				

				//plant[p].branches[i].width = (float)(id.age)/id.iRadialGrowth + id.iWidth;
				if(plant[p].branches[i].VoxelPosition[1] > plant[p].MAX_Y) plant[p].MAX_Y = plant[p].branches[i].VoxelPosition[1];

				j=i;
				//while(j){
					
					plant[p].branches[j=plant[p].branches[i].parentApex->id].energy += plant[p].branches[i].energy;
					
					/*if(plant[p].branches[i].branchApex->age == 1){
						while(j){Printf("j %i \n", j);
							plant[p].branches[j=plant[p].branches[j].parentApex->id].children++;
						}}*/
					
				//}
					plant[p].branches[i].energyBudRatio = plant[p].branches[i].energy / Max(plant[p].branches[i].num_childBranches + 1 , 1);
					
					//if(i> MAX_UNITS/2)Printf(" apex %i has enerhyBudRatio %f \n", i, plant[p].branches[i].energyBudRatio);
				//Printf("apex %i with order %i has light %f and energy %f, q: %f, pY: %i \n", i, plant[p].branches[i].branchApex->order,
				//		plant[p].branches[i].branchApex->light, plant[p].branches[i].energy, 
				//		plant[p].branches[i].energyBudRatio,
				//		plant[p].branches[i].branchApex->VoxelPosition[1]);
				if(plant[p].branches[i].lastChild > plant[p].MAX_CHILD) plant[p].MAX_CHILD = plant[p].branches[i].lastChild;
					
				if(plant[p].getBranchDistance(plant[p].branches[i].branchApex->id) > MAX_dist) {
					MAX_i = i; MAX_dist = plant[p].getBranchDistance(plant[p].branches[i].branchApex->id);
				}
				

			}
			//else Printf("no branchApex %i\n", i);
		}

	//Printf("plant has energy : %f \n", plant[p].branches[0].energy);
	//Printf("plant has children : %i \n\n", plant[p].branches[0].num_childBranches);

	//plant[p].updateBranchGrowthSpeed = false;
	}

	}
	}
	if(world.isActive){
	
	for(p = 0 ; p < world.seedCount; p++)
	plant[p].branches[0].parentVigor = plant[p].branches[0].energy;//plant[p].branches[0].num_childBranches + 1;
	Printf("Hallo\n");
	}
	
	
}

EndEach:
{
	
	int i,p;

	if(world.isActive){
	for(p = 0 ; p < world.seedCount; p++){

		//Printf("total number of apeces %i , total number of internodes %i, droop apices %i droop inter %i\n", plant[p].apId, plant[p].interId,
	Printf("total number of apeces %i , total number of internodes %i, species %i\n", plant[p].apId, plant[p].interId, plant[p].params.leafMode,
		plant[p].apDroopId, plant[p].interDroopId);

	plant[p].oldFreeId = plant[p].lastFreeId;
	if(IsForward()){
	for(i=plant[p].apId; i>=0/*plant[p].branches[0].branchApex->num_childBuds && i<10*/; i--)
		{
			if(plant[p].branches[i].branchApex != NULL && plant[p].branches[i].energy > 0.1f) {
				//Printf("Enegry not zeroed \n");
				plant[p].branches[i].energy = 0.0f;
			}
	}}

	Printf("End each steps %i and MAX_Y %i, MAX_CHILD %i, of plant %i\n", steps, plant[p].MAX_Y, plant[p].MAX_CHILD, p);
	
	}
	steps++;
	}
}

void Plant::PropagateEnergyToChildren(float energy, int parentId)
{
	int i;
	float totalEnergy = 0.0f;
	//Printf("Enter\n");
	if(branches[parentId].branchApex != NULL){
	if(/*branches[parentId].branchApex->age < 7*/ branches[parentId].apicalDominanceFactor > 0.1f)
	sort(branches[parentId].childBranches+1, branches[parentId].childBranches + (branches[parentId].lastChild-1), cmp);
	else
		sort(branches[parentId].childBranches, branches[parentId].childBranches + (branches[parentId].lastChild-1), cmp);
	}
	/*for(i = 0; i < branches[parentId].lastChild; i++)
	{
		Printf("sort  id %i of order %i has energyratio %f\n", branches[parentId].childBranches[i]->id,
			branches[parentId].childBranches[i]->order, branches[parentId].childBranches[i]->energyBudRatio);
	}*/

	//Printf("Parent %i order %i checks %i \n", parentId, branches[parentId].order, branches[parentId].lastChild);
	
	//branches[parentId].oldVigor -= 0.7f;

	for(i = 0; i < branches[parentId].lastChild; i++)
	{

		if(branches[parentId].childBranches[i]->branchApex == NULL) continue;

		if(//branches[parentId].childBranches[i]->branchApex->age > 5 && ran(1.0f) >0.95f &&
			i==0 && branches[parentId].order > 0 && !sapling//!dominance //&& branches[parentId].apicalDominanceFactor < 0.1f 
			) 
		{
			branches[parentId].childBranches[i]->branchApex->order = branches[parentId].order;
			branches[parentId].childBranches[i]->order = branches[parentId].order;
			//branches[parentId].childBranches[i]->vigorCap = branches[parentId].vigorCap;
			//branches[parentId].childBranches[i]->childNumber = branches[parentId].childNumber;
			//branches[parentId].apicalDominanceFactor = 7.0f;
			//branches[parentId].childBranches[i]->apicalDominanceFactor = 7.0f;
		}

		/*if(sapling)branches[parentId].childBranches[i]->rank = distributeEnergyGeom(i, 0.5f,
			branches[parentId].lastChild, 5.0f);
		else*/ branches[parentId].childBranches[i]->rank = distributeEnergyGeom(i, 0.5f,
			branches[parentId].lastChild, 2.0f);

		if(branches[parentId].childBranches[i]->id != parentId){
			branches[parentId].childBranches[i]->vigor = branches[parentId].childBranches[i]->rank *
			(float)(branches[parentId].childBranches[i]->num_childBranches+1);
			
			totalEnergy += branches[parentId].childBranches[i]->vigor;

			/*Printf("id %i of order %i has rank %f params %i %i \n", branches[parentId].childBranches[i]->id,
				branches[parentId].childBranches[i]->order,
				branches[parentId].childBranches[i]->rank , i,
				branches[parentId].lastChild);
		*/}
		else{
			branches[parentId].childBranches[i]->vigor = branches[parentId].childBranches[i]->rank;
			
			totalEnergy += branches[parentId].childBranches[i]->vigor;
		}
	}
	//Printf("total energy %f vs. number of apeces %f \n", totalEnergy, branches[parentId].parentVigor);

	for(i = 0; i < branches[parentId].lastChild; i++)
	{
		if(branches[parentId].childBranches[i]->branchApex == NULL) continue;

		if(branches[parentId].childBranches[i]->id != parentId){
			branches[parentId].childBranches[i]->parentVigor = branches[parentId].childBranches[i]->vigor / totalEnergy * 
			branches[parentId].parentVigor;	
			//Printf("vigor of id %i is %f\n", branches[parentId].childBranches[i]->id, branches[parentId].childBranches[i]->parentVigor);
		}
		else {
			if(branches[parentId].childBranches[i]->rank > 0.88f)
			branches[parentId].childBranches[i]->oldVigor += branches[parentId].childBranches[i]->vigor / totalEnergy * 
			branches[parentId].parentVigor;
			
			else if(branches[parentId].childBranches[i]->rank < 0.7f)
				branches[parentId].childBranches[i]->oldVigor = 0.0f;
			
			if(branches[parentId].childBranches[i]->oldVigor > branches[parentId].childBranches[i]->vigorCap) 
				branches[parentId].childBranches[i]->oldVigor = branches[parentId].childBranches[i]->vigorCap;

			else if(branches[parentId].childBranches[i]->oldVigor < 0.0f)
				branches[parentId].childBranches[i]->oldVigor = 0.0f;

			//Printf("vigor of id %i is %f final\n", branches[parentId].childBranches[i]->id, branches[parentId].childBranches[i]->oldVigor);
		}
	}
	
	//Printf("\n");
}

float photo(float light, float optimum, float maximum)
{
	if(light<optimum) return 1/optimum * light;
	else if( light <= maximum) return 1/(optimum-maximum)*(light - maximum);
	else return 0.0f;
}

/*bool createInternode(ApexPointer ap)
{
	ap->light;
}*/



void World::updateVoxels(int start_x, int start_y, int start_z, float strength)
{
	int posX_posZ[3], negX_negZ[3], negX_posZ[3], posX_negZ[3];

	posX_posZ[0]=negX_negZ[0]=negX_posZ[0]=posX_negZ[0]=start_x;
	posX_posZ[1]=negX_negZ[1]=negX_posZ[1]=posX_negZ[1]=start_y;
	posX_posZ[2]=negX_negZ[2]=negX_posZ[2]=posX_negZ[2]=start_z;
	
	float shadow = strength * shadowRedux;
	//Printf("strength %f \n", shadow);
	
	int x=start_x, y=start_y, z=start_z, i=0, j=0, k=0, depth=0;

	//Printf("START voxel: %i %i %i has value %f\n", x,y,z,world.voxels[x][y][z].light);

	//world.voxels[x][y-i+1][z].light = world.voxels[x][y-i+1][z].light + shadow/shadowRedux/shadowRedux/shadowRedux/shadowRedux;

	while(Abs(shadow) > Abs(shadowThreshold) && (y-i>=0))
	{
		//if(y-i > VOXEL_DENSITY -1) continue;
		world.voxels[x][y-i][z].light = world.voxels[x][y-i][z].light + shadow;
		//Printf("voxel: %i %i %i has value %f\n", x,y-i,z,world.voxels[x][y-i][z].light);

		shadow = shadow / shadowRedux;
		i++;
	}
	strength = strength / shadowRedux /shadowRedux;
	shadow = strength;

	depth=i-2;

	//Printf("depth %i \n", depth);

	posX_posZ[0] = posX_posZ[0]; posX_posZ[1] = posX_posZ[1]+1; posX_posZ[2] = posX_posZ[2];
	negX_posZ[0] = negX_posZ[0]; negX_posZ[1] = negX_posZ[1]+1; negX_posZ[2] = negX_posZ[2];
	posX_negZ[0] = posX_negZ[0]; posX_negZ[1] = posX_negZ[1]+1; posX_negZ[2] = posX_negZ[2];
	negX_negZ[0] = negX_negZ[0]; negX_negZ[1] = negX_negZ[1]+1; negX_negZ[2] = negX_negZ[2];
	
	/*for(k=1; k<depth; k++)
	{
		posX_posZ[0] = posX_posZ[0]; posX_posZ[1] = posX_posZ[1]+1; posX_posZ[2] = posX_posZ[2];
		negX_posZ[0] = negX_posZ[0]; negX_posZ[1] = negX_posZ[1]+1; negX_posZ[2] = negX_posZ[2];
		posX_negZ[0] = posX_negZ[0]; posX_negZ[1] = posX_negZ[1]+1; posX_negZ[2] = posX_negZ[2];
		negX_negZ[0] = negX_negZ[0]; negX_negZ[1] = negX_negZ[1]+1; negX_negZ[2] = negX_negZ[2];

	}*/

	for(k=1; k<=depth; k++)
	{
	//update four corners below our last voxels
	posX_posZ[0] = posX_posZ[0]+1; posX_posZ[1] = posX_posZ[1]-1; posX_posZ[2] = posX_posZ[2]+1;
	negX_posZ[0] = negX_posZ[0]-1; negX_posZ[1] = negX_posZ[1]-1; negX_posZ[2] = negX_posZ[2]+1;
	posX_negZ[0] = posX_negZ[0]+1; posX_negZ[1] = posX_negZ[1]-1; posX_negZ[2] = posX_negZ[2]-1;
	negX_negZ[0] = negX_negZ[0]-1; negX_negZ[1] = negX_negZ[1]-1; negX_negZ[2] = negX_negZ[2]-1;

	//if(k==2) strength = strength / shadowRedux/shadowRedux;
	//else strength = strength * shadowRedux;

	for(i=negX_negZ[0]; i<posX_negZ[0]; i++)
	{
		if(i < 0 ) continue;
		if(i > VOXEL_DENSITY - 1 || (unsigned int)negX_negZ[2] > VOXEL_DENSITY -1) break;

		for(j=0; j<depth; j++)
		{
			if(negX_negZ[1]-j < 0) break;
			if(negX_negZ[1]-j > VOXEL_DENSITY - 1) continue;
			world.voxels[i][negX_negZ[1]-j][negX_negZ[2]].light += shadow;
			//Printf("voxel %i %i %i \n", i, negX_negZ[1]-j, negX_negZ[2]);

			shadow /= shadowRedux;
		}
		shadow = strength;
	}
	for(i=posX_negZ[2]; i<posX_posZ[2]; i++)
	{
		if(i < 0 ) continue;
		if(i > VOXEL_DENSITY - 1 || (unsigned int)posX_negZ[0] > VOXEL_DENSITY -1) break;

		for(j=0; j<depth; j++)
		{
			if(posX_negZ[1]-j < 0) break;
			if(posX_negZ[1]-j > VOXEL_DENSITY - 1) continue;
			world.voxels[posX_negZ[0]][posX_negZ[1]-j][i].light += shadow;
			//Printf("voxel %i %i %i \n", posX_negZ[0], posX_negZ[1]-j, i);

			shadow /= shadowRedux;
		}
		shadow = strength;
	}
	for(i=posX_posZ[0]; i>negX_posZ[0]; i--)
	{
		if(i < 0 || (unsigned int)posX_posZ[2] > VOXEL_DENSITY -1) break;
		if(i > VOXEL_DENSITY - 1) continue;

		for(j=0; j<depth; j++)
		{
			if(posX_posZ[1]-j < 0) break;
			if(posX_posZ[1]-j > VOXEL_DENSITY - 1) continue;
			world.voxels[i][posX_posZ[1]-j][posX_posZ[2]].light += shadow;
			//Printf("voxel %i %i %i \n", i, posX_posZ[1]-j, posX_posZ[2]);

			shadow /= shadowRedux;
		}
		shadow = strength;
	}
	for(i=negX_posZ[2]; i>negX_negZ[2]; i--)
	{
		if(i < 0 || (unsigned int)negX_posZ[0] > VOXEL_DENSITY -1) break;
		if(i > VOXEL_DENSITY - 1) continue;

		for(j=0; j<depth; j++)
		{
			if(negX_posZ[1]-j < 0) break;
			if(negX_posZ[1]-j > VOXEL_DENSITY - 1) continue;
			world.voxels[negX_posZ[0]][negX_posZ[1]-j][i].light += shadow;
			//Printf("voxel %i %i %i \n", negX_posZ[0],negX_posZ[1]-j, i);

			shadow /= shadowRedux;
		}
		shadow = strength;
	}
	
	}

}

void queryChildBranchFitness()
{
	
}

void queryVoxel(float x, float y, float z, /*results->*/ int* a, int* b, int* c)
{
	*a=(int)(x/20.0f) + VOXEL_DENSITY/2;
	*b=(int)((y + 660.0f)/20.0f );
	*c=(int)z/20.0f + VOXEL_DENSITY/2;

	if((unsigned int)*a > VOXEL_DENSITY-1 || (unsigned int)*b > VOXEL_DENSITY-1 || (unsigned int)*c > VOXEL_DENSITY-1) 
	{
		Printf("Boundary overwrite %i %i %i!\n", *a, *b, *c);

		if(*a < 0) *a =0;
		else if(*a > VOXEL_DENSITY-1) *a = VOXEL_DENSITY-1;

		if(*b < 0) *b =0;
		else if(*b > VOXEL_DENSITY-1) *b = VOXEL_DENSITY-1;

		if(*c < 0) *c =0;
		else if(*c > VOXEL_DENSITY-1) *c = VOXEL_DENSITY-1;
	}
}

void queryLookupDirection(float x_head, float y_head, float z_head, float* x, float* y, float* z)
{
	float X,Y,Z;
	
	*x=0.0f;
	*y=0.0f;
	*z=0.0f;
	

	if(x_head >= 0.0f)	X=1.0f;
	else				X = -1.0f;
	if(y_head >= 0.0f)  Y= 1.0f;
	else				Y = -1.0f;
	if(z_head >= 0.0f)  Z= 1.0f;
	else				Z = -1.0f;


	if((abs(x_head) >= abs(y_head)) && (abs(x_head) >= abs(z_head))) *x=X;
	else if((abs(y_head) > abs(x_head)) && (abs(y_head) > abs(z_head))) *y=Y;
	else *z=Z;

}

void lookupLocalVoxels(int a, int b, int c, float head_x, float head_y, float head_z,
					   /*results-> direction and strength*/ float *x, float *y, float *z, float *light,
					   float up, float down, float horizontal=0.0f, float _droopU = 0.0f)
{
	float currentNext[3];
	float mini=100000000.0f;
	int t;

	float u = up, h = horizontal, r = 0.0f, d = down, droopU = _droopU;

	//h = 100.0f;

	queryLookupDirection(head_x, head_y, head_z, x,y,z);

	//Printf("Voxel %i %i %i light: %f \n", a,b,c, world.voxels[a][b][c].light);
	if(*x!=0.0f)
	{
		t = (int)*x;

		if(world.voxels[a]	[b+1][c]	.light-u<mini) {currentNext[0]=	0.0f;	currentNext[1]=1.0f;	currentNext[2]=	0.0f;	mini=world.voxels[a][b+1][c].light-u;}
		if(world.voxels[a]	[b+1][c-1]	.light<mini) {currentNext[0]=	0.0f;	currentNext[1]=1.0f;	currentNext[2]=	-1.0f;	mini=world.voxels[a][b+1][c-1].light;}
		if(world.voxels[a]	[b+1][c+1]	.light<mini) {currentNext[0]=	0.0f;	currentNext[1]=1.0f;	currentNext[2]=	1.0f;	mini=world.voxels[a][b+1][c+1].light;}
		if(world.voxels[a+t][b+1][c]	.light<mini) {currentNext[0]=	*x;		currentNext[1]=1.0f;	currentNext[2]=	0.0f;	mini=world.voxels[a+t][b+1][c].light;}
		if(world.voxels[a+t][b+1][c-1]	.light<mini) {currentNext[0]=	*x;		currentNext[1]=1.0f;	currentNext[2]=	-1.0f;	mini=world.voxels[a+t][b+1][c-1].light;}
		if(world.voxels[a+t][b+1][c+1]	.light<mini) {currentNext[0]=	*x;		currentNext[1]=1.0f;	currentNext[2]=	1.0f;	mini=world.voxels[a+t][b+1][c+1].light;}


		if(world.voxels[a]	[b]	[c-1]	.light<mini) {currentNext[0]=	0.0f;	currentNext[1]=0.0f;	currentNext[2]=	-1.0f;	mini=world.voxels[a][b][c-1].light-r;}
		if(world.voxels[a]	[b]	[c+1]	.light<mini) {currentNext[0]=	0.0f;	currentNext[1]=0.0f;	currentNext[2]=	1.0f;	mini=world.voxels[a][b][c+1].light-r;}
		if(world.voxels[a+t][b]	[c]		.light-h<mini) {currentNext[0]=	*x;		currentNext[1]=0.0f;	currentNext[2]=	0.0f;	mini=world.voxels[a+t][b][c].light-h-r;}
		if(world.voxels[a+t][b]	[c-1]	.light-h<mini) {currentNext[0]=	*x;		currentNext[1]=0.0f;	currentNext[2]=	-1.0f;	mini=world.voxels[a+t][b][c-1].light-h-r;}
		if(world.voxels[a+t][b]	[c+1]	.light-h<mini) {currentNext[0]=	*x;		currentNext[1]=0.0f;	currentNext[2]=	1.0f;	mini=world.voxels[a+t][b][c+1].light-h-r;}

		if(world.voxels[a]	[b-1][c]	.light-d<mini) {currentNext[0]=	0.0f;	currentNext[1]=-1.0f;	currentNext[2]=0.0f;	mini=world.voxels[a][b-1][c].light-d-r;}
		if(world.voxels[a]	[b-1][c-1]	.light-d<mini) {currentNext[0]=	0.0f;	currentNext[1]=-1.0f;	currentNext[2]=-1.0f;	mini=world.voxels[a][b-1][c-1].light-d-r;}
		if(world.voxels[a]	[b-1][c+1]	.light-d<mini) {currentNext[0]=	0.0f;	currentNext[1]=-1.0f;	currentNext[2]=1.0f;	mini=world.voxels[a][b-1][c+1].light-d-r;}
		if(world.voxels[a+t][b-1][c]	.light-d<mini) {currentNext[0]=	*x;		currentNext[1]=-1.0f;	currentNext[2]=0.0f;	mini=world.voxels[a+t][b-1][c].light-d-r;}
		if(world.voxels[a+t][b-1][c-1]	.light-d<mini) {currentNext[0]=	*x;		currentNext[1]=-1.0f;	currentNext[2]=-1.0f;	mini=world.voxels[a+t][b-1][c-1].light-d-r;}
		if(world.voxels[a+t][b-1][c+1]	.light-d<mini) {currentNext[0]=	*x;		currentNext[1]=-1.0f;	currentNext[2]=1.0f;	mini=world.voxels[a+t][b-1][c+1].light-d-r;}
		//else {currentNext[0]=	*x;		currentNext[1]=0.0f;	currentNext[2]=	0.0f;	mini=world.voxels[a+t][b][c].light;}
		//Printf("current next x: %f %f %f \n", currentNext[0], currentNext[1], currentNext[2]);
		*x=currentNext[0]; *y=currentNext[1]; *z=currentNext[2];
		//if(*y < 0){ *x = 0.0f; *y = -1.0f; *z = 0.0f;}
	}
	else if (*y!=0.0f)
	{
		t = (int)*y;

		//if(*y > 0) {droopU *= -1; /*u *= -1.0f;*/}
		
		if(world.voxels[a+1][b]	 [c]	.light<mini) {currentNext[0]=	1.0f;	currentNext[1]=0.0f;	currentNext[2]=	0.0f;	mini=world.voxels[a+1][b][c].light-r;}
		if(world.voxels[a+1][b]	 [c-1]	.light<mini) {currentNext[0]=	1.0f;	currentNext[1]=0.0f;	currentNext[2]=	-1.0f;	mini=world.voxels[a+1][b][c-1].light-r;}
		if(world.voxels[a+1][b]	 [c+1]	.light<mini) {currentNext[0]=	1.0f;	currentNext[1]=0.0f;	currentNext[2]=	1.0f;	mini=world.voxels[a+1][b][c+1].light-r;}
		if(world.voxels[a+1][b+t][c]	.light-droopU<mini) {currentNext[0]=	1.0f;	currentNext[1]=*y;		currentNext[2]=	0.0f;	mini=world.voxels[a+1][b+t][c].light-r-droopU;}
		if(world.voxels[a+1][b+t][c-1]	.light-droopU<mini) {currentNext[0]=	1.0f;	currentNext[1]=*y;		currentNext[2]=	-1.0f;	mini=world.voxels[a+1][b+t][c-1].light-r-droopU;}
		if(world.voxels[a+1][b+t][c+1]	.light-droopU<mini) {currentNext[0]=	1.0f;	currentNext[1]=*y;		currentNext[2]=	1.0f;	mini=world.voxels[a+1][b+t][c+1].light-r-droopU;}
		//Printf("[a+1][b][c] is %f and mini is %f\n",world.voxels[a+1][b][c].light, mini);
		
		if(world.voxels[a]	[b]	 [c-1]	.light<mini) {currentNext[0]=	0.0f;	currentNext[1]=0.0f;	currentNext[2]=	-1.0f;	mini=world.voxels[a][b][c-1].light-r;}
		if(world.voxels[a]	[b]	 [c+1]	.light<mini) {currentNext[0]=	0.0f;	currentNext[1]=0.0f;	currentNext[2]=	1.0f;	mini=world.voxels[a][b][c+1].light-r;}
		if(world.voxels[a]	[b+t][c]	.light-u-droopU<mini) {currentNext[0]=	0.0f;	currentNext[1]=*y;		currentNext[2]=	0.0f;	mini=world.voxels[a][b+t][c].light-u-r-droopU;}
		if(world.voxels[a]	[b+t][c-1]	.light-droopU<mini) {currentNext[0]=	0.0f;	currentNext[1]=*y;		currentNext[2]=	-1.0f;	mini=world.voxels[a][b+t][c-1].light-r-droopU;}
		if(world.voxels[a]	[b+t][c+1]	.light-droopU<mini) {currentNext[0]=	0.0f;	currentNext[1]=*y;		currentNext[2]=	1.0f;	mini=world.voxels[a][b+t][c+1].light-r-droopU;}
		//Printf("[a][b+t][c] is %f and mini is %f\n",world.voxels[a][b+t][c].light-0.1f, mini);

		if(world.voxels[a-1][b]	 [c]	.light<mini) {currentNext[0]=	-1.0f;	currentNext[1]=0.0f;	currentNext[2]=0.0f;	mini=world.voxels[a-1][b][c].light-r;}
		if(world.voxels[a-1][b]	 [c-1]	.light<mini) {currentNext[0]=	-1.0f;	currentNext[1]=0.0f;	currentNext[2]=-1.0f;	mini=world.voxels[a-1][b][c-1].light-r;}
		if(world.voxels[a-1][b]	 [c+1]	.light<mini) {currentNext[0]=	-1.0f;	currentNext[1]=0.0f;	currentNext[2]=1.0f;	mini=world.voxels[a-1][b][c+1].light-r;}
		if(world.voxels[a-1][b+t][c]	.light-droopU<mini) {currentNext[0]=	-1.0f;	currentNext[1]=*y;		currentNext[2]=0.0f;	mini=world.voxels[a-1][b+t][c].light-r-droopU;}
		if(world.voxels[a-1][b+t][c-1]	.light-droopU<mini) {currentNext[0]=	-1.0f;	currentNext[1]=*y;		currentNext[2]=-1.0f;	mini=world.voxels[a-1][b+t][c-1].light-r-droopU;}
		if(world.voxels[a-1][b+t][c+1]	.light-droopU<mini) {currentNext[0]=	-1.0f;	currentNext[1]=*y;		currentNext[2]=1.0f;	mini=world.voxels[a-1][b+t][c+1].light-r-droopU;}
		//else {currentNext[0]=	0.0f;	currentNext[1]=*y;		currentNext[2]=	0.0f;	mini=world.voxels[a][b+t][c].light-0.1f;}
		//Printf("current next y: %f %f %f y: %f \n", currentNext[0], currentNext[1], currentNext[2], *y);
		*x=currentNext[0]; *y=currentNext[1]; *z=currentNext[2];
		//if(*y < 0 && ran(10.0f) > 3.6f){ *x = 0.0f; *y = -1.0f; *z = 0.0f;}
	}
	else
	{
		t = (int)*z;

		if(world.voxels[a]	[b+1][c]	.light-u<mini) {currentNext[0]=	0.0f;	currentNext[1]=1.0f;	currentNext[2]=	0.0f;	mini=world.voxels[a][b+1][c].light-u;}
		if(world.voxels[a-1][b+1][c]	.light<mini) {currentNext[0]=	-1.0f;	currentNext[1]=1.0f;	currentNext[2]=	0.0f;	mini=world.voxels[a-1][b+1][c].light;}
		if(world.voxels[a+1][b+1][c]	.light<mini) {currentNext[0]=	1.0f;	currentNext[1]=1.0f;	currentNext[2]=	0.0f;	mini=world.voxels[a+1][b+1][c].light;}
		if(world.voxels[a]	[b+1][c+t]	.light<mini) {currentNext[0]=	0.0f;	currentNext[1]=1.0f;	currentNext[2]=	*z;		mini=world.voxels[a][b+1][c+t].light;}
		if(world.voxels[a-1][b+1][c+t]	.light<mini) {currentNext[0]=	-1.0;	currentNext[1]=1.0f;	currentNext[2]=	*z;		mini=world.voxels[a-1][b+1][c+t].light;}
		if(world.voxels[a+1][b+1][c+t]	.light<mini) {currentNext[0]=	1.0f;	currentNext[1]=1.0f;	currentNext[2]=	*z;		mini=world.voxels[a+1][b+1][c+t].light;}
		//Printf("[a][b+1][c] is %f and mini is %f\n",world.voxels[a][b+1][c].light-0.1f, mini);

		if(world.voxels[a-1][b]	[c]		.light<mini) {currentNext[0]=	-1.0f;	currentNext[1]=0.0f;	currentNext[2]=	0.0f;	mini=world.voxels[a-1][b][c].light-r;}
		if(world.voxels[a+1][b]	[c]		.light<mini) {currentNext[0]=	1.0f;	currentNext[1]=0.0f;	currentNext[2]=	0.0f;	mini=world.voxels[a+1][b][c].light-r;}
		if(world.voxels[a]	[b]	[c+t]	.light-h<mini) {currentNext[0]=	0.0f;	currentNext[1]=0.0f;	currentNext[2]=	*z;		mini=world.voxels[a][b][c+t].light-h-r;}
		if(world.voxels[a-1][b]	[c+t]	.light-h<mini) {currentNext[0]=	-1.0f;	currentNext[1]=0.0f;	currentNext[2]=	*z;		mini=world.voxels[a-1][b][c+t].light-h-r;}
		if(world.voxels[a+1][b]	[c+t]	.light-h<mini) {currentNext[0]=	1.0f;	currentNext[1]=0.0f;	currentNext[2]=	*z;		mini=world.voxels[a+1][b][c+t].light-h-r;}
		//Printf("[a]	[b]	[c+t] is %f and mini is %f\n",world.voxels[a][b][c+t].light, mini);

		if(world.voxels[a]	[b-1][c]	.light-d<mini) {currentNext[0]=	0.0f;	currentNext[1]=-1.0f;	currentNext[2]=0.0f;	mini=world.voxels[a][b-1][c].light-d-r;}
		if(world.voxels[a-1][b-1][c]	.light-d<mini) {currentNext[0]=	-1.0f;	currentNext[1]=-1.0f;	currentNext[2]=0.0f;	mini=world.voxels[a-1][b-1][c].light-d-r;}
		if(world.voxels[a+1][b-1][c]	.light-d<mini) {currentNext[0]=	1.0f;	currentNext[1]=-1.0f;	currentNext[2]=0.0f;	mini=world.voxels[a+1][b-1][c].light-d-r;}
		if(world.voxels[a]	[b-1][c+t]	.light-d<mini) {currentNext[0]=	0.0f;	currentNext[1]=-1.0f;	currentNext[2]=*z;		mini=world.voxels[a][b-1][c+t].light-d-r;}
		if(world.voxels[a-1][b-1][c+t]	.light-d<mini) {currentNext[0]=	-1.0f;	currentNext[1]=-1.0f;	currentNext[2]=*z;		mini=world.voxels[a-1][b-1][c+t].light-d-r;}
		if(world.voxels[a+1][b-1][c+t]	.light-d<mini) {currentNext[0]=	1.0f;	currentNext[1]=-1.0f;	currentNext[2]=*z;		mini=world.voxels[a+1][b-1][c+t].light-d-r;}
		//else {currentNext[0]=	0.0f;	currentNext[1]=0.0f;	currentNext[2]=	*z;		mini=world.voxels[a][b][c+t].light;}
		//Printf("current next z: %f %f %f \n", currentNext[0], currentNext[1], currentNext[2]);
		*x=currentNext[0]; *y=currentNext[1]; *z=currentNext[2];
		//if(*y < 0){ *x = 0.0f; *y = -1.0f; *z = 0.0f;}
	}
	
	*light = world.voxels[a + (int)*x][b + (int)*y][c + (int)*z].light;
}



bool placeBud(int x, int y, int z, float threshold)
{
	int voxel_x, voxel_y, voxel_z;
	
	queryVoxel(x,y,z, &voxel_x, &voxel_y, &voxel_z);
	
	if(  (world.voxels[voxel_x][voxel_y][voxel_z].light < threshold)) return true;

	return false;
}

int Plant::generateId()
{
	/*if(oldFreeId) {
		//Printf("free Id %i of current %i \n", freeIds[oldFreeId -1], oldFreeId);
		lastFreeId = oldFreeId -1;
		return freeIds[--oldFreeId];
	}*/

	return ++apId;
}

int Plant::generateInternodeId()
{
	return ++interId;
}

int Plant::generateInternodeDroopId()
{
	return interDroopId++;
}
int Plant::generateDroopId()
{
	return apDroopId++;
}


int Plant::createInternode(ApexPointer ap, bool place)
{
	int tmp;
	if(dominance) tmp = -1;
	else tmp =0;

	int iId = generateInternodeId();
	internodes[iId].id = iId;
	internodes[iId].age=0;
	/*if(branches[ap->id].oldVigor > ran(5.0f) && branches[ap->id].oldVigor > 1.6f) internodes[iId].iLength = root.iLength;
	else*/
	if(ap->order == 0) internodes[iId].iLength = params.internodeLengthTrunk / (ap->order + 2 + tmp);
	else internodes[iId].iLength = params.internodeLength * branches[ap->id].oldVigor/(ap->order+1);//root.iLength /(ap->order+1);

	internodes[iId].iWidth = 0.9f;
	internodes[iId].iRadialGrowth = radialGrowth + 0.0f;
	//internodes[iId].otherPipes = 0.0f;
	internodes[iId].branch = &branches[ap->id];
		
	internodes[iId].lastInternode = ap->lastInternode;

	internodes[iId].isLast = place;

	return iId;
}

int Plant::createInternodeDroop(ApexDroopPointer ap, bool place)
{
	int tmp;
	if(dominance) tmp = -1;
	else tmp =0;

	int iId = generateInternodeDroopId();
	internodesDroop[iId].id = iId;
	internodesDroop[iId].age=0;
	/*if(branches[ap->id].oldVigor > ran(5.0f) && branches[ap->id].oldVigor > 1.6f) internodes[iId].iLength = root.iLength;
	else*/
	/*if(ap->order == 0) internodesDroop[iId].iLength = root.iLength / (ap->order + 2 + tmp);
	else*/ internodesDroop[iId].iLength = 9.0f;//root.iLength /(ap->order+1);

	internodesDroop[iId].iWidth = 0.9f;
	internodesDroop[iId].iRadialGrowth = radialGrowth + 0.0f;
	//internodes[iId].otherPipes = 0.0f;
	internodesDroop[iId].branch = &branches[ap->id];
		
	if(ap->lastInternode != NULL) ap->lastInternode->nextInternode = &internodesDroop[iId];

	ap->lastInternode = &internodesDroop[iId];

	//internodesDroop[iId].isLast = place;

	return iId;
}

void Plant::createBud(ApexPointer ap, BudData &bu)
{
	bu.age = 0;
	/*if(!dominance && ap->order>1) bu.direction = 50.0f;
	//else if(ap->order ==0 && ap->age >3 && ap->age <7 ) bu.direction = 106.0f;
	//else if(ap->order ==0 && ap->age >7 && ap->age <10) bu.direction = 56.0f;
	else*/ bu.direction = ap->plant->params.direction[0];

	bu.order=ap->order;
	//bu.positionAngle = ap.budAngle;
	bu.positionAngle = fmod(ap->lastBudAngle + ap->budAngle, 360.0f);
	ap->lastBudAngle = bu.positionAngle;
	//Printf("bu.positionAngle %f  ap.lastBudAngle %f  ap.budAngle %f \n", bu.positionAngle, ap.lastBudAngle, ap.budAngle);
	
	/*if(ap->order > ran(9.0f))bu.budmode=shortNode;
	else*/ bu.budmode=longNode;
		
	bu.id = generateId();
	bu.parentId = ap->id;
	bu.parentInternode = ap->lastInternode;

	/*
	if(ran(1.0f)>0.7f)bu.sleepCycles = (int)ran(7.0f);
	else*/ bu.sleepCycles = 0;
			
	ap->lastBudId=bu.id;
	ap->num_childBuds++;
			
	branches[bu.id].parentApex = ap;
	branches[bu.id].childNumber = ++ap->lastChildNumber;
	branches[bu.id].energy = 0.0f;
	branches[bu.id].num_childBranches = 0;

	branches[bu.id].order = ap->order + 1;

	branches[bu.id].radialWidth = 0.1f;

	//branches[bu.id].children = 0;

	branches[bu.id].branchmode = bu.budmode;
	branches[bu.id].rank = -1.0f;
	branches[bu.id].childBranches[0] = &branches[bu.id];
	branches[bu.id].lastChild = 1;
	branches[bu.id].oldVigor = 0.0f;

	/*if(branches[ap->id].lastChild > 15 || (!dominance && steps<=120))branches[bu.id].vigorCap = 4.5f;
	else if(steps > 120) {branches[bu.id].vigorCap = 2.5f; //branches[ap->id].vigorCap = 2.9f;
	}
	else*/ branches[bu.id].vigorCap = 3.4f;//branches[ap->id].vigorCap;

	branches[bu.id].VoxelPosition[0] = ap->VoxelPosition[0];
	branches[bu.id].VoxelPosition[1] = ap->VoxelPosition[1];
	branches[bu.id].VoxelPosition[2] = ap->VoxelPosition[2];

	branches[bu.id].leafDownAngle = ran(36.0f);

	branches[bu.id].childId = branches[ap->id].lastChild;

	if(dominance)branches[bu.id].apicalDominanceFactor = params.apicalDomThreshold[0] ;
	else branches[bu.id].apicalDominanceFactor = params.apicalDomThreshold[1];

	//if(ap->order > 1) branches[bu.id].apicalDominanceFactor = 5.0f;

	branches[ap->id].childBranches[branches[ap->id].lastChild++] = &branches[bu.id];
	branches[ap->id].num_childBranches++;

	bpd[bu.id].branchApex = branches[bu.id].branchApex;
	branches[bu.id].branchPoint = &bpd[bu.id];
			
	int j=ap->id;
	while(branches[j].branchApex != NULL && branches[j].branchApex->order > 0)
	{
		branches[j=branches[j].parentApex->id].num_childBranches++;
	}
	
	branches[bu.id].plant = branches[ap->id].plant;
	bu.plant = ap->plant;
}

group 0:
production:
Internode(internode)>GetHead(x,y,z)GetUp(ux,uy,uz):
{
	internode->iRadialGrowth = 0.0f;

	internode->age++;

	if( internode->age == 15) 
	{
		float next[3], head[3], rot_axxis[3], up[3];
		next[0]=0.0f; next[1]=-1.0f; next[2]=0.0f;
		
		head[0]=x; head[1]=y; head[2]=z;
		
		up[0]=ux; up[1]=uy; up[2]=uz;
 		
		
		cross(head, next, rot_axxis);

		R(0.25f, rot_axxis, head,0);

		nproduce SetHead(head[0],head[1],head[2],up[0],up[1],up[2]);
	}

	HERE;
	produce Internode(internode);
}


BranchPoint(bp):
{
	//if(bp->branchApex->id == 0)Printf("Branchpoint energy %f\n", bp->energy);
	//bp->age++;
	int tmp, j;	

	
	/*if(bp->internode != NULL && bp->internode->age == 15) 
	{
		next[0]=0.0f; next[1]=-1.0f; next[2]=0.0f;
		
		head[0]=dX; head[1]=dY; head[2]=dZ;
		
		if(ly > uy) {up[0]=lx; up[1]=ly; up[2]=lz;}
		else {up[0]=ux; up[1]=uy; up[2]=uz;}
 		
		
		cross(head, next, rot_axxis);

		if(ap->plant->dominance)R(0.03f, rot_axxis, head,0);

		nproduce SetHead(1,0,0,0,1,0);
	}*/
	
	bp->branch->plant->PropagateEnergyToChildren(bp->branch->plant->branches[bp->branchApex->id].energy, bp->branchApex->id);
	HERE;
	produce BranchPoint(bp);
}


Apex(ap)>GetHead(dX,dY,dZ)GetPos(x,y,z)GetUp(ux,uy,uz)GetLeft(lx, ly, lz):
{
	//Printf("%f %f %f,\n %f %f %f,\n %f %f %f,\n %f %f %f\n", 
	//	dX, dY, dZ, x, y, z, ux, uy, uz, lx, ly, lz);
	if(ap->plant->branches[ap->id].branchApex == NULL) {/*Printf("Apex out %i \n",ap->id);*/return;}
	//Printf("Apex %i oldVigor %f \n",ap->id, ap->plant->branches[ap->id].oldVigor);
	float dx=0.f, dy=0.0f, dz=0.0f, next_x, next_y, next_z, /*light,*/ hx=0.0f, hy=0.0f, hz=0.0f;
	int tmp, voxel_x, voxel_y, voxel_z, j, i;
	float head[3];
	float up[3]; 
 	float rot_axxis[3];
	float next[3];

	if(ap->order == 0 && ap->plant->branches[0].apicalDominanceFactor <= 0.1f) 
	{//Printf("trunk\n");	
	HERE;
		if(ap->plant->dominance)
		{
		HERE;
			ap->plant->dominance = false;
			
			//ap->plant->branches[ap->id].childBranches[j]->vigorCap
			/*Printf("plant dominance over: lastChild %i \n", ap->plant->branches[0].lastChild);
			for(j=1; j < ap->plant->branches[0].lastChild; j++)
			{
				ap->plant->branches[ap->id].childBranches[j]->vigorCap =  3.4f;

				tmp = ap->plant->branches[ap->id].childBranches[j]->id;
				
				for(i=1; i < ap->plant->branches[tmp].lastChild; i++)
					ap->plant->branches[tmp].childBranches[i]->vigorCap = ap->plant->branches[tmp].vigorCap;
			}
			
			*/
			return;
		}
		else 
		{
			return;	
		}
		
		
	}
	else if(ap->order == 0 && ap->plant->branches[1].apicalDominanceFactor - ap->plant->branches[0].apicalDominanceFactor <= 0.0f)
	{
	HERE;
		ap->plant->sapling = false;
	}

	ap->plant->branches[ap->id].apicalDominanceFactor -=1.0f;

	ap->headY = dY; ap->upY = uy; 
	/*if(ap->order)
	Printf(" vigor of apex %i of order %i with parent %i is %f\n", ap->id, ap->order, ap->plant->branches[ap->id].parentApex->id,
		ap->plant->branches[ap->id].vigor);
	else {
		Printf(" vigor of apex %i of order %i with parent %i is %f\n", ap->id, ap->order, -1,
		ap->plant->branches[ap->id].vigor);*/
		if(ap->age < 10 && !ap->order) ap->plant->branches[ap->id].vigor = Max(ap->plant->branches[ap->id].vigor, 2.5f);
	//}

		//ap->plant->branches[ap->id].energyBudRatio = ap->plant->branches[ap->id].energy / Max(ap->plant->branches[ap->id].num_childBranches + 1 , 1);

	if( ap->order &&  (dY > -0.15f || (ap->VoxelPosition[1] < ap->plant->voxelPos[1] + ap->plant->params.minCutHeight && steps > 30 && !ap->plant->dominance
//		||
		//dY > -0.5f 
		|| ap->plant->branches[ap->id].energyBudRatio < ap->plant->params.cutThreshold * 0.01f
		)
		) && ( (!ap->plant->dominance && (ap->plant->branches[ap->id].energyBudRatio < ap->plant->params.cutThreshold //+ ran(0.16f)-0.08f 
		&& ap->age < 12//&& ap->plant->branches[ap->id].branchPoint->internode->iWidth < 9.0f
		|| (ap->VoxelPosition[1] < ap->plant->voxelPos[1] + ap->plant->params.minCutHeight && steps > 30)
		)
		|| (ap->plant->branches[ap->id].energyBudRatio < ap->plant->params.cutThreshold * 3 / (10 - ap->order))
		) 
		//|| (ap->age > 25 && ap->plant->branches[ap->id].childNumber < 
		//ap->plant->branches[ap->plant->branches[ap->id].parentApex->id].lastChild / 2 && ap->order==1)
		//|| ( ran((float)ap->plant->MAX_Y * 0.3f) > (float)ap->VoxelPosition[1] )
		//|| ( ran((float)ap->plant->MAX_Y * 0.7f) > (float)ap->VoxelPosition[1] && steps > 150 && ran(1.0f)>0.9f)//&& ap->plant->branches[ap->id].energyBudRatio < 0.25f * (float)Min(ap->order,2) )
		
		//|| (ap->age > 25 && ap->plant->branches[ap->id].branchPoint->internode->iWidth < 16.0f && ap->plant->branches[ap->id].energyBudRatio < ap->plant->cutThreshold)
		/*|| (ap->age >15 && ap->plant->branches[ap->id].energy < 10.0f)*/)
		//photo(ap->plant->branches[ap->id].branchApex->light, 2.5f, 160.0f) < ran(0.1f)
		)
	{
		HERE;
		//if(ap->plant->branches[ap->id].branchPoint != NULL)
		//{
		//Printf("trunk\n");
			ap->plant->branches[ap->id].branchPoint->cut = true;
			
			ap->plant->branches[ap->id].branchPoint->cutDelay = 1;//3*(5-k);
			
			//ap->plant->cutThreshold -= 0.001f;

		tmp=ap->id;

		for(j=ap->plant->branches[ap->id].childId; j < ap->plant->branches[ap->plant->branches[tmp].parentApex->id].lastChild-1; j++)
		{
			ap->plant->branches[ap->plant->branches[tmp].parentApex->id].childBranches[j] = 
			ap->plant->branches[ap->plant->branches[tmp].parentApex->id].childBranches[j+1];
			
			ap->plant->branches[ap->plant->branches[tmp].parentApex->id].childBranches[j]->childId = j;
		}

		ap->plant->branches[ap->plant->branches[tmp].parentApex->id].lastChild--;

		while(tmp)
		{	
			ap->plant->branches[tmp=ap->plant->branches[tmp].parentApex->id].num_childBranches--;
		}
		
		
		ap->plant->branches[ap->id].branchApex = NULL;
		
		return;
		//}
	}
	
	//PropagateEnergyToChildren(ap->plant->branches[ap->id].energy, ap->id);

	ap->plant->branches[ap->id].energy=0.0f;


	//if(ap->age>3)world.updateVoxels(ap->VoxelPosition[0],ap->VoxelPosition[1],ap->VoxelPosition[2], -3.4f);

	queryVoxel(x,y,z, &voxel_x, &voxel_y, &voxel_z);

	//if(voxel_x != ap->VoxelPosition[0] || voxel_y != ap->VoxelPosition[1] || voxel_z != ap->VoxelPosition[2])
	//{
		ap->VoxelPosition[0] = voxel_x;
		ap->VoxelPosition[1] = voxel_y;
		ap->VoxelPosition[2] = voxel_z;

		world.updateVoxels(voxel_x,voxel_y,voxel_z, 3.4f /*/ Min(Max(ap->order,1), 4.0f)*/);
		

		//if(!ap->plant->dominance && steps<=125)lookupLocalVoxels(voxel_x, voxel_y, voxel_z, dX, dY, dZ, &next_x, &next_y, &next_z, &ap->light, -1.15f, 2.5f);
		//else if(steps > 125) lookupLocalVoxels(voxel_x, voxel_y, voxel_z, dX, dY, dZ, &next_x, &next_y, &next_z, &ap->light, 0.15f, 20.5f, 2.5f, -5.0f);
		//else lookupLocalVoxels(voxel_x, voxel_y, voxel_z, dX, dY, dZ, &next_x, &next_y, &next_z, &ap->light, 0.15f, -3.0f);
		if(!ap->plant->dominance && steps<=120)lookupLocalVoxels(voxel_x, voxel_y, voxel_z, dX, dY, dZ, &next_x, &next_y, &next_z, &ap->light, ap->plant->params.plantVigorousStrategy[0], ap->plant->params.plantVigorousStrategy[1]);
		else if(steps > 120) lookupLocalVoxels(voxel_x, voxel_y, voxel_z, dX, dY, dZ, &next_x, &next_y, &next_z, &ap->light, ap->plant->params.plantMatureStrategy[0], ap->plant->params.plantMatureStrategy[1], ap->plant->params.plantMatureStrategy[2], ap->plant->params.plantMatureStrategy[3]);
		else lookupLocalVoxels(voxel_x, voxel_y, voxel_z, dX, dY, dZ, &next_x, &next_y, &next_z, &ap->light, ap->plant->params.plantSaplingStrategy[0], ap->plant->params.plantMatureStrategy[2]);
		

		next[0]=next_x; next[1]=next_y; next[2]=next_z;
		//if(ap->rank < 1.0f && ap->age > 3){ next[0]=0.0f; next[1] =-1.0f; next[2]=0.0f;}
		//else if(ap->rank < 0.9f && ap->age > 3) next[1] =0.0f;
		
		head[0]=dX; head[1]=dY; head[2]=dZ;
		
		/*if(ly > uy) {up[0]=lx; up[1]=ly; up[2]=lz;}
		else*/ {up[0]=ux; up[1]=uy; up[2]=uz;}
 		
		
		cross(head, next, rot_axxis);

		if(steps > 120)R(0.21f, rot_axxis, head,0);
		else R(0.09f, rot_axxis, head,0);

	//}
/*
	if(ap->order){
	if(ran(1.0f) > 0.5f )head[0] += ran(0.05f); else head[0] -= ran(0.05f);
	if(ran(1.0f) > 0.5f )head[1] += ran(0.05f); else head[1] -= ran(0.05f);
	if(ran(1.0f) > 0.5f )head[2] += ran(0.05f); else head[2] -= ran(0.05f);
	//}*/


	
	ap->age++;
	
	/*if(ap->order){
	ap->growthSpeed = ap->growthSpeed * sqrt(sqrt(sqrt(sqrt(sqrt((float)ap->plant->branches[ap->id].childNumber / 
		(float)ap->plant->branches[ap->plant->branches[ap->id].parentApex->id].num_childBranches)))));

	Printf("childnumber %i of id %i and num childs %i of id %i \n", ap->plant->branches[ap->id].childNumber, ap->id,
		ap->plant->branches[ap->plant->branches[ap->id].parentApex->id].num_childBranches, ap->plant->branches[ap->id].parentApex->id);
	}*/
	//if(ap->order)nproduce RollToVert();
	if(!(ap->age % 1)  &&  ap->order < 9  && ap->plant->branches[ap->id].oldVigor > 2.3f && !(!ap->order && ap->age < ap->plant->params.minAge)
		&& (ran(1.0f) > ap->plant->params.branchProb || !ap->order) // || (!ap->order && ap->age < 5)//&& ap->branchProb >= ran(1.0f)//
		)
	{
	HERE;
		//ap->plant->branches[ap->id].oldVigor -= 1.6f;
		if(ap->plant->dominance) tmp = -1;
		else tmp =0;

		int iId = ap->plant->createInternode(ap, false);
		if(iId > MAX_INTERNODES-1){ return;}

		/*float MIN;
		if (MIN=(float)getBranchDistance(ap->id)<7.0f);
		else MIN =7.0f;
*/
		//Printf("ap->order %i id %i has childnumber %i and light %f and lastChild %i \n", ap->order, ap->id, ap->plant->branches[ap->id].childNumber, 
		//	ap->plant->branches[ap->id].directEnergy, ap->plant->branches[ap->id].lastChild);
		
		if(ap->plant->branches[ap->id].lastChild < MAX_CHILDREN && ap->plant->branches[ap->id].directEnergy > 0.001f
			 //&& (( Min(getBranchDistance(ap->id),20) < ran(30.0f) && ap->plant->dominance ) 
			 //&& ( ran((float)ap->plant->MAX_Y * 1.0f) < (float)ap->VoxelPosition[1] )
			 //|| (!ap->plant->dominance && Min(getBranchDistance(ap->id),20) < ran(75.0f)) )
			 //&& ( ran((float)ap->plant->branches[0].VoxelPosition[1]) < (float)ap->VoxelPosition[1])
			 //&& ran(1.0f) > 0.1f
			 //ran(1.0f) > 0.5f
			/*&& (ap->plant->branches[ap->id].childNumber > ran(4.0f) || ap->age < 7 || (!ap->order)) 
			// || (ap->id==1)
			//ran(6.5f) > MIN || !ap.order&& ap.order<2  //&& ran(21.0f) > world.voxels[voxel_x][voxel_y][voxel_z].light
			/*ran(15.0f) > world.voxels[voxel_x][voxel_y][voxel_z].light - (float)ap.id  || !ap.order*/){
			
			
			//Printf("apex at voxel %i %i %i has light %f\n", voxel_x,voxel_y,voxel_z, world.voxels[voxel_x][voxel_y][voxel_z].light);
			//ap->plant->branches[bu.id].parentId = ap->id;
			//ap->plant->branches[bu.id].id = ap->plant->branches[ap->id].lastChildId++;
			//Printf("order %i , id branch %i \n", ap->order, ap->plant->branches[bu.id].id);
			//ap->plant->branches[bu.id].num_childBranches = 0;


				//if(ap->growthSpeed > ran(3.0f) && ap->growthSpeed > 1.0f) nproduce Internode(&ap->plant->internodes[iId]);
				HERE;
			BudData bu;
			
			ap->plant->createBud(ap, bu);

			if(bu.id > MAX_UNITS-1) return;
			
			ap->lastInternode = &ap->plant->internodes[iId];

			
			/*if(//ap->order > tmp 
					 ((Min(ap->plant->getBranchDistance(ap->id),20) < ran(30.0f))  
					||(!ap->plant->dominance && Min(getBranchDistance(ap->id),20) < ran(35.0f))) )
				{

					ap->plant->internodes[iId].iLength *= ap->plant->branches[ap->id].oldVigor;
					world.updateVoxels(voxel_x,voxel_y+2,voxel_z, 1.0f );
				}
				*/
			
				if(ap->plant->branches[ap->id].oldVigor > 2.8f && ap->order > tmp 
					//&& ((Min(ap->plant->getBranchDistance(ap->id),20) < ran(30.0f))
					//&& ( ran((float)ap->plant->MAX_Y * 2.0f) < (float)ap->plant->voxelPos[1] //&& ap->order//(float)ap->VoxelPosition[1] 
					//|| (!ap->plant->dominance && Min(ap->plant->getBranchDistance(ap->id),20) < ran(35.0f))
					//)
					
					// && ( ran((float)ap->plant->branches[0].VoxelPosition[1]) < (float)ap->VoxelPosition[1])
					 //&& ( ran((float)ap->plant->MAX_Y * 1.0f) < (float)ap->VoxelPosition[1])
					)
				{
				HERE;
					ap->plant->internodes[iId].iLength *= 2.0f;
					world.updateVoxels(voxel_x,voxel_y+1,voxel_z, 0.5f );


					//nproduce Internode(&ap->plant->internodes[iId]);
				}
				if(ap->plant->branches[ap->id].oldVigor > 3.3f   && ap->order > tmp
					//&& ( ran((float)ap->plant->MAX_Y * 2.0f) < (float)ap->plant->voxelPos[1]
					//&& ((Min(ap->plant->getBranchDistance(ap->id),20) < ran(23.0f) /*&& ap->order*/ 
					 //|| !ap->plant->dominance && Min(ap->plant->getBranchDistance(ap->id),20) < ran(55.0f)&& ap->order )
					 //)
					 //|| ran((float)ap->plant->MAX_Y * 0.7f) < (float)ap->plant->voxelPos[1] && ap->order) 
					// && ( ran((float)ap->plant->MAX_Y * 1.0f) < (float)ap->plant->voxelPos[1]//(float)ap->VoxelPosition[1] 
					//||(!ap->plant->dominance && Min(ap->plant->getBranchDistance(ap->id),20) < ran(55.0f)))
					 //&& ( ran((float)ap->plant->branches[0].VoxelPosition[1]) < (float)ap->VoxelPosition[1])
					 //&& ( ran((float)ap->plant->MAX_Y * 1.0f) < (float)ap->VoxelPosition[1])
					)
				{
				HERE;
					world.updateVoxels(voxel_x,voxel_y+2,voxel_z, 0.5f );
					ap->plant->internodes[iId].iLength *= 1.5f;
					//nproduce Internode(&ap->plant->internodes[iId]);
				}
				/*if(ap->plant->branches[ap->id].oldVigor > 4.3f   && ap->order > tmp 
					&& ((Min(ap->plant->getBranchDistance(ap->id),20) < ran(23.0f) && ap->order) 
					 || (!ap->plant->dominance && Min(getBranchDistance(ap->id),20) < ran(55.0f) && ap->order))
					 //&& ( ran((float)ap->plant->branches[0].VoxelPosition[1]) < (float)ap->VoxelPosition[1])
					 //&& ( ran((float)ap->plant->MAX_Y * 1.0f) < (float)ap->VoxelPosition[1])
					)
				{
					//world.updateVoxels(voxel_x,voxel_y+3,voxel_z, 0.5f );
					ap->plant->internodes[iId].iLength *= 1.025f;
					//nproduce Internode(&ap->plant->internodes[iId]);
				}*/
			
			
			
			
			//ap->plant->branches[ap->id].num_childBranches++;
			//ap->plant->branches[ap->id].lastChildId = ap->plant->branches[bu.id].id;
			//ap->plant->branches[ap->id].lastChildNumber = ap->plant->branches[bu.id].childNumber;
			
			if( 0 && ap->apexDroop == NULL && steps > 35 && ap->plant->apDroopId < MAX_UNITSDROOP - 1//ran((float)ap->plant->MAX_Y * 0.7f) > (float)ap->VoxelPosition[1]//&& ap->plant->apDroopId < 10//&& ap->age == 7 //ran(1.0f) > 1.0f /*&& !ap->plant->dominance*/
				){
				HERE;
					int apDroopId = ap->plant->generateDroopId();
					ap->plant->apDroop[apDroopId].id = apDroopId;
					ap->plant->apDroop[apDroopId].age=0;
					ap->plant->apDroop[apDroopId].budAngle = apex[0].budAngle;
					ap->plant->apDroop[apDroopId].lastBudAngle = 0.0f;
					ap->plant->apDroop[apDroopId].order = ap->plant->branches[ap->id].order;
					ap->plant->apDroop[apDroopId].lastBudId=0;
					ap->plant->apDroop[apDroopId].parentId = ap->id;
					ap->plant->apDroop[apDroopId].num_childBuds = 0;
					ap->plant->apDroop[apDroopId].lastChildNumber = 0;
					//ap->plant->apDroop[apDroopId].lastInternode = &ap->plant->internodes[iId];

					//ap->plant->apDroop[bu.id].apicalDominanceFactor = 0.0f;

					ap->plant->apDroop[apDroopId].VoxelPosition[0] = voxel_x;
					ap->plant->apDroop[apDroopId].VoxelPosition[1] = voxel_y;
					ap->plant->apDroop[apDroopId].VoxelPosition[2] = voxel_z;

					ap->apexDroop = &ap->plant->apDroop[apDroopId];

					ApexDroopPointer apDroop = &ap->plant->apDroop[apDroopId];
					/*/**/
					HERE;
					produce

					Bud(bu)GetPos(x,y,z) 
					Internode(&ap->plant->internodes[iId])GetHead(x,y,z)GetUp(ux,uy,uz)		

					SetHead(head[0],head[1],head[2],up[0],up[1],up[2])
					Apex(ap)
					SB
					ApexDroop(apDroop)
					GetHead(dX,dY,dZ)GetPos(x,y,z)GetUp(ux,uy,uz)
					EB
					;
/*
				//BudData bu1;
				//createBud(ap, bu1);

				
				//ap->lastInternode = &ap->plant->internodes[iId];
				HERE;
			produce  
				//GetHead(dx,dy,dz)
				Bud(bu1)GetPos(x,y,z) 
				Bud(bu)GetPos(x,y,z) 
				Internode(&ap->plant->internodes[iId])//GetPos(x,y,z)
				//SetHead(dX,dY,dZ,up[0],up[1],up[2])
				SetHead(head[0],head[1],head[2],up[0],up[1],up[2])
				Apex(ap)
				;*/
			}
			else{
				
			//ap->lastInternode = &ap->plant->internodes[iId];
			HERE;
			produce  
				//GetHead(dx,dy,dz)
				//Bud(bu1)GetPos(x,y,z) 
				Bud(bu)GetPos(x,y,z) 
				Internode(&ap->plant->internodes[iId])GetHead(x,y,z)GetUp(ux,uy,uz)//GetPos(x,y,z)
				//SetHead(dX,dY,dZ,up[0],up[1],up[2])
				SetHead(head[0],head[1],head[2],up[0],up[1],up[2])
				Apex(ap);
			}

		}
		else
			{
				int iId = ap->plant->createInternode(ap, false);
				ap->lastInternode = &ap->plant->internodes[iId];
				HERE;
		produce  		
			//Bud(bu)GetPos(x,y,z) 
			Internode(&ap->plant->internodes[iId])GetHead(x,y,z)GetUp(ux,uy,uz)
			//SetHead(dX,dY,dZ,up[0],up[1],up[2])
			SetHead(head[0],head[1],head[2],up[0],up[1],up[2])
			Apex(ap);
		}
	
	}
	else
	{
		if(ap->plant->branches[ap->id].oldVigor > 2.3f//(photo(ap->plant->branches[ap->id].branchApex->light, 2.5f, 250.0f) > ran(0.9f) 
			//&& (Min(getBranchDistance(ap->id),20) < ran(30.0f) )
			//&& ran(1.0f) > 0.5f
			//&& ap->growthSpeed > ran(1.0f)) 
			)
		{
			//ap->plant->branches[ap->id].oldVigor -= 1.4f;
			if(ap->plant->dominance) tmp = -1;
			else tmp =0;

			int iId = ap->plant->createInternode(ap, false);

			ap->lastInternode = &ap->plant->internodes[iId];
			HERE;
		produce  		
			//SetHead(dX,dY,dZ,up[0],up[1],up[2])
			Internode(&ap->plant->internodes[iId])GetHead(x,y,z)GetUp(ux,uy,uz)
			SetHead(head[0],head[1],head[2],up[0],up[1],up[2])
			Apex(ap);
		}
		else{

			world.updateVoxels(voxel_x,voxel_y,voxel_z, -3.4f/(float)(Max(ap->order,1)));
			//Printf("trunk\n");
			//ap->lastInternode = &ap->plant->internodes[iId];
			HERE;
			produce  		
			//SetHead(dX,dY,dZ,up[0],up[1],up[2])
			//Internode(in)GetPos(x,y,z)
			SetHead(head[0],head[1],head[2],up[0],up[1],up[2])
			Apex(ap);
		}
	}	
}

ApexDroop(ap)>GetHead(dX,dY,dZ)GetPos(x,y,z)GetUp(ux,uy,uz):
{
	//Printf("Debug \n");
	
	float head[3], rot_axxis[3], up[3], next_x, next_y, next_z, next[3];
	int voxel_x, voxel_y, voxel_z;


	queryVoxel(x,y,z, &voxel_x, &voxel_y, &voxel_z);

	ap->VoxelPosition[0] = voxel_x;
	ap->VoxelPosition[1] = voxel_y;
	ap->VoxelPosition[2] = voxel_z;

	if(dY > 0.5f)
	{
		lookupLocalVoxels(voxel_x, voxel_y, voxel_z, dX, dY, dZ, &next_x, &next_y, &next_z, &ap->light, -100.15f, 3000.0f, -100.0f, -1000.0f);
	}
	else	lookupLocalVoxels(voxel_x, voxel_y, voxel_z, dX, dY, dZ, &next_x, &next_y, &next_z, &ap->light, 100.15f, 3000.0f);
		

	next[0]=next_x; next[1]=-1.0f; next[2]=next_z;

	head[0]=dX; head[1]=dY; head[2]=dZ;
	up[0]=ux; up[1]=uy; up[2]=uz;
 		
		
	cross(head, next, rot_axxis);

	R(0.25f, rot_axxis, head,0);

	int iId = ap->plant->createInternodeDroop(ap, false);
	if(iId > MAX_INTERNODESDROOP-1) return;

	ap->age ++;

	if(/*ap->age < 25 && (ran(1.0f) > 0.5f) &&*/ ap->age < 9 && (dY < 0.0f || ran(1.0f) > 0.9f)) {
		HERE;
		nproduce InternodeDroop(&ap->plant->internodesDroop[iId])GetHead(dX,dY,dZ)GetUp(ux,uy,uz);

	if(/*ran(1.0f) > 0.15f &&*/ ap->plant->branches[ap->parentId].branchApex != NULL && ap->order< 2 && 0)
	{
	HERE;
		nproduce 
			SetHead(head[0],head[1],head[2],up[0],up[1],up[2])
					ApexDroop(ap);

		R(1.5f, rot_axxis, head,0);

		int apDroopId = ap->plant->generateDroopId();
					ap->plant->apDroop[apDroopId].id = apDroopId;
					ap->plant->apDroop[apDroopId].age=0;
					ap->plant->apDroop[apDroopId].order = ap->plant->branches[ap->id].order + 1;
					ap->plant->apDroop[apDroopId].parentId = ap->id;
					ap->plant->apDroop[apDroopId].num_childBuds = 0;
					ap->plant->apDroop[apDroopId].lastChildNumber = 0;
					//ap->plant->apDroop[apDroopId].lastInternode = &ap->plant->internodes[iId];


					ap->plant->apDroop[apDroopId].VoxelPosition[0] = voxel_x;
					ap->plant->apDroop[apDroopId].VoxelPosition[1] = voxel_y;
					ap->plant->apDroop[apDroopId].VoxelPosition[2] = voxel_z;

					//ap->apexDroop = &ap->plant->apDroop[apDroopId];

					ApexDroopPointer apDroop = &ap->plant->apDroop[apDroopId];
					/*/**/
					HERE;
					produce		

					
					SB
					Left(-45.0f + ran(90.0f))
					ApexDroop(apDroop)
					GetHead(dX,dY,dZ)GetPos(x,y,z)GetUp(ux,uy,uz)
					EB
					;
	}
	}

	
	HERE;
	produce
		//InternodeDroop(&ap->plant->internodesDroop[iId])
		
		SetHead(head[0],head[1],head[2],up[0],up[1],up[2])
		//MoveTo(x,y, z)
		//f(10.0f)
		ApexDroop(ap);
		
}

InternodeDroop(in):
{
	//Printf("InternodDroop \n");
	produce
		InternodeDroop(in);

}

Bud(bu)GetPos(x,y,z):
{
	if(bu.budmode == longNode ||  bu.budmode == wait ){
	//if(bu.age == 5)bu.budmode=longNode;
	//else {bu.age++; bu.budmode=wait;}
	//if(ran(1.0f)>0.9f)bu.budmode=longNode;
	//else bu.budmode=wait;
	if(bu.plant->sapling){
	if(steps%14==0 /*&& placeBud(x,y,z,15.0f)*/) bu.budmode=longNode;
	else bu.budmode=wait;
	}
	//if(bu.id==1) {bu.budmode=wait; bu.id++;}
	//if((bu.sleepCycles--) > 0) bu.budmode=wait;
	//else  bu.budmode = longNode; 
	//Printf("sleepCycles %i \n", bu.sleepCycles);
	
	}
	
	float dx=0.f, dy=0.0f, dz=0.0f, h1=0.0f, h2=0.0f, h3=0.0f, u1=0.0f, u2=0.0f, u3=0.0f;
	//Printf("BUD turtle position: %f %f %f \n",x,y,z);

	if(bu.budmode != wait //&& world.voxels[bu.plant->branches[bu.id].VoxelPosition[0]][bu.plant->branches[bu.id].VoxelPosition[1]]
	//[bu.plant->branches[bu.id].VoxelPosition[2]].light < 100.0f
	)
	{
		//if(bu.id > MAX_UNITS/2)Printf("bu id %i \n", bu.id);
		

		int iId = bu.plant->generateInternodeId();
		if(iId > MAX_INTERNODES-1) return;

		bu.plant->internodes[iId].id = iId;
		bu.plant->internodes[iId].age=0;
		bu.plant->internodes[iId].iLength = root[0].iLength * 0.15f;
		bu.plant->internodes[iId].iWidth = 0.7f;
		bu.plant->internodes[iId].iRadialGrowth = bu.plant->radialGrowth;
		bu.plant->internodes[iId].isLast = false;
		
		

		int iId1 = bu.plant->generateInternodeId();
		if(iId1 > MAX_INTERNODES-1) return;

		bu.plant->internodes[iId1].id = iId1;
		bu.plant->internodes[iId1].age=1;
		bu.plant->internodes[iId1].iLength = root[0].iLength * 0.15f;
		bu.plant->internodes[iId1].iWidth = 0.7f;
		bu.plant->internodes[iId1].iRadialGrowth = bu.plant->radialGrowth;
		bu.plant->internodes[iId1].isLast = false;
		bu.plant->internodes[iId1].lastInternode = &bu.plant->internodes[iId];


		bu.plant->ap[bu.id];
		bu.plant->ap[bu.id].age=0;
		bu.plant->ap[bu.id].budAngle = apex[0].budAngle;
		bu.plant->ap[bu.id].lastBudAngle = 0.0f;
		bu.plant->ap[bu.id].order = bu.plant->branches[bu.id].order;
		bu.plant->ap[bu.id].lastBudId=0;
		bu.plant->ap[bu.id].id=bu.id;
		bu.plant->ap[bu.id].parentId = bu.parentId;
		bu.plant->ap[bu.id].num_childBuds = 0;
		bu.plant->ap[bu.id].lastChildNumber = 0;
		bu.plant->ap[bu.id].lastInternode = &bu.plant->internodes[iId1];
		/*if(ran(1.0f)>0.95f) bu.plant->ap[bu.id].indeterminate=true;
		else bu.plant->ap[bu.id].indeterminate=false;*/
		bu.plant->ap[bu.id].apicalDominanceFactor = 0.0f;

		bu.plant->ap[bu.id].plant = bu.plant;

		bu.plant->internodes[iId].branch = &bu.plant->branches[bu.id];
		bu.plant->internodes[iId1].branch = &bu.plant->branches[bu.id];
		

		int voxel_x, voxel_y, voxel_z;
		float next_x, next_y, next_z;

		queryVoxel(x,y,z, &voxel_x, &voxel_y, &voxel_z);

		bu.plant->ap[bu.id].VoxelPosition[0] = voxel_x;
		bu.plant->ap[bu.id].VoxelPosition[1] = voxel_y;
		bu.plant->ap[bu.id].VoxelPosition[2] = voxel_z;

		//world.updateVoxels(voxel_x,voxel_y,voxel_z, 2.0f / bu.plant->ap[bu.id].order);

		//if(!bu.plant->dominance)lookupLocalVoxels(voxel_x, voxel_y, voxel_z, 0.0f, 1.0f, 0.0f, &next_x, &next_y, &next_z, &bu.plant->ap[bu.id].light, 0.15f, 2.5f);
		//else lookupLocalVoxels(voxel_x, voxel_y, voxel_z, 0.0f, 1.0f, 0.0f, &next_x, &next_y, &next_z, &bu.plant->ap[bu.id].light, 0.15f, -3.0f);
		
		if(!bu.plant->dominance)lookupLocalVoxels(voxel_x, voxel_y, voxel_z, 0.0f, 1.0f, 0.0f, &next_x, &next_y, &next_z, &bu.plant->ap[bu.id].light, bu.plant->params.plantMatureStrategy[0], bu.plant->params.plantMatureStrategy[1]);
		else lookupLocalVoxels(voxel_x, voxel_y, voxel_z, 0.0f, 1.0f, 0.0f, &next_x, &next_y, &next_z, &bu.plant->ap[bu.id].light, bu.plant->params.plantSaplingStrategy[0], bu.plant->params.plantMatureStrategy[1]);
		
		/*if(bu.budmode == longNode)
		{
			if(bu.plant->branches[bu.id].parentApex == NULL) Printf("ID ERROR!\n");
			bu.plant->branches[bu.id].branchmode = longShoot;

			if( bu.plant->dominance) 
			{
				bu.plant->ap[bu.id].growthSpeed =  bu.plant->branchGrowthSpeed  ;// bu.plant->ap[bu.id].order;//photo(bu.plant->ap[bu.id].light, 2.5f, 20.0f);//ran(1.0f);
				bu.plant->ap[bu.id].branchProb = 0.25f / bu.plant->ap[bu.id].order;
			}
			else  
			{
				bu.plant->ap[bu.id].growthSpeed =  bu.plant->branchGrowthSpeed ;// bu.plant->ap[bu.id].order;
				bu.plant->ap[bu.id].branchProb = 0.25f / bu.plant->ap[bu.id].order;
			}
		}
		else
		{
			//Printf("shortNode\n");
			bu.plant->ap[bu.id].growthSpeed = bu.plant->branchGrowthSpeed * 0.5f;
			bu.plant->ap[bu.id].branchProb = 0.25f;
		}*/

		
		ApexPointer AP = &bu.plant->ap[bu.id];

		//BranchPointData bpd;
		//bu.plant->bpd[bu.id].age=0;
		//bu.plant->bpd[bu.id].iLength = root.iLength;
		//bu.plant->bpd[bu.id].iWidth = root.iWidth;
		//bu.plant->bpd[bu.id].iRadialGrowth = root.iRadialGrowth;
		bu.plant->bpd[bu.id].cut=false;
		bu.plant->bpd[bu.id].branchApex = &bu.plant->ap[bu.id];

		bu.plant->bpd[bu.id].branch = &bu.plant->branches[bu.id];

		bu.plant->bpd[bu.id].internode = &bu.plant->internodes[iId];
		bu.plant->bpd[bu.id].parentInternode = bu.parentInternode;

		bu.plant->bpd[bu.id].previousBranchPoint = bu.plant->branches[bu.plant->branches[bu.id].parentApex->id].branchPoint;

		BranchPointer BP = &bu.plant->bpd[bu.id];

		bu.plant->branches[bu.id].branchApex = &bu.plant->ap[bu.id];
		bu.plant->branches[bu.id].branchPoint = &bu.plant->bpd[bu.id];

		

		//Printf("bud production of apex %i and internode %i \n", AP->id, bu.plant->bpd[bu.id].internode->id);
		HERE;
		produce
			
			SB()
			BranchPoint(BP)//Temp(AP)
			RollToVert()
			RollL(bu.positionAngle)
			Left(bu.direction * 0.33f)
			//RollToVert()
			RollR(bu.positionAngle)

			Internode(&bu.plant->internodes[iId])
			
			RollL(bu.positionAngle)
			Left(bu.direction * 0.33f)
			RollR(bu.positionAngle)
			Internode(&bu.plant->internodes[iId1])

			RollL(bu.positionAngle)
			Left(bu.direction * 0.33f)
			RollR(bu.positionAngle)
			Internode(&bu.plant->internodes[iId1])

			/*RollL(bu.positionAngle)
			Left(bu.direction * 0.25f)
			RollR(bu.positionAngle)
			Internode(&bu.plant->internodes[iId1])
				
			//RollL(bu.positionAngle)
			Left(bu.direction * 0.25f)
			//RollR(bu.positionAngle)
			Internode(&bu.plant->internodes[iId])*/
			//RollR(bu.positionAngle)
			GetHead(dx,dy,dz)GetUp(x,y,z) //GetPos(x,y,z)//
			//RollL(90.0f)
			Apex(AP)GetHead(dx,dy,dz)GetPos(x,y,z)GetUp(dx,dy,dz)GetLeft(u1,u2,u3)
			EB()
			/*SB()
			Right(bu.direction)
			BranchPoint(bp)
			Internode(in)GetPos(x,y,z)GetHead(dx,dy,dz) Apex(ap)GetHead(dx,dy,dz)GetPos(x,y,z)GetUp(dx,dy,dz)
			EB()*/;
		
	}
	else
	{
	HERE;
		produce
			Bud(bu)GetPos(x,y,z);
	}
	 	
}

MouseIns():
{
	produce ;
}

MouseInsPos(mousestatus):
{
	Printf("IsActive \n");
	world.isActive = !world.isActive;

	produce ;
}

Seed(s):
{
	Printf("Seed\n");
	world.seedCount++;
	DX=DZ=0.0f;
	DY = 1.0f;
	int tex;
	produce 
		MoveTo3f(s.position)
		
		SB()CurrentTexture(world.activeSpecies + 6)SetHead(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f)
		BranchPoint(branchPointer[world.seedCount-1])Internode(rootPointer[world.seedCount-1])GetHead(DX,DY,DZ)GetUp(UX,UY,UZ)
		Apex(apexPointer[world.seedCount-1])GetHead(DX,DY,DZ)GetPos(X,Y,Z)GetUp(UX,UY,UZ)GetLeft(LX, LY, LZ)EB();
}

Spawn():
{
	V3f position(0,0,0), seedPos(0.0f,0.0f,0.0f);
	int i;
	//Printf("freeId 0 plant 0 %i, plant %i\n", plant[0].freeIds[0], plant[1].freeIds[0]);
	//world.spawn = false;
	
	if(world.seedGrass) {nproduce Grass(); world.seedGrass = false;}

	if(world.spawn){
		world.spawn = false;
			
			nproduce Spawn();
			if (world.activeSpecies == 0)
				nproduce PovRayStart("Pov_Tree_Species0_",Single);
			else if (world.activeSpecies == 1)
				nproduce PovRayStart("Pov_Tree_Species1_",Single);
			else if (world.activeSpecies == 2)
				nproduce PovRayStart("Pov_Tree_Species2_",Single);
			else
				nproduce PovRayStart("Pov_Tree_UnClassified_",Single);
			HERE;
			produce	Seed(seed[world.seedCount]);
			}
	

	if(world.randomSeed ){
		if(world.seedCount >= /*MAX_PLANTS_SMALL*/ MAX_PLANTS -1){world.randomSeed = false; /*world.spawn = false;*/}
		
			
		//nproduce Spawn();
		
		if(!world.seedCount) {position.x = houseX + 50.0f; position.z = 0.0f;position.y = 2.0f*1920.0f; }
			//position.x = 0.0f; position.z = 350.0f;position.y = 2.0f*1920.0f; }
		else if(world.seedCount ==1){position.x = house3X - 20.0f; position.z = 0.0f;position.y = 2.0f*1920.0f; }
		else{
			//if(ran(1.0f)>0.5f){
			position.x = ran(2.0f*1220.0f) - 1220.0f;
			position.y = 2.0f*1920.0f;
			position.z = ran(2.0f*1220.0f) - 1220.0f;
		}
			
			Printf("RanSpawn\n");
			
			if(terrainHeightAt(position, seedPos)){
				//if(seedPos.y > 55.0f /*&& seedPos.y <250.0f*/  ){
					//for(i=0; i<world.seedCount; i++) 
					//	if(seedPos.x - seed[i].position.x < 10.0f || seedPos.z - seed[i].position.z < 10.0f)
					//		break;
				Printf("Seeding %f %f %f \n", seedPos.x, seedPos.y, seedPos.z);
				//int temp = (int)ran((float) MAX_SPECIES); Printf("active species %i \n", temp); 
				float r=ran(1.0f);

				if(r > 0.8f && !(world.bigPlantsCount >= MAX_PLANTS -1)) world.activeSpecies = 0; 
				else if(r > 0.65f) world.activeSpecies= 1; 
				else world.activeSpecies = 2;

				if(!world.seedCount){world.activeSpecies = 1; seedPos.y = 300.0f;}
				else if(world.seedCount == 1){world.activeSpecies = 0; seedPos.y = 300.0f;}

				seedPlant(seedPos , params[world.activeSpecies]);
				//}
				//produce Spawn() Seed(seed[world.seedCount]);
			}
			else Printf("OUTSIDE\n");
			//}
		//}
		HERE;
		produce Spawn();
	}
	//else 
			HERE;
	produce Spawn();
}

Terrain(t):
{
	cameraPos = GetCameraPosition(0);
	produce Terrain(cameraPos);
}

VoxelCut(last):
{
	if(world.showVoxelBounds)
					produce VoxelCut(last);
	else if(!last)	produce Cut();
	else			produce VoxelBounds() Cut();
}

interpretation: 

maximum depth: 1;

Internode(id):
{
	//if(!id->branch->order) nproduce ContourSides(16);
	//else nproduce ContourSides(3);
	//if(steps >=53)Printf("Here");
	//if(steps >=53 && id->branch == NULL )Printf("Here");
	if(id->branch != NULL &&id->branch->branchApex != NULL /*&& id->branch->plant->params.leafMode > 0*/ && world.showLeaves ){
		//if(steps >=53)Printf("before leaf2");
		if(id->branch->plant->params.leafMode ==0 && id->age < 5)
		{
		int r= (int)ran(7.9f);
		if(r > 3)
		{
			if(r < 6) r = 1;
			else r = r - 4;
		}
		else r = 0;
		/*if(id->age == 0)
		{
			nproduce 
				SB f(7.0f) Left(70.0f + ran(10.0f) - 5.0f)RollR(45.0f + ran(10.0f) - 5.0f)Down(id->branch->leafDownAngle) Surface(r,7.5f) EB
				SB f(7.0f) Right(70.0f + ran(10.0f) - 5.0f)RollL(45.0f + ran(10.0f) - 5.0f)Down(id->branch->leafDownAngle)  Surface(r,7.5f) EB
				SB Left(70.0f + ran(10.0f) - 5.0f)RollR(45.0f + ran(10.0f) - 5.0f)Down(id->branch->leafDownAngle) Surface(r,7.5f) EB
				SB Right(70.0f + ran(10.0f) - 5.0f)RollL(45.0f + ran(10.0f) - 5.0f)Down(id->branch->leafDownAngle)  Surface(r,7.5f) EB
				;
		}
		else*/ if(id->age % 1 == 0)
		{
			if(id->branch->branchApex->headY > 0.0f)
			nproduce 
			SB Left(70.0f + ran(20.0f) - 10.0f)RollR(45.0f + ran(20.0f) - 10.0f)Down(id->branch->leafDownAngle) Surface(r,7.5f) EB
			SB Right(70.0f+ ran(20.0f) - 10.0f)RollL(45.0f + ran(20.0f) - 10.0f)Down(id->branch->leafDownAngle) Surface(r,7.5f) EB
			;
			else
			nproduce 
			SB Left(70.0f + ran(20.0f) - 10.0f)Down(id->branch->leafDownAngle)   Surface(r,7.5f) EB
			SB Right(70.0f + ran(20.0f) - 10.0f)Down(id->branch->leafDownAngle) Surface(r,7.5f) EB
			;
		}
		else if(id->age % 2 == 3)
		{
			if(id->branch->branchApex->headY > 0.0f)
			nproduce 
			SB Right(70.0f + ran(10.0f) - 5.0f)RollL(45.0f + ran(10.0f) - 5.0f)Down(id->branch->leafDownAngle)  Surface(r,7.5f) EB
			;
			else 
			nproduce 
			SB Right(70.0f + ran(10.0f) - 5.0f)Down(id->branch->leafDownAngle + ran(10.0f) - 5.0f)   Surface(r,7.5f) EB;
		}
		}

		if(id->branch->plant->params.leafMode ==2 && id->age < 70)
		{
		int r= (int)ran(7.9f);
		if(r > 4)
		{
			if(r < 7) r = 1;
			else r = 2;
		}
		else r = 3;

		if(id->age % 4 == 0)
			if(id->branch->branchApex->headY > 0.0f)
			nproduce 
			SB /*SetColor(40)*/Left(70.0f)RollR(45.0f)Down(id->branch->leafDownAngle) Surface3(r,2.5f, 14.5f, 13.5f) EB
			//SB /*SetColor(40)*/Right(70.0f)RollL(45.0f)Down(id->branch->leafDownAngle) Surface(r,7.5f) EB
			;
			else
			nproduce 
			SB /*SetColor(40)*/Left(70.0f)Down(id->branch->leafDownAngle)  Surface3(r,2.5f, 14.5f, 13.5f) EB;
			//SB /*SetColor(40)*/Right(70.0f)Down(id->branch->leafDownAngle) Surface(r,7.5f) EB;
		else if(id->age % 4 == 2)
			if(id->branch->branchApex->headY > 0.0f)
			nproduce 
			SB /*SetColor(40)*/Right(70.0f)RollL(45.0f)Down(id->branch->leafDownAngle) Surface3(r,2.5f, 14.5f, 13.5f) EB
			;
			else 
			nproduce 
			SB /*SetColor(40)*/Right(70.0f)Down(id->branch->leafDownAngle)  Surface3(r,2.5f, 14.5f, 13.5f) EB;
		}

		else if(id->branch->plant->params.leafMode ==1 && id->age < 16) 
		{
			int r= (int)ran(7.9f);
			if(r > 3)
		{
			if(r < 6) r = 0;
			else r = 2;
		}
		else r = 1;

		if(id->age % 2 == 0){
			if(id->branch->branchApex->headY > 0.0f)
			nproduce 
			SB /*SetColor(40)*/Left(70.0f + ran(10.0f))RollR(45.0f )Down(id->branch->leafDownAngle + ran(20.0f)) Surface3(r,3.4f, 3.4f, 3.4f) EB
			//SB /*SetColor(40)*/Right(70.0f)RollL(45.0f)Down(id->branch->leafDownAngle) Surface3(r,4.5f, 4.5f, 4.5f) EB
			;
			else
			nproduce 
			SB /*SetColor(40)*/Left(70.0f + ran(10.0f))Down(id->branch->leafDownAngle + ran(20.0f))  Surface3(r,3.4f, 3.4f, 3.4f) EB
			//SB /*SetColor(40)*/Right(70.0f)Down(id->branch->leafDownAngle) Surface3(r,4.5f, 4.5f, 4.5f) EB
			;
		}
		/*else if(id->age % 4 == 2){
			if(id->branch->branchApex->headY > 0.0f)
			nproduce 
			SB Right(70.0f)RollL(45.0f)Down(id->branch->leafDownAngle) Surface3(r,2.5f, 2.5f, 2.5f) EB
			;
			else 
			nproduce 
			SB Right(70.0f)Down(id->branch->leafDownAngle)  Surface3(r,2.5f, 2.5f, 2.5f) EB;
		}*/
		}

	}
	//if(steps >=53)Printf("After");
	if(id->id == 0) 
	{
	produce 

		//SetWidth(id->iWidth) F(id->iLength)
		StartGC()
 		SetWidth(id->iWidth * 1.4f) F(2.0f * id->iLength)
		;
	}
	/*else if(id->age == 0 )
	{
		produce
		//SetWidth(id->iWidth) F(id->iLength)
		EndGC()

 		SetWidth(id->iWidth) F(2.0f * id->iLength)
		;
	}
	else if(id->age < 7)
	{
		produce
			Left(90.0f)
			RollL(30.0f)
		SB Surface(0,5.5f) EB
		RollR(30.0f)
		Right(180.0f)
		RollR(30.0f)
		SB Surface(0,5.5f) EB
		RollL(30.0f)
		Left(90.0f)
 		SetWidth(id->iWidth) F(2.0f * id->iLength)
		;
	}*/
	else if( id->iWidth < 2.1f && world.hideThinBranches && id->branch->plant->params.leafMode == 0)
	{
		if(id->age == 0 )
		{
			//if(id->lastInternode != NULL){
			produce
			//EndGC()
 			SetWidth(id->iWidth) f(2.0f * id->iLength)
			
			;
			/*}
			else {
			produce
			EndGC()
			ContourSides(16)
			StartGC()
			
 			SetWidth(id->iWidth) f(2.0f * id->iLength)EndGC()
			;
			}*/
		}
		produce f(2.0f * id->iLength);
	}
	else if(id->age == 0 )
	{
		//if(id->lastInternode != NULL){
			produce
			
 			SetWidth(id->iWidth) F(2.0f * id->iLength)
			//EndGC()
			;
			/*}
			else {
			produce
			EndGC()
			ContourSides(16)
			StartGC()
			
 			SetWidth(id->iWidth) F(2.0f * id->iLength)EndGC()
			;
			}*/
	}
	else
	{
		produce

		//SetWidth(id->iWidth) F(id->iLength)
		//StartGC()
		//PointGC()
 		SetWidth(id->iWidth) F(2.0f * id->iLength)
		
		//EndGC()
		;
	}

}

Apex(ap):
{
	//if(steps < 150){
		
	if(world.showLeaves)	{

		if(ap->upY < -0.5f) nproduce RollR(120.0f);

		int r= (int)ran(7.9f);
		if(r > 3)
		{
			if(r < 6) r = 1;
			else r = r - 4;
		}
		else r = 0;

		if(ap->plant->params.leafMode == 0){
		if(ap->headY > 0.0f)
		produce
		f(-3.0f)
		SB Down(45.0f + ran(10.0f) - 5.0f)Surface(r,7.5f) EB 

		f(-7.0f)
		SB Left(70.0f)RollR(45.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(r,7.5f) EB	
		SB Right(70.0f)RollL(45.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(r,7.5f) EB

		/*f(-12.0f)
		SB Left(70.0f)RollR(45.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(r,7.5f) EB	
		SB Right(70.0f)RollL(45.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(r,7.5f) EB

		f(-12.0f)
		SB Left(70.0f)RollR(45.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(r,7.5f) EB	
		SB Right(70.0f)RollL(45.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(r,7.5f) EB
		*/
		;
		
		else
		produce
		f(-3.0f)
		SB Surface(r,7.5f) EB 

		f(-7.0f)
		SB Left(70.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(r,7.5f) EB
		SB Right(70.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(r,7.5f) EB
		
		/*f(-12.0f)
		SB Left(70.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(r,7.5f) EB		
		SB Right(70.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(r,7.5f) EB
		
		f(-12.0f)
		SB Left(70.0f)Down(plant->branches[ap->id].leafDownAngle)Surface(r,7.5f) EB		
		SB Right(70.0f)Down(plant->branches[ap->id].leafDownAngle)Surface(r,7.5f) EB*/
		;
		}
		else if(ap->plant->params.leafMode == 2)
		{
			if(ap->headY > 0.0f)
			produce
			f(-3.0f)
			SB Surface3(r,2.5f, 14.5f, 13.5f) EB 			
			;

			else
			produce
			f(-3.0f)
			SB /*SetColor(1)*/Down(45.0f)Surface3(r,2.5f, 14.5f, 13.5f) EB
			;
		}
		else if(ap->plant->params.leafMode == 1)
		{
			if(ap->headY > 0.0f)
			produce
			f(-3.0f)
			SB Surface3(r,2.5f, 2.5f, 2.5f) EB 			
			;

			else
			produce
			f(-3.0f)
			SB /*SetColor(1)*/Down(45.0f)Surface3(r,2.5f, 2.5f, 2.5f) EB
			;
		}
		
		
		/*}
		else {
			produce
		//EndGC()
		f(-3.0f)
		SB Surface(0,4.5f) EB

		f(-7.0f)
		SB Left(90.0f)RollR(45.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(0,4.5f) EB
		
		SB Right(90.0f)RollL(45.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(0,4.5f) EB
		
		f(-12.0f)
		SB Left(90.0f)RollR(45.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(0,4.5f) EB		
		
		SB Right(90.0f)RollL(45.0f)Down(plant->branches[ap->id].leafDownAngle) Surface(0,4.5f) EB
		
		f(-12.0f)
		SB Left(90.0f)RollR(45.0f)Down(plant->branches[ap->id].leafDownAngle)Surface(0,4.5f) EB
		
		
		SB Right(90.0f)RollL(45.0f)Down(plant->branches[ap->id].leafDownAngle)Surface(0,4.5f) EB;
		}*/
	}
}
Grass():
{
	int i;
	float r1, r2;
	for(i=0; i< MAX_GRASS -1; i++){
		r1 = ran(90.0f) - 45.0f;
		r2 = ran(90.0f) - 45.0f;
	nproduce SB MoveTo(world.grass[i].x, world.grass[i].y, world.grass[i].z)SetHead(0,1,0,1,0,0)RollL(r1)Left(r2)Surface3(0,1.0f, 10.5f, 10.5f) EB;
	}

	produce SB MoveTo(world.grass[MAX_GRASS-1].x, world.grass[MAX_GRASS-1].y, world.grass[MAX_GRASS-1].z)SetHead(0,1,0,1,0,0)Surface3(0,1.0f, 10.5f, 10.5f) EB;
}

InternodeDroop(id):
{
	produce 
		Left(90.0f)
			RollL(30.0f)
		SB Surface(0,4.5f) EB
		RollR(30.0f)
		Right(180.0f)
		RollR(30.0f)
		SB Surface(0,4.5f) EB
		RollL(30.0f)
		Left(90.0f)
		SetWidth(id->iWidth) F( id->iLength);
}

ApexDroop(ap):
{
	produce 
		SetColor(1)
		f(-3.0f)
		SB Surface(0,4.5f) EB

		f(-7.0f)Left(90.0f)Down(plant->branches[ap->id].leafDownAngle)
		SB Surface(0,4.5f) EB
		Up(plant->branches[ap->id].leafDownAngle)
		
		Right(180.0f)Down(plant->branches[ap->id].leafDownAngle)
		SB Surface(0,4.5f) EB
		Up(plant->branches[ap->id].leafDownAngle)Left(90.0f)
		SetColor(40);
	//}
}

VoxelBounds():
{
	if(world.showVoxelBounds){
	produce
		VoxelCut(false)
		SB SetWidth(12.0f)
		MoveTo3f(V3f(((float)VOXEL_DENSITY * (-10.0f)), 0.0f,  ((float)VOXEL_DENSITY * (-10.0f))))
		SetHead(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 SetHead(0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 SetHead(-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)F((float)VOXEL_DENSITY * 20.0f)
		 SetHead(0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 EB
		 VoxelCut(false)SB SetWidth(20.0f)
		MoveTo3f(V3f(((float)VOXEL_DENSITY * (-10.0f)), 0.0f,  ((float)VOXEL_DENSITY * (-10.0f))))
		SetHead(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 SetHead(0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 SetHead(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f)F((float)VOXEL_DENSITY * 20.0f)
		 SetHead(0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 EB
		 VoxelCut(false) SB SetWidth(20.0f)
		MoveTo3f(V3f(((float)VOXEL_DENSITY * (10.0f)), 0.0f,  ((float)VOXEL_DENSITY * (-10.0f))))
		SetHead(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 SetHead(0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 SetHead(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f)F((float)VOXEL_DENSITY * 20.0f)
		 SetHead(0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 EB
		 VoxelCut(true) SB SetWidth(20.0f)
		MoveTo3f(V3f(((float)VOXEL_DENSITY * (-10.0f)), 0.0f,  ((float)VOXEL_DENSITY * (10.0f))))
		SetHead(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 SetHead(0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 SetHead(-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f)F((float)VOXEL_DENSITY * 20.0f)
		 SetHead(0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f)F((float)VOXEL_DENSITY * 20.0f) 
		 EB;
	}
	//else produce VoxelBounds();
}

group 1:
production:

Internode(id)://>Internode(id1):
{
	float rad;
	
	//if(id->lastInternode != NULL) id->iWidth = id->lastInternode->iWidth + 0.9f;
	//else id->iWidth = /*(float)(id.age)/id.iRadialGrowth +*/ branch->plant->radialWidth;
	//if(id->iRadialGrowth == 0.0f) id->iRadialGrowth = branch->plant->radialWidth;
	id->iWidth += (rad=calcNextRadius(id->iWidth, 2.0f * PI * id->iWidth * id->iWidth + id->iRadialGrowth));
	
	if(id->lastInternode != NULL) 
	{
		id->lastInternode->iRadialGrowth += id->iRadialGrowth;
		//id->lastInternode->otherPipes += id->otherPipes;
		
		//Printf("order %i id %i int id %i radialgrowth %f + %f and width %f last int %i has radial %f\n", id->branch->order, id->branch->id, id->id,
		//id->iRadialGrowth, rad, id->iWidth, id->lastInternode->id, id->lastInternode->iRadialGrowth);

		id->iRadialGrowth = 0.0f;
	}
	else {
		
		//Printf("Debug");
		//Printf("enter\n");
		//Printf("order %i id %i int id %i radialgrowth %f + %f and width %f\n", id->branch->order, id->branch->id, id->id,
		//id->iRadialGrowth, rad, id->iWidth);
		//id->iWidth = id->iRadialGrowth + 0.1f;
	}	
	//Printf("first %i second %i \n", first, second);
	//id->iWidth = /*(float)(id.age)/id.iRadialGrowth +*/ branch->plant->radialWidth;
	//id1.iWidth = ((float)(id1.age)/id1.iRadialGrowth + branch->plant->radialWidth);
	//Printf("Hallo\n");
	//id.iWidth = 
	//id->otherPipes = 0.0f;
	produce Internode(id);//Internode(id1)GetPos(x1,y1,z1);
}

BranchPoint(bp):
{
	float rad;
	//Printf("branchpoint with internode %i\n", bp->internode->id);
	if(bp->parentInternode != NULL)
	{
		
		//if(bp->parentInternode->iRadialGrowth == 0.0f) bp->parentInternode->iRadialGrowth = branch->plant->radialWidth;
		if(bp->previousBranchPoint->parentInternode == NULL) bp->parentInternode->iRadialGrowth += bp->internode->iRadialGrowth * 0.79f;
		else bp->parentInternode->iRadialGrowth += bp->internode->iRadialGrowth;
			//bp->internode->iRadialGrowth+(rad=calcNextRadius(bp->internode->iWidth, 2.0f * PI * bp->internode->iWidth * bp->internode->iWidth + 0.25f));
		//Printf("parent internode order %i id %i int id %i gets %f from id %i and has radial %f\n", bp->parentInternode->branch->order, bp->parentInternode->branch->id,
			//bp->parentInternode->id, rad, bp->internode->id, bp->parentInternode->iRadialGrowth);
		//bp->parentInternode->otherPipes += bp->internode->otherPipes + 0.1f;
		 bp->internode->iRadialGrowth = 0.0f;
	}
	bp->branch->plant->PropagateEnergyToChildren(bp->branch->plant->branches[bp->branchApex->id].energy, bp->branchApex->id);
	
	//else Printf("lastInternode\n");

	//if(bp->branchApex != NULL) bp->energy += bp->branchApex->directEnergy;

	//if(bp->previousBranchPoint != NULL) bp->previousBranchPoint->energy += bp->energy;
	
	//bp->upkeepEnergyRatio = bp->energy / Max(branch->plant->branches[bp->branchApex->id].num_childBranches + 1 , 1);

	if(bp->cut){
		if( (--bp->cutDelay) < 0){
			//bp->branch->plant->freeIds[bp->branch->plant->lastFreeId++] = bp->branchApex->id;
			bp->branch->plant->branches[bp->branchApex->id].branchApex = NULL;
			bp->branch->plant->branches[bp->branchApex->id].branchPoint = NULL;

			//bp->parentInternode->isLast = true;

		produce Cut();
		}}
	else {
		produce BranchPoint(bp);}
}

Apex(ap):
{
	produce Apex(ap);
}


Bud(b):
{

	produce Bud(b);
}

group 2:
production:

Internode(id):
{
	produce Internode(id);
}

BranchPoint(bp):
{
	produce BranchPoint(bp);
}

Apex(ap):
{
	produce Apex(ap);
}

Bud(b):
{
	produce Bud(b);
}

GetPos(x, y, z):
{
	produce GetPos(x,y,z);
}
GetHead(x, y, z):
{
	produce GetHead(x,y,z);
}
GetUp(x, y, z):
{
	produce GetUp(x,y,z);
}

GetLeft(lx, ly, lz):
{
	produce GetLeft(lx,ly,lz);
}




axiom: 
SetColor(1)
SB() f(2300.0f) EB()
TextureVCoeff(0.003f)
ContourSides(8)
VoxelBounds()
Spawn()
PovRayStart("DELETE_ME",Instance);


derivation length: 150;