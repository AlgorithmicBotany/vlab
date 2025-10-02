#include <lpfgall.h>
#include "quaternion.hpp"
#include "hlu.hpp"

const float INIT_LEAF_AREA = 0.1f;                            /* initial leaf area (cm^2) */
const float LEAF_RAPID_EXPANSION = 3.3f;                      /* leaf growth rate parameter (days) */
const float INTER_RAPID_EXPANSION = 0.5*LEAF_RAPID_EXPANSION; /* tau should be half for internodes - 5.53 is for leaves. */
const float INTER_DENSITY = 0.1;                             /* density of internodes (kg cm^-3) */
const float PHYLLOCHRON = 1.2f;                               /* days between appearance of leaves */
const float PHYLLOTAXIS = 144.f;                              /* phyllotactic angle */
const float MAX_GROWTH = 240.f;                               /* number of days in season */
const float MAX_NODES = 9.f;                                 /* max number of nodes on a long shoot */
const float DT = 0.1f;                                        /* change in time, integration over days */

const float SHEAR_MODULUS = 10000.0;
const float YOUNG_MODULUS = 50000.0; // GPa 600000.015157
const V3f GRAVITY(0,-9.81,0);
const V3f TROPISM(0,10.0,0);
const float RELAX_MULTIPLIER = 0.1;

/* Biomechanics data */
struct BiomechanicsData
{
  // Geometric properties
  float length;            // Length of internode
  Qf orientChange;         // Change of rotation along the internode
  V3f position;            // Position of the node
  Qf orientQ;              // Orientation at the node

  // Physical properties
  float mass;              // Mass of the internode
  DiagTensor elasticity;   // Elastic properties of the internode
  V3f torque;              // Torque at the node
  V3f torqueGravity;
  V3f torqueTropism;

  // Derived properties
  float massSum;
  Orientation orient;    // It is useful to know the (H,L,U) axes corresponding to orientQ
  Orientation initialOrient; // the orientation before relaxation method; the internode is rotated around this orientation
};

float total_error; /* error in relaxation */
float sim_time;    /* days from budbreak */
int phase;         /* current computational phase - L+C group */

/* computational phases */
#define DEVELOP      1 // development of structure
#define UPDATE       2 // update biomechanics
#define RELAX_FOLD   3 // accumulate forces and torques
#define RELAX_UNFOLD 4 // calculate new positions and orientations

Start:
{
  total_error = 0.f;
  sim_time = 0.f;
  phase = DEVELOP;

  UseView(Visualisation);
  UseView(Biomechanics);
}
StartEach:
{
  UseGroup(phase);
  switch (phase)
  {
    case DEVELOP:
      Backward(); // set derivation to backward for pipe model
      break;
    case UPDATE:
      Forward();
      break;
    case RELAX_FOLD:
      Backward();
      break;
    case RELAX_UNFOLD:
      Forward();
      total_error = 0.0;
      break;
  }
}
EndEach:
{
  switch (phase)
  {
    case DEVELOP:
      if (sim_time < MAX_GROWTH)
        sim_time += DT;
      phase = UPDATE;
      break;
    case UPDATE:
      phase = RELAX_FOLD;
      break;
    case RELAX_FOLD:
      phase = RELAX_UNFOLD;
      break;
    case RELAX_UNFOLD:
      if (total_error > 0.001)
        phase = RELAX_FOLD;
      else
        phase = DEVELOP;
      break;
  }
}

/* Functions */
float Phyllochron (float node)
{
  if (node <= 7)
    return (0.09f);
  else
    return (PHYLLOCHRON*2.f);
}

float LeafExpansionRate (float node)
{
  if (node <= 7)
    return (9.3*PHYLLOCHRON+1.36*(node-7.f));
  else if (node <= 18)
    return (2.3*PHYLLOCHRON + PHYLLOCHRON*node);
  else
    return (20.0*PHYLLOCHRON);
}

float InternodeExpansionRate (float node)
{
  return (Phyllochron(node) + 6.f);
}

float Boltzmann (float node, float initial_value, float final_value, float rapid_expansion, float timing_expansion)
/* used for area of an individual leaf */
{
  float timing_appearance, lower_asymptote;
  float value;

  timing_appearance = Phyllochron(node);

  lower_asymptote = initial_value + (initial_value - final_value) *
    (exp(-(timing_expansion-timing_appearance)/rapid_expansion));

  value = (final_value - lower_asymptote) *
    exp((sim_time-timing_expansion)/rapid_expansion) /
    (1.0 + exp((sim_time-timing_expansion)/rapid_expansion)) +
    lower_asymptote;

  if (value < initial_value)
    return (0.f);
  else
    return (value);
}

