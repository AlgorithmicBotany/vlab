#define STEPS 320		// Derivation length
#define PLASTOCHRON 5	// Steps between production of consecutive leaves
#define ICOL 255		// Initial color of leaf segments
#define IANG 0			// Initial angle of leaf insertion
#define DANG 0.78		// increment in the angle of leaf insertion
#define DEL 120			// Leaf lifetime, before it falls
#define MAXAGE 260		// Time at which apex initiates terminal inflorescence
#define STEM_WIDTH 4	// Line width for visualizing the trunk
#define LEAF_WIDTH 4	// Line width for visualizing leaves

Lsystem: 0
derivation length: STEPS
Start: {age = 0;}
StartEach: {age = age+1;}
Axiom: !(STEM_WIDTH) A(0) 

A(m) : m<PLASTOCHRON && age < MAXAGE --> FA(m+1)
A(m) : age < MAXAGE --> F[X(DEL) -(IANG) !(LEAF_WIDTH) B] /(180) A(0)

A(m) --> K(0)

K(t) --> K(t+1)

B --> ,(ICOL) F B

X(d) : d>0 --> X(d-1)
X(d) : d==0 --> U%

,(c) --> ,(c-1)
-(a) --> -(a+DANG)
+(a) --> +(a+DANG)

U --> @o(8)

homomorphism

K(t) : 1 {size = t/5;} 
	--> ,(16) {[.+f(size)][f(10*size)][-f(size)]}

endlsystem
