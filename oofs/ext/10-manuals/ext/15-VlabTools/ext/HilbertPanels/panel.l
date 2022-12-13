target: Lsystem
panel name: L-system
background: 0,0,0
size: 178 294
font: Arial,14,-1,5,50,0,0,0,0,0

type: SLIDER
name: Steps
colors: 255,255,255 255,0,0
origin: 16 27
min/max: 0 10
value: 3
message: d STEPS %d 1

type: SLIDER
name: Tile size
colors: 255,255,255 255,0,0
origin: 16 102
min/max: 0 101
value: 50
message: d c %d 100

type: BUTTON
name: Yes
tricolor: 255,255,255 0,0,255 0,0,0
origin: 36 176
value: 1
message: d GRID 1 1

type: BUTTON
name: No
tricolor: 255,255,255 0,0,255 0,0,0
origin: 36 205
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
origin: 69 242
