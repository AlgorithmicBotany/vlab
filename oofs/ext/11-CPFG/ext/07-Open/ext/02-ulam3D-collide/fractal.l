#define VIGOR 0.5 /* arbitrary value between 0 an 1 */

Lsystem: 1
derivation length: 1

Axiom: [IA][+IA][-IA][|IA][^IA][&IA]

/* replace a tentacle that is allowed to grow
   with a line terminated by an apex */

f?E(x) : x==1 --> ;IA

/* tentacle that is not allowed to grow persists */

f?E(x) : x==0 --> f?E(VIGOR)

decomposition

/* replace an apex by five tentacles with sensors */

A --> P[+f?E(VIGOR)][-f?E(VIGOR)]
	[&f?E(VIGOR)][^f?E(VIGOR)]f?E(VIGOR)

homomorphism

I --> F[&@c]

endlsystem
