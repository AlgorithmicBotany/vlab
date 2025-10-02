#define MAXNO 8 /* maximum order of branching */
#define STEPS 4
#define NR 10
#define WID  1.2
#define WR 0.8
#define BRANG 18


Lsystem: 1
seed: 1
Define: {array len[MAXNO] = {1,5,10,10,10,3,1,3};}
derivation length: STEPS
Axiom: #(2*WID)@Gt(1.5)@Gsf(2)#(WID)
	[-(5)^(0)/(40)+(20)-(20)f(len[0])A(1,WID)]
	[-(70)^(0)/(10)+(20)-(20)f(len[0])A(1,WID)]
	[+(70)^(0)/(10)+(20)-(20)f(len[0])A(1,WID)]

A(n,w) --> 
	[-(BRANG)/(85)f(len[n])#(w*WR)@Gc(NR)A(n+1,w*WR)]
	[+(BRANG)/(85)f(len[n])#(w*WR)@Gc(NR)A(n+1,w*WR)] : 10

A(n,w) --> 
	[-(BRANG)/(85)L(n,w)]
	[+(BRANG)/(85)f(len[n])#(w*WR)@Gc(NR)A(n+1,w*WR)] : n

A(n,w) --> 
	[-(BRANG)/(85)f(len[n])#(w*WR)@Gc(NR)A(n+1,w*WR)]
	[+(BRANG)/(85)L(n,w)] : n

homomorphism
L(n,w) --> f(7)#(0.35)@Gc(NR)f(0.5)#(0.2)@Ge(NR);(3)@O

A(n,w) -->
	[-(BRANG)/(85)f(len[n])@Ge(NR);(2)@O(1)]
	[+(BRANG)/(85)f(len[n])@Ge(NR);(2)@O(1)]

endlsystem


