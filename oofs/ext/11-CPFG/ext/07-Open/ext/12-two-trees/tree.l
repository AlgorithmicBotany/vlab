#define STEPS 0
#define R0Angle 20
#define R1Angle 32
#define D1 137.5
/* vigor decrement */
#define VDECR 0.95

#define SEED 25

#define LSPAN  5 /* how long a leaf stays */

#define WID 0.03  /* width scaling */
#define WE  0.40   /* width exponent */

#define LEAFAGE  5

#define LP 8       /* leaf product for 100% of light */
#define LM (LP/8)  /* amount of product necessary for maintaining
		      a leaf */

#define Mmult  (0.32)  /* multiplicative coefficient for the maintenance */
#define ME     (0.75/WE)  /* width->maintenance exponent */

#define STAYS  25  /* how many terminal shoots a branch must have to 
		      stay */

#define PG (0.4) /* amount of product necessary for further growth */
#define PB (2*PG)   /* amount of product necessary for branching */


/* Straightening */
#define Sstep  1   /* straightening step */
#define SAngle 5   /* angle after straigtening */
#define Swidth (15*WID) /* max. width of a branch succeptible to 
		           straightening */

#define STEM(vig,flow,n) !(WID,1,flow)F(vig,n)


#define WHENMOVE 184
#define NUMSTEPS  90
#define MOVE_STEP (2/NUMSTEPS)

#define GROUNDL [,(6)@M(-6,0,-15)&(90)+(180){f(30)+(90)f(6)+(90)f(30)+(90)f(6)}]
#define GROUNDR [,(6)@M(6,0,-15)^(90){f(30)+(90)f(6)+(90)f(30)+(90)f(6)}]

Lsystem: 1
Start: {DEL=3;d=DEL+1;n=1;srand(SEED);}
StartEach: {printf("Step %0.f\n",n);n=n+1;
	  d=d-1;
	  if(d==0){DEL=DEL+1;d=DEL+1;printf("Year %.0f\n",d-2);}
	  }
