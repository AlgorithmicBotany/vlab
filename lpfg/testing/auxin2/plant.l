#include <lpfgall.h>		// Declaration of constants
#include <math.h>

/* Useful functions to have around; only max is used at present in the model */

double min(double a, double b)
{
	if (a>b) return b; else return a;
}

double max(double a, double b)
{
	if (a>b) return a; else return b;
}

double clamp (double x, double a, double b)
{
	if (x<a) return a;
	if (x>b) return b;
	return x;
} 

/* Mini-library of 2D vector functions */

#define epsilon 0.00001

double VecLength2f(V2d v)
{
	return sqrt (v.x*v.x + v.y*v.y);
}

V2d VecNormalize2f(V2d v)
{
	if (VecLength2f(v) < epsilon) {
		Printf("Error: normalizing null vector\n");
		exit(1);
	}
	else 
		return v/VecLength2f(v);
}

V2d VecTurnLeft2f(V2d v)
{
	V2d result;
	result.x = -v.y;
	result.y = v.x;
	return result;
}

V2d VecTurnRight2f(V2d v)
{
	V2d result;
	result.x = v.y;
	result.y = -v.x;
	return result;
}

/* Declaration of constants */

// switches controling overall simulation and visualization

#define NR		6	// number of cells in the sequence
#define OutputStepNR	1	// 0 - don't print step number, 1 - print
#define ShowArrowheads	1	// arrows indicate direction of auxin flux
				// 0 - don't show; 1 - show
#define BW		0	// 0 - colors for black background, 16 - for white background


// enumeration of simulation phases

#define Initialize	1
#define GrowNormal	4
#define ToChain		5
#define GrowChain	6
#define ToPositionLeft	7
#define ToPositionRight 8
#define ToTurtle	9
#define GrowTurtle	10
#define ToPosition	11
#define Divide		12
//Add additional phases here
#define ComputeFluxes	2
#define TransPol	3


// visualization parameters

#define Len 		10.0	// metamer length
#define Wid 		10.0	// metamer width
#define Href		10.0	// reference auxin concentration for visualization purposes

#define dt 0.050000	// time step
//0-0.54ish,0.0125,0.0125
//0-0.56ish,0.0125,0.006125
//0-0.575ish ,0.0125,0.0
//Fractal Growth Parameters
#define NGROWTH 2.0
#define WNGROWTH 1.0
#define LGROWTH 1.0
#define OPT_LGROWTH 0.0125
/********************* END OF KEY PARAMETERS *****************************/

/* graphically-defined functions */

#define MIN_GROWTH 0.1
#define MAX_GROWTH 1.2
#define MAX_GAUXIN 15
double maximum_auxin = 0;
double GrowthRate(double c)
{
if(c>maximum_auxin){
	maximum_auxin = c;
	//Printf("%f\n",c);
	}
//	return func(1, c/15);
if(c>MAX_GAUXIN)
	c=MAX_GAUXIN;
if(c<0)
	c=0;
double dc= MAX_GROWTH-MIN_GROWTH;
return -2*dc/pow(MAX_GAUXIN,3.0)*pow(c,3.0)+3*dc/pow(MAX_GAUXIN,2.0)*pow(c,2.0)+MIN_GROWTH;

} 


/* Structure declarations and initialization */

struct cell_state {
//Molecular Attributes
double flux;   //Auxin flux

//Geometric Attributes
	V2td pos;	// position of current cell center
	V2td posl;	// position of left neighbor center
	V2td posr;	// position of right neighbor center
	V2td dright;	// displacement to right neighbor
	V2td Lpos;	// position from left integration
	V2td Rpos;	// position from right integration
	V2td normal;
	double NormalGrowth; //growth in normal direction 
	double angle;
	double tangle;


};

struct wall_state {
//Molecular Attributes
	double c;	// auxin concentration 
	double cuc;     //cuc concentration
	double PINl;	// PIN concentration in left membrane 
	double PINr;	// PIN concentration in right membrane 
	double loc_sigma; // local production of auxin
	double loc_H; 	// local target auxin level


//Geometric Attributes
V2td dright;
double length;
double NormalGrowth;
double LengthGrowth;
V2td posl;
V2td posr;
V2td normall;
V2td normalr;

V2td wallnl;
V2td wallnr;
V2td walln;
};

V2td Dpos;
V2td Epos;


/* Module definitions */
module C(cell_state);	// a cell
module W(wall_state);	// a wall 
module A();		// generates the initial string

module EmptyRectangle(double, double);	// rectangle frame (length,width)
module FilledRectangle(double, double);	// filled rectangle (length, width)
module Arrowhead(double, double, double);	// (length, width at start, width at end)

module EmptyQuad2f(V2td, V2td, V2td, V2td);	// quadilateral frame, given vertices
module FilledQuad2f(V2td, V2td, V2td, V2td); // filled rectangle, given vertices 

module SetCUCColor(bool,double);
module SetAuxinColor(bool,double);

/**************** KEY PARAMETERS OF THE MODEL *****************************/
// auxin transport
#define T 	0.4	// polar transport coefficient: default 0.5, reduce to 0.2 in axr3
#define D 	2.5	// diffusion coefficient - was 0.02

// PIN allocation
#define rho 	10.0	// PIN allocation rate; default 10; reduce to 5 in tir3 and max,tir3
#define n 	3.0f	// Hill exponent
#define K	0.5f	// Hill saturation constant
#define rho_0	0.15	// base PIN production - was 0.15
#define mu	7.5	// PIN turnover: default 7.5; reduce to 2.5 in max and max,tir3
double K_power = pow(K, n);

// auxin production and turnover
#define H 	10	// target auxin concentration at the source - increase to 14 in axr1
#define H_r 	2.5	// target auxin concentration after flowering: default 2.5; 10 for axr3
#define sigma 	10	// auxin production rate
#define sigma_r 0.4	// residual auxin production: default 10, decrease to 0.5 for basipetal 
#define nu 	0.005	// auxin decay rate: default 0.005, 0.03 for convergent; 0.2 for acropetal
#define nu_root	1.0	// auxin turnover in the root

