#include <string.h>
#include <sys/param.h>

#include <Xm/CascadeB.h>
#include <Xm/MainW.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/Label.h>
#include <Xm/Separator.h>
#include <Xm/BulletinB.h>

#include <Xm/SelectioB.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>
#include <Xm/DialogS.h>
#include <Xm/MessageB.h>

#include "platform.h"
#include "platformmenu.h"
#include "control.h"

#include "test_malloc.h"

/* common variables */
int enabled_menus = 1;
int is_menu = 0;

extern COMLINEPARAM clp;
extern void MyExit(int status);
void do_resize(Widget w, XtPointer client_data, XtPointer call);

/*********************** Motif Menus **********************/

extern int animateFlag;
int animateMenu;

/* ------------------------ FUNCTION DECLARATIONS -------------------------- */
void file_call_back(Widget w, XtPointer client_data, XtPointer call_data);
void resolution_call_back(Widget w, XtPointer client_data, XtPointer call_data);
void message_call_back(Widget w, XtPointer client_data, XtPointer call);

void post_menu_handler(Widget w, void *menu, union _XEvent *event, char *str);
void do_it(Widget w, XtPointer client_data, XtPointer call_data);

static void Set_SaveFileName(int i, char *name);

/****************************************/

Widget main_menu, animate_menu, file_select, resolution_box, dialog_shell,
    messageD;
Widget main_menu_top, animate_menu_top;

struct xs_menu_struct {
  _Xconst _XtString item_name;
  void (*call_back)(Widget, XtPointer, XtPointer);
  XtPointer call_back_name;
  struct xs_menu_struct *submenu;
};

static int save_menu = -1;
static Widget save_menus[SAVE_COUNT][3] = {{NULL, NULL, NULL},
                                           /*{NULL, NULL, NULL},*/
                                           {NULL, NULL, NULL},
                                           {NULL, NULL, NULL},
                                           {NULL, NULL, NULL},
                                           {NULL, NULL, NULL},
                                           {NULL, NULL, NULL},
                                           {NULL, NULL, NULL},
                                           {NULL, NULL, NULL},
                                           {NULL, NULL, NULL},
                                           {NULL, NULL, NULL}};

static Widget input_menus[2][3] = {{NULL, NULL, NULL}, {NULL, NULL, NULL}};

/*
static struct xs_menu_struct RGBAsaveMenu[] =
{
  { "RGBA", NULL, "title", NULL},
  { NULL, NULL, "separator", NULL},
  { "Save as *", do_it, "s12", NULL},
  { "Save as ...", do_it, "n12", NULL},
  { NULL, NULL, NULL, NULL }
};
*/

