
#ifdef WIN32
#endif

#ifndef WIN32
#error This file to be compiled in MS Windows version only
#endif

#include "warningset.h"

#include <string.h>

#include "platform.h"
#include "platformmenu.h"
#include "control.h"
#ifdef SGI_VISUAL
#include "sgi_visual.h"
#endif

#include "test_malloc.h"

/* common variables */
int enabled_menus = 1;
int is_menu = 0;
extern HWND hMain;

/*extern COMLINEPARAM clp;*/
extern void MyExit(int status);

/*********************** Motif Menus **********************/

extern int animateFlag;
int animateMenu;

/* ------------------------ FUNCTION DECLARATIONS -------------------------- */

void save_file(int value) {
  if (value != 0) {
    if (value > 0) /* save file */
      value = SAVE_OFFSET + value - 1;
    else /* new file */
      value = NEW_FILE_OFFSET + (-value) - 1;

    SelectInMenu(value);

    SetIdle();
  }

  is_menu = 0;
}

void do_it(int value) {
  if (value > 0) {
    SelectInMenu(value);

    SetIdle();
  }

  is_menu = 0;
}

int is_menu_up(void) { return 0; }

/****************************************************************************/
void SetAnimateMenu(void) { animateMenu = 1; }

extern int double_buffering;

/****************************************************************************/
void SetMainMenu(void) {
  animateMenu = 0;
  double_buffering = 1; /* ??? */
}

void InitializeMenus(void) { animateMenu = animateFlag; }

void Dialog_Box(char *str, char *fn, int f) {
  if (IDYES == MessageBox(hMain, str, "Warning", MB_YESNO | MB_ICONWARNING))
    SaveFile(fn, f);
}