//CUC coefficients
//7 and 15
#define CUC_int 7
#define PROD_CUC 0.35
#define DECAY_CUC 0.2
#define DR_AUXIN 0.17
#define UR_CUC 0.00
#define KCUC 0.1
#define cuc_mutant false
//Basal growth parameters
#define GROWTHK 0.0825
#define RDIST 300.0
#define GLENGTH 100.0

#define ALPHA 0.0  //KNOX like parameter

/* Basipedal growth function */

float BGrowth(float y,float t)
{
/*
float rd =0;

if(t<GLENGTH)
  rd = (GLENGTH-t)/GLENGTH*RDIST;

if(y>rd)
  y=rd;
if(y<0)
  y=0;

float gr = GROWTHK*y-GROWTHK/rd*y*y/2.0;
if(rd <= 0)
  gr = 0;
return gr;
*/
float rd =0;

if(t<GLENGTH)
  rd = (GLENGTH-t)/GLENGTH*RDIST;

if(y>rd)
  y=rd;
if(y<0)
  y=0;

float gr = GROWTHK*(rd-y)/rd;
if(y<0 || y>rd)
  gr = 0;
return gr;



}





/* Organization of computation into phases */

int Phase;				// simulation phase
int step;				// counts simulation steps

Start: 
{
	Phase = Initialize;
	step = 0;
}

StartEach:
{
	UseGroup(Phase);		// set up current computation phase
}
int chemstep;
EndEach:
{
 
 	switch (Phase) {		// determine next computation phase
  	case Initialize:
		Phase = GrowNormal;
		break;
	case GrowNormal:
		Phase = ToChain;
		break;
	case ToChain:
		Phase = GrowChain;
		break;
	case GrowChain:
		Phase = ToPositionLeft;
		Forward();
		break;
	case ToPositionLeft:
		Phase = ToPositionRight;
		Backward();
		break; 
	case ToPositionRight:
		Phase = ToTurtle;
		Forward();
		break;
	case ToTurtle:
		Phase = GrowTurtle;
		break;
	case GrowTurtle:
		Phase = ToPosition;
		break;
	case ToPosition:
		Phase = Divide;
		break;
 	case Divide:
		step++;
		if(step%5==0)
		DisplayFrame();
		Phase = ComputeFluxes;
		chemstep = 0;
		break;
	case ComputeFluxes:
		Phase = TransPol;
		break;
	case TransPol:
		if(chemstep == 50)
			Phase = GrowNormal;
		else
			Phase = ComputeFluxes;
		chemstep++;
		break;
  	}
//	Printf("%d",Phase);
}	

End: {
	//Printf("Simulation completed\n");
}


derivation length: 750167 ;
ignore: Right ;

ring L-system: 0;



Axiom: SB Right(180) f(250) EB SB f(400) EB SB Right(90) A()EB ;


group Initialize:
decomposition:
maximum depth: 1000 ;

A() : 
{
	V2d position(0,0);
	cell_state current_cell_state;
	wall_state current_wall_state;
	for (int i=-NR; i<=NR; i+=2) {
		//Initialize Geometric Attributes
		current_cell_state.pos.x=i*Len;
		current_cell_state.pos.y = 0;
		current_cell_state.posl.x=(i-1)*Len;
		current_cell_state.posl.y = 0;
		current_cell_state.posr.x=(i+1)*Len;
		current_cell_state.posr.y=0;
		current_wall_state.length = Len;
		current_cell_state.angle = 0;
		current_cell_state.NormalGrowth = NGROWTH;
		current_wall_state.LengthGrowth = LGROWTH;
		if(abs(i) == NR){
		current_cell_state.NormalGrowth = NGROWTH;
		current_wall_state.LengthGrowth = LGROWTH;
		}
		if(i-2 == -NR)
		current_wall_state.LengthGrowth = LGROWTH;


		current_wall_state.NormalGrowth = WNGROWTH;
	
		//Initialize Chemical Attributes
		current_cell_state.flux = 0;
		current_wall_state.c=0;
		current_wall_state.cuc=CUC_int;
		current_wall_state.PINl=0;
		current_wall_state.PINr=0;
		current_wall_state.loc_sigma=sigma_r;
		current_wall_state.loc_H=H;

if (i<0) current_cell_state.pos.x = -3*Len;
		if (i==0) current_cell_state.pos.x = 0;
		if (i>0) current_cell_state.pos.x = 3*Len;
		if (i<0) {
			current_cell_state.posl.y = (double) 2*Len*(i-2);
			current_cell_state.pos.y = (double) 2*Len*i+(double)2.0*NR*Len; 
			current_cell_state.posr.y = (double) 2*Len*(i+2);
		}
		if (i==0) {
			current_cell_state.posl.y = -4;
			current_cell_state.pos.y = (double)2.0*NR*Len;
			current_cell_state.posr.y = 4;
		}
	        if (i>0) {	
			current_cell_state.posl.y = -(double) 2*Len*(i-2);
			current_cell_state.pos.y = -(double) 2*Len*i +(double)2.0*NR*Len; 
			current_cell_state.posr.y = -(double) 2*Len*(i+2);
		}

		if(i!=-NR)
		nproduce W(current_wall_state) C(current_cell_state);
		else 
		nproduce C(current_cell_state);
	}
	produce ;
}



