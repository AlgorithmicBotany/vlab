#include <lpfgall.h>


module A;

axiom: A;

Start: { UseView(one); UseView(two); UseView(three); }

derivation length: 1;

interpretation:

vgroup two:

A() : { produce Circle0; }

vgroup three:

A() : { produce F(1) Right(45) F(1) Right(45) F(1); }

vgroup one:

A() : { produce F(1) Left(90) F(1); }