float FinalLeafArea (float node)
{
  float d0 = 0.026f;    /* constants in cm^-2 */
  float d1 = -0.00131f; /* fit to data for shoots in */
  float d2 = 0.00012f;  /* high temperature treatment */

  // return area in cm^2
  return (0.5 * node / (d0 + d1*node + d2*node*node));  
}


/* start of L-system code */

/* Module declaration */
module Apex(float);                        /* node number */
module Internode(float,float,float,float); /* node number, final length, length, radius */
module Leaf(float,float);                  /* node number, area */
module AxBud(int,int);                     /* number of leaves, terminated or non-terminated */
module B(BiomechanicsData);                /* biomechanics component */

derivation length: 4000;

Axiom: SB() Right(0) GetHead(0,0,0) GetLeft(0,0,0) GetUp(0,0,0) AxBud(9,0) EB()
       Right(90) f(4) Left(90)
       ;

group DEVELOP:

consider: Internode;

production:

/* growth or development rules */
GetHead(hx,hy,hz) GetLeft(lx,ly,lz) GetUp(ux,uy,uz) Apex(node):
{
  float timing_appearance;

  timing_appearance = Phyllochron(node);

  if (sim_time >= timing_appearance && node < MAX_NODES)
  {
  BiomechanicsData bm_data;

  bm_data.length = 0.1f;
  bm_data.orientChange = Qf(1);
  bm_data.position = V3f(0,0,0); 
  bm_data.orientQ = Qf(1);       

  bm_data.mass = INTER_DENSITY * 3.14159 * 0.1f * 0.1f;
  bm_data.massSum = 0.0;

  DiagTensor InitialInternodeElasticity(0,0,0);
  bm_data.elasticity = InitialInternodeElasticity;
  bm_data.torque = V3f(0,0,0);
  bm_data.torqueGravity = V3f(0,0,0);
  bm_data.torqueTropism = V3f(0,0,0);

  Orientation InitialOrientation(V3f(hx,hy,hz) , V3f(lx,ly,lz) , V3f(ux,uy,uz)); 
  bm_data.orient = InitialOrientation;
  bm_data.initialOrient = InitialOrientation;

    produce Internode(node,10.f,0.1f,0.1f) B(bm_data)
            RollR(144) SB() Leaf(node,0) EB()
            GetHead(hx,hy,hz) GetLeft(lx,ly,lz) GetUp(ux,uy,uz) Apex(node+1);
  }
  else
    produce GetHead(hx,hy,hz) GetLeft(lx,ly,lz) GetUp(ux,uy,uz) Apex(node);
}

// thickening and elongation of internodes
Internode(node,final_length,length,radius) >> SB() Internode(bnode,bfinal,blength,bradius) EB() Internode(rnode,rfinal,rlength,rradius):
{
  float new_length;
  float new_radius;

  new_length = Boltzmann(node,length,final_length,INTER_RAPID_EXPANSION,InternodeExpansionRate(node));
  new_radius = 0.75*(bradius + rradius);

  produce Internode(node,final_length,new_length,new_radius);

}

Internode(node,final_length,length,radius) >> Internode(rnode,rfinal,rlength,rradius):
{
  float new_length;
  float new_radius;

  new_length = Boltzmann(node,length,final_length,INTER_RAPID_EXPANSION,InternodeExpansionRate(node));
  new_radius = 1.15*rradius;

  produce Internode(node,final_length,new_length,new_radius);
}

Internode(node,final_length,length,radius):
{
  float new_length;
  float new_radius;

  new_length = Boltzmann(node,length,final_length,INTER_RAPID_EXPANSION,InternodeExpansionRate(node));
  new_radius = radius;

  produce Internode(node,final_length,new_length,new_radius);
}


Leaf(node,area):
{
  float new_area = Boltzmann(node+1.0,INIT_LEAF_AREA,FinalLeafArea(node+1.0),LEAF_RAPID_EXPANSION,LeafExpansionRate(node));

  produce Leaf(node,new_area);
}