group GrowNormal: 		// Compute growth at each point and apply level set method to grow along the normal
production:

 C(sl)W(wl)<C(s) > W(wr)C(sr) :		// a cell
{

	V2d vl = s.pos - sl.pos;
	V2d vr = sr.pos - s.pos;
	V2d perpl = VecTurnLeft2f(vl);
	V2d perpr = VecTurnLeft2f(vr);
	V2d normal = VecNormalize2f(perpl + perpr);
	V2d normal_growth(0,0);
	double auxin =0.5*(wl.c+wr.c);
	double cuc = 0.5*(wl.cuc+wr.cuc);
	if(s.NormalGrowth)
	normal_growth+=BGrowth(sl.pos.y+sr.pos.y,0.1)*s.NormalGrowth*normal*max(0.75*auxin*auxin+59,0);

	if(wl.NormalGrowth)
	normal_growth+=BGrowth(sl.pos.y+sr.pos.y,0.1)*wl.NormalGrowth*VecNormalize2f(perpl)*max(0.75*wl.c*wl.c+59,0);

	if(wr.NormalGrowth)
	normal_growth+=BGrowth(sl.pos.y+sr.pos.y,0.1)*wr.NormalGrowth*VecNormalize2f(perpr)*max(0.75*wr.c*wr.c+59,0);


//	s.pos+=normal_growth*dt;
//Compound leaf growth
	//s.pos += s.NormalGrowth*ALPHA*20 * normal * max(pow(auxin,2.0f)-100 , 0) * dt*BGrowth(0.5*(fabs(s.pos.y)+fabs(s.pos.x/2.0)),0.1)*(12.0-cuc)/12.0/10.0;
//Arab like leaf growth

	
		double n_growth = max(0,(1-ALPHA)*100 * s.NormalGrowth * (GrowthRate(auxin)) * dt*BGrowth(s.pos.y/2.0,0.1)*(55.0-(cuc_mutant?0:cuc))/55.0/10.0);
	if(s.pos.y>100)
		s.pos.x += s.pos.x*(0.1*BGrowth((s.pos.y),0.1))/20.0;
		s.pos += normal*n_growth;



	s.normal = normal;
	produce C(s);
}


C(s)>W(wr)C(sr) :
{
	V2d vr = sr.pos - s.pos;
	V2d perpr = VecTurnLeft2f(vr);
	V2d normal = VecNormalize2f(perpr);
	V2d normal_growth(0,0);
	double auxin = wr.c;

//	if(wr.NormalGrowth)
//	normal_growth+=2*wr.NormalGrowth*VecNormalize2f(perpr)*max(0.75*wr.c*wr.c+0.2,0);

//	if(s.NormalGrowth)
//	normal_growth+=2*s.NormalGrowth*normal*max(0.75*auxin*auxin+0.2,0);
//	s.pos+=normal_growth*dt;
	s.normal = normal;
	produce C(s);
}

C(sl)W(wl)<C(s) :
{
	V2d vl = s.pos - sl.pos;
	V2d perpl = VecTurnLeft2f(vl);
	V2d normal = VecNormalize2f(perpl);
	V2d normal_growth(0,0);
	double auxin = wl.c;
//	if(wl.NormalGrowth)
//	normal_growth+=2*wl.NormalGrowth*VecNormalize2f(perpl)*max(0.75*wl.c*wl.c+0.2,0);

//	if(s.NormalGrowth)
//	normal_growth+=2*s.NormalGrowth*normal*max(0.75*auxin*auxin+0.2,0);
//	s.pos+=normal_growth*dt;
	s.normal = normal;
	produce C(s);
}



group ToChain:
production:

C(sl)W(wl) < C(s) > W(wr)C(sr) :
{
s.dright = sr.pos - s.pos;
produce C(s);
}

C(s)> W(wr)C(sr) :
{
s.dright = sr.pos - s.pos;
Dpos = s.pos;
produce C(s);
}

C(sl)W(wl)<C(s):
{
Epos = s.pos;
produce C(s);
}

group GrowChain:
production:

C(sl)W(wl)<C(s)>W(wr)C(sr):
{
//s.dright.y+=0.004*s.dright.y;
//s.dright.x+=0.1*s.dright.x*BGrowth(0.5*(s.pos.y+sr.pos.y),0.1)/10.0;

//Compound leaf
//s.dright.y+= 2.0*ALPHA*(wr.cuc)/20.0*(0.1*BGrowth(0.5*(fabs(s.pos.y+sr.pos.y)+fabs(s.pos.x+sr.pos.x)/2.0),0.1))*s.dright.y/10.0;
//Arab like leaf
s.dright.y+= (1-ALPHA)*(0.1*BGrowth(0.5*(s.pos.y+sr.pos.y),0.1))*s.dright.y/10.0;



produce C(s);
}

C(s)>W(wr)C(sr):
{
//s.dright.y+=0.004*s.dright.y;
//s.dright.x+=0.1*s.dright.x*BGrowth(0.5*(s.pos.y+sr.pos.y),0.1)/10.0;
//s.dright.y+= 2.0*ALPHA*(wr.cuc)/20.0*(0.1*BGrowth(0.5*(fabs(s.pos.y+sr.pos.y)+fabs(s.pos.x+sr.pos.x)/2.0),0.1))*s.dright.y/10.0;
//Arab like leaf
s.dright.y+= (1-ALPHA)*(0.1*BGrowth(0.5*(s.pos.y+sr.pos.y),0.1))*s.dright.y/10.0;
produce C(s);
}

C(sl)W(wl)<C(s):
{
produce C(s);
}

group ToPositionLeft:
production:

C(sl)W(wl)<< C(s)>W(wr)C(sr) :
{
s.Lpos= sl.Lpos+sl.dright;

produce C(s);
}

C(sl)W(wl)<<C(s) :
{
s.Lpos= sl.Lpos+sl.dright;
produce C(s);

}

C(s):
{
//if(Dpos.y<0)
//  Dpos.y=0;
s.Lpos = Dpos;
produce C(s);
}


group ToPositionRight:
production:

C(sl)W(wl)< C(s)>>W(wr)C(sr) :
{
s.Rpos = sr.Rpos-s.dright;
s.pos = 0.5*(s.Rpos+s.Lpos);
produce C(s);
}

C(s)>>W(wr)C(sr) :
{
s.Rpos = sr.Rpos-s.dright;
s.pos = 0.5*(s.Rpos+s.Lpos);
produce C(s);

}

C(s):
{
//if(Epos.y<0)
//Epos.y=0;

s.Rpos = Epos;
s.pos = 0.5*(s.Lpos+s.Rpos);

produce C(s);
}

group ToTurtle:
C(sl)<<W(w)>C(sr):
{
V2d direction = sr.pos - sl.pos;
w.length = VecLength2f(direction);

produce W(w);
}

