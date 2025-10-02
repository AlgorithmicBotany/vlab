
#define GROUNDSIZE 40
#define GROUND [&(90)f(GROUNDSIZE/2)+(90)F(GROUNDSIZE/2)+(90)F(GROUNDSIZE)+(90)F(GROUNDSIZE)+(90)F(GROUNDSIZE)+(90)F(GROUNDSIZE/2)]

#define MAX_ORD 6  /* max. order */
#define MAX_LAYER 5 /* max. number of layers */
#define MAX_SEMINALS 10
#define MAX_NODALS   10

#define FILENAME "roots.spec"

#define MEM 1.00 /* how much the new direction depends on the memorized
		    heading direction */
#define GrW  0.4    /* gradient weight */
 
#define SEED 25

lsystem: 1
define: {
	array grow[MAX_ORD],
	 width[MAX_ORD],
	 maxwidth[MAX_ORD],
	 widthincr[MAX_ORD],
	 maxlength[MAX_ORD],
	 geo[MAX_ORD],
	 grsen[MAX_ORD],
	 depl[MAX_ORD],
	 defl[MAX_LAYER][MAX_ORD],
	 brlag[MAX_LAYER][MAX_ORD],
	 branch[MAX_LAYER][MAX_ORD],
	 depth[MAX_LAYER],
	 semprob[MAX_SEMINALS],
	 semtime[MAX_SEMINALS],
	 nodprob[MAX_NODALS],
	 nodtime[MAX_NODALS];
	}
derivation length: 1
Consider: @v?PHUIE
Start: {/* default values */
	germ_time = 5*24;  /* 5 days */
	initial_defl = 0;    /* initial deflection index for seminal exes */
	
	fp = fopen(FILENAME,"r");
	fscanf(fp,"Time step: %lf\n",&time_step);
	fscanf(fp,"Seeding depth: %lf\n",&seeding_depth);
	fscanf(fp,"Min temperature: %lf\n",&min_temp);
	fscanf(fp,"Orders: %lf\n",&ord);
	fscanf(fp,"Growth rate:"); i = 0;
	while(i<ord) { fscanf(fp,"%lf", &grow[i]); i=i+1;}
	fscanf(fp,"\nWidth:"); i = 0;
	while(i<ord) { fscanf(fp,"%lf", &width[i]); i=i+1;}
	fscanf(fp,"\nWidth increment:"); i = 0;
	while(i<ord) { fscanf(fp,"%lf", &widthincr[i]); i=i+1;}
	fscanf(fp,"\nMaximum width:"); i = 0;
	while(i<ord) { fscanf(fp,"%lf", &maxwidth[i]); i=i+1;}
	fscanf(fp,"\nMaximum length:"); i = 0;
	while(i<ord) { fscanf(fp,"%lf", &maxlength[i]); i=i+1;}
	fscanf(fp,"\nGeotropism index:"); i = 0;
	while(i<ord) { fscanf(fp,"%lf", &geo[i]); i=i+1;}
	fscanf(fp,"\nGradient sensitivity:"); i = 0;
	while(i<ord) { fscanf(fp,"%lf", &grsen[i]); i=i+1;}
	fscanf(fp,"\nDepletion:"); i = 0;
	while(i<ord) { fscanf(fp,"%lf", &depl[i]); i=i+1;}
	fscanf(fp, "\n");
	
	fscanf(fp,"Layers: %lf\n", &layers); l = 0;
	while(l<layers) {
	  fscanf(fp,"Layer depth: %lf\n",&depth[l]);
	  fscanf(fp,"Deflection index:"); i = 0;
	  while(i<ord) { fscanf(fp,"%lf", &defl[l][i]); i=i+1;}
	  fscanf(fp,"\nBranch interval:"); i = 0;
	  while(i<ord) { fscanf(fp,"%lf", &branch[l][i]); i=i+1;}
	  fscanf(fp,"\nBranching lag:"); i = 0;
	  while(i<ord) { fscanf(fp,"%lf", &brlag[l][i]); i=i+1;}
	  fscanf(fp, "\n");
	  l=l+1;
	}
	fscanf(fp,"Number of seminal pairs: %lf\n", &sem_num); 
	i = 0;
	while(i<sem_num) { 
	  fscanf(fp,"pair %lf: %lf %lf\n", &j, &semprob[i], &semtime[i]);
	  i=i+1;
	}
	fscanf(fp,"Number of nodal pairs: %lf\n", &nod_num); 
	i = 0;
	while(i<sem_num) { 
	  fscanf(fp,"pair %lf: %lf %lf\n", &j, &nodprob[i], &nodtime[i]);
	  i=i+1;
	}
	step = 1;
	srand(SEED);
	}
EndEach: {printf("Step %.0lf\n",step); step=step+1;tp=0;}
axiom: ;(4)[+(180)f(seeding_depth)
	[[+(180)T(1,0)][R]-(90)f(2)+(90)[+(180)T(2,0)]R]S(0,sem_num)N(0,nod_num)]

/* aging of the tip */
T(n,age) --> T(n,age+1)

S(i,max) : i>= max --> *
S(i,max) : ran(100)<=semprob[i] --> 
			S(semtime[i]-i*time_step)S(i+1,max)
S(i,max) --> S(i+1,max)
S(t) : t<=time_step --> [\(ran(360))[R][R]]
S(t) --> S(t-time_step)

N(i,max) : i>= max --> *
N(i,max) : ran(100)<=nodprob[i] --> 
			N(nodtime[i]-i*time_step)N(i+1,max)
