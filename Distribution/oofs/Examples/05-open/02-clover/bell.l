/* ---------------------------- DEFINES ------------------------------------- */

#define STEPS 1 /* number of years */

/* following parameters represent AVERAGE values in FULL LIGHT 
   and nutrient-rich soil for Trifolium repens (see Ming Dong's 
   thesis) */

#define IntLen  (2.0*2)   /* length of internode (cm) */
#define IntLenVar 0.10  /* variation (0-1) */

#define MinLight 0.1 /* min. light intensity required for growth */

#define ModInt  20   /* internodes per module */
#define ModIntVar 0.1

#define ModBr   12   /* branches per module */

#define ModRam   6  /* leaf ramets per module */

#define RamLA    (2.8*4)  /* leaf area per ramet (cm^2) */
#define RamLAVar 0.2

#define RamHght  6.0  /* height per ramet (cm) */
#define RamHghtVar 0.1

#define IntAge 20

#define BAngle 45   /* branching angle */

#define ZA 5

#define ran_val(x,p) ((x)+nran(0,(x)*(p)))

#define SEED 22

#define SIZE 102.5

#define BACKGROUND [,(3)@Tx(1)@M(-SIZE/2,-SIZE/2,-0.1){f(SIZE)-(90)f(SIZE)\
	-(90)f(SIZE)-(90)f(SIZE)}]

/*================================================================*/
Lsystem: 1
derivation length: STEPS
Start: {srand(SEED);n=0;}
EndEach: {n=n+1;printf("Step: %.0f\n",n);} 
Consider: ?E
Axiom: @M(20,25,0)!(0.2)BACKGROUND+(45) A(0,0,1)?E(1)

/* AGING ---------------------------------------------------------*/

F(l,a) : a>=1 --> F(l,a-1)
F(l,a) : a==0 --> f(l)

L(la,a,rn) : a>=1 --> L(la,a-1,rn)
L(la,a,rn) : a==0 --> ?E(2,la)

S(h,a) : a>=1 --> S(h,a-1)
S(h,a) : a==0 --> *

/* new module with 'int' internodes */
A(int,n,dir) > ?E(r) : n==int 
	{num = ModInt - ModInt*fabs(r-0.4);}
	--> A(ran_val(num,ModIntVar),0,dir)

/* add an internode */
A(int,n,dir) > ?E(r) : r >= MinLight
	--> R(r,int)B(r,int,dir)F(ran_val(IntLen,IntLenVar), IntAge)
	    +((ZA+nran(0,2))*dir)A(int,n+1,-dir)

A(int,n,dir) --> *

/* branching */
#define ModBrNoLight  0.2

B(r,int,dir) : {br_prob = ModBr*(ModBrNoLight+r*(1-ModBrNoLight));}
	(ran(1) < br_prob/int)
	--> [+(BAngle*dir)F(ran_val(IntLen,IntLenVar),IntAge)
	     A(0,0,1)?E(1)]
B(r,int,dir) --> *

/* ramets */
#define ModRamNoLight  0.1
#define LANoLight      1.5

R(r,int) : {rm_prob = ModRam*(ModRamNoLight+r*2*(1-ModRamNoLight));}
	(ran(1) < rm_prob/int)
	   {la = RamLA*(LANoLight+r*(1-LANoLight));
	    la = ran_val(la,RamLAVar);
	    h = ran_val(RamHght,RamHghtVar);
	    rn = floor(nran(0,10)+50)*10000+floor(nran(0,10)+50)*100+
		 floor(nran(0,10)+50)+ran(1);}
	--> [^(90)/(ran(180))+(nran(0,20))S(h,IntAge+1)
	     L(la,IntAge,rn)?E(1,la)]

R(r,int) --> *

?E(t,la) --> *

homomorphism

S(h,age) : age>=1 --> !(0.2)@Tf@Ts(4,0.6)F(h/3)F(h/3)F(h/3)@Ts(4,0)
S(h,age) --> !(0.2)@Tf@Ts(4,0.6)f(h/3)f(h/3)f(h/3)@Ts(4,0)

#define LEAVES 1

#if LEAVES==1

#define frac(x) ((x)-floor(x))

#define LEAF3(la) {.+(35)f(len)-(60)f(len/4)-(65)f(len/3)\
	-(55)f(len/5)+(2*55)f(len/5)-(55)f(len/3)-(55)f(len/4)}

L(la,age,rn) : frac(rn) > 0.03 {len = sqrt(la/3.14);} 
	-->[,(2)&(90)@Ts(4,0)[LEAF3(len)]+(120+rn/10000-50)[LEAF3(len)]
	+(120+frac(rn/10000)*100-50)[LEAF3(len)]]

L(la,age,rn) : * {len = sqrt(la/3.14);}
	-->[,(2)&(90)@Ts(4,0)[LEAF3(len)]+(90+rn/10000-50)[LEAF3(len)]
		+(90+frac(rn/10000)*100-50)[LEAF3(len)]
		+(90+frac(rn/100)*100-50)[LEAF3(len)]]

#else
L(la,age,rn) -->[,(2)&(90)@c(2.0*sqrt(la/3.14))]
#endif

endlsystem
