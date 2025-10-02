#include <lpfgall.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

const int NR = 10;
const float L = 5.5;
const float W = 2.5;
const float H = 30.0;

const float NUM_X=1000.0;
const float NUM_Y=1000.0;
const float DIFFUSE_X = 1.0;
const float DIFFUSE_Y = 0.5;
const float DILUTE_X = 0.1;
const float DILUTE_Y = 0.1;

struct CellData
{
  void Set(float X_, float Y_, float dX_, float dY_, float rX_, float rY_, bool special_)
  {
	X = X_;
	Y = Y_;
	dX = dX_;
	dY = dY_;
	rX = rX_;
	rY = rY_;
	special = special_;
  }

  float X;    /* number of molecules of substance X */
  float rX;   /* decay rate for substance X */
  float dX;   /* diffusion rate for substance X */
  float Y;    /* number of molecules of substance Y */
  float rY;   /* decay rate for substance Y */
  float dY;   /* diffusion rate for substance Y */
  float p0, p1, p2, p3, pSum; /* the probabilites for the reactions */
  float time; /* time of next reaction */
  bool special;
};

/* this must be defines? */
#define SEARCH 1
#define NORMAL 2
#define REACTION 3
#define LEFT_RIGHT_DIFFUSE 4
#define RIGHT_LEFT_DIFFUSE 5
int whichPhase;
float rand01, rateSum, currentSum;
bool found_event;
Start:
{
  whichPhase = SEARCH;
  UseGroup (SEARCH);
}
StartEach:
{
  if (whichPhase == SEARCH)
  {
    rateSum = 0.0;
    currentSum = 0.0;
    rand01 = ran(1);
    found_event = false;
  }
}
EndEach:
{
  UseGroup (whichPhase);
}

void NextReactionTime (struct CellData &cd)
/* updates celldata according to Gillespie's method */
{
  /* compute new reaction and diffusion rate probabilites */
  cd.p0 = cd.X*cd.dX;
  cd.p1 = cd.p0 + cd.Y*cd.dY;
  cd.p2 = cd.p1 + cd.X*cd.rX;
  cd.p3 = cd.p2 + cd.Y*cd.rY;

  /* put the last one into a sum variable so that when new
     probabilites are added only the next line will change */
  cd.pSum = cd.p3;

  /* find next reaction time for this cell */
  if (cd.pSum > 1e-7)
    cd.time = cd.time - log(ran(1)) / (cd.pSum);
}

module Cell(CellData);
module ReactionCell(CellData, float);
module DiffuseCell(CellData, float);
module LeftRightDiffuse(int);
module RightLeftDiffuse(int);
module A(int);
module Boundary();

derivation length: 20000;
ignore: F;

Axiom: Right(90) Boundary() A(1) Boundary();

/* --------------------------------------------------------- */
group SEARCH:

Cell(c):
{
  rateSum += c.pSum;
  whichPhase = NORMAL;
}

/* --------------------------------------------------------- */
group NORMAL:

Cell(c):
{
  float nextReaction;

  currentSum += c.pSum;

  nextReaction = rand01 * rateSum;

  if (nextReaction < currentSum && found_event == false)
  {
    /* we have found the cell where an event takes place and
       must prevent lpfg from finding anymore */
    found_event = true;

    /* pick between a reaction event and a diffusion event */
    /* scale next reaction so it is in [0, cell.pSum) not [0,rateSum) */
    nextReaction = nextReaction-(currentSum-c.pSum);
    if (nextReaction < c.p1)
    {
      /* diffusion */
      //fprintf (stderr, "DIFFUSION\n");
      if (ran(1) < 0.5)
        whichPhase = LEFT_RIGHT_DIFFUSE;
      else
        whichPhase = RIGHT_LEFT_DIFFUSE;

      produce DiffuseCell(c, nextReaction);
    }
    else
    {
      /* reaction */
      //fprintf (stderr, "REACTION\n");
      whichPhase = REACTION;
      produce ReactionCell(c, nextReaction);
    }
  }
  else
    produce Cell(c);
}

/* --------------------------------------------------------- */
group REACTION:

ReactionCell (c, whichReaction):
{
  CellData cd = c;

  /* decay reaction */
  if (c.special == false)
    {
  if (whichReaction < c.p2)
  {
    if (c.X >= 1)
      cd.X = cd.X - 1.0;
  }
  else if(whichReaction < c.p3)
  {
    if (c.Y >= 1)
      cd.Y = cd.Y - 1.0;
  }
  else
  {
    fprintf (stderr, "WARNING: reaction group called incorrectly\n");
  }
    }

  NextReactionTime(cd);
  whichPhase = SEARCH;
  produce Cell(cd);
}

/* --------------------------------------------------------- */
group LEFT_RIGHT_DIFFUSE:

