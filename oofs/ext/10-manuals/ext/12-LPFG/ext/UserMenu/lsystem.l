#include <lpfgall.h>

derivation length: 0 ;

const int BLUE = 1, GREEN = 2, RED = 4;

struct ShapeData
{
  int colour;
  enum { SD_Square, SD_Circle } shape;

  ShapeData(void) : colour(BLUE), shape(SD_Circle) {}
};

module Shape(ShapeData);

Axiom: Shape(ShapeData());

int choice;

Start:
{
	UserMenuItem("Show a BLUE SQUARE",0);
	UserMenuItem("Show a BLUE CIRCLE",1);
	UserMenuItem("Show a RED SQUARE",2);
	UserMenuItem("Show a RED CIRCLE",3);
}

StartEach:
{
	choice = UserMenuChoice();
}

production:

Shape(sd) :
{
	switch(choice)
	{
	case 0: // blue square
		sd.colour = BLUE;
		sd.shape = ShapeData::SD_Square;
		break;
	case 1: // blue circle
		sd.colour = BLUE;
		sd.shape = ShapeData::SD_Circle;
		break;
	case 2: // red square
		sd.colour = RED;
		sd.shape = ShapeData::SD_Square;
		break;
	case 3: // red circle
		sd.colour = RED;
		sd.shape = ShapeData::SD_Circle;
		break;
	default: // probably no choice made
		break;
	}
	produce Shape(sd);
}

interpretation:
maximum depth: 3;

module Square(float);

Shape(sd) :
{
	nproduce SetColor(sd.colour);
	if(sd.shape == ShapeData::SD_Square) produce Square(1);
	else if(sd.shape == ShapeData::SD_Circle) produce Circle(1);
	else produce;
}

Square(size) :
{
	produce SB f(size) Left(90) f(size) SP
		PP Left(90) f(size*2)
		PP Left(90) f(size*2)
		PP Left(90) f(size*2)
		PP Left(90) f(size*2) EP EB;
}
