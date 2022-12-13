// to avoid GLu Warning due to deprecated function on MacOs
#define GL_SILENCE_DEPRECATION

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WINDOWS
#include <gl/glew.h>
#include <GL/gl.h>
#endif
#include "shaders.h"

#include "viewparam.h"
#include "drawparam.h"
#include "utility.h"
#include "matrix.h"
#include "platform.h"
#include "control.h"
#include "interpret.h"
#include "comlineparam.h"

// external global variables
extern DRAWPARAM drawparam;
extern VIEWPARAM viewparam;
extern int gl_numlights; // number of lights - from irisGL.c

// internal global variables
static GLuint shadowMapTexture, shadowMapResolution, shadowMapFBO;
static GLuint lightMatrixLocation, shadowMapLocation, shadowColorLocation;
static GLuint textureLocation, textureShadowLocation, numLightsLocation;
static GLfloat lightMatrix[16]; // the projection-model-view matrix for
                                // rendering from the light source

// shader program variables and function for loading them
static GLuint mainShaderProgramID, shadowShaderProgramID;
static GLuint initShaders(const char *vertex_file_path,
                          const char *fragment_file_path);

void initShadowMap(const char *app_path) {
#ifdef _WINDOWS
  static char path[_MAX_PATH + 1], vfile[_MAX_PATH + 1], ffile[_MAX_PATH + 1];
  size_t l;
#else
  static char path[PATH_MAX], vfile[PATH_MAX], ffile[PATH_MAX];
#endif
  static GLubyte whiteImage[4] = {255, 255, 255, 255};
  static GLfloat ones[4] = {1.f, 1.f, 1.f, 1.f};
  GLenum error;

  // set internal globals
  shadowMapFBO = 0;
  shadowMapTexture = 0;
  lightMatrixLocation = 0;
  shadowMapLocation = 0;
  shadowColorLocation = 0;
  textureLocation = 0;
  textureShadowLocation = 0;
  numLightsLocation = 0;
  mainShaderProgramID = 0;
  shadowShaderProgramID = 0;
  shadowMapResolution = drawparam.shadow_map_size;

  // change default OpenGL texture to white texel instead of black
  // that way whenever a fragment is not textured, it uses this default white
  // texture it may be faster to use a uniform variable in the shader, but this
  // requires no changes to lpfg's texturing
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               whiteImage);

  // create a shadow-map texture
  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1, &shadowMapTexture);
  glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapResolution,
               shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_R_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, ones);

  // create a framebuffer for the shadow map
  glGenFramebuffers(1, &shadowMapFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         shadowMapTexture, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    Message("Warning! the 'render mode: shadows' view option will not work.\n"
            "Failed to make complete frame buffer object. Error(%d)\n",
            error);
  }
  // bind render to back buffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glActiveTexture(GL_TEXTURE0);

  // setup shaders - set path to directory in L-studio
  if (app_path != NULL)
    strcpy(path, app_path);
  else
    Message("Warning! Path to cpfg not found. render mode 'shadows' will not "
            "work\n");
#ifdef _WINDOWS
  strcat(path, "..\\shaders\\");
#else
#ifdef VLAB_MACX
  strcat(path, "/../../../lpfg.app/Contents/Resources/shaders/");
#else
  strcat(path, "/../share/shaders/");
#endif
#endif

  // open main shaders
  strcpy(vfile, path);
  strcpy(ffile, path);
  strcat(vfile, "main_vshader.glsl");
  strcat(ffile, "main_fshader.glsl");
  mainShaderProgramID = initShaders(vfile, ffile);

  // open shadow mapping shaders
  strcpy(vfile, path);
  strcpy(ffile, path);
  strcat(vfile, "shadow_vshader.glsl");
  strcat(ffile, "shadow_fshader.glsl");
  shadowShaderProgramID = initShaders(vfile, ffile);

  // set location of uniform variables
  // for the main shaders
  lightMatrixLocation =
      glGetUniformLocation(mainShaderProgramID, "lightMatrix");
  shadowMapLocation = glGetUniformLocation(mainShaderProgramID, "shadowMapTex");
  shadowColorLocation =
      glGetUniformLocation(mainShaderProgramID, "shadowMapColor");
  textureLocation = glGetUniformLocation(mainShaderProgramID, "texture");
  numLightsLocation = glGetUniformLocation(mainShaderProgramID, "numLights");

  // for the shadow mapping shaders
  textureShadowLocation =
      glGetUniformLocation(shadowShaderProgramID, "texture");
}

void freeShadowMap(void) {
  if (mainShaderProgramID != 0) {
    glDeleteProgram(mainShaderProgramID);
    mainShaderProgramID = 0;
  }
  if (shadowShaderProgramID != 0) {
    glDeleteProgram(shadowShaderProgramID);
    shadowShaderProgramID = 0;
  }
  if (shadowMapFBO != 0) {
    glDeleteFramebuffers(1, &shadowMapFBO);
    shadowMapFBO = 0;
  }
  if (shadowMapTexture != 0) {
    glDeleteTextures(1, &shadowMapTexture);
    shadowMapTexture = 0;
  }
}

