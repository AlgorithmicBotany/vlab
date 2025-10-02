angle factor: 40 
initial color: 3 3 /*having two numbers switches it into two sided color mode*/
color increment: 1
initial line width: 0.5
line width increment: 0.1
viewpoint: 0,0,1
view reference point: 0,0,0
projection: parallel
front distance: -100
back distance: 100.0
scale factor: 0.85
z buffer: on
/* render mode: wireframe */
render mode: shadows
shadow map: size: 2048
line style: cylinder
tapered lines: on
twist of cylinders: on
contour sides: 32
contour: 1 leaf1.con
contour: 2 leaf2.con
contour: 3 petal_base.con
contour: 4 petal_tip.con
contour: 5 gynoecium.con
contour: 6 anther.con
light: V: 4.0,2.0,2.0
light: V: -4.0,-2.0,-2.0
diffuse reflection: 16
tropism: T: 0.0 -1.0 0.0
tropism: T: 0.0  1.0 0.0  A:90
torque:  T: 0.0  1.0 0.0
surface ambient: .1
surface diffuse: 0.85
texture: F: leaft.rgb H:l L:l E:m R:10
/* H,L: n - not smoothed, l - smoothed
   E: m - modulate, d - decalc
   R - scale factor
*/
