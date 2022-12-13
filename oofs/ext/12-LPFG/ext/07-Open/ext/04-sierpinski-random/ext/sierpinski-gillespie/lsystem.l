#include <lpfgall.h>
#include <math.h>

#define VIGOR 1.f

#define LENGTH 1
#define WIDTH 3

#define MAX_DEPTH 31
#define UNIFORM_GROWTH 1.f /* set to 0 for pure stochastic growth,
                              increase for more uniform growth (values much greater than 1 cause precision errors) */

#define SEED 100

#ifdef SEED
Start: { SeedGillespie(SEED); }
#endif

int nextGroup = 1;
StartEach:{UseGroup(nextGroup);}
EndEach:{if(++nextGroup > 1) nextGroup = 0;}

module A(int);
module B(int);
module tentA(int);
module tentB(int);

derivation length: 500;

Axiom: SetWidth(WIDTH) TurnAround() A(1) B(1);

ggroup 1:

A(depth):
{
  propensity 1.f/(expf((float)depth*UNIFORM_GROWTH)) produce SB() Right(30) tentA(depth) E1(VIGOR) EB();
}

B(depth):
{
  propensity 1.f/(expf((float)depth*UNIFORM_GROWTH)) produce SB() Left(30) tentB(depth) E1(VIGOR) EB();
}

group 0:

tentA(depth) E1(x):
{
  if (x == 1 && depth <= MAX_DEPTH)
    produce SetColor(depth) F(LENGTH) Left(30) E1(1) A(depth+1) B(depth+1);
}

tentB(depth) E1(x):
{
  if (x == 1 && depth <= MAX_DEPTH)
    produce SetColor(depth) F(LENGTH) Right(30) E1(1) A(depth+1) B(depth+1);
}

interpretation:

maximum depth: 1;

A(depth):
{
  produce SB() Right(30) SetWidth(1) SetColor(255) F(LENGTH) EB();
}

B(depth):
{
  produce SB() Left(30) SetWidth(1) SetColor(255) F(LENGTH) EB();
}

tentA(depth):
{
  produce f(LENGTH);
}

tentB(depth):
{
  produce f(LENGTH);
}