static GLuint initShaders(const char *vertex_file_path,
                          const char *fragment_file_path) {
  FILE *infile;
  unsigned long length;
  char *VertexShaderCode, *FragmentShaderCode;
  GLuint VertexShaderID, FragmentShaderID;
  GLint Result;
  int InfoLogLength;
  char *InfoLogErrorMessage;
  GLuint ProgramID;

  // Create the shader ids
  VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  if ((infile = fopen(vertex_file_path, "rb")) == NULL) {
    Message(
        "Cannot open vertex shader %s\n'render mode: shadows' will not work.\n",
        vertex_file_path);
    return (0);
  }
  fseek(infile, 0, SEEK_END);
  length = ftell(infile);
  fseek(infile, 0, SEEK_SET);

  VertexShaderCode = (char *)malloc(length + 1);
  fread(VertexShaderCode, length, 1, infile);
  fclose(infile);
  VertexShaderCode[length] = 0;

  // Read the Fragment Shader code from the file
  if ((infile = fopen(fragment_file_path, "rb")) == NULL) {
    Message("Cannot open fragment shader %s\n'render mode: shadows' will not "
            "work.\n",
            fragment_file_path);
    return (0);
  }
  fseek(infile, 0, SEEK_END);
  length = ftell(infile);
  fseek(infile, 0, SEEK_SET);

  FragmentShaderCode = (char *)malloc(length + 1);
  fread(FragmentShaderCode, length, 1, infile);
  fclose(infile);
  FragmentShaderCode[length] = 0;

  // Compile Vertex Shader
  glShaderSource(VertexShaderID, 1, (const GLchar **)&VertexShaderCode, NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  Result = GL_FALSE;
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  if (Result == GL_FALSE) {
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    InfoLogErrorMessage = (char *)malloc(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL,
                       &InfoLogErrorMessage[0]);
    Message(InfoLogErrorMessage);
    return (0);
  }

  // Compile Fragment Shader
  glShaderSource(FragmentShaderID, 1, (const GLchar **)&FragmentShaderCode,
                 NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  Result = GL_FALSE;
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  if (Result == GL_FALSE) {
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    InfoLogErrorMessage = (char *)malloc(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL,
                       &InfoLogErrorMessage[0]);
    Message(InfoLogErrorMessage);
    return (0);
  }

  // Link the program
  ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  if (Result == GL_FALSE) {
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    InfoLogErrorMessage = (char *)malloc(InfoLogLength);
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL,
                        &InfoLogErrorMessage[0]);
    Message(InfoLogErrorMessage);
    return (0);
  }

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

void resizeShadowMap(void) {
  static GLfloat ones[4] = {1.f, 1.f, 1.f, 1.f};

  if (shadowMapResolution == (unsigned int)drawparam.shadow_map_size)
    return;

  shadowMapResolution = drawparam.shadow_map_size;

  // create a shadow-map texture
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapResolution,
               shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_R_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, ones);

  // bind default texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void beginShadowMap(void) {
  light_parms *ls;
  float gl_projection[16];
  float gl_modelview[16];
  float lightDir[3];
  static float lightUp[3] = {0.0, 1.0, 0.0};

  if (shadowShaderProgramID == 0)
    return;

  // set OpenGL depth testing for generating the shadow map
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClearDepth(1.0);

  // render to shadow buffer
  glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, shadowMapResolution, shadowMapResolution);

  // to address shadow acne, use polygon offset in generating depth map
  glPolygonOffset(drawparam.shadow_offset[0], drawparam.shadow_offset[1]);
  glEnable(GL_POLYGON_OFFSET_FILL);

  // save and reset the projection and modelview matrix before rendering with
  // the shadow shader because they are used to render from the lights point of
  // view
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // get the light source
  ls = get_light(0);
  if (ls == NULL) {
    // if no lights, use default OpenGL light position
    ls->position[0] = 0.f;
    ls->position[1] = 0.f;
    ls->position[2] = 1.f;
    ls->position[3] = 0.f;
  }

  // set the light's "up" vector
  if (ls->position[0] == 0.0 && ls->position[2] == 0.0) {
    lightUp[0] = 0.0;
    lightUp[1] = 0.0;
    lightUp[2] = 1.0;
  }

  if (ls->position[3] == 0.f) {

    // use's cpfg CalculateWindow function to compute bounds in orthographic
    // projection
    RECTANGLE shadow_window;
    VIEWPARAM shadow_view;

    shadow_view.vrp[0] = 0.0;
    shadow_view.vrp[1] = 0.0;
    shadow_view.vrp[2] = 0.0;
    shadow_view.viewpoint[0] = ls->position[0];
    shadow_view.viewpoint[1] = ls->position[1];
    shadow_view.viewpoint[2] = ls->position[2];
    shadow_view.view_up[0] = lightUp[0];
    shadow_view.view_up[1] = lightUp[1];
    shadow_view.view_up[2] = lightUp[2];
    shadow_view.scale = 1.0;
    shadow_view.min[0] = viewparam.min[0];
    shadow_view.max[0] = viewparam.max[0];
    shadow_view.min[1] = viewparam.min[1];
    shadow_view.max[1] = viewparam.max[1];
    shadow_view.min[2] = viewparam.min[2];
    shadow_view.max[2] = viewparam.max[2];

    CalculateWindow(&shadow_view, &shadow_window, 1.f);

    // directional light orthographic projection depends on projection mode
    // do not use viewparam.min/max - the bounding volume does not seem to be
    // calculated correctly
    glMatrixMode(GL_PROJECTION);
    if (viewparam.parallel_projection_on) {
      glOrtho(shadow_window.left, shadow_window.right, shadow_window.bottom,
              shadow_window.top, viewparam.front_dist, viewparam.back_dist);
    } else {
      // in perspective mode, negate back distance to ensure the entire object
      // is seen from light this is a hack. it would be better to use the
      // bounding volume but it is not always calculated correctly
      glOrtho(shadow_window.left, shadow_window.right, shadow_window.bottom,
              shadow_window.top, -viewparam.back_dist, viewparam.back_dist);
    }

    // compute light's view matrix, and set gl_ModelViewMatrix in shader
    lightDir[0] = ls->position[0];
    lightDir[1] = ls->position[1];
    lightDir[2] = ls->position[2];
    Normalize(lightDir);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(lightDir[0], lightDir[1], lightDir[2], shadow_view.vrp[0],
              shadow_view.vrp[1], shadow_view.vrp[2], lightUp[0], lightUp[1],
              lightUp[2]);
  } else {
    // spot light
    if (ls->spot_cutoff == 180.f) {
      Message("Warning: Lpfg does not support rendering shadows from "
              "omnidirectional light sources.\n");
    }

    // compute projection matrix, which sets the gl_projection_matrix in the
    // shadow shader
    glMatrixMode(GL_PROJECTION);
    gluPerspective(ls->spot_cutoff * 2.0, 1., 1., viewparam.back_dist);

    // compute light ModelView matrix
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(ls->position[0], ls->position[1], ls->position[2],
              ls->position[0] + ls->spot_direction[0],
              ls->position[1] + ls->spot_direction[1],
              ls->position[2] + ls->spot_direction[2], lightUp[0], lightUp[1],
              lightUp[2]);
  }

  // bind the shadow shader for rendering
  glUseProgram(shadowShaderProgramID);

  // set location of default texture (a white pixel) to match
  // glActiveTexture(GL_TEXTURE0)
  glUniform1i(textureShadowLocation, 0);

  // save the light's projection * modelview matrix
  glGetFloatv(GL_PROJECTION_MATRIX, gl_projection);
  glGetFloatv(GL_MODELVIEW_MATRIX, gl_modelview);
  MultMatrices(gl_modelview, gl_projection, lightMatrix);

  // ensure OpenGL in ModelView matrix state before rendering
  glMatrixMode(GL_MODELVIEW);
}