C(sl)W(wl)<<C(s)>W(wr)C(sr):
{
V2d direction = sr.pos - s.pos;

s.tangle = atan2(direction.y,direction.x)/M_PI*180.0;
s.angle = s.tangle-sl.tangle;
//Printf("%d ",s.angle);
produce C(s);
}

C(s)>W(wr)C(sr):
{
V2d direction = sr.pos - s.pos;

Dpos = s.pos;
s.angle = atan2(direction.y,direction.x)/M_PI*180.0;
s.tangle = s.angle;
//Printf("%d\n",s.angle);
produce C(s);
}

group GrowTurtle:

C(sl)<W(w)>C(sr):
{
//if(w.LengthGrowth)
 //w.length+= (0.1*BGrowth(0.5*(sl.pos.y+sr.pos.y),0.1))/10.0*w.length;
// w.length += BGrowth(sl.pos.y+sr.pos.y,0.1)*max((w.cuc)*(w.cuc)+0.25,0)*w.LengthGrowth*w.length*dt;

produce W(w);
}

C(sl)<W(w):
{
//if(w.LengthGrowth)
// w.length+= (0.1*BGrowth(sl.pos.y,0.1))/10.0*w.length;
// w.length += BGrowth(sl.pos.y*2,0.1)*max((w.cuc)*(w.cuc)+0.25,0)*w.LengthGrowth*w.length*dt;

produce W(w);
}

W(w)>C(sr):
{
//if(w.LengthGrowth)
// w.length+= (0.1*BGrowth(sr.pos.y,0.1))/10.0*w.length;
// w.length += BGrowth(2.0*sr.pos.y,0.1)*max((w.cuc)*(w.cuc)+0.25,0)*w.LengthGrowth*w.length*dt;

produce W(w);
}

group ToPosition:

C(sl)W(wl)<<C(s) :
{
s.pos = sl.pos + wl.length*V2d(cos(sl.tangle/180*M_PI),sin(sl.tangle/180*M_PI));
s.tangle = sl.tangle + s.angle;

produce C(s);
}

C(s)>W(w) :
{
s.pos = Dpos;
s.tangle = s.angle;

produce C(s);
}

group Divide: 		// Divide cells that are too long
production:

C(sl)<W(wl)>C(sr):
{
V2d vl = sl.pos-sr.pos;
wl.walln = VecNormalize2f(VecTurnLeft2f(vl));
if(VecLength2f(vl)>50)
{

nproduce W(wl);
V2d tangents = 25*(VecTurnLeft2f(sl.normal)-VecTurnLeft2f(sr.normal));
sl.pos -=0.5*vl+0.125*tangents;
sl.flux = 0;
//double check that this is kosher
sl.NormalGrowth = 0.5*(sl.NormalGrowth+sr.NormalGrowth);
nproduce C(sl);


}
produce W(wl);

}


/* C(sl)W(wl)<C(s)>W(wr)C(sr) :		// a cell
{

	s.posr = sr.pos;
	s.posl = sl.pos;

	V2d vl = s.pos - sl.pos;
	V2d vr = sr.pos - s.pos;
	double length = VecLength2f(vl+vr) + 0* VecLength2f(vr);
	
	if (length > 100) {
		V2d current_pos = s.pos;
		s.pos = current_pos-0.4*vl;
		C.flux = 0;
		nproduce C(s);
		wall_state new_wall;
		new_wall.NormalGrowth = wr.NormalGrowth;
		new_wall.LengthGrowth = wr.LengthGrowth;
		new_wall.flux = 0.0*(wl.flux+wr.flux);
		nproduce W(new_wall);
		s.pos = current_pos + 0.4*vr;
	}
	produce C(s);
}

C(s) > W(wr)C(sr):
{
	V2d vr = sr.pos - s.pos;
	V2d vl = vr;
	s.posr = sr.pos;
	s.posl = s.pos-vr;

	double length = VecLength2f(vl+vr) + 0* VecLength2f(vr);
	if (length > 100) {
		V2d current_pos = s.pos;
		s.pos = current_pos-0.4*vl;
		nproduce C(s);
		wall_state new_wall;
		new_wall.flux = 0.0*(wr.flux);
		new_wall.NormalGrowth = wr.NormalGrowth;
		new_wall.LengthGrowth = wr.LengthGrowth;
		nproduce W(new_wall);
		s.pos = current_pos + 0.4*vr;
	}

	produce C(s);



}

C(sl)W(wl)<C(s):
{

	V2d vl = s.pos - sl.pos;
	V2d vr = vl;
	s.posl = sl.pos;
	s.posr = s.pos+vl;

	double length = VecLength2f(vl+vr) + 0* VecLength2f(vr);
	if (length > 100) {
		V2d current_pos = s.pos;
		s.pos = current_pos-0.4*vl;
		nproduce C(s);
		wall_state new_wall;
		new_wall.flux = 0.0*(wl.flux);
		new_wall.NormalGrowth = wl.NormalGrowth;
		new_wall.LengthGrowth = wl.LengthGrowth;
		nproduce W(new_wall);
		s.pos = current_pos + 0.4*vr;
	}
	produce C(s);

}*/

group ComputeFluxes: // Calculate fluxes through the wall 
production:

W(wl) < C(s) > W(wr) :	// Auxin flux throught membrane, PIN allocation
{
	s.flux = T * ( wl.c*wl.PINr - wr.c*wr.PINl) + D*(wl.c-wr.c);
	
	produce C(s);
}


group TransPol: 	// Auxin production, transport, and decay; PIN polarization
production:

