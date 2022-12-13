target: Lsystem
panel name: L-system
background: 0,0,0
size: 194 405
font: Arial,14,-1,5,50,0,0,0,0,0

type: SLIDER
name: Horizontal semi-axis x 10
colors: 255,255,255 255,0,0
origin: 25 112
min/max: 1 200
value: 50
message: d a %d 10

type: SLIDER
name: Vertical semi-axis x10
colors: 255,255,255 255,0,0
origin: 25 52
min/max: 1 200
value: 100
message: d b %d 10

type: LABEL
name: Bounding ellipse
color: 255,0,0
origin: 44 20

type: SLIDER
name: Branching angle
colors: 255,255,255 195,255,9
origin: 25 280
min/max: 1 135
value: 60
message: d ANG %d 1

type: SLIDER
name: Internode length x 100
colors: 255,255,255 204,255,0
origin: 25 224
min/max: 50 200
value: 100
message: d l %d 100

type: SLIDER
name: Tropism strength x 100
colors: 255,255,255 185,255,13
origin: 25 336
min/max: -50 50
value: 0
message: d TROPISM %d 100

type: LABEL
name: Branching structure
color: 212,255,14
origin: 35 185