GetHead(hx,hy,hz) GetLeft(lx,ly,lz) GetUp(ux,uy,uz) AxBud(num_leaves,terminated):
// NOTE: the node number does not include bud scales (1-4) and transitional leaves(5-7)
{
  BiomechanicsData bm_data;

  bm_data.length = 1.5f;
  bm_data.orientChange = Qf(1);
  bm_data.position = V3f(0,0,0); 
  bm_data.orientQ = Qf(1);       

  bm_data.mass = INTER_DENSITY * 3.14159 * 0.1f * 1.5f;
  bm_data.massSum = 0.0;

  DiagTensor InitialInternodeElasticity(0,0,0);
  bm_data.elasticity = InitialInternodeElasticity;
  bm_data.torque = V3f(0,0,0);
  bm_data.torqueGravity = V3f(0,0,0);
  bm_data.torqueTropism = V3f(0,0,0);

  Orientation InitialOrientation(V3f(hx,hy,hz) , V3f(lx,ly,lz) , V3f(ux,uy,uz)); 
  bm_data.orient = InitialOrientation;
  bm_data.initialOrient = InitialOrientation;

  // the first four nodes are bud scales, so are not included
  // the next three nodes (5-7) have transitional leaves and reproductive meristems
  // nodes 8-22 have normal leaf structure
  for (int i = 0; i < num_leaves; i++)
  {
    if (terminated == 0)
    {
      bm_data.length = 0.1f;
      bm_data.mass = INTER_DENSITY * 3.14159 * 0.1f * 0.1f;
      nproduce Internode((float)i,10.0f,0.1f,0.1f) B(bm_data);
    }
    else if (num_leaves <= 9)
    {
      bm_data.length = 2.0f;
      bm_data.mass = INTER_DENSITY * 3.14159 * 0.1f * 2.0f;
      nproduce Internode((float)i,2.0f,2.0f,0.1f) B(bm_data);
    }
    else
    {
      bm_data.length = 0.1f;
      bm_data.mass = INTER_DENSITY * 3.14159 * 0.1f * 0.1f;
      nproduce Internode((float)i,10.0f,0.1f,0.1f) B(bm_data);
    }
    
    nproduce RollR(144);

    // produce a branch at the 5th node
    if (i == 5)
    {
      nproduce SB() Down(30) GetHead(0,0,0) GetLeft(0,0,0) GetUp(0,0,0) Apex(1) EB();
    }

    nproduce SB() Leaf((float)i,0.f) EB();
  }

  produce;
}

group UPDATE:

production:

Internode(node,final_length,length,radius) B(bm_data):
{
  bm_data.length = length;

  bm_data.mass = INTER_DENSITY * 3.14159 * radius * length;
  bm_data.massSum = 0.0;

  // rigidity is dependent on direction in which torque is applied

  // first, in H direction, G*PI*r^2*0.5
  float torsional_rigidity = SHEAR_MODULUS * 3.14159 * powf(radius,4.0) * 0.5;

  // second in L,U directions, E*PI*r^4*0.25
  float flexural_rigidity = YOUNG_MODULUS * 3.14159 * powf(radius,4.0) * 0.25;

  DiagTensor InternodeElasticityDirection(1.0/torsional_rigidity,1.0/flexural_rigidity,1.0/flexural_rigidity); 
  bm_data.elasticity = InternodeElasticityDirection;

  produce Internode(node,final_length,length,radius) B(bm_data);
}

group RELAX_FOLD:

consider: B;

B(bm_data) >> SB() B(bbm_data) EB() B(rbm_data):
{
  bm_data.massSum = bbm_data.massSum + rbm_data.massSum + bm_data.mass;

  // gravity and tropism
  bm_data.torqueGravity = rbm_data.torqueGravity + (rbm_data.length * bm_data.orient.H) % (GRAVITY * rbm_data.massSum)
                        + bbm_data.torqueGravity + (bbm_data.length * bm_data.orient.H) % (GRAVITY * bbm_data.massSum);
  bm_data.torqueTropism = (rbm_data.length * bm_data.orient.H) % TROPISM
                        + (bbm_data.length * bm_data.orient.H) % TROPISM;

  bm_data.torque = bm_data.torqueGravity + bm_data.torqueTropism;

  produce B(bm_data);
}

B(bm_data) >> B(rbm_data):
{
  bm_data.massSum = rbm_data.massSum + bm_data.mass;

  // gravity and tropism
  bm_data.torqueGravity = rbm_data.torqueGravity + (rbm_data.length * bm_data.orient.H) % (GRAVITY * rbm_data.massSum);
  bm_data.torqueTropism = (rbm_data.length * bm_data.orient.H) % TROPISM;

  bm_data.torque = bm_data.torqueGravity + bm_data.torqueTropism;

  produce B(bm_data);
}

