#define MAC
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#endif
#include "graphics.h"
#include <stdio.h>

#include "rayshade.h"


void GraphicsInit(int xsize, int ysize, char *name, int *argc, char **argv) {
  glutInitWindowSize(xsize, ysize);
  glutInit(argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutCreateWindow(name);

  mouse_state.lmb = GLUT_UP;
  mouse_state.mmb = GLUT_UP;
  mouse_state.rmb = GLUT_UP;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, xsize, ysize, 0, -1, 1);
}

void MouseCallback(int button, int state, int x, int y) {
  switch (button) {
  case GLUT_LEFT_BUTTON:
    mouse_state.lmb = state;
    break;
  case GLUT_MIDDLE_BUTTON:
    mouse_state.mmb = state;
    if (state == GLUT_DOWN) {
      mouse_state.xstart = x;
      mouse_state.ystart = y;
    } else
      mouse_state.dragged = TRUE;
    break;
  case GLUT_RIGHT_BUTTON:
    mouse_state.rmb = state;
    break;
  }
  mouse_state.xpos = x;
  mouse_state.ypos = y;

  return;
}

void MouseMoveCallback(int x, int y) {
  mouse_state.xpos = x;
  mouse_state.ypos = y;

  return;
}

void GraphicsRedraw(void) {
  /*** This is ugly...I'm basically relying on the draw pixel and
   *** draw rectangle routines to do updates, and hope that the
   *** display function isn't called. Ever.
   ***/

  /*
  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
          GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glFlush();
  */

  return;
}

void GraphicsMainLoop(void (*func)(void)) {
  glutMouseFunc(MouseCallback);
  glutMotionFunc(MouseMoveCallback);
  glutDisplayFunc(GraphicsRedraw);
  glutIdleFunc(func);
  glutMainLoop();
}

int GraphicsLeftMouseEvent(void) { return (mouse_state.lmb == GLUT_DOWN); }

int GraphicsMiddleMouseEvent(void) { return (mouse_state.mmb == GLUT_DOWN); }

int GraphicsRightMouseEvent(void) { return (mouse_state.rmb == GLUT_DOWN); }

int GraphicsDragEvent(void) {
  if (mouse_state.dragged) {
    mouse_state.dragged = FALSE;
    return TRUE;
  } else
    return FALSE;
}

void GraphicsGetMousePos(int *x, int *y) {
  (*x) = mouse_state.xpos;
  (*y) = mouse_state.ypos;
  return;
}

void GraphicsGetMouseDrag(int *x1, int *y1, int *x2, int *y2) {
  (*x1) = mouse_state.xstart;
  (*y1) = mouse_state.ystart;
  (*x2) = mouse_state.xpos;
  (*y2) = mouse_state.ypos;
  return;
}

void GraphicsDrawPixel(int xp, int yp, unsigned char color[3]) {
  glRasterPos2i(xp, yp);
  glDrawPixels(1, 1, GL_RGB, GL_UNSIGNED_BYTE, color);
  glFlush();

  return;
}

void GraphicsDrawRectangle(int xp, int yp, int xs, int ys, unsigned char ll[3],
                           unsigned char lr[3], unsigned char ur[3],
                           unsigned char ul[3]) {
  /* The width adjustment is necessary under Windows, but not under Linux;
   * however, under Linux it's not really noticeable, so for simplicity I'll
   * leave it as 1 for both.
   */
#define RECT_WIDTH_ADJUST 1
  glBegin(GL_QUADS);
  glColor3ubv(ll);
  glVertex2i(xp, yp);

  glColor3ubv(lr);
  glVertex2i(xp + xs + RECT_WIDTH_ADJUST, yp);

  glColor3ubv(ur);
  glVertex2i(xp + xs + RECT_WIDTH_ADJUST, yp + ys + RECT_WIDTH_ADJUST);

  glColor3ubv(ul);
  glVertex2i(xp, yp + ys + RECT_WIDTH_ADJUST);
  glEnd();
  glFlush();

  return;
}
