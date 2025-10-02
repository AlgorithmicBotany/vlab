#define A_SW 1
#define SHOW_PETALS 1
#define MAX_0 9
#define MAX_1 8
#define MAX_2 3

#define STEPS 28

#define TWIST 85

#define B_ANG 20
#define B_ANG_INCR 1.5

#define D_INIT -2
#define D_INCR 0.2

#define E_INIT 0.73
#define E_RATIO 0.9

#define P_SIZE 3.2
#define P_ANG 17

#define INIT_POS -(30,0)/(-10);(140)

Lsystem: 1

Define: { array int_len_0[MAX_0], int_len_1[MAX_1];
	}

/* Read internode lengths for branch orders 0, 1.  
   The internodes are listed from the bottom up.  
   The lowest internode is numbered 0 */

Start: {
	fp = fopen("plant.p", "r");
	i=0;
	while(i<MAX_0) {
		fscanf(fp, "%lf", &int_len_0[i]);
	i=i+1;
	}
	i=0;
	while(i<MAX_1) {
		fscanf(fp, "%lf", &int_len_1[i]);
	i=i+1;
	}
	fclose(fp);
}

End: {printf("Simulation completed\n");}

derivation length: STEPS

Axiom: INIT_POS!(D_INIT)F(70)[/+(20,0)/(90)A(0,0)]
	[-(20,0)/(90)A(0,0)]

/* A(o,k) represents an apex
   o  -  branch order (main axis has order 0),
   k  -  internode number along the axis
*/

A(o,k) : (o == 0 && k < MAX_0) --> F(int_len_0[k])
	[!(D_INIT)_(E_INIT)+(B_ANG)/(90)A(o+1,k)]
	[!(D_INIT)_(E_INIT)-(B_ANG)/(90)A(o+1,k)]
	/(TWIST)!(D_INIT)_(E_INIT)A(o,k+1)
A(o,k) : (o == 0 && k == MAX_0) --> B

A(o,k) : (o == 1 && k < MAX_1) --> F(int_len_1[k]) 
	[!(D_INIT)_(E_INIT)+(B_ANG)/(90)A(o+1,k)]
	[!(D_INIT)_(E_INIT)-(B_ANG)/(90)A(o+1,k)]
	/(TWIST)!(D_INIT)_(E_INIT)A(o,k+1)
A(o,k) : (o == 1 && k == MAX_1) --> B

A(o,k) : (o == 2 && k < MAX_2) --> F(5)
	[!(D_INIT)_(E_INIT)+(B_ANG)F(5)B]
	[!(D_INIT)_(E_INIT)-(B_ANG)F(5)B]
	F(5)B
A(o,k) : (o == 2 && k >= MAX_2) --> !(D_INIT)F(5)B

/* For the first-order branches we assume that branch 
   k+1 is the top part of branch k, minus its most basal 
   segment with the associated second-order branches.  
   The number of internodes in the first-order branch 
   is a linear function of the branch position, with 
   the coefficient of proportionality equal to 1 */

/* For the second-order branches, the distal third
   -order branches carry a single flower, and proximal 
   ones carry three flowers.  The number of internodes 
   in the second-order branches can also be viewed as a 
   linear function of this branch position on the first
   -order branch, except that the coefficient of propor-
   tionality is less then one.  Ther number of 
   internodes is, of course, truncated - or rounded off, 
   to an integer. */

!(d) --> !(d+D_INCR)
_(e) --> _(e*(E_RATIO))
+(ang) --> +(ang+B_ANG_INCR)
-(ang) --> -(ang+B_ANG_INCR)
&(ang) --> &(ang+B_ANG_INCR)
^(ang) --> ^(ang+B_ANG_INCR)


B : SHOW_PETALS  --> \\[F[;(255)#(0.1)gG];(36)~c(4)^^^
	[&(P_ANG)~p(P_SIZE)]+++
	[&(P_ANG)~p(P_SIZE)]+++
	[&(P_ANG)~p(P_SIZE)]+++
	[&(P_ANG)~p(P_SIZE)]]

endlsystem
