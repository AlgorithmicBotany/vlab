#define dx 0.02	/* increment along the axis */

#define width(x) 0.05*func(1,x)
#define turn(x) 1000*func(2,x)*dx
#define bend(x) 100*func(3,x)*dx
#define twist(x) 3000*func(4,x)*dx
#define rotate_contour(x) 3000*func(5,x)*dx

Lsystem: 1

derivation length: 0
Axiom: \(25) ;(16) @#(1) #(width(0)) @Gs C(0,-20) 

homomorphism
maximum depth: 3000

C(x,phi) : x+dx < 1 {x=x+dx; phi=phi+rotate_contour(x);} -->
	+(turn(x)) &(bend(x)) /(twist(x))
	f(dx) #(width(x)) /(phi) @Gc(1) \(phi)
	C(x,phi)

C(x,phi) : x+dx >= 1 {leftover = 1-x; x=1; 
	phi=phi+rotate_contour(x) * leftover / dx;} -->
	+(turn(x)) &(bend(x)) /(twist(x))
	f(leftover) #(width(x)) /(phi) @Ge(1)

endlsystem
