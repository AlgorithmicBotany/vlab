#version 120

void main() {
  // need to multiple by gl_ModelViewMatrix because some objects in lpfg
  // (like spheres and surfaces) change this matrix. Otherwise, will not be
  // rendered correctly to the shadow map
  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}