EndEach: {if(d==1){stop(1);}}
derivation length: STEPS
Consider: ?E[]!L
Axiom: S;(1),(3)
	[GROUNDL @M(-2,0,0)!(WID,1,1)@GsF(2,0)#(WID)L(LP,LEAFAGE,DEL,1)A(1,DEL,1,1)@Ge[!(0,0,0)]!(WID,1,0)]
	[GROUNDR @M(2,0,0)!(WID,1,1)@GsF(2,0)#(WID)L(LP,LEAFAGE,DEL,1)A(1,DEL,1,1)@Ge[!(0,0,0)]!(WID,1,0)]

/* remove all active parts after 183 steps */
A(dir,del,vig,n) : n >= WHENMOVE --> *
L(f,age,del,vig) : (n >= WHENMOVE)&&(age>0)&&(f>0) --> L(del,vig)
L(f,age,del,vig) : (n >= WHENMOVE) --> *

@M(x,y,z) : (n > WHENMOVE)&&(n<=WHENMOVE+NUMSTEPS)&&(x>0)
	--> @M(x+MOVE_STEP,y,z)
@M(x,y,z) : (n > WHENMOVE)&&(n<=WHENMOVE+NUMSTEPS)&&(x<0)
	--> @M(x-MOVE_STEP,y,z)

/* waiting DEL steps */
A(dir,del,vig,n) : del>0 --> A(dir,del-1,vig,n)

L(f,age,del,vig) : (age>0)&&(del>1) --> L(f,age,del-1,vig)
L(f,age,del,vig) : (age>0)&&(del==1) --> L(f,age,0,vig)?E(vig*0.5)

/* Create two long shoots */
?E(r) < A(dir,del,vig,n) : {r=r*LP-LM;} r>PB --> /(D1)
	-(R0Angle*dir)[+((R1Angle+R0Angle)*dir)STEM(vig*VDECR,-PB,n)#(WID)
		L(LP,LEAFAGE,DEL,vig*VDECR)A(dir,DEL,vig*VDECR,n+1)@Ge
		[!(0,0,0)]!(WID,1,0)]
	STEM(vig*VDECR,0,n)#(WID)
		L(LP,LEAFAGE,DEL,vig*VDECR)A(-dir,DEL,vig*VDECR,n+1)


/* Create a long shoot */
?E(r) < A(dir,del,vig,n) : {r=r*LP-LM;} r>PG --> /(D1)
	-(SAngle*dir)[!(0,0,0)]STEM(vig*VDECR,-PG,n)#(WID)
		L(LP,LEAFAGE,DEL,vig*VDECR)A(-dir,DEL,vig*VDECR,n+1)

A(dir,del,vig,n) --> A(dir,DEL,vig,n)

/* straightening of main branch */
-(a) > [!(w1,n1,f1)L : (n < WHENMOVE)&&(f1<=0)&&(w1<=Swidth) 
		{dir=a>0?1:-1;} 
		--> -(a,Sstep*dir)
-(a,step) : (n < WHENMOVE)&&(fabs(a) > SAngle) --> -(a-step,step)
-(a,step) --> -(a)

/* straightening of lateral branch */
-(a) > !(w1,n1,f1)L : (n < WHENMOVE)&&(f1<=0)&&(w1<=Swidth) {dir=a>0?1:-1;} 
		--> -(a,Sstep*dir,0)
-(a,step,t) : (n < WHENMOVE)&&(fabs(a) < R0Angle+R1Angle-SAngle) 
		--> -(a+step,step,t)
-(a,step,t) --> -(a)

/* death of a leaf */
L(f,age,del,vig): age ==0 --> L(0,0,0,0)
L(f,age,del,vig) > ?E(r) : (r*LP<LM) --> L(0,0,0,0)

/* obtaining leaf product */
L(f,age,del,vig) > ?E(r) : age>0 --> L(LP*r-LM,age-1,DEL,vig)

?E(r) --> *

!(w0,n0,f0)  > L(fl,age,del,vig)[!(w1,n1,f1)=]!(w2,n2,f2) :
	{w=(w1^(1/WE)+w2^(1/WE))^WE;
	 flow=f1+f2+fl-Mmult*(w/WID)^ME;} flow > 0
		--> !(w,n1+n2,flow)

!(w0,n0,f0)  > L(fl,age,del,vig)[!(w1,n1,f1)=]!(w2,n2,f2) : 
		n1+n2>=STAYS
		{w=(w1^(1/WE)+w2^(1/WE))^WE;}
		--> !(w,n1+n2,0)

!(w0,n0,f0)  > L(fl,age,del,vig)[!(w1,n1,f1)=]!(w2,n2,f2)
		--> !(w0,0,0)L@Ge%

* < #(w0) > L(fl,age,del,vig)[!(w1,n1,f1)=]!(w2,n2,f2) : *
	{ wr= w1<w2? w2:w1;} --> #(wr)

homomorphism

,(c) : c==3 --> ,(1)

/*@M(x,y,z) --> @M(x+2*sign(x),y,z)*/

#define GEN_CYL_STEM 0    /* are ALL stems generalized cylinders? */
#define GEN_CYL_LEVEL 0   /* how many initial stems are drawn by
			     generalized cylinders? 
			     applicable only when GEN_CYL_STEM is 0 */
#define TAPERED_STEM 1    /* applicable only when GEN_CYL_STEM is 0 */
#define STEM_END     1     
			  /* 1-SPHERE, or 0-NOTHING
			     applicable only when TAPERED_STEM is 1
			     GEN_CYL should be used when TEXTURE_STEM
			     is 1 */
#define TEXTURE_STEM 1    /* 0 off, 1 on */

#define GEN_CYL_STALK 0   /* zero seems better */
#define LEAVES 0      /* 2 at the end */

/* branch texture on/off  */
S : (n > WHENMOVE+NUMSTEPS+1) {srand(SEED);} --> @Tx(TEXTURE_STEM)
S --> @Tx(TEXTURE_STEM)


#if GEN_CYL_STEM==1
F(len,n) --> f(len/2)@Gc(4)f(len/2)
#else
@G --> *  
#if TAPERED_STEM==0

*<#(w) --> *
#else

F(vig,n): (n==0)&&(GEN_CYL_LEVEL > 0) --> @Gsf(vig*3/4)@Gc(1)f(vig/4)
F(vig,n):  n < GEN_CYL_LEVEL-1 --> f(vig/2)@Gc(4)f(vig/2)

#if STEM_END==1

F(vig,n)#(w):  n==GEN_CYL_LEVEL-1 --> f(vig/2)@Gc(4)f(vig/2)#(w)@Ge@O
F(vig,n)#(w) --> F(vig)#(w)@O
#endif  /* STEM_END==1   SPHERE */
#endif  /* TAPERED_STEM==0 */

F(vig,n)#(w):  n==GEN_CYL_LEVEL-1 --> f(vig/2)@Gc(4)f(vig/2)#(w)@Ge
@G --> *
#endif /* GEN_CYL_STEM==1 */


/******* leaf shoot *******/
#define LAngle  20


#define LE 0.7 /* leaf elasticity */
#define TE 0.9  /* torque elasticity */
#define GE 0.3 /* gravitational elasticity */
#define Ls 0.25    /* leaf size */
#define SL (Ls*0.8)  /* stalk length */

#define SLi  (Ls/3*0.7)
#define Ai   55

#define ransize(x)  ((x)*0.9 + ran(0.1*(x)))

#define COMPOUND(vig) ~m(ransize(Ls*vig*1.5))

#define COMPOUND5(vig) @Ts(1,0.1)\
	[+(90)~m(ransize(Ls*vig*0.5))][-(90)~m(ransize(Ls*vig*0.5))]\
	G(SLi*5*vig)[+(90)~m(ransize(Ls*vig*0.5))][-(90)~m(ransize(Ls*vig*0.5))]\
	G(SLi*vig)~m(ransize(Ls*vig*0.5))

#define COMPOUND7(vig) @Ts(1,0.1)\
	[+(90)~m(ransize(Ls*vig*0.5))][-(90)~m(ransize(Ls*vig*0.5))]\
	G(SLi*5*vig)[+(90)~m(ransize(Ls*vig*0.5))][-(90)~m(ransize(Ls*vig*0.5))]\
	G(SLi*5*vig)[+(90)~m(ransize(Ls*vig*0.5))][-(90)~m(ransize(Ls*vig*0.5))]\
	G(SLi*vig)~m(ransize(Ls*vig*0.5))

#define COMPOUND9(vig) @Ts(1,0.1)\
	[+(90)~m(ransize(Ls*vig*0.5))][-(90)~m(ransize(Ls*vig*0.5))]\
	G(SLi*5*vig)[+(90)~m(ransize(Ls*vig*0.5))][-(90)~m(ransize(Ls*vig*0.5))]\
	G(SLi*5*vig)[+(90)~m(ransize(Ls*vig*0.5))][-(90)~m(ransize(Ls*vig*0.5))]\
	G(SLi*5*vig)[+(90)~m(ransize(Ls*vig*0.5))][-(90)~m(ransize(Ls*vig*0.5))]\
	G(SLi*vig)~m(ransize(Ls*vig*0.5))

/*#define COMPOUND(vig) @Ts(1,0.2)[G(SLi)~l(ransize(Ls*vig))]\
	[+(Ai)G(SLi/3)~l(ransize(Ls*vig*0.9))]\
	[-(Ai)G(SLi/3)~l(ransize(Ls*vig*0.9))]\
	[+(2*Ai+3)~l(ransize(Ls*vig*0.7))]\
	[-(2*Ai+3)~l(ransize(Ls*vig*0.7))]
*/
#if LEAVES>0

#if GEN_CYL_STALK==1

L(f,age,del,vig) : (age>0)&&(f>0)--> 
	[@Tx(0)&(LAngle),(2)!(0.01)@Tf@Gs 
	@Ts(2,LE)@Ts(3,TE)f(SL/4)@Tf@Gc(4)f(SL/4)@Tf@Gc(4)@Ts(2,0)
	@Ts(1,GE*(0.6+ran(0.4)))f(SL/4)@Tf@Gc(4)f(SL/4)@Tf@Ge(4)@Ts(3,0)@Ts(1,GE)
	COMPOUND(vig)]
#else

#if LEAVES==1

L(f,age,del,vig) : (age>0)&&(f>0) --> 
	[@Tx(0)&(LAngle),(2)!(0.01)@Tf 
	@Ts(2,LE/2)@Ts(3,TE/2)G(SL/8)G(SL/8)G(SL/8)@Ts(2,0)
	@Ts(1,GE*(0.7+ran(0.3))/2)
	G(SL/8)G(SL/8)G(SL/8)G(SL/8)G(SL/8)@Ts(3,0)@Ts(1,GE)
	COMPOUND(vig)]
#else
L(f,age,del,vig) : (age>0)&&(f>0)&&(ran(1)<0.3) --> 
	[@Tx(0)&(LAngle),(2)!(0.01)@Tf 
	@Ts(2,LE/2)@Ts(3,TE/2)G(SL/8)G(SL/8)G(SL/8)@Ts(2,0)
	@Ts(1,GE*(0.7+ran(0.3))/2)
	G(SL/8)G(SL/8)G(SL/8)G(SL/8)G(SL/8)@Ts(3,0)@Ts(1,GE)
	COMPOUND5(vig)]

L(f,age,del,vig) : (age>0)&&(f>0)&&(ran(1)<0.3) --> 
	[@Tx(0)&(LAngle),(2)!(0.01)@Tf 
	@Ts(2,LE/2)@Ts(3,TE/2)G(SL/8)G(SL/8)G(SL/8)@Ts(2,0)
	@Ts(1,GE*(0.7+ran(0.3))/2)
	G(SL/8)G(SL/8)G(SL/8)G(SL/8)G(SL/8)@Ts(3,0)@Ts(1,GE)
	COMPOUND7(vig)]

L(f,age,del,vig) : (age>0)&&(f>0) --> 
	[@Tx(0)&(LAngle),(2)!(0.01)@Tf 
	@Ts(2,LE/2)@Ts(3,TE/2)G(SL/8)G(SL/8)G(SL/8)@Ts(2,0)
	@Ts(1,GE*(0.7+ran(0.3))/2)
	G(SL/8)G(SL/8)G(SL/8)G(SL/8)G(SL/8)@Ts(3,0)@Ts(1,GE)
	COMPOUND9(vig)]
#endif

#endif  /* GEN_CYL_STALK==1 */

#else
/*L(f,age,del,vig) : (age>0)&&(f>0)--> 
	[@Tx(0)&(LAngle)#(0.01),(2(n > WHENMOVE+NUMSTEPS+1)(n > WHENMOVE+NUMSTEPS+1))F(0.3*vig)]*/

L(del,vig) : (n > WHENMOVE+NUMSTEPS+1)&&(ran(1)<0.3) --> 
	[@Tx(0)&(LAngle),(2)!(0.01)@Tf 
	@Ts(2,LE/2)@Ts(3,TE/2)G(SL/8)G(SL/8)G(SL/8)@Ts(2,0)
	@Ts(1,GE*(0.7+ran(0.3))/2)
	G(SL/8)G(SL/8)G(SL/8)G(SL/8)G(SL/8)@Ts(3,0)@Ts(1,GE)
	COMPOUND5(vig)]

L(del,vig) : (n > WHENMOVE+NUMSTEPS+1)&&(ran(1)<0.3) --> 
	[@Tx(0)&(LAngle),(2)!(0.01)@Tf 
	@Ts(2,LE/2)@Ts(3,TE/2)G(SL/8)G(SL/8)G(SL/8)@Ts(2,0)
	@Ts(1,GE*(0.7+ran(0.3))/2)
	G(SL/8)G(SL/8)G(SL/8)G(SL/8)G(SL/8)@Ts(3,0)@Ts(1,GE)
	COMPOUND7(vig)]

L(del,vig) : n > WHENMOVE+NUMSTEPS+1--> 
	[@Tx(0)&(LAngle),(2)!(0.01)@Tf 
	@Ts(2,LE/2)@Ts(3,TE/2)G(SL/8)G(SL/8)G(SL/8)@Ts(2,0)
	@Ts(1,GE*(0.7+ran(0.3))/2)
	G(SL/8)G(SL/8)G(SL/8)G(SL/8)G(SL/8)@Ts(3,0)@Ts(1,GE)
	COMPOUND9(vig)]
#endif  /* LEAVES==1 */

endlsystem