W(sl)C(wl) < W(s) > C(wr)W(sr) :		// a cell
{
	s.posl = wl.pos;
	s.posr = wr.pos;
	s.normall=wl.normal;
	s.normalr=wr.normal;
	s.wallnl = sl.walln;
	s.wallnr = sr.walln;

	s.c += (wl.flux-wr.flux-nu*s.c)*dt;	// auxin flows in, out, and decays
	s.c += (/*0.8+*/s.loc_sigma*(s.loc_H-s.c))*dt;	// produce/maintain high auxin concentration

	double totalPIN = 1;
	double e = 0.01 ;
	// s.PINl = totalPIN * (sl.c+e) / (sl.c + sr.c+2*e) ;
	// s.PINr = totalPIN * (sr.c+e) / (sl.c + sr.c +2*e) ;
/*	if(s.cuc>1){
	s.PINl = totalPIN * pow((sl.c+e),3.0f) / (pow((sl.c+e),3.0f) + pow((sr.c+e),3.0f)) ;
	s.PINr = totalPIN * pow((sr.c+e),3.0f) / (pow((sl.c+e),3.0f) + pow((sr.c+e),3.0f)) ;
	}*/
	if(s.cuc>1){
	s.PINl = totalPIN * pow(6.0, (sl.c+e)) / (pow(6.0, (sl.c+e)) + pow(6.0, (sr.c+e))) ;
	s.PINr = totalPIN * pow(6.0, (sr.c+e)) / (pow(6.0, (sl.c+e)) + pow(6.0,(sr.c+e))) ;
	}
//	else{
//	s.PINl = 0.9999*(s.PINl)+0.0001*totalPIN * pow(3.0f, (sl.c+e)) / (pow(3.0f, (sl.c+e)) + pow(3.0f, (sr.c+e))) ;
//	s.PINr = 0.9999*(s.PINr)+0.0001*totalPIN * pow(3.0f, (sr.c+e)) / (pow(3.0f, (sl.c+e)) + pow(3.0f,(sr.c+e))) ;
//	}

	//CUC
	s.cuc += 18.0*(PROD_CUC*s.cuc/(KCUC+DR_AUXIN*s.c) -DECAY_CUC*s.cuc )*dt;
	if(s.cuc <0.05)
	  s.cuc = 0;
	if(s.cuc > CUC_int)
	  s.cuc = CUC_int;
	if(step>900 && cuc_mutant)
	  s.cuc = 0;

	//s.posl = sl.pos;
	//s.posr = sr.pos;

	produce W(s);
}

W(s)>C(sr)W(wr)C(sr2):
{
s.posr = sr.pos;
s.posl = sr.pos + (sr.pos-sr2.pos);
s.normalr=sr.normal;
s.normall=s.normalr;
s.wallnr=wr.walln;
s.wallnl=wr.walln;
s.c = 0;
s.PINl = wr.PINl;
s.PINr = wr.PINr;
	if(step>900 && cuc_mutant)
	  s.cuc = 0;
produce W(s);
}

C(sl2)W(wl)C(sl)<W(s):
{
s.posl = sl.pos;
s.posr = sl.pos +(sl.pos-sl2.pos);
s.normall=sl.normal;
s.normalr=s.normall;
s.wallnl=wl.walln;
s.wallnr=wl.walln;
s.c = 0;
s.PINl = wl.PINl;
s.PINr = wl.PINr;
	if(step>900 && cuc_mutant)
	  s.cuc = 0;
produce W(s);
}


interpretation:
maximum depth: 10;
group 0:
			

