#define STEPS 1
#define SHOW_NO_LABELS 1
#define SHOW_COUNT 0
#define SHOW_FLUX 0
#define PHI 180.0  /* could be 13705 for a 3D patern */
#define alpha_1 15 /* branching angle - straigh segment */
#define alpha_2 25 /* branching angle - lateral segment */
#define beta_1 15 /* branching angle - straigh segment */
#define beta_2 60 /* branching angle - lateral segment */
#define lambda 0.15 /* default flux portion flowing into the main segment */
#define lambda2 0.4
#define gamma 1.65
#define v_th 5.0
#define v_th2 15.0
#define eta 0.91
#define sigma_0 7 /* CHANGE TO 7 */
#define LEN 1
#define LS 0.5

#define SHOOT 0
#define ROOT 1

Lsystem: 1
ignore: +-/|X
Start: {alpha = alpha_1 + alpha_2; go=1; shoot=1;root=1;}
Define: {
	/* colour=1 for neutral, 2 for terminals down, 3 for flux up*/
	array len[2]={1, 0.85};
	}
derivation length: STEPS
/* Axiom:  N(1)I(0,2,0,1)A */
Axiom:  N[I(0,2,0,1,SHOOT,1)A][|I(0,2,0,1,ROOT,1)A]

/* A */
/* I(b,m,v,c,w) */

/* b - Type - 0=base, 1=major, 2=minor */
/* m - MType - 0=normal 1=flux up 2=terminals down */
/* v - Flux - Flux through the internode */
/* c - TerminalsSupported by this internode */
/* s - SHOOT or ROOT */
/* w - maximum width so far
/* First branch at any node is dominant */

/* terminal numbers reached base; start flux moving up */

N > [I(bs,ms,vs,cs,ss,w)=] 
	[I(br,mr,vr,cr,sr,wr)=] : 
	bs==0 && ms==2 && br==0 && mr==2 
	{go=1; shoot = cs; root=cr;
	printf("shoot = %lf, root = %lf\n", shoot, root);} -->
		N

N < I(b,m,v,c,s,w) : 
	b==0 && m==2 && s == SHOOT && go==1
	{flux=5*root+6;} -->
		I(b,1,flux,c,s,w)

N < I(b,m,v,c,s,w) : 
	b==0 && m==2 && s == ROOT && go==1
	{flux=5*shoot+6; go=0;} -->
		I(b,1,flux,c,s,w)

XI(b,m,v,c,s,w) --> I(b,2,0,0,s,0)D% 

/* flux reaching apices */
/* branch if sufficient flux */
/* note that this starts sending the terminal counting message down */

I(b,m,v,c,s,w) < A : m==1 && v>v_th && s == SHOOT -->
	/(PHI)[-(alpha_2,alpha_2)J(2,2,v*0.5*(1-lambda),1,s,1)]
	+(alpha_1,alpha_1)J(1,2,v*0.5*(1+lambda),1,s,1)

I(b,m,v,c,s,w) < A : m==1 && v>v_th && s == ROOT -->
	/(PHI)[-(beta_2)J(2,2,v*0.5*(1-lambda),1,s,1)]
	+(beta_1)J(1,2,v*0.5*(1+lambda),1,s,1)

I(b,m,v,c,s,w) > D : m==1 --> I(b,2,0,0,s,0) 

I(bl,ml,vl,cl,sl,wl) < +(a,a_new) > I(b,m,v,c,s,w) :
	m==1 && s==SHOOT
	--> +(a,alpha*(1-c/cl))

I(bl,ml,vl,cl,sl,wl) < -(a,a_new) > I(b,m,v,c,s,w) : 
	m==1 && s==SHOOT
	--> -(a,alpha*(1-c/cl))

+(a,a_new) --> +(a+0.1*(a_new-a),a_new)
-(a,a_new) --> -(a+0.1*(a_new-a),a_new)