DiffuseCell(c, whichSubstance) > Cell(cr):
{
  CellData cd;
  int substanceId;

  cd = c;

  /* if there are no molecules to diffuse, then id==-1 will ensure
     that no new molecules are added to the neighbour */
  substanceId = -1;

  /* diffuse one molecule from this cell */
  if (whichSubstance < c.p0)
  {
    if (cd.X >= 1)
    {
      cd.X = cd.X - 1;
      substanceId = 0;
    }
  }
  else if (whichSubstance < c.p1)
  {
    if (cd.Y >= 1)
    {
      cd.Y = cd.Y - 1;
      substanceId = 1;
    }
  }
  else
  {
    fprintf (stderr, "WARNING: group left-right diffusion incorrect\n");
  }

  NextReactionTime (cd);

  /* diffuse one molecule to the right cell */
  if (substanceId != -1)
    produce Cell(cd) LeftRightDiffuse(substanceId);
  else
    whichPhase = SEARCH;
}

LeftRightDiffuse(id) < Cell(c):
{
  CellData cd;

  cd = c;
  if (id == 0 && c.special==false)
    cd.X = cd.X + 1;
  else if (id == 1 && c.special==false)
    cd.Y = cd.Y + 1;
  else if (c.special==false)
    fprintf (stderr, "WARNING: substance id unknown\n");

  NextReactionTime (cd);

  produce Cell(cd);
}

LeftRightDiffuse(id):
/* erase the LeftRightDiffuse marker */
{
  whichPhase = SEARCH;
  produce;
}

DiffuseCell(c, whichSubstance) > Boundary():
/* this is a cell without a neighbour to the right */
{
  /* diffuse to the right - i.e., back into this cell!!! */
  /* number of molecules does not change */
  NextReactionTime (c);
  whichPhase = SEARCH;
  produce Cell(c);
}

/* --------------------------------------------------------- */
group RIGHT_LEFT_DIFFUSE:

Cell(cl) < DiffuseCell(c, whichSubstance):
{
  CellData cd;
  int substanceId;

  cd = c;

  /* if there are no molecules to diffuse, then id==-1 will ensure
     that no new molecules are added to the neighbor */
  substanceId = -1;

  /* diffuse one molecule from this cell */
  if (whichSubstance < c.p0)
  {
    if (cd.special == true)
      substanceId = 0;
    else if (cd.X >= 1)
    {
      cd.X = cd.X - 1;
      substanceId = 0;
    }
  }
  else if (whichSubstance < c.p1)
  {
    if (cd.special == true)
      substanceId = 1;
    else if (cd.Y >= 1)
    {
      cd.Y = cd.Y - 1;
      substanceId = 1;
    }
  }
  else
  {
    fprintf (stderr, "WARNING: group left-right diffusion incorrect\n");
  }

  NextReactionTime (cd);

  /* diffuse one molecule to the left cell */
  if (substanceId != -1)
    produce RightLeftDiffuse(substanceId) Cell(cd);
  else
    whichPhase = SEARCH;
}

Cell(c) > RightLeftDiffuse(id):
{
  CellData cd;

  cd = c;
  if (id == 0)
    cd.X = cd.X + 1;
  else if (id == 1)
    cd.Y = cd.Y + 1;
  else
    fprintf (stderr, "WARNING: substance id unknown\n");

  NextReactionTime (cd); 

  produce Cell(cd);
}

RightLeftDiffuse(id):
/* erase the RightLeftDiffuse marker */
{
  whichPhase = SEARCH;
  produce;
}

Boundary() < DiffuseCell(c, whichSubstance):
/* this is a cell without a neighbour to the left */
{
  /* diffuse to the right - i.e., back into this cell!!! */
  /* number of molecules does not change */
  NextReactionTime (c);
  whichPhase = SEARCH;
  produce Cell(c);
}

/* --------------------------------------------------------- */
group 0:

decomposition:
maximum depth: 10;

A(n) :
{
  if (n < NR)
  {
    CellData cd;
	cd.Set(0.0, 0.0, DIFFUSE_X, DIFFUSE_Y, DILUTE_X, DILUTE_Y, false);
    NextReactionTime(cd);
    produce F(L)Cell(cd)A(n+1);
  }
  else
  {
    CellData cd;
	cd.Set(NUM_X, NUM_Y, DIFFUSE_X, DIFFUSE_Y, DILUTE_X, DILUTE_Y, true);
    NextReactionTime(cd);
    produce F(L)Cell(cd);
  }
}

interpretation:
maximum depth: 10;

DiffuseCell(c, whichSubstance):
{
  produce Cell(c);
}

ReactionCell(c, whichReaction):
{
  produce Cell(c);
}

Cell(c) :
{
  produce SB()
            SetColor(2)
            SetWidth(10)
            TurnAround()
            SB()
              SB()
                f(W/2) Right(90) F(c.X/NUM_X * H)
              EB()
            EB()
            SetColor(1)
            SetWidth(1)
            SB()
              F(W) Right(90) F(H) Right(90) F(W) Right(90) F(H)
            EB()
          EB()

          SB()
            SetColor(34)
            SetWidth(10)
            SB()
              SB()
                f(-W/2) Right(90) F(c.Y/NUM_Y * H)
              EB()
            EB()
            SetColor(1)
            SetWidth(1)
            SB()
              Right(90) F(H) Right(90) F(W) Right(90) F(H)
            EB()
          EB();
}
