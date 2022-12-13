#include <lpfgall.h>
#include <math.h>

const int Delay = 1;
const float BranchingAngle = 40.0;
const float LengthGrowthRate = 1.33;

derivation length: 17;

struct InternodeData
{ float length, area; };

module A(int,float);
module Metamer(float);
module Internode(InternodeData);

Start: { Backward(); }
ignore: Right;

axiom: A(0,BranchingAngle);

A(t,angle) :
{
  if (t<0) // young apex
    produce A(t+1,angle);  
  else     // mature apex
    produce Metamer(angle) A(0,-angle);
}

Internode(id) >> A(t,angle):
{
  id.area = 1.0;
  id.length *= LengthGrowthRate;
  produce Internode(id);
}

Internode(id) >> SB() Internode(id2) EB() Internode(id3) :
{
  id.area = id2.area + id3.area;
  id.length *= LengthGrowthRate;
  produce Internode(id);
}

Internode(id) >> Internode(idr) :
{
  id.area = idr.area;
  id.length *= LengthGrowthRate;
  produce Internode(id);
}

decomposition:
Metamer(angle) :
{
  InternodeData id = {1, 1};
  produce 
      Internode(id) 
      SB() Right(angle) A(-Delay,angle) EB()
      Internode(id);
}

interpretation:
Internode(id) :
{
  produce SetColor(1 + (int) (2*log(id.area))) 
	  SetWidth(pow(id.area, .35)) F(id.length); 
}