W(s):
{
	//Fix the color in each cell
	bool CUCcolor = true;
	bool AuxinColor = false;

	//Vary the size of elements based on concentration?
	bool AuxinSize = false;
	bool CUCSize = true;
	//Normalized size
	double SizeAuxin = (AuxinSize) ? 0.7*s.c/Href :1;
	double SizeCUC = (CUCSize) ? 0.1*(s.cuc-0) :1;

	//Visualize concentration as the background color for the cell
	bool BGCUC = false;
	bool BGAuxin = true;
	
	//Visualize concentration using a reveal
	bool RevealAuxin = false;
	bool RevealCUC = false;

	//Visualize CUC and or Auxin as icons in the cell (circle and box respectively)
	bool CUCcircle = true;
	bool AuxinBox = false;
	bool CUCAuxinIcon = false;

	//Visualize as strips and half strips
	bool CUCStrip = false;
	bool AuxinStrip = false;
	bool HalfStrip = false;

	V2d pos = 0.5*(s.posr+s.posl);
	V2d perpl = 10.0*s.normall;
	V2d perpr = 10.0*s.normalr;


	if(step>10){
	perpl = VecNormalize2f(perpl);
	perpr = VecNormalize2f(perpr);
	s.walln = VecNormalize2f(s.walln);
	s.wallnl = VecNormalize2f(s.wallnl);
	s.wallnr = VecNormalize2f(s.wallnr);
	}
	float al = 15.0/2.0*(1/fabs(perpl*s.walln)+1/fabs(perpl*s.wallnl));
	float ar = 15.0/2.0*(1/fabs(perpr*s.walln)+1/fabs(perpr*s.wallnr));
	al = min(al,17);
	ar = min(ar,17);
	perpl = al*perpl;
	perpr = ar*perpr;





	V2d tl = (s.posl) + 0.25*s.cuc*perpl;
	V2d bl = (s.posl) - 0.25*s.cuc*perpl;
	V2d tr = (pos) + 0.25*s.cuc*(perpl+perpr)*0.5;
	V2d br = (pos) - 0.25*s.cuc*(perpl+perpr)*0.5;


//	nproduce SetColor(32+(int)(67.0f*CUC_int/10.0*s.cuc))FilledQuad2f(tl,tr,br,bl);

	 tl = (s.posr) + 0.2*s.c*perpr;
	 bl = (s.posr) - 0.2*s.c*perpr;
	 tr = (pos) + 0.2*s.c*(perpr+perpl)*0.5;
	 br = (pos) - 0.2*s.c*(perpr+perpl)*0.5;


//

//Background Box -visualization


	V2d topleftBB = (s.posl) + perpl;
	V2d bottomleftBB = (s.posl ) - perpl;
	V2d toprightBB = (s.posr) + perpr;
	V2d bottomrightBB = (s.posr) - perpr;

	//Auxin background
	if(BGAuxin || RevealAuxin){
	nproduce SetAuxinColor(AuxinColor,s.c);
	nproduce FilledQuad2f(topleftBB,toprightBB,bottomrightBB,bottomleftBB);
	if(RevealAuxin){
	double SizeAR = max(1.0-SizeAuxin,0);
	topleftBB = (s.posl) + SizeAR*perpl;
	bottomleftBB = (s.posl ) - SizeAR*perpl;
	toprightBB = (s.posr) + SizeAR*perpr;
	bottomrightBB = (s.posr) - SizeAR*perpr;
	nproduce SetColor(19);
	nproduce FilledQuad2f(topleftBB,toprightBB,bottomrightBB,bottomleftBB);
	}
	}

	//Cuc background
	if(BGCUC || RevealCUC){
	nproduce SetCUCColor(CUCcolor,s.cuc);
	nproduce FilledQuad2f(topleftBB,toprightBB,bottomrightBB,bottomleftBB);

	if(RevealCUC){
	double SizeCUCR = max(1.0-SizeCUC,0);
	topleftBB = (s.posl) + SizeCUCR*perpl;
	bottomleftBB = (s.posl ) - SizeCUCR*perpl;
	toprightBB = (s.posr) + SizeCUCR*perpr;
	bottomrightBB = (s.posr) - SizeCUCR*perpr;
	nproduce SetColor(19);
	nproduce FilledQuad2f(topleftBB,toprightBB,bottomrightBB,bottomleftBB);
	}

	}





//Centered Icon

	double sizeA = min(6.0,6.0*SizeAuxin);
	double sizeC = min(9.0,SizeCUC*10.0);

	//CUC circle
	if(CUCcircle || CUCAuxinIcon){
	nproduce SetCUCColor(CUCcolor,s.cuc);
	if(!CUCAuxinIcon){
	//	nproduce SB()MoveTo2d(pos)Circle(sizeC)EB();
		double cell_length = VecLength2f(s.posl-s.posr);
		double a = 9.0* pow(s.PINl, 1.0)/cell_length;	// proportion of PIN in the left wall	
		double b = 9.0* pow(s.PINr, 1.0)/cell_length;	// proportion of PIN in the left wall	
		V2d midpCUC= (1-a + b)/2.0*s.posl+(a+1-b)/2.0*s.posr;
		nproduce SB()MoveTo2d(midpCUC)Circle(sizeC)EB();
	}
	else
		nproduce SB()MoveTo2d(2.0/3.0*s.posr+1.0/3.0*s.posl)Circle(sizeC*0.75)EB();
	}

	//Auxin Strip
	if(AuxinStrip || HalfStrip){
	sizeA/=7;
	V2d topleftCB = (s.posl) + sizeA*perpl;
	V2d bottomleftCB = (s.posl ) - sizeA*perpl;
	V2d toprightCB = (s.posr) + sizeA*perpr;
	V2d bottomrightCB = (s.posr) - sizeA*perpr;
	//Half Auxin Strip
	if(HalfStrip){
	topleftCB = (s.posl) + (sizeA/2)*perpl+0.5*perpl;
	bottomleftCB = (s.posl ) - (sizeA/2)*perpl+0.5*perpl;
	toprightCB = (s.posr) + (sizeA/2)*perpr+0.5*perpr;
	bottomrightCB = (s.posr) - (sizeA/2)*perpr+0.5*perpr;
	}
	nproduce SetAuxinColor(AuxinColor,s.c);
	nproduce  FilledQuad2f(topleftCB,toprightCB,bottomrightCB,bottomleftCB);
	}
	//CUC Strip
	if(CUCStrip || HalfStrip){
	sizeC/=7;
	V2d topleftCB = (s.posl) + sizeC*perpl;
	V2d bottomleftCB = (s.posl ) - sizeC*perpl;
	V2d toprightCB = (s.posr) + sizeC*perpr;
	V2d bottomrightCB = (s.posr) - sizeC*perpr;
	//Half Cuc Strip
	if(HalfStrip){
	topleftCB = (s.posl) + (sizeC/2)*perpl-0.5*perpl;
	bottomleftCB = (s.posl ) - (sizeC/2)*perpl-0.5*perpl;
	toprightCB = (s.posr) + (sizeC/2)*perpr-0.5*perpr;
	bottomrightCB = (s.posr) - (sizeC/2)*perpr-0.5*perpr;
	}
	if(s.cuc>0.00001){
	nproduce SetCUCColor(CUCcolor,s.cuc);
	nproduce  FilledQuad2f(topleftCB,toprightCB,bottomrightCB,bottomleftCB);
	}

	}


//Centered Box

	if(AuxinBox || CUCAuxinIcon){
	V2d axis2 = VecTurnLeft2f(s.walln);
	V2d cent = (CUCAuxinIcon)? 2.0/3.0*s.posl+1.0/3.0*s.posr : pos;
	sizeA *= (CUCAuxinIcon)? 0.75:1.0;
	V2d topleftCB = (cent) + 0.75*sizeA*(s.walln+axis2);
	V2d bottomleftCB = (cent ) - 0.75*sizeA*(s.walln-axis2);
	V2d toprightCB = (cent) + 0.75*sizeA*(s.walln-axis2);
	V2d bottomrightCB = (cent) - 0.75*sizeA*(s.walln+axis2);

	nproduce SetAuxinColor(AuxinColor,s.c);
	nproduce  FilledQuad2f(topleftCB,toprightCB,bottomrightCB,bottomleftCB);
	}



//Pin Visualization
	V2d topleftp = (s.posl) + 0.9*perpl;
	V2d bottomleftp = (s.posl ) - 0.9*perpl;
	V2d toprightp = (s.posr) + 0.9*perpr;
	V2d bottomrightp = (s.posr) - 0.9*perpr;
	double cell_length = VecLength2f(s.posl-s.posr);
	double a = 9.0* pow(s.PINl, 1.0)/cell_length;	// proportion of PIN in the left wall

	nproduce SetColor(BW+15);		// PIN color;
	nproduce FilledQuad2f(topleftp, (1-a)*topleftp + a*toprightp,
			(1-a)*bottomleftp + a*bottomrightp, bottomleftp);

	double b = 9.0* pow(s.PINr, 1.0)/cell_length;	// proportion of PIN in the left wall

	nproduce FilledQuad2f(b*topleftp +(1-b)*toprightp, toprightp,
			bottomrightp, b*bottomleftp + (1-b)*bottomrightp);


//Cell Frame Visualization
	V2d topleft = (s.posl) + perpl;
	V2d bottomleft = (s.posl ) - perpl;
	V2d topright = (s.posr) + perpr;
	V2d bottomright = (s.posr) - perpr;

	nproduce SetColor(BW+5);		 // draw cell frame
	nproduce EmptyQuad2f(topleft, topright, bottomright, bottomleft);

produce;
}


