/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#ifndef __buttons_h__
#define __buttons_h__

#include <stdlib.h>
// #include <qgl.h>

typedef enum {
  B1DOWN,
  B1UP,
  B1CLICK1,
  B1CLICK2,
  B2DOWN,
  B2UP,
  B2CLICK1,
  B2CLICK2,
  B3DOWN,
  B3UP,
  B3CLICK1,
  B3CLICK2
} BUTTON_ACTION;

/* the default chunk of memory to allocate for the buttons */
#define NBUTTONS_ALLOC 10

typedef void *BUTTON_DATA_PTR;

typedef void (*BUTTON_CALLBACK_PROC)(BUTTON_DATA_PTR, BUTTON_ACTION);

struct BUTTON {
  int lbX;                       /* top X coordinate of the button */
  int lbY;                       /* top Y coordinate of the button */
  int rtX;                       /* right top X coordinate of the button */
  int rtY;                       /* right top Y coordinate of the button */
  BUTTON_DATA_PTR dataPtr;       /* data pointer to pass to the callback
                                  * routine */
  BUTTON_CALLBACK_PROC callback; /* the callback procedure for this
                                  * button */
  int enabled;                   /* is this button enabled ? */
};

struct BUTTONS {
  int nButtons;         /* number of buttons in this list */
  int allocatedButtons; /* for how many buttons we already
                         * have memory */
  BUTTON *button;       /* the array of buttons */
  int enabled;          /* is this list of buttons enabled ? */
};

void addButton(BUTTONS *, int, int, int, int, BUTTON_DATA_PTR,
               BUTTON_CALLBACK_PROC);
void delAllButtons(BUTTONS *);
void disableButtons(BUTTONS *);
void enableButtons(BUTTONS *);
void initButtons(BUTTONS *);
int checkMouseClickButton(BUTTONS *, int, int, BUTTON_ACTION);
void freeAllButtons(BUTTONS *buttons);
BUTTON_DATA_PTR getUserDataFromButtonAt(BUTTONS *, int, int);

#endif /* #ifndef __buttons_h__ */
