#define STEPS 78
#define INITDELAY 20
#define POTENTIAL 0
#define DELAY 2
#define BUDLIFE 1
#define FLOWERLIFE 2
#define FRUITLIFE 2
#define FRUIT2LIFE 4
lsystem: 1
derivation length: STEPS

ignore: /+-,;#!@oZG

Axiom: I(INITDELAY)FA(0) 
S < A(a) --> T(POTENTIAL)[B(0,BUDLIFE),,H@o(0.2)]
    A(a) : a>0 --> A(a-1)
    A(a) : a==0 --> [+(30)G]F/(180)A(DELAY)
S < F --> ,#F;!S
    F > T(a) --> T(a+1)l(2)Fr(2)U(a-1)

U(a) < G --> I(a)FA(2)
I(a) : a>0 --> I(a-1)
I(a) : a==0 --> S
S --> Z
T(a) --> Z

/* These 4 productions delay the appearence of the signal lifting the apical 
   dominance */
l(s) : s>0 --> l(s-1)
l(s) : s==0 --> ;;
r(s) : s>0 --> r(s-1)
r(s) : s==0 --> ,,
 /* B(t,a) represents the blossom in its various stages */
 /* O(t) draws the organs with a one step delay to accomodate cut */
H --> ,H
o(r) --> o(r+0.03)
B(t,a) : a>0 --> B(t,a-1)
B(t,a) : a==0&&t==0 --> B(1,FLOWERLIFE)O(1)%
B(t,a) : a==0&&t==1 --> B(2,FRUITLIFE)O(2)%
B(t,a) : a==0&&t==2 --> B(3,FRUIT2LIFE)O(3)%
B(t,a) : a==0&&t==3 --> ZO(4)%

O(t) : t==1 --> ,,,,,,[!
	[{.-f(0.25)+f(0.25)|-f(0.25)+f(0.25)}]++
	[{.-f(0.25)+f(0.25)|-f(0.25)+f(0.25)}]++
	[{.-f(0.25)+f(0.25)|-f(0.25)+f(0.25)}]++
	[{.-f(0.25)+f(0.25)|-f(0.25)+f(0.25)}]++
	[{.-f(0.25)+f(0.25)|-f(0.25)+f(0.25)}]++
	[{.-f(0.25)+f(0.25)|-f(0.25)+f(0.25)}]]
O(t) : t==2 --> ,,,,,,,[!-{.++f(0.25)--f(0.25)|++f(0.25)--f(0.25)}]
O(t) : t==3 --> ,,,,,,,,[!-{.++f(0.40)--f(0.40)[--[G(0.25)]+[G(0.25)]+
	[G(0.25)]+[G(0.25)]+[G(0.25)]]|++f(0.40)--f(0.40)}]
O(t) : t==4 --> ,,,,,,,,,[!-{.f(0.40)++++f(0.40)++++f(0.40)}]
endlsystem


