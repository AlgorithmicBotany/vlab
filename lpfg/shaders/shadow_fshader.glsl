#version 120

uniform sampler2D texture;

void main() {
  // if the texture is transparent, set fragment depth to be far away (assuming glClearDepth(1.0) was called) 
  vec4 texColor = texture2D(texture, gl_TexCoord[0].st);
  if (texColor.a > 0.0)
      gl_FragDepth = gl_FragCoord.z;
  else
      gl_FragDepth = 1.0;

  // writing the fragment color should not be necessary, but in lpfg it seems to be???
  gl_FragColor = vec4(vec3(gl_FragCoord.z),1.0);
}