/*
W(s):
{
	//Fix the color in each cell
	bool CUCcolor = false;
	bool AuxinColor = false;

	//Vary the size of elements based on concentration?
	bool AuxinSize = false;
	bool CUCSize = false;
	//Normalized size
	double SizeAuxin = (AuxinSize) ? 0.7*s.c/Href :1;
	double SizeCUC = (CUCSize) ? 0.1*s.cuc :1;

	//Visualize concentration as the background color for the cell
	bool BGCUC = false;
	bool BGAuxin = true;
	
	//Visualize concentration using a reveal
	bool RevealAuxin = false;
	bool RevealCUC = false;

	//Visualize CUC and or Auxin as icons in the cell (circle and box respectively)
	bool CUCcircle = false;
	bool AuxinBox = false;
	bool CUCAuxinIcon = false;

	//Visualize as strips and half strips
	bool CUCStrip = false;
	bool AuxinStrip = false;
	bool HalfStrip = true;

	V2d pos = 0.5*(s.posr+s.posl);
	V2d perpl = 10.0*s.normall;
	V2d perpr = 10.0*s.normalr;


	if(step>10){
	perpl = VecNormalize2f(perpl);
	perpr = VecNormalize2f(perpr);
	s.walln = VecNormalize2f(s.walln);
	s.wallnl = VecNormalize2f(s.wallnl);
	s.wallnr = VecNormalize2f(s.wallnr);
	}
	float al = 10.0/2.0*(1/fabs(perpl*s.walln)+1/fabs(perpl*s.wallnl));
	float ar = 10.0/2.0*(1/fabs(perpr*s.walln)+1/fabs(perpr*s.wallnr));
	al = min(al,20);
	ar = min(ar,20);
	perpl = al*perpl;
	perpr = ar*perpr;





	V2d tl = (s.posl) + 0.25*s.cuc*perpl;
	V2d bl = (s.posl) - 0.25*s.cuc*perpl;
	V2d tr = (pos) + 0.25*s.cuc*(perpl+perpr)*0.5;
	V2d br = (pos) - 0.25*s.cuc*(perpl+perpr)*0.5;


//	nproduce SetColor(32+(int)(67.0f*CUC_int/10.0*s.cuc))FilledQuad2f(tl,tr,br,bl);

	 tl = (s.posr) + 0.2*s.c*perpr;
	 bl = (s.posr) - 0.2*s.c*perpr;
	 tr = (pos) + 0.2*s.c*(perpr+perpl)*0.5;
	 br = (pos) - 0.2*s.c*(perpr+perpl)*0.5;


//

//Background Box -visualization


	V2d topleftBB = (s.posl) + perpl;
	V2d bottomleftBB = (s.posl ) - perpl;
	V2d toprightBB = (s.posr) + perpr;
	V2d bottomrightBB = (s.posr) - perpr;

	//Auxin background
	if(BGAuxin || RevealAuxin){
	nproduce SetAuxinColor(AuxinColor,s.c);
	nproduce FilledQuad2f(topleftBB,toprightBB,bottomrightBB,bottomleftBB);
	if(RevealAuxin){
	double SizeAR = max(1.0-SizeAuxin,0);
	topleftBB = (s.posl) + SizeAR*perpl;
	bottomleftBB = (s.posl ) - SizeAR*perpl;
	toprightBB = (s.posr) + SizeAR*perpr;
	bottomrightBB = (s.posr) - SizeAR*perpr;
	nproduce SetColor(19);
	nproduce FilledQuad2f(topleftBB,toprightBB,bottomrightBB,bottomleftBB);
	}
	}

	//Cuc background
	if(BGCUC || RevealCUC){
	nproduce SetCUCColor(CUCcolor,s.cuc);
	nproduce FilledQuad2f(topleftBB,toprightBB,bottomrightBB,bottomleftBB);

	if(RevealCUC){
	double SizeCUCR = max(1.0-SizeCUC,0);
	topleftBB = (s.posl) + SizeCUCR*perpl;
	bottomleftBB = (s.posl ) - SizeCUCR*perpl;
	toprightBB = (s.posr) + SizeCUCR*perpr;
	bottomrightBB = (s.posr) - SizeCUCR*perpr;
	nproduce SetColor(19);
	nproduce FilledQuad2f(topleftBB,toprightBB,bottomrightBB,bottomleftBB);
	}

	}





//Centered Icon

	double sizeA = min(6.0,6.0*SizeAuxin);
	double sizeC = min(6.0,SizeCUC*s.cuc);

	//CUC circle
	if(CUCcircle || CUCAuxinIcon){
	nproduce SetCUCColor(CUCcolor,s.cuc);
	if(!CUCAuxinIcon)
		nproduce SB()MoveTo2d(pos)Circle(sizeC)EB();
	else
		nproduce SB()MoveTo2d(2.0/3.0*s.posr+1.0/3.0*s.posl)Circle(sizeC*0.75)EB();
	}

	//Auxin Strip
	if(AuxinStrip || HalfStrip){
	sizeA/=7;
	V2d topleftCB = (s.posl) + sizeA*perpl;
	V2d bottomleftCB = (s.posl ) - sizeA*perpl;
	V2d toprightCB = (s.posr) + sizeA*perpr;
	V2d bottomrightCB = (s.posr) - sizeA*perpr;
	//Half Auxin Strip
	if(HalfStrip){
	topleftCB = (s.posl) + (sizeA/2)*perpl+0.5*perpl;
	bottomleftCB = (s.posl ) - (sizeA/2)*perpl+0.5*perpl;
	toprightCB = (s.posr) + (sizeA/2)*perpr+0.5*perpr;
	bottomrightCB = (s.posr) - (sizeA/2)*perpr+0.5*perpr;
	}
	nproduce SetAuxinColor(AuxinColor,s.c);
	nproduce  FilledQuad2f(topleftCB,toprightCB,bottomrightCB,bottomleftCB);
	}
	//CUC Strip
	if(CUCStrip || HalfStrip){
	sizeC/=7;
	V2d topleftCB = (s.posl) + sizeC*perpl;
	V2d bottomleftCB = (s.posl ) - sizeC*perpl;
	V2d toprightCB = (s.posr) + sizeC*perpr;
	V2d bottomrightCB = (s.posr) - sizeC*perpr;
	//Half Cuc Strip
	if(HalfStrip){
	topleftCB = (s.posl) + (sizeC/2)*perpl-0.5*perpl;
	bottomleftCB = (s.posl ) - (sizeC/2)*perpl-0.5*perpl;
	toprightCB = (s.posr) + (sizeC/2)*perpr-0.5*perpr;
	bottomrightCB = (s.posr) - (sizeC/2)*perpr-0.5*perpr;
	}
	if(s.cuc>0.00001){
	nproduce SetCUCColor(CUCcolor,s.cuc);
	nproduce  FilledQuad2f(topleftCB,toprightCB,bottomrightCB,bottomleftCB);
	}

	}


//Centered Box

	if(AuxinBox || CUCAuxinIcon){
	V2d axis2 = VecTurnLeft2f(s.walln);
	V2d cent = (CUCAuxinIcon)? 2.0/3.0*s.posl+1.0/3.0*s.posr : pos;
	sizeA *= (CUCAuxinIcon)? 0.75:1.0;
	V2d topleftCB = (cent) + 0.75*sizeA*(s.walln+axis2);
	V2d bottomleftCB = (cent ) - 0.75*sizeA*(s.walln-axis2);
	V2d toprightCB = (cent) + 0.75*sizeA*(s.walln-axis2);
	V2d bottomrightCB = (cent) - 0.75*sizeA*(s.walln+axis2);

	nproduce SetAuxinColor(AuxinColor,s.c);
	nproduce  FilledQuad2f(topleftCB,toprightCB,bottomrightCB,bottomleftCB);
	}



//Pin Visualization
	V2d topleftp = (s.posl) + 0.9*perpl;
	V2d bottomleftp = (s.posl ) - 0.9*perpl;
	V2d toprightp = (s.posr) + 0.9*perpr;
	V2d bottomrightp = (s.posr) - 0.9*perpr;
	double cell_length = VecLength2f(s.posl-s.posr);
	double a = 6.0* pow(s.PINl, 1.0f)/cell_length;	// proportion of PIN in the left wall

	nproduce SetColor(BW+15);		// PIN color;
	nproduce FilledQuad2f(topleftp, (1-a)*topleftp + a*toprightp,
			(1-a)*bottomleftp + a*bottomrightp, bottomleftp);

	double b = 6.0* pow(s.PINr, 1.0f)/cell_length;	// proportion of PIN in the left wall

	nproduce FilledQuad2f(b*topleftp +(1-b)*toprightp, toprightp,
			bottomrightp, b*bottomleftp + (1-b)*bottomrightp);


//Cell Frame Visualization
	V2d topleft = (s.posl) + perpl;
	V2d bottomleft = (s.posl ) - perpl;
	V2d topright = (s.posr) + perpr;
	V2d bottomright = (s.posr) - perpr;

	nproduce SetColor(BW+5);		 // draw cell frame
	nproduce EmptyQuad2f(topleft, topright, bottomright, bottomleft);

produce;
}
*/
/*
C(s) :			// Visualize cell
{
	V2f pos(s.pos.x,s.pos.y);

	nproduce LineTo2f(pos)Circle(8.0);
	produce ;
}
*/

