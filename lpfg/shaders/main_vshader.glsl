#version 120

varying vec3 V;
varying vec3 N;

varying vec4 shadowMapCoord;
uniform mat4 lightMatrix;

void main(void) {
    vec4 P = gl_ModelViewMatrix * gl_Vertex;
    V = vec3(P)/P.w;
    N = normalize(gl_NormalMatrix * gl_Normal);
    
    shadowMapCoord = lightMatrix * P;

    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = ftransform();
}

