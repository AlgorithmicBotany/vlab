target: Lsystem
panel name: L-system
background: 0,0,0
size: 178 505
font: Arial,14,-1,5,50,0,0,0,0,0

type: BUTTON
name: Left
tricolor: 255,255,255 0,0,255 0,0,0
origin: 36 18
value: 1
message: d LEFT 1 1

type: BUTTON
name: Right
tricolor: 255,255,255 0,0,255 0,0,0
origin: 36 47
value: 0
message: d LEFT 0 1

type: GROUP
color: 255,255,255
Left
Right
ENDGROUP

type: LABEL
name: Orientation
color: 255,255,255
origin: 55 83

type: SLIDER
name: Steps
colors: 255,255,255 255,0,0
origin: 19 123
min/max: 0 10
value: 3
message: d STEPS %d 1

type: SLIDER
name: Tile size
colors: 255,255,255 255,0,0
origin: 19 178
min/max: 0 101
value: 50
message: d c %d 100

type: BUTTON
name: Yes
tricolor: 255,255,255 0,0,255 0,0,0
origin: 36 241
value: 1
message: d GRID 1 1

type: BUTTON
name: No
tricolor: 255,255,255 0,0,255 0,0,0
origin: 36 270
value: 0
message: d GRID 0 1

type: GROUP
color: 255,255,255
Yes
No
ENDGROUP

type: LABEL
name: Grid
color: 255,255,255
origin: 76 305

type: SLIDER
name: Line width
colors: 255,255,255 253,252,252
origin: 19 344
min/max: 0 10
value: 3
message: d WW %d 1

type: SLIDER
name: Line width
colors: 255,255,255 255,42,251
origin: 19 398
min/max: 0 10
value: 3
message: d YW %d 1

type: SLIDER
name: Line width
colors: 255,255,255 0,168,0
origin: 19 452
min/max: 0 10
value: 1
message: d GW %d 1
