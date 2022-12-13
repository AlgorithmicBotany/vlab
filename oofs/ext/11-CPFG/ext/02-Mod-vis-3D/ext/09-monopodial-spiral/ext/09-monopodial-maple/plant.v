initial color: 1
color increment: 1
initial line width: 1
line width increment:0.002 
viewpoint: 0,10,30
view reference point: 0,0,0
projection: parallel
front distance: -100.0
back distance: 100.0
scale factor: 1.1
z buffer: on
render mode: shaded
line style: cylinder
contour sides: 8
light: V: 5 3 1 D: 1.3 1.5 1.3
// light: V: 5 3 1 D: 1.3 1.5 1.3
// light: V: 5 3 1 D: 1.3 1.5 1.3
light: V: -1 -1 0 D: 0.6 0.4 0.3
light: V: 0 1 0 D: 0.3 0.5 0.3
tropism direction: 0,1.0,0
initial elasticity: 0.05
function: internode.func 100
function: leaf-length.func 100
function: leaf-width.func 100
function: branching-angle.func 100
function: color.func 100
surface: l leaf.s 1 8 4
texture: F: maple.rgb H:n L:n E:m S: /* texture on the entire surface */