EmptyQuad2f(tl, tr, br, bl) :
{
	V2f tlf(tl.x,tl.y);
	V2f trf(tr.x,tr.y);
	V2f brf(br.x,br.y);
	V2f blf(bl.x,bl.y);
	produce SB() SetWidth(2)MoveTo2f(tlf) LineTo2f(trf) LineTo2f(brf) LineTo2f(blf) LineTo2f(tlf) EB();
}

FilledQuad2f(tl, tr, br, bl) :
{
	V2f tlf(tl.x,tl.y);
	V2f trf(tr.x,tr.y);
	V2f brf(br.x,br.y);
	V2f blf(bl.x,bl.y);
	produce SB() SP() MoveTo2f(tlf) PP LineTo2f(trf) PP LineTo2f(brf) PP LineTo2f(blf) PP LineTo2f(tlf) PP EP() EB();
}


EmptyRectangle(l, w) :
{
	nproduce SB() TurnAround() f(l/2.0) Right(90) f(w/2.0); // go from center to left bottom corner
	produce Right(90) F(l) Right(90) F(w) Right(90) F(l) Right(90) F(w) EB(); // draw frame
}

FilledRectangle(l, w) :
{
	nproduce SB() TurnAround() f(l/2.0) Right(90) f(w/2.0); // go from center to left bottom corner
	produce Right(90) SP() f(l) Right(90) f(w) Right(90) f(l) Right(90) f(w) EP() EB(); // draw rectangle
}

Arrowhead(l, ws, we) :
{
	nproduce SB() SP ();	// start polygon
	nproduce SB () TurnAround() g(l/2.0) Right(90) g(ws/2.0) PP() EB(); // bottom left corner
	nproduce SB () Left(90) g(we/2.0) PP() EB(); 	// top left corner
	nproduce SB () Right(90) g(we/2.0) PP() EB(); 	// top right corner
	nproduce SB () TurnAround() g(l/2.0) Left(90) g(ws/2.0) PP() EB(); // bottom right corner
	produce EP() EB();	// end polygfon
}

SetCUCColor(fixed,cuc):
{
	if(!fixed)
		produce SetColor(32+(int)(67.0f*1.0/CUC_int*cuc));
	else
		produce SetColor(99);
}

SetAuxinColor(fixed,auxin):
{
	if(!fixed)
		produce SetColor(100 + (int)(128.0f * 0.55* auxin/Href));
	else
		produce SetColor(168);
}