N(i,max) --> S(i+1,max)
N(t) : t<=time_step --> [\(ran(360))&(90)[R]|[R]]
N(t) --> N(t-time_step)

R --> ?E(depl[0],0,grsen[0],0,0,0,0)A(0,0,0)@v?P(0,0,0)?H(0,0,0)?U(0,0,0)

/* A(tl,l,o) - root tip
   tl - total length 
   l - length of the apical part (from the last branching point)
   o - branch order - main axis has order 0
 */
?E(nut,x,y,z,ox,oy,oz)A(tl,l,o) : tl > maxlength[o] --> *

?E(nut,nx,ny,nz,ox,oy,oz)A(tl,l,o) > @v?P(x,y,z)?H(hx,hy,hz)?U(ux,uy,uz) 
	:  (nut>0)&&(o>=0)&&((ox==0)&&(oy==0)&&(oz==0)) 
	{/* find a layer */
	 layer = 0;
	 dp = -y;
	 while(dp > depth[layer]) {
           dp = dp - depth[layer];
           layer = layer + 1;
         }
	
	 /* should a branch be initiated? */
	 br = 0;
	 if (l>branch[layer][o]) { 
	   br = 1;
	   l = 0;
	 } 
	 r1 = (ran(1)>0.5 ? -1 : 1)*180*(ran(1.0)^(1/(1-geo[o])));
	 r2 = 360*((ran(1.0)/2)^(1/defl[layer][o]))*(uy<0 ? 1 : -1);
	 /* this "if" condition prevents roots from growing out of the soil */
	 if(y > -grow[0]) { 
	   r1 = (r1>90 ? r1-90 : (r1 < -90 ? r1+90 : r1)); 
	   if(hy<=0) { 
	     r2 = (r2>90 ? r2-90 : (r2<-90 ? r2+90 : r2)); 
	   } else {
	     /* dr2 = acos(hy/sqrt(hx*hx+hy*hy+hz*hz)); */
	     dr2 = acos(hy); /* vector H is normalized */
	     r2 = (r2<dr2 ? dr2 : r2); 
	   }
	 }
	 len = grow[o]/24*time_step;
	}
	--> @R(hx+nx,hy+ny,hz+nz)@v\(r1)&(r2)
		B(o+1,br,brlag[layer][o])
		!(width[o],o,tl)L(len)I(hx,hy,hz)?E(depl[0],0,grsen[o],0,len-0.01,0,0)
		A(tl+len,l+len,o) 

/* obstacle or no nutrients - no growth */
?E(nut,x,y,z,ox,oy,oz)A(tl,l,o) --> 
	?E(0.0001,0,grsen[o],0,grow[o]/24*time_step-0.01,0,0)A(tl,l,o) 

/* pre branch marker (makes a branch) 
   o - branch number
   b - branch or not
   d - delay before branching
*/
B(o,br,d) :  (o>=ord) || (br==0 )--> *
B(o,br,d) :  d>0 --> B(o,br,d-time_step)
B(o,br,d) --> 
	[;(o+4)\(ran(360))+(90)
	?E(0.0001,0,grsen[o],0,0,0,0)A(0,0,o)
	 @v?P(0,0,0)?H(0,0,0)?U(0,0,0)]

/* obstacle avoiding */

L(len) > I(hx,hy,hz)?E(nut,x,y,z,ox,oy,oz) : (ox!=0)||(oy!=0)||(oz!=0) 
	{rx = MEM*hx + (1-MEM)*ox;
	 ry = MEM*hy + (1-MEM)*oy;
	 rz = MEM*hz + (1-MEM)*oz;}
	--> @R(ox,oy,oz)L(len)@R(rx,ry,rz)

/* thickening */
!(w,o,tl) : w+widthincr[o]>maxwidth[o] --> !(maxwidth[o],o,tl)
!(w,o,tl) --> !(w+widthincr[o],o,tl)

homomorphism

!(w,o,tl) : {wid = width[o]+widthincr[o]*
		 (maxlength[o]-tl)/(grow[o]/24*time_step);}
	(o>0)&&(w>wid) --> !(wid)

L(l) --> @O F(l)

#define STEPS_TO_FULL_SIZE  100

T(t,age) : * {w = width[0] + age*widthincr[0];
	      if(w>maxwidth[0]) {w=maxwidth[0];}
	      rat = age/STEPS_TO_FULL_SIZE; if(rat>1){rat=1;}}
	--> !(w)F(seeding_depth);(1)/(145*(t-1))-(3*t)
	F(10*rat)f(-5*rat)
	[;(2)@#(1)!(1.04*w)@Gs -(2*rat)f(5*rat)!(2*w)@Gc 
	-(7*rat)f(5*rat)!(2.5*w)@Gc!(3*w)
	-(7*rat)f(5*rat)@Gc 
	-(7*rat)f(5*rat)@Gc
	-(7*rat)f(5*rat)@Gc
	-(7*rat)f(5*rat)@Ge]
	f(5*rat)/(175)
	[;(2)@#(1)!(1.04*w)@Gs -(2*rat)f(5*rat)!(2*w)@Gc
	-(7*rat)f(5*rat)!(2.5*w)@Gc!(3*w)
	-(7*rat)f(5*rat)@Gc
	-(7*rat)f(5*rat)@Gc
	-(7*rat)f(5*rat)@Gc
	-(7*rat)f(5*rat)@Ge]
	F(30*rat)

endlsystem