B(bm_data): 
{
  bm_data.torque = V3f(0,0,0);
  bm_data.massSum = bm_data.mass;

  produce B(bm_data);
}

group RELAX_UNFOLD:

consider: B;

B(lbm_data) << B(bm_data):
{
  // First: torque -> bend

  // Calculate the rotations in {H,L,U} frame
  V3f rotateH = (bm_data.torque * bm_data.orient.H * bm_data.elasticity.H) * bm_data.initialOrient.H;
  V3f rotateL = (bm_data.torque * bm_data.orient.L * bm_data.elasticity.L) * bm_data.initialOrient.L;
  V3f rotateU = (bm_data.torque * bm_data.orient.U * bm_data.elasticity.U) * bm_data.initialOrient.U;

  // Now fullBend is the rotation corresponding to the accumulated torque
  Qf fullBend = axisAngle(rotateH + rotateL + rotateU);

  Qf difference = fullBend - bm_data.orientChange;
  total_error += difference.norm();

  // apply relaxation method
  bm_data.orientChange = (1.0 - RELAX_MULTIPLIER) * bm_data.orientChange +
                                RELAX_MULTIPLIER * fullBend;
  bm_data.orientChange.normalize();

  // Second: update orientation and position
  bm_data.orientQ = lbm_data.orientQ * bm_data.orientChange;
  bm_data.orient = bm_data.initialOrient.reoriented(bm_data.orientQ);

  // The rotation is applied at this pseudocell's node (after the internode)
  bm_data.position = lbm_data.position + (bm_data.length * lbm_data.orient.H);

  produce B(bm_data);
}

B(bm_data):
{
  // First: torque -> bend

  // Calculate the rotations in {H,L,U} frame
  V3f rotateH = (bm_data.torque * bm_data.orient.H * bm_data.elasticity.H) * bm_data.initialOrient.H;
  V3f rotateL = (bm_data.torque * bm_data.orient.L * bm_data.elasticity.L) * bm_data.initialOrient.L;
  V3f rotateU = (bm_data.torque * bm_data.orient.U * bm_data.elasticity.U) * bm_data.initialOrient.U;

  // Now fullBend is the rotation corresponding to the accumulated torque
  Qf fullBend = axisAngle(rotateH + rotateL + rotateU);

  Qf difference = fullBend - bm_data.orientChange;
  total_error += difference.norm();

  // apply relaxation method
  bm_data.orientChange = (1.0 - RELAX_MULTIPLIER) * bm_data.orientChange +
                                RELAX_MULTIPLIER * fullBend;
  bm_data.orientChange.normalize();

  // Second: update orientation and position
  bm_data.orientQ = bm_data.orientChange;
  bm_data.orient = bm_data.initialOrient.reoriented(bm_data.orientQ);

  // The rotation is applied at this pseudocell's node (after the internode)
  //bm_data.position = lbm_data.position + (bm_data.length * lbm_data.orient.H);

  produce B(bm_data);
}

group 0:

/* Interpretation of modules */
interpretation:

vgroup Visualisation:

Internode(node,final_length,length,radius):
{
  produce SetColor(1) SetWidth(radius) F(length);
}

B(bmd):
{
  produce Rotate(bmd.orientChange.getAxis(),RAD2DEG(bmd.orientChange.getAngle()));
}

Leaf(node,area):
{
  float radius = sqrtf(area/M_PI);
  produce SetColor(2)

          SetElasticity(0,0.2)
          Down(33)

          // leaf stalk
          SetWidth(radius*0.05)
          F(radius*0.25) F(radius*0.25) F(radius*0.25) F(radius*0.25)
          
          RollToVert() // set the turtle's up vector to point up

          // leaf
          Down(90)
          // Circle(radius) // the following surface should be the same size as the circle
          SB() Surface(0,radius*0.8) EB()
          SetElasticity(0,0.0)
          ;

}

vgroup Biomechanics:

B(bmd) :
{
  // Rotation is after the internode
  produce Orient() SetColor(3) SetWidth(0.25) F(bmd.length) Rotate(bmd.orientChange.getAxis(),RAD2DEG(bmd.orientChange.getAngle())) SetColor(4) Sphere(bmd.mass) /*f(bmd.mass)*/;
}

