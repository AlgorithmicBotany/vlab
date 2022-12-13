#ifndef GRAPHICS_H
#define GRAPHICS_H

typedef struct {
  int lmb, mmb, rmb;
  int xpos, ypos;
  int xstart, ystart;
  int dragged;
} MouseState;

static MouseState mouse_state;

void GraphicsInit(int xsize, int ysize, char *name, int *argc, char **argv) ;
void MouseCallback(int button, int state, int x, int y) ;
void MouseMoveCallback(int x, int y);
void GraphicsRedraw(void) ;
void GraphicsMainLoop(void (*func)(void));

int GraphicsLeftMouseEvent(void);

int GraphicsMiddleMouseEvent(void);

int GraphicsRightMouseEvent(void);

int GraphicsDragEvent(void);

void GraphicsGetMousePos(int *x, int *y);

void GraphicsGetMouseDrag(int *x1, int *y1, int *x2, int *y2);

void GraphicsDrawPixel(int xp, int yp, unsigned char color[3]) ;

void GraphicsDrawRectangle(int xp, int yp, int xs, int ys, unsigned char ll[3],
                           unsigned char lr[3], unsigned char ur[3],
                           unsigned char ul[3]);



#endif
