#define STEPS 71
#define SHOW_COUNT 0
#define SHOW_FLUX 1
#define alpha_1 10 /* branching angle - straigh segment */
#define alpha_2 32 /* branching angle - lateral segment */
#define lambda 0.7 /* default flux portion flowing into the main segment */
#define v_th 5.0
#define eta 0.89
#define sigma_0 7 /* CHANGE 7 TO 17 */
#define LEN 1

Lsystem: 1
ignore: +-/
Define: {
	/* colour=1 for neutral, 2 for terminals down, 3 for flux up*/
	array color[3]={1, 3, 2};
	}
derivation length: STEPS
Axiom:  N(1)I(0,2,0,1)A

/* A */
/* I(b,m,v,c) */

/* b - Type - 0=base, 1=major, 2=minor */
/* m - MType - 0=normal 1=flux up 2=terminals down */
/* v - Flux - Flux through the internode */
/* c - TerminalsSupported by this internode */
/* First branch at any node is dominant */

/* terminal numbers reached base; start flux moving up */
N(k) < I(b,m,v,c) : b==0 && m==2 
	{flux= sigma_0*2^((k-1)*(eta^k));} -->
	[;(3)+(90)F(0.03*flux)][;(3)-(90)F(0.03*flux)]
	I(b,1,flux,c)
N(k) > I(b,m,v,c) : b==0 && m==2  --> N(k+1)

/* flux reaching apices */
/* branch if sufficient flux */
/* note that this starts sending the terminal counting message down */
I(b,m,v,c) < A : m==1 && v>v_th --> 
	[-(alpha_2)I(2,2,v*(1-lambda),1)A]+(alpha_1)I(1,2,v*lambda,1)B
I(b,m,v,c) < B : m==1 && v>v_th -->
	[+(alpha_2)I(2,2,v*(1-lambda),1)B]-(alpha_1)I(1,2,v*lambda,1)A
/* Reset the internode message state */
/* and if the apex didn't branch start terminal down message */
I(b,m,v,c) > A : m==1 && v <= v_th --> I(b,2,v,c) 
I(b,m,v,c) > B : m==1 && v <= v_th --> I(b,2,v,c) 


/* upward movement of flux */
I(b_l,m_l,v_l,c_l) < I(b,m,v,c) : m_l==1&&b==1 
	--> I(b,m_l,v_l-v_l*(1-lambda)*((c_l-c)/c),c)
I(b_l,m_l,v_l,c_l) < I(b,m,v,c) : m_l==1&&b==2 
	--> I(b,m_l,v_l*(1-lambda)*(c/(c_l-c)),c)

/* downward movement of terminal stats */
I(b,m,v,c) > [I(b_2,m_2,v_2,c_2)=]I(b_1,m_1,v_1,c_1) :
	m==0 && m_1==2&&m_2==2 --> I(b,2,v,c_1+c_2)

/* reset message once transferred */
I(b,m,v,c) : m==1 --> I(b,0,v,c)
/* reset message */
I(b_l,m_l,v_l,c_l) < I(b,m,v,c) : m_l==2&&m==2 --> I(b,0,v,c) 

homomorphism
I(b,m,v,c) : SHOW_COUNT--> #((c*0.01)^0.7);(color[m])F(LEN)@c
	[|f(LEN/3)&(90)f(LEN)^(90);(5)@L(" %.f", c)] 
I(b,m,v,c) : SHOW_FLUX  --> #((c*0.01)^0.7);(color[m])F(LEN)@c
	[|f(LEN/3)&(90)f(LEN)^(90);(5)@L(" %.f", v)]
endlsystem




