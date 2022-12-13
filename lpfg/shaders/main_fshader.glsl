#version 120

// Implementation of a Phong shader and shadow mapping based on code from:
// https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/lighting.php
// Rost et al. (2006), OpenGL Shading Language (2nd Edition).
// Eisemann et al. (2002). Real-time Shadows.

varying vec3 V;
varying vec3 N;

uniform sampler2D texture;

uniform int numLights;

varying vec4 shadowMapCoord;
uniform sampler2DShadow shadowMapTex;
uniform vec4 shadowMapColor;

vec4 lookupShadow (float x, float y)
// look up value in shadow map and return vec4(1) for visible else vec4(0)
{
    const float Epsilon = 0.0005;

    float depth = shadow2DProj(shadowMapTex, shadowMapCoord + vec4(x,y,0,0) * Epsilon).x;
    vec4 visibility = vec4(1,1,1,1);
    if (depth < 1.0)
      visibility = vec4(0,0,0,0);
    return visibility;
}

void directionalLight (in vec4 ambient, in vec4 diffuse, in vec4 specular,
                       in float shininess, in int light, in vec3 normal, 
                       inout vec4 Iamb, inout vec4 Idiff, inout vec4 Ispec)
{
    // ambient term
    Iamb += ambient;

    // diffuse term
    float NdotL = max(dot(normal,normalize(gl_LightSource[light].position.xyz)),0.0);
    Idiff += clamp(diffuse * NdotL, 0.0, 1.0);

    // specular term
    // infinite half angle vector: halfVector = normalize(normalize(lightPos) + (0, 0, 1))
    float NdotH = max(dot(normal, gl_LightSource[light].halfVector.xyz), 0.0);
    float pf = 0.0;
    if (NdotL != 0.0)
      pf = pow(NdotH, shininess);
    Ispec += clamp(specular * pf, 0.0, 1.0);
}

void pointLight (in vec4 ambient, in vec4 diffuse, in vec4 specular,
                 in float shininess, in int light, in vec3 normal, 
                 inout vec4 Iamb, inout vec4 Idiff, inout vec4 Ispec)
{
    // vector from surface to light position
    vec3 L = gl_LightSource[light].position.xyz - V;
    float d = length(L);
    L = normalize(L);

    // attenuation of light dependent on distance to light source
    float attenuation = 1.0 / (gl_LightSource[light].constantAttenuation +
                               gl_LightSource[light].linearAttenuation * d +
                               gl_LightSource[light].quadraticAttenuation * d * d);
    // ambient term
    Iamb += ambient * attenuation;

    // if there is a cut-off angle for a spot light, modify attenuation
    if (gl_LightSource[light].spotCutoff <= 90.0) { // 180 -> a point light
      float spotAttenuation = 0.0;
      // check if point on surface is in the cone illuminated by spot light
      float spotDot = dot(-L, normalize(gl_LightSource[light].spotDirection));
      if (spotDot >= gl_LightSource[light].spotCosCutoff)
        spotAttenuation = pow(spotDot, gl_LightSource[light].spotExponent);
      attenuation *= spotAttenuation;
    }

    // diffuse term
    float NdotL = max(dot(normal,L), 0.0);
    Idiff += clamp(diffuse * NdotL * attenuation, 0.0, 1.0);
    
    // specular term - Blinn-Phong
    vec3 E = -normalize(V);
    vec3 H = normalize(L + E);
    float NdotH = max(dot(normal,H),0.0);
    float pf = 0.0;
    if (NdotL != 0.0)
      pf = pow(NdotH, shininess); 
    Ispec += clamp(specular * pf * attenuation, 0.0, 1.0);

    // specular term - Phong
    //vec3 R = normalize(-reflect(L,normal));
    //vec3 E = normalize(-V);
    //Ispec += clamp(specular * pow(max(dot(R,E),0.0), 0.25 * shininess) * attenuation, 0.0, 1.0);
}


void main()
{
    // loop through the enabled lights, but, careful! Iamb, Idiff, and Ispec are accumulated in the light functions above
    vec4 Iamb, Idiff, Ispec; // break down into ambient, diffusive and specular terms
    Iamb = vec4(0); Idiff = vec4(0); Ispec = vec4(0);
    for (int i = 0; i < numLights; i++) {
      // depends if fragment is front or back facing
      if (gl_FrontFacing) {

        // compute diffuse and specular term for front face dependent on the type of light source
        if (gl_LightSource[i].position.w == 0.0)
          directionalLight(gl_FrontLightProduct[i].ambient, gl_FrontLightProduct[i].diffuse, gl_FrontLightProduct[i].specular,
                           gl_FrontMaterial.shininess, i, N, Iamb, Idiff, Ispec);
        else
          pointLight(gl_FrontLightProduct[i].ambient, gl_FrontLightProduct[i].diffuse, gl_FrontLightProduct[i].specular,
                     gl_FrontMaterial.shininess, i, N, Iamb, Idiff, Ispec);
      }
      else {

        // compute diffuse and specular term for back face dependent on the type of light source
        if (gl_LightSource[i].position.w == 0.0)
          directionalLight(gl_BackLightProduct[i].ambient, gl_BackLightProduct[i].diffuse, gl_BackLightProduct[i].specular,
                           gl_BackMaterial.shininess, i, -N, Iamb, Idiff, Ispec);
        else
          pointLight(gl_BackLightProduct[i].ambient, gl_BackLightProduct[i].diffuse, gl_BackLightProduct[i].specular,
                     gl_BackMaterial.shininess, i, -N, Iamb, Idiff, Ispec);
      }
    }

    // compute shadow term - 16 samples
    vec4 shadowSum = vec4(0,0,0,0); // sum of visibility look up for shadows
    float x, y;
    for (y = -1.5; y <= 1.5; y += 1.0)
        for (x = -1.5; x <= 1.5; x += 1.0)
            shadowSum += lookupShadow(x, y);
    shadowSum *= 0.0625;
/*
    if (CPFG/!front_and_back_shadows and gl_FrontFacing) {
        // odd? producing shadow on Frontfacing fragments produces incorrect shadows on Bezier's surfaces in lpfg/cpfg
        shadowSum = vec4(1,1,1,1);
    } 
*/
    // write the color of the fragment
    if (gl_FrontFacing) {
        gl_FragColor = gl_FrontLightModelProduct.sceneColor;
    }
    else {
        gl_FragColor = gl_BackLightModelProduct.sceneColor;
    }

    // apply the ambient, diffuse, specular, and shadow components to fragment color
    // ambient has no shadow (it is ambient after all)
    // but lights do not block 100% of diffuse shading: Idiff * visibile + Idiff * (1-visibile) * dimming
    gl_FragColor += Iamb + Idiff*(shadowSum + (vec4(1,1,1,1)-shadowSum)*shadowMapColor);
  
    // apply texture to fragment (the default texture is a single white texel)
    vec4 texColor = texture2D(texture, gl_TexCoord[0].st);
    gl_FragColor *= texColor;

    // apply specular term after texture so the highlight is the colour of the light source not the surface
    // also no specular highlights in shadowed regions
    gl_FragColor += Ispec*shadowSum*vec4(1,1,1,texColor.a);
}

