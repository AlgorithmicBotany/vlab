#define STEPS 256
#define DEL 16
#define R0Angle 20
#define R1Angle 32
/* vigor decrement */
#define VDECR 0.9

#define MAX(a,b) (((a)>(b))?(a):(b))

Lsystem: 1
Start: {n=1; srand(0); }
StartEach: {p=(2*n+1.0)/(n*n); q=1-p;}
EndEach: {n=n+1/DEL;} 
derivation length: STEPS
Consider: ![]
Axiom: /(10)A(1,0,2)

/* Main apex */
/* Introduce a delay, so that the branch width has time to propagate */
A(dir,del,vig) : del > 0 --> A(dir,del-1,vig)

/* Create two long shoots */
A(dir,del,vig) : ran(1) < p --> 
	;(127,127)!(2,2,0,1)F(vig)/(90)
	[+(R1Angle*dir)A(dir,DEL,vig*VDECR)]
	-(R0Angle*dir)A(-dir,DEL,vig*VDECR)

/* Create a long shoot and a short shoot */
A(dir,del,vig) --> ;(127,127)!(2,2,0,1)F(vig)/(90)
	[+(R1Angle*dir)L(53);(3,3)!(2,2,0,2.5)f(0.1)Y(0.4)]
	-(R0Angle*dir)A(-dir,DEL,vig*VDECR)

/* Apply pipe model to determine branch width */

!(r1,a1,d1,f1) > [!(r2,a2,d2,f2)=]!(r3,a3,d3,f3) --> !(MAX(r1,(a2+a3+d2+d3)^0.5),a2+a3,d2+d3,f2+f3-1)
!(r1,a1,d1,f1) > [!(r2,a2,d2,f2)=] --> !(MAX(r1,(a2+d2)^0.5),a2,d2,f2-1)
!(r1,a1,d1,f1) > !(r3,a3,d3,f3) --> !(MAX(r1,(a3+d3)^0.5),a3,d3,f3-1)

/* Leaf ageing */
L(age) : age > 0 --> L(age-1)
L(age);(c,b)!(r,a,d,f) : age == 0 --> ;(4,4)!(2,0,2,0)D(5)

D(n) : n>0 --> D(n-1)
D(n) : n==0 --> %

/* show flow by color */
;(c,b) > !(r,a,d,f) : f>=-1 --> ;(b+f,b)
;(c,b) > !(r,a,d,f) --> %

Y(s) --> {/(90)-F(s)++F(s)-|-F(s)++F(s)}

endlsystem