static struct xs_menu_struct RGBsaveMenu[] = {
    {"RGB", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Save as *", do_it, "s00", NULL},
    {"Save as ...", do_it, "n00", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct RASsaveMenu[] = {
    {"RAS", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Save as *", do_it, "s01", NULL},
    {"Save as ...", do_it, "n01", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct TGAsaveMenu[] = {
    {"TGA", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Save as *", do_it, "s02", NULL},
    {"Save as ...", do_it, "n02", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct RLEsaveMenu[] = {
    {"RLE", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Save as *", do_it, "s03", NULL},
    {"Save as ...", do_it, "n03", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct RAYsaveMenu[] = {
    {"Rayshade", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Save as *", do_it, "s04", NULL},
    {"Save as ...", do_it, "n04", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct PSsaveMenu[] = {
    {"Postscript", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Save as *", do_it, "s05", NULL},
    {"Save as ...", do_it, "n05", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct STRsaveMenu[] = {
    {"String (text)", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Save as *", do_it, "s06", NULL},
    {"Save as ...", do_it, "n06", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct VVsaveMenu[] = {
    {"View volume", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Save as *", do_it, "s07", NULL},
    {"Save as ...", do_it, "n07", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct SDsaveMenu[] = {
    {"String (binary)", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Save as *", do_it, "s08", NULL},
    {"Save as ...", do_it, "n08", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct GLSsaveMenu[] = {
    {"Gls format", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Save as *", do_it, "s09", NULL},
    {"Save as ...", do_it, "n09", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct OutputImageMenu[] = {
    {"Output image", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"RGB", do_it, "o1", RGBsaveMenu},
    /*{ "RGBA", do_it, "o5", RGBAsaveMenu},*/
    {"RAS", do_it, "o2", RASsaveMenu},
    {"TGA", do_it, "o3", TGAsaveMenu},
    {"RLE", do_it, "o4", RLEsaveMenu},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct OutputStringMenu[] = {
    {"Output string", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"text", do_it, "oS", STRsaveMenu},
    {"binary", do_it, "oD", SDsaveMenu},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct SDinputMenu[] = {
    {"String (binary)", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Input from *", do_it, "I08", NULL},
    {"Input from ...", do_it, "i08", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct STRinputMenu[] = {
    {"String (text)", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Input from *", do_it, "I06", NULL},
    {"Input from ...", do_it, "i06", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct InputStringMenu[] = {
    {"Input string", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"text", do_it, "iS", STRinputMenu},
    {"binary", do_it, "iD", SDinputMenu},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct OutputMenu[] = {
    {"Output", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Image", NULL, "I", OutputImageMenu},
    {"Rayshade", do_it, "oR", RAYsaveMenu},
    {"Postscript", do_it, "oP", PSsaveMenu},
    {"String", NULL, "O", OutputStringMenu},
    {"Gls format", do_it, "od", GLSsaveMenu},
    {"View volume", do_it, "oV", VVsaveMenu},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct InputMenu[] = {
    {"Input", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"String", do_it, "IS", InputStringMenu},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct BegRecImageMenu[] = {
    {"Record images", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"RGB", do_it, "b00", NULL},
    /*{ "RGBA", do_it, "b11", NULL},*/
    {"RAS", do_it, "b01", NULL},
    {"TGA", do_it, "b02", NULL},
    {"RLE", do_it, "b03", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct BegRecStringMenu[] = {
    {"Record strings", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"text", do_it, "b06", NULL},
    {"binary", do_it, "b08", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct BegRecMenu[] = {
    {"Record", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Image", NULL, "RI", BegRecImageMenu},
    {"Rayshade", do_it, "b04", NULL},
    {"Postscript", do_it, "b05", NULL},
    {"String", NULL, "RI", BegRecStringMenu},
    {"Gls format", do_it, "b09", NULL},
    {"View volume", do_it, "b07", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct MainMenuTop[] = {
    {"New model", do_it, "11", NULL},
    {"New L-system", do_it, "12", NULL},
    {"New homomorphism", do_it, "16", NULL},
    {"New view", do_it, "13", NULL},
#ifdef CPFG_ENVIRONMENT
    {"New environment", do_it, "15", NULL},
#endif
    {NULL, NULL, "separator", NULL},
    {"Window size", do_it, "17", NULL},
    {"Output", NULL, "O", OutputMenu},
    {"Input", NULL, "I", InputMenu},
    {NULL, NULL, "separator", NULL},
    {"Exit", do_it, "99", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct MainMenu[] = {
    {"Main", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"New model", do_it, "11", NULL},
    {"New L-system", do_it, "12", NULL},
    {"New homomorphism", do_it, "16", NULL},
    {"New view", do_it, "13", NULL},
#ifdef CPFG_ENVIRONMENT
    {"New environment", do_it, "15", NULL},
#endif
    {NULL, NULL, "separator", NULL},
    {"Window size", do_it, "17", NULL},
    {"Output", NULL, "O", OutputMenu},
    {"Input", NULL, "I", InputMenu},
    {NULL, NULL, "separator", NULL},
    {"Animate mode", do_it, "20", NULL},
    {NULL, NULL, "separator", NULL},
    {"Exit", do_it, "99", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct AnimateMenuTop[] = {
    {"Step", do_it, "21", NULL},
    {"Run", do_it, "22", NULL},
    {"Forever", do_it, "23", NULL},
    {"Stop", do_it, "24", NULL},
    {"Rewind", do_it, "25", NULL},
    {"Clear", do_it, "26", NULL},
    {NULL, NULL, "separator", NULL},
    {"New animate", do_it, "14", NULL},
    {NULL, NULL, "separator", NULL},
    {"Begin Recording", NULL, "B", BegRecMenu},
    {"Stop Recording", do_it, "98", NULL},
    {NULL, NULL, NULL, NULL}};

static struct xs_menu_struct AnimateMenu[] = {
    {"Animate", NULL, "title", NULL},
    {NULL, NULL, "separator", NULL},
    {"Step", do_it, "21", NULL},
    {"Run", do_it, "22", NULL},
    {"Forever", do_it, "23", NULL},
    {"Stop", do_it, "24", NULL},
    {"Rewind", do_it, "25", NULL},
    {"Clear", do_it, "26", NULL},
    {NULL, NULL, "separator", NULL},
    {"New model", do_it, "11", NULL},
    {"New L-system", do_it, "12", NULL},
    {"New homomorphism", do_it, "16", NULL},
    {"New view", do_it, "13", NULL},
#ifdef CPFG_ENVIRONMENT
    {"New environment", do_it, "15", NULL},
#endif
    {"New animate", do_it, "14", NULL},
    {NULL, NULL, "separator", NULL},
    {"Window size", do_it, "17", NULL},
    {"Output", NULL, "O", OutputMenu},
    {"Input", NULL, "I", InputMenu},
    {"Begin Recording", NULL, "B", BegRecMenu},
    {"Stop Recording", do_it, "98", NULL},
    {NULL, NULL, "separator", NULL},
    {"Don't animate", do_it, "27", NULL},
    {NULL, NULL, "separator", NULL},
    {"Exit", do_it, "99", NULL},
    {NULL, NULL, NULL, NULL}};

/** xs_create_menu ()
    Copyright (c) 1994 Radomir Mech
    Creates a motif menu system from the structures xs_menu_struct (see above).

!!  For internal purposes: it's necessary to remember widgets of menu buttons
      containing the actual savename - so it can be updated later with the
      changed file name.
**/

Widget xs_create_menu(Widget parent, struct xs_menu_struct *minfo,
                      int submenu) {
  Widget menu = NULL, button;
  /* next line just for internal purposes */
  char *str;
  Arg arg[10];
  int argc = 0, i;

  if (minfo != NULL) {

#ifdef SGI_VISUAL
    if (clp.overlay_menus)
      SG_getPopupArgs(XtDisplay(parent), DefaultScreen(XtDisplay(parent)), arg,
                      &argc);
#endif
    if (submenu)
      menu = XmCreatePulldownMenu(parent, (char *)minfo->item_name, arg, argc);
    else {
      menu = XmCreatePopupMenu(parent, (char *)minfo->item_name, arg, argc);
    }

    /* next line just for internal purposes */
    if (minfo == OutputMenu)
      save_menu++;

    while (minfo->call_back_name != NULL) {
      if (minfo->item_name == NULL) {
        XtCreateManagedWidget("separator", xmSeparatorWidgetClass, menu, arg,
                              argc);
      } else if (minfo->submenu != NULL) {
        button = XtCreateManagedWidget(
            minfo->item_name, xmCascadeButtonWidgetClass, menu, arg, argc);
        xs_create_menu(button, minfo->submenu, 1);
      } else if (minfo->call_back == NULL) {
        XtCreateManagedWidget(minfo->item_name, xmLabelWidgetClass, menu, arg,
                              argc);
      } else {
        button = XtCreateManagedWidget(
            minfo->item_name, xmCascadeButtonWidgetClass, menu, arg, argc);

        XtAddCallback(button, XmNactivateCallback, minfo->call_back,
                      minfo->call_back_name);
        /* next few lines just for internal purposes */
        if (strlen(str = (char *)minfo->call_back_name) == 3) {
          if (str[0] == 's')
            save_menus[(str[1] - '0') * 10 + (str[2] - '0')][save_menu] =
                button;
          if (str[0] == 'I') {
            i = (str[1] - '0') * 10 + (str[2] - '0');
            if (i == 6 || i == 8)
              input_menus[(i - 6) / 2][save_menu] = button;
          }
        }
      }
      minfo++;
    }

    if (submenu)
      XtVaSetValues(parent, XmNsubMenuId, menu, NULL);
  }
  return menu;
}

/*************************************************************************/
char *FindInMenuStruct(char *string, struct xs_menu_struct *minfo) {
  char *val;
  char /*str[512],*/ *ptr;

  if ((ptr = strchr(string, '|')) != NULL) {
    *ptr = 0;
  }

  if (minfo != NULL) {
    while (minfo->call_back_name != NULL) {
      /* if it is not a separator */
      if (minfo->item_name != NULL) {
        if (minfo->submenu != NULL) {
          if (ptr != NULL &&
              !strncasecmp(string, minfo->item_name, strlen(minfo->item_name)))
            if ((val = FindInMenuStruct(ptr + 1, minfo->submenu)) != NULL)
              return val;
        }
        /* if not a menu label */
        else if (minfo->call_back != NULL) {
          if (!strncasecmp(string, minfo->item_name, strlen(minfo->item_name)))
            return minfo->call_back_name;
        }
      }
      minfo++;
    }
  }

  if (ptr != NULL)
    *ptr = '|';

  return NULL;
}

/*************************************************************************/
int FindMenuValuesForString(char *string) {
  char *ptr;

  if (string == NULL)
    return 0;

  /* go through the whole menu structure and look for the string */
  if (animateMenu)
    ptr = FindInMenuStruct(string, AnimateMenu);
  else
    ptr = FindInMenuStruct(string, MainMenu);

  if (ptr != NULL) {
    do_it(NULL, ptr, NULL);
    return 1;
  } else
    Message("Command sent through a socket not recognized!\n");

  return 0;
}

/*************************************************************************/
static void RemoveItem(char *item, struct xs_menu_struct *menu) {
  int i;

  if (menu == NULL)
    return;

  for (;;) {
    if (menu->call_back_name == NULL) /* item not found */
      return;

    if (menu->item_name != NULL && !strcmp(menu->item_name, item)) {
      /* item found, move all subsequent items one down */
      i = 1;

      while (menu[i].call_back_name != NULL) {
        menu[i - 1] = menu[i];
        i++;
      }
      menu[i - 1] = menu[i];
      return;
    }

    menu++;
  }
}

/*************************************************************************/
extern Widget menu_bar, frame;

void Initialize_Menus(Widget main_window, Widget top_shell) {
  Arg args[20];
  int n = 0;
  Widget cascade;

#ifdef CPFG_ENVIRONMENT
  /* remove the item "New environment" from all menus, if environment not used
   */
  if (clp.communication != COMM_LOCAL) {
    RemoveItem("New environment", AnimateMenu);
    RemoveItem("New environment", MainMenu);
    RemoveItem("New environment", MainMenuTop);
  }
#endif

  if (clp.menu_bar) {
    /* create menu bar */
    menu_bar = XmCreateMenuBar(main_window, "menu_bar", args, 0);
    XtManageChild(menu_bar);

    main_menu_top = xs_create_menu(menu_bar, MainMenuTop, 1);

    animate_menu_top = xs_create_menu(menu_bar, AnimateMenuTop, 1);

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, main_menu_top);
    n++;
    cascade = XmCreateCascadeButton(menu_bar, "file", args, n);
    XtManageChild(cascade);

    n = 0;
    XtSetArg(args[n], XmNsubMenuId, animate_menu_top);
    n++;
    cascade = XmCreateCascadeButton(menu_bar, "animate", args, n);
    XtManageChild(cascade);

    n = 0;
    XtSetArg(args[n], XmNspacing, 10);
    n++;
    XtSetValues(menu_bar, args, n);
  } else {
    menu_bar = NULL;
    save_menu++;
  }

  dialog_shell = XtCreatePopupShell(
      "dial_sh", applicationShellWidgetClass,
      /*				    xmDialogShellWidgetClass,*/
      top_shell, NULL, 0);
  XtManageChild(dialog_shell);

  main_menu = xs_create_menu(dialog_shell, MainMenu, 0);

  animate_menu = xs_create_menu(dialog_shell, AnimateMenu, 0);

  animateMenu = animateFlag;

#ifdef SGI_VISUAL
  if (clp.overlay_menus)
    SG_getOverlayArgs(XtDisplay(dialog_shell), 0, args, &n);
#else
  n = 0;
#endif
  file_select = NULL;
  resolution_box = NULL;

  /* dialog box */
  XtSetArg(args[n], XmNokLabelString,
           XmStringCreateLtoR("Overwrite", XmSTRING_DEFAULT_CHARSET));
  n++;
  XtSetArg(args[n], XmNdialogType, XmDIALOG_WARNING);
  n++;
  XtSetArg(args[n], XmNdialogTitle,
           XmStringCreateLtoR("Warning", XmSTRING_DEFAULT_CHARSET));
  n++;
  messageD = XmCreateMessageDialog(dialog_shell, "save_box", args, n);

  XtUnmanageChild(XmMessageBoxGetChild(messageD, XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmMessageBoxGetChild(messageD, XmDIALOG_HELP_BUTTON));

  XtAddCallback(messageD, XmNokCallback, message_call_back, "y");
  XtAddCallback(messageD, XmNcancelCallback, message_call_back, "n");

  for (n = 0; n < SAVE_COUNT; n++)
    Set_SaveFileName(n, clp.savefilename[n]);
}

/************* POPUP MENU HANDLING ********************/

static int act_file_save_menu;

/* Sets proper filename to SaveFileName menu. (has to change the name of the
   proper button widget. */

static void Set_SaveFileName(int i, char *name) {
  char fname[255];
  XmString compound;
  Arg arg;
  int len;

  strcpy(fname, "Save as ");

  if ((len = strlen(name)) > MAXWINLEN) {
    strcpy(fname, "...");
    strcat(fname, &name[len - MAXWINLEN]);
  } else
    strcat(fname, name);

  if (((save_menus[i][0] == NULL) && (clp.menu_bar)) ||
      (save_menus[i][1] == NULL) || (save_menus[i][2] == NULL)) {
    Message("NULL widget (save menu button)!\n");
    return; /*MyExit(0);*/
  }

  compound = XmStringCreate(fname, XmSTRING_DEFAULT_CHARSET);
  XtSetArg(arg, XmNlabelString, compound);
  if (clp.menu_bar)
    XtSetValues(save_menus[i][0], &arg, 1);
  XtSetValues(save_menus[i][1], &arg, 1);
  XtSetValues(save_menus[i][2], &arg, 1);

  XmStringFree(compound);

  if (i == 6 || i == 8) {
    i = (i - 6) / 2;

    strcpy(fname, "Input from ");

    if ((len = strlen(name)) > MAXWINLEN) {
      strcpy(fname, "...");
      strcat(fname, &name[len - MAXWINLEN]);
    } else
      strcat(fname, name);

    compound = XmStringCreate(fname, XmSTRING_DEFAULT_CHARSET);
    XtSetArg(arg, XmNlabelString, compound);

    if (clp.menu_bar)
      XtSetValues(input_menus[i][0], &arg, 1);
    XtSetValues(input_menus[i][1], &arg, 1);
    XtSetValues(input_menus[i][2], &arg, 1);

    XmStringFree(compound);
  }
}

/************************************************************************/
void GetRidOfSpaces(char *name) {
  char *ptr = name;

  while (*name != 0) {
    if (*name != ' ')
      *(ptr++) = *name;
    name++;
  }
  *ptr = 0;
}

/****************************************************************************/
/* Callback routine for file selection box */
void file_call_back(Widget w, XtPointer client_data, XtPointer call) {
  XmFileSelectionBoxCallbackStruct *call_data;
  char *data = (char *)client_data;
  char *str, *name;
  /*int i, len;*/

  if (data[0] != 'n') {
    call_data = (XmFileSelectionBoxCallbackStruct *)call;

    XmStringGetLtoR(call_data->value, XmSTRING_DEFAULT_CHARSET, &str);
  }

  if (data[0] == 'y') {
    /* change filename in Output menus */
    GetRidOfSpaces(str);

    if (clp.savefilename[act_file_save_menu] != NULL)
      Free(clp.savefilename[act_file_save_menu]);
    clp.savefilename[act_file_save_menu] = Strdup(str);

    Set_SaveFileName(act_file_save_menu, clp.savefilename[act_file_save_menu]);
    /*    SaveFile(clp.savefilename[act_file_save_menu], act_file_save_menu);*/
    XtUnmanageChild(file_select);

    name = Strdup(clp.savefilename[act_file_save_menu]);
    OpenOutputFile(name, act_file_save_menu);
  } else
    XtUnmanageChild(file_select);

  if (data[0] == 'i') {
    /* input from a file */
    switch (act_file_save_menu) {
    case SAVE_STRINGDUMP:
      /* binary string */
      setcursor(CURSOR_HOURGLASS, 0, 0);
      InputString(str, 'b');
      setcursor(CURSOR_ARROW, 0, 0);
      break;

    case SAVE_STRING:
      /* text string */
      setcursor(CURSOR_HOURGLASS, 0, 0);
      InputString(str, 't');
      setcursor(CURSOR_ARROW, 0, 0);
      break;
    }
  }
}

static int save_format;
static char *save_name;

void message_call_back(Widget w, XtPointer client_data, XtPointer call) {
  extern Widget main_window;
  char *str = (char *)client_data;

  XtUnmanageChild(messageD);

  XSelectInput(XtDisplay(main_window), XtWindow(main_window),
               ExposureMask | KeyReleaseMask | ResizeRedirectMask);
  XFlush(XtDisplay(w));

  if (str[0] == 'y')
    SaveFile(save_name, save_format);

  Free(save_name);
  XSelectInput(XtDisplay(main_window), XtWindow(main_window),
               ExposureMask | ButtonPressMask | KeyReleaseMask |
                   ResizeRedirectMask);
}

void Dialog_Box(char *str, char *filename, int format) {
  Arg arg;

  save_name = filename;
  save_format = format;

  XtSetArg(arg, XmNmessageString,
           XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET));
  XtSetValues(messageD, &arg, 1);
  XtManageChild(messageD);
}

/****************************************************************************/
/* Creates a file selection box. Parameter 'i' determines the type.
   When i is positive, the selection box is used for change of filename in
   output menus.
   When i is negative, the box is used for input of a file. */
void Change_Filename(int i) {
  XmString compound;
  Arg args[10];
  int n = 0;
  char input = 0; /* file input ? */
  /*extern Widget top_shell;*/

  if (i < 0) {
    i = -i;
    input = 1;
  }

  act_file_save_menu = i;

  if (file_select == NULL) {
    XtSetArg(args[n], XmNallowShellResize, True);
    n++;
    XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW);
    n++;

    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL);
    n++;

#ifdef SGI_VISUAL
    if (clp.overlay_menus)
      SG_getOverlayArgs(XtDisplay(dialog_shell), 0, args, &n);
#endif

    file_select = XmCreateFileSelectionDialog(dialog_shell, "fselbox", args, n);

    XtUnmanageChild(
        XmFileSelectionBoxGetChild(file_select, XmDIALOG_HELP_BUTTON));
    XtAddCallback(file_select, XmNcancelCallback, file_call_back, "n");

  } else
    XtRemoveAllCallbacks(file_select, XmNokCallback);

  /* different parameter for OK callback */
  if (input)
    XtAddCallback(file_select, XmNokCallback, file_call_back, "i");
  else
    XtAddCallback(file_select, XmNokCallback, file_call_back, "y");

  compound = XmStringCreateLtoR(clp.savefilename[i], XmSTRING_DEFAULT_CHARSET);

  n = 0;
  XtSetArg(args[0], XmNdirSpec, compound);
  n++;
  /* different box title */
  if (input)
    XtSetArg(args[n], XmNdialogTitle,
             XmStringCreateLtoR("Select a file", XmSTRING_DEFAULT_CHARSET));
  else
    XtSetArg(args[n], XmNdialogTitle,
             XmStringCreateLtoR("New file name", XmSTRING_DEFAULT_CHARSET));
  n++;

  XtSetValues(file_select, args, n);

  XmStringFree(compound);
  XtManageChild(file_select);
}

/****************************************************************************/
/* Creates a file selection box. Parameter 'i' determines the type.
   When i is positive, the selection box is used for change of filename in
   output menus.
   When i is negative, the box is used for input of a file. */
void Change_Resolution(void) {
  Arg args[10];
  int n = 0;
  /*char input = 0;*/ /* file input ? */
  /*extern Widget top_shell;*/

  if (resolution_box == NULL) {
    XtSetArg(args[n], XmNallowShellResize, True);
    n++;
    XtSetArg(args[n], XmNresizePolicy, XmRESIZE_GROW);
    n++;

    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL);
    n++;

    XtSetArg(args[n], XmNselectionLabelString,
             XmStringCreateLtoR("Enter new window resolution (HxV):",
                                XmSTRING_DEFAULT_CHARSET));
    n++;
    /* different box title */
    XtSetArg(args[n], XmNdialogTitle,
             XmStringCreateLtoR("Select resolution", XmSTRING_DEFAULT_CHARSET));
    n++;

#ifdef SGI_VISUAL
    if (clp.overlay_menus)
      SG_getOverlayArgs(XtDisplay(dialog_shell), 0, args, &n);
#endif

    resolution_box =
        XmCreatePromptDialog(dialog_shell, "resolutionbox", args, n);
    XtUnmanageChild(
        XmSelectionBoxGetChild(resolution_box, XmDIALOG_HELP_BUTTON));
  }

  XtAddCallback(resolution_box, XmNcancelCallback, resolution_call_back, "n");

  XtAddCallback(resolution_box, XmNokCallback, resolution_call_back, "y");

  XtManageChild(resolution_box);
}

/****************************************************************************/
/* Callback routine for file selection box */
void resolution_call_back(Widget w, XtPointer client_data, XtPointer call) {
  XmSelectionBoxCallbackStruct *call_data;
  char *data = (char *)client_data;
  char *str, *token;
  int x, y;
  /*Arg args[4];*/
  /*int n = 0;*/

  if (data[0] != 'n') {
    call_data = (XmSelectionBoxCallbackStruct *)call;

    XmStringGetLtoR(call_data->value, XmSTRING_DEFAULT_CHARSET, &str);
  }

  if (data[0] == 'y' && str != NULL) {
    if ((token = strtok(str, " x\t")) != NULL) {
      x = atoi(token);

      if ((token = strtok(NULL, " x\t")) != NULL) {
        y = atoi(token);

        if (clp.verbose)
          Message("New resolution: (%dx%d).\n", x, y);

        clp.xsize = x;
        clp.ysize = y;

        do_resize(w, client_data, NULL);
      } else
        Message("Warning! Cannot read the second value!\n");
    } else
      Message("Warning! Cannot read the first value!\n");
  }

  XtUnmanageChild(resolution_box);
}

/****************************/
extern void SetIdle(void);

void do_it(Widget w, XtPointer client_data, XtPointer call_data) {
  char *str = (char *)client_data;
  int val;

  /*
    printf("%s selected\n", str);
  */
  if (strlen(str) == 3)
    val = (str[1] - '0') * 10 + (str[2] - '0');

  switch (str[0]) {
  case 'b': /* begin recording */
    val += OUTPUT_ANIMATE_OFFSET;
    break;
  case 's': /* save file */
    val += SAVE_OFFSET;
    break;
  case 'n': /* new file */
    val += NEW_FILE_OFFSET;
    break;
  case 'i': /* input with selection box */
    Change_Filename(-val);
    val = 0;
  case 'I': /* input without selection box */
    /* input from a file */
    switch (val) {
    case SAVE_STRINGDUMP:
      /* binary string */
      setcursor(CURSOR_HOURGLASS, 0, 0);
      InputString(clp.savefilename[val], 'b');
      setcursor(CURSOR_ARROW, 0, 0);
      break;

    case SAVE_STRING:
      /* text string */
      setcursor(CURSOR_HOURGLASS, 0, 0);
      InputString(clp.savefilename[val], 't');
      setcursor(CURSOR_ARROW, 0, 0);
      break;
    }

    val = 0;
  default:
    val = atoi(str);
  }

  if (val > 0)
    SelectInMenu(val);

  SetIdle();
  is_menu = 0;
}

void post_menu_handler(Widget w, void *menu, union _XEvent *event, char *str) {
  menu = animateMenu ? animate_menu : main_menu;
  is_menu = 1;
  XmMenuPosition((Widget)menu, (XButtonPressedEvent *)event);
  XtManageChild((Widget)menu);
  XFlush(XtDisplay(w));
}

int is_menu_up(void) {
  if (is_menu) {
    /* just in case that menu was unmanaged without a selection */
    return is_menu = XtIsManaged(animateMenu ? animate_menu : main_menu);
  } else
    return 0;
}

void SetAnimateMenu(void) { animateMenu = 1; }

extern int double_buffering;

void SetMainMenu(void) {
  animateMenu = 0;
  double_buffering = 1;
}