/* Reset the internode message state */
/* and if the apex didn't branch start terminal down message */

I(b,m,v,c,s,w) > A : m==1 && v <= v_th --> I(b,2,v,c,s,w) 

/* upward movement of flux */
I(bl,ml,vl,cl,sl,wl) < I(b,m,v,c,s,w) : 
	ml==1 && b==1 && c>0 && cl !=c
	{new_v = vl * ((1+lambda) * c)^gamma / 
		(((1+lambda) * c)^gamma + 
		((1-lambda) * (cl-c))^gamma);
	if (new_v < 0) 
	{printf("first, cl = %lf, c = %lf\n", cl, c);}}
	--> I(b,ml,new_v,c,s,w)
I(bl,ml,vl,cl,sl,wl) < I(b,m,v,c,s,w) : ml==1 && b==2 && c>0 && cl !=c
	{new_v = vl* ((1-lambda) * c)^gamma / 
		(((1-lambda) * c)^gamma + 
		((1+lambda) * (cl-c))^gamma);
	if (new_v < 0) 
	{printf("second, cl = %lf, c = %lf\n", cl, c);}}
	--> I(b,ml,new_v,c,s,w)
I(bl,ml,vl,cl,sl,wl) < I(b,m,v,c,s,w) : ml==1 && c>0 && cl==c
	--> I(b,ml,vl,c,s,w)
I(bl,ml,vl,cl,sl,wl) < I(b,m,v,c,s,w) : ml==1 && c==0
	--> I(b,ml,0,c,s,w)

/* downward movement of terminal stats */
I(b,m,v,c,s,w) > [I(b2,m2,v2,c2,s2,w2)=]I(b1,m1,v1,c1,s1,w1) :
	m==0 && m1==2&&m2==2 
	{
		new_c = c1+c2;
		if (w<new_c) {w = new_c;}
	} --> I(b,2,v,new_c,s,w)

/* reset message once transferred */
I(b,m,v,c,s,w) : m==1 --> I(b,0,v,c,s,w)
/* reset message */
I(bl,ml,vl,cl,sl,wl) < I(b,m,v,c,s,w) : ml==2&&m==2 --> 
	I(b,0,v,c,s,w) 

decomposition
maximum depth: 20
J(b,m,v,c,s,w) : v>v_th2 && s == SHOOT -->
	/(180)I(b,0,v,c,s,w)
	[-(alpha_2,alpha_2)J(2,2,v*0.5*(1-lambda2),1,s,1)]
	+(alpha_1,alpha_1)J(1,2,v*0.5*(1+lambda2),1,s,1)
J(b,m,v,c,s,w) : v>v_th2 && s == ROOT -->
	/(180)I(b,0,v,c,s,w)
	[-(beta_2)J(2,2,v*0.5*(1-lambda2),1,s,1)]
	+(beta_1)J(1,2,v*0.5*(1+lambda2),1,s,1)
J(b,m,v,c,s,w) : v<=v_th2 --> I(b,m,v,c,s,w)A

homomorphism
I(b,m,v,c,s,w) : SHOW_NO_LABELS --> 
	!((w*0.01)^0.5);(1+m+3*s)F(len[s])@c
I(b,m,v,c,s,w) : SHOW_COUNT--> 
	!((w*0.01)^0.5);(1+m+3*s)F(len[s])@c
	[|f(len[s]/3)@R(0,0,1,0,1,0);(5)@L(" %.f", c)] 
I(b,m,v,c,s,w) : SHOW_FLUX  --> 
	!((w*0.01)^0.5);(1+m+3*s)F(len[s])@c
	[|f(len[s]/3)@R(0,0,1,0,1,0)f(len[s]);(5)@L(" %.f", v)]
/* N --> [;(2)+(90)F(0.03*shoot)][;(3)-(90)F(0.03*root)] */
N --> [;(32)+(90)F(6)][;(32)-(90)F(6)]
endlsystem




