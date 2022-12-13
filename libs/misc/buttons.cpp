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



#include "buttons.h"
#include "xmemory.h"
#include <cassert>
#include <stdlib.h>

/******************************************************************************
 *
 * initialize the buttons to 0 buttons, disabled, and allocate the default
 * chunk of buttons
 */

void initButtons(BUTTONS *buttons) {
  buttons->nButtons = 0;                      /* zero buttons now */
  buttons->allocatedButtons = NBUTTONS_ALLOC; /* zero memory for buttons */
  buttons->button = (BUTTON *)xmalloc(sizeof(BUTTON) * NBUTTONS_ALLOC);
  buttons->enabled = 0;
}

/******************************************************************************
 *
 * delete the definitions of the buttons in this list, but don't erase them
 * from the memory
 */

void delAllButtons(BUTTONS *buttons) {
  disableButtons(buttons);
  buttons->nButtons = 0;
}

/******************************************************************************
 *
 * free the memory taken up by the buttons
 */

void freeAllButtons(BUTTONS *buttons) {

  assert(buttons->button != NULL);

  disableButtons(buttons);
  xfree(buttons->button);
  buttons->button = NULL;
}

/******************************************************************************
 *
 * disables the buttons in the current list of buttons
 */

void disableButtons(BUTTONS *buttons) { buttons->enabled = 0; }

/******************************************************************************
 *
 * enables the buttons in the current list of buttons
 */

void enableButtons(BUTTONS *buttons) { buttons->enabled = 1; }

/******************************************************************************
 *
 * add a button to the list of buttons
 */

void addButton(BUTTONS *buttons, int lbX, int lbY, int rtX, int rtY,
               BUTTON_DATA_PTR dataPtr, BUTTON_CALLBACK_PROC callbackProc) {

  /***    fprintf( stderr, "Add button #%d(%f, %f), (%f, %f)\n",
   ***	     buttons-> nButtons, lbX, lbY, rtX, rtY);
   ***/

  /* do we need some new memory for the buttons ? */
  if (buttons->nButtons == buttons->allocatedButtons) {
    /* if we do, reallocate some */
    buttons->button = (BUTTON *)xrealloc(
        buttons->button, sizeof(BUTTON) * (buttons->nButtons + NBUTTONS_ALLOC));
    buttons->allocatedButtons += NBUTTONS_ALLOC;
  }

  /* now add the new buton */
  buttons->button[buttons->nButtons].lbX = lbX;
  buttons->button[buttons->nButtons].lbY = lbY;
  buttons->button[buttons->nButtons].rtX = rtX;
  buttons->button[buttons->nButtons].rtY = rtY;
  buttons->button[buttons->nButtons].dataPtr = dataPtr;
  buttons->button[buttons->nButtons].callback = callbackProc;
  buttons->nButtons += 1;
}

/******************************************************************************
 *
 * this function should be called on the required mouse click to check all
 * the buttons - if there was a click on one of the buttons, the
 * appropriate callback function is invoked ( with the data)
 *
 * the function will do nothing if the buttons are disabled
 *
 * the function will return True, if there was an action, otherwise it will
 * return false
 */
int checkMouseClickButton(BUTTONS *buttons, int x, int y,
                          BUTTON_ACTION action) {
  int i;

  if (buttons->enabled)
    return 0;

  for (i = 0; i < buttons->nButtons; i++) {
    if (buttons->button[i].callback != NULL && x >= buttons->button[i].lbX &&
        y >= buttons->button[i].lbY && x <= buttons->button[i].rtX &&
        y <= buttons->button[i].rtY) {
      /* fprintf( stderr, "Button #%d pressed\n", i); */
      buttons->button[i].callback(buttons->button[i].dataPtr, action);
      return 1;
    }
  }

  return 0;
}

/******************************************************************************
 *
 * will search for the button at x,y and return its user data
 *
 */
BUTTON_DATA_PTR getUserDataFromButtonAt(BUTTONS *buttons, int x, int y) {
  int i;

  if (buttons->enabled)
    return NULL;

  for (i = 0; i < buttons->nButtons; i++)
    if (buttons->button[i].callback != NULL)
      if (x >= buttons->button[i].lbX)
        if (y >= buttons->button[i].lbY)
          if (x <= buttons->button[i].rtX)
            if (y <= buttons->button[i].rtY)
              return (buttons->button[i].dataPtr);

  return NULL;
}
