#define SIZE 0.2
#define SQ 0.707106781

Lsystem: 0
Define: {array turn0[7] = {0,45,-45,90,-90,135,-135};
	 array len0[7] = {SQ,1,1,SQ,SQ,1,1};
	 array type0[7] = {0,1,1,0,0,1,1};
	 array turn1[7] = {45,-45,90,-90,0,135,-135};
	 array len1[7] = {SQ,SQ,1,1,1,SQ,SQ};
	 array type1[7] = {0,0,1,1,1,0,0}; }

derivation length: 174
Consider: ?E[]
Axiom: XY 

X --> [f(1.00)-(90)f(0.0)+(90)-(135)?E(0)A(0,0)?E(1)]
Y --> [f(5.00)-(90)f(0.0)+(90)-(45)?E(0)A(0,0)?E(1)]

?E(x)A(i,t)?E(y) : (i<7) && (y==1) --> ?E(0)A(i+1,t)?E(1) 
?E(x)A(i,t)?E(y) : (i==7) && (y==1) --> @o(SIZE) 
?E(x)A(i,t)?E(y) : (i<7) && (t==0) && (y==0) --> B(i,t)?E(0)A(0,type0[i])?E(1) 
?E(x)A(i,t)?E(y) : (i<7) && (t==1) && (y==0) --> B(i,t)?E(0)A(0,type1[i])?E(1) 

homomorphism
maximum depth: 2

A(i,t) : t==0 --> +(turn0[i])F(len0[i])
B(i,t) : t==0 --> +(turn0[i])F(len0[i])

A(i,t) : t==1 --> +(turn1[i])F(len1[i])
B(i,t) : t==1 --> +(turn1[i])F(len1[i])

endlsystem