void endShadowMap(void) {
  if (shadowShaderProgramID == 0)
    return;

  // release the shadow shader after rendering
  glUseProgram(0);

  // render to main frame buffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, clp.xsize, clp.ysize);
  glDisable(GL_POLYGON_OFFSET_FILL);

  // put back the projection and modelview matrix before rendering with main
  // shader
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void beginMainShader(void) {
  const GLfloat offset[16] = {0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0,
                              0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0};
  GLfloat modelview[16], inv_modelview[16], light_modelview[16];

  if (mainShaderProgramID == 0)
    return;

  glUseProgram(mainShaderProgramID);

  // get the inverse of the view matrix before rendering
  // this undoes view transformations so the correct shadow map coordinate is
  // computed the inverse must be calucated here before the turtle changes the
  // modelview matrix using the builtin inverse matrices (gl_ModelViewInverse or
  // gl_NormalMatrix) in the shader will not work
  glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
  InverseMatrixFast(modelview, inv_modelview);

  // pass light projection-model-view matrix to main shader program
  MultMatrices(
      lightMatrix, offset,
      modelview); // multiply by saved projection-modelview matrix (lightMatrix)
  MultMatrices(inv_modelview, modelview,
               light_modelview); // multiply by inverse of current view matrix
                                 // (before turtle interpretation)
  glUniformMatrix4fv(lightMatrixLocation, 1, GL_FALSE, light_modelview);

  // pass texture to main shader - value used in glActiveTexture()
  glUniform1i(textureLocation, 0);

  // bind shadow-map texture for reading - value used in glActiveTexture()
  glUniform1i(shadowMapLocation, 1);

  // pass shadow color to main shader program
  glUniform4f(shadowColorLocation, drawparam.shadow_color[0],
              drawparam.shadow_color[1], drawparam.shadow_color[2], 1.0);

  // pass the number of enabled lights
  glUniform1i(numLightsLocation, gl_numlights);
}

void endMainShader(void) {
  if (mainShaderProgramID != 0)
    glUseProgram(0); // release shader from pipeline
}
