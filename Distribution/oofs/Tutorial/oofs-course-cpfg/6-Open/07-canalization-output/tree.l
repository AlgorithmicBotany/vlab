#define NUMBER 10
#define RADIUS 0.9

Lsystem: 1
derivation length: NUMBER
Start: {total=0; count=0;}
StartEach: {total=total+1; count=0;}
EndEach: {printf("Total = %.lf, ray florets = %.lf \n", 
	total, count);}

Axiom: M

M --> MX[A(0,total)]
X --> %

C(n,t) > ?E(x): x==0 --> ;(1+floor(253*n/t))@c(RADIUS)
C(n,t) > ?E(x): x==1 {count=count+1;} --> ;(255){+F-F-F+|+F-F-F}

decomposition
maximum depth: 1000

A(n,t) : n <= t --> 
	[+(n*137.5)f(0.5*n^0.5)C(n,t)?E(n)]A(n+1,t)

endlsystem
