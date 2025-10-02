#include <lpfgall.h>

#define GROW 1
#define PIPE_MODEL 2

module PipeWidth(float /*width*/);
module PipeIncrement(float /*width_increment*/);

module Internode(float /* length */);
module Apex(bool /*right?*/);
module Root();

derivation length: 15;

float PipeExponent = 2;
float singlePipeWidth = 0.03;
float singleInternodeLength = 1;

Axiom: SetColor(4) Root() PipeWidth(0) Apex(true) PipeIncrement(singlePipeWidth);

Start:
{
  Backward();
  UseGroup(PIPE_MODEL);
}

StartEach:
{
  if(CurrentGroup() == PIPE_MODEL)
    UseGroup(GROW);
  else if(CurrentGroup() == GROW)
    UseGroup(PIPE_MODEL);
}

group GROW:
consider: Root Apex Internode;
production:
Root() < Apex(right) PipeIncrement(i):
{
  // root -> long internode
  nproduce Internode(3 * singleInternodeLength) PipeWidth(0) SB;
  if(right) nproduce Right(45); else nproduce Left(45);
  produce Apex(!right) PipeIncrement(singlePipeWidth) EB
    Apex(!right) PipeIncrement(singlePipeWidth);
}

Apex(right) PipeIncrement(i):
{
  nproduce Internode(singleInternodeLength) PipeWidth(0) SB;
  if(right) nproduce Right(45); else nproduce Left(45);
  produce Apex(!right) PipeIncrement(singlePipeWidth) EB
    Apex(!right) PipeIncrement(singlePipeWidth);
}


group PIPE_MODEL:
consider: PipeWidth PipeIncrement;
production:
PipeWidth(w):
{
  float wacc = 0;
  float wid;
  while(true)
  {
    if(InNewRightContext(SB PipeWidth(wid) EB)) wacc += pow(wid,PipeExponent);
    else if(InNewRightContext(SB PipeIncrement(wid) EB)) wacc += pow(wid,PipeExponent);
    else break;
  }

  if(InNewRightContext(PipeWidth(wid))) wacc += pow(wid,PipeExponent);
  else if(InNewRightContext(PipeIncrement(wid))) wacc += pow(wid,PipeExponent);

  produce PipeWidth(pow(wacc,1.f/PipeExponent));
}


group 0:
interpretation:
Internode(l): { produce F(l); }
PipeWidth(w): { produce SetWidth(w); }
