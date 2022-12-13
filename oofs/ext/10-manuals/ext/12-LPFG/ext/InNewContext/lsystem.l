#include <lpfgall.h>
#include <math.h>

const int Delay = 2;
const float BranchingAngle = 40.0;
const float LengthGrowthRate = 1.23;

derivation length: 22;

struct InternodeData
{ float length, area; };

module A(int,float);
module Metamer(float);
module Internode(InternodeData);

Start: { Backward(); }
ignore: Right Left;

axiom: Right(0.1*BranchingAngle) A(0,BranchingAngle);

A(t,angle) :
{

  if (t<0) // young apex
    produce A(t+1,angle);  
  else {    // mature apex
   InternodeData id = {1, 0};
   produce 
      Internode(id) 
      SB() Right(angle) A(-Delay,angle) EB()
      Left(0.4*angle) A(0,-angle);
   }
}

Internode(id) :
{
  struct InternodeData idr;
  int t;
  float angle;
  id.area = 0;
  if (InRightContext(A(t,angle))) 
 	id.area = +1.0;
  while (InNewRightContext(SB() Internode (idr) EB() ))
	id.area += idr.area ;
  if (InNewRightContext(Internode (idr) ))
 	id.area += idr.area;
  id.length *= LengthGrowthRate;
  produce Internode(id);
}

interpretation:
Internode(id) :
{
  produce SetColor(1 + (int) (2*log(id.area))) 
	  SetWidth(pow(id.area, .35)) F(id.length); 
}
