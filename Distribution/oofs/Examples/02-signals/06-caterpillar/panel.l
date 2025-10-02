panel name: L-system
background: 520
size: 170 220

type: BUTTON
name: Line
colors: 523 524
origin: 35 160
value: 0
message: d AXIOM 0 1

type: BUTTON
name: Polygon
colors: 523 524
origin: 35 120
value: 1
message: d AXIOM 1 1

type: GROUP
color: 525
Line
Polygon
ENDGROUP

type: LABEL
name: Axiom
color: 525
origin: 63 100

type: SLIDER
name: Derivations
colors: 521 522
origin: 20 25
min/max: 0 10
value: 3
message: d STEPS %d 1

