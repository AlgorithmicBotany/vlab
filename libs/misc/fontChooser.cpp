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



#include <X11/Intrinsic.h>
#include <Xm/XmAll.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "fontChooser.h"
#include "xstring.h"

#define DEFAULT_FONT "-adobe-helvetica-medium-r-normal--14-*-*-*-*-*-*-*"

static Widget shell;
static Widget familyList;
static Widget styleList;
static Widget sizeList;
static Widget okButton;
static Widget applyButton;
static Widget cancelButton;
static Widget helpButton;
static Widget defaultFontButton;
static Widget form;
static Widget testTextLabel;
static Widget fontNameTextWidget;
static Bool okButtonWorking;
static Bool applyButtonWorking;
static char fontName[4096];

static char **fontNames;
static int nFonts;

static char **familyNames = NULL; /* array of all family names */
static char **styleNames = NULL;  /* array of all style names */
static char **sizeNames = NULL;   /* array of all sizes */
static int nFamilyNames = 0;      /* number of distinct families */
static int nStyleNames = 0;       /* number of distinct styles */
static int nSizeNames = 0;        /* numbe of distinct sizes */
static Bool *familyValid = NULL;  /* family name_i valid? */
static Bool *styleValid = NULL;   /* style name_i valid? */
static Bool *sizeValid = NULL;    /* size name_i valid? */

static int familyChoice;
static int styleChoice;
static int sizeChoice;

typedef struct {
  char *fullName;  /* the full name of this font */
  Bool valid;      /* is this font valid (has all 14 entries) */
  int familyIndex; /* index into the array 'familyNames' */
  int styleIndex;  /* index into the array 'styleNames' */
  int sizeIndex;   /* index into the array 'sizeNames' */
  char *familyStr; /* the string for the family */
  char *styleStr;  /* the string for the style */
  char *sizeStr;   /* the string for the size */
} FontItem;

static FontItem *fontItem;

/*** Prototypes ***/
static void font_get_lists(char ***familyNamesPtr, int *nFamilyNamesPtr,
                           Bool **familyValidPtr, char ***styleNamesPtr,
                           int *nStyleNamesPtr, Bool **styleValidPtr,
                           char ***sizeNamesPtr, int *nSizeNamesPtr,
                           Bool **sizeValidPtr);
static void font_choice_update(int familyChoice, int styleChoice,
                               int sizeChoice);
static char *font_choice_get_name(int familyChoice, int styleChoice,
                                  int sizeChoice);
static void set_choices_from_font_name(char *fontName);
static Bool fontname_valid(char *fontName);
static char *skip(int, char, char *);
static char *extractField(int n, char *str, char *buffer);
static void createLists(void);
static void update_font_lists(void);
static void update_test_font(void);
static void update_font_name(void);
static void addToList(Widget w, char *item, int position, Bool selected);
static int maxStrWidth(char **table, int nItems, XmFontList fontList);
static void familyListCB(Widget _w, XtPointer _p1, XtPointer cd);
static void styleListCB(Widget _w, XtPointer _p1, XtPointer cd);
static void sizeListCB(Widget _w, XtPointer _p1, XtPointer cd);
static void fontNameTextCB(Widget _w, XtPointer _p1, XtPointer cd);
static void defaultFontCB(Widget _w, XtPointer _ud, XtPointer _cd);
static void defaultCancelButtonCB(Widget _w, XtPointer _ud, XtPointer _cd);
static int strptrcmp(const void *s1, const void *s2);
static int strptrnumcmp(const void *s1, const void *s2);
static int xmStringLookup(XmString xmString, char **names, int nNames);
static void update_buttons(void);

/*** end of prototypes ***/

/******************************************************************************
 *
 * create a fontChooser dialog
 *
 * In the top portion, there will be 3 lists: family, style and size.
 *
 * In the middle, there will be an example of the font currently selected.
 * Below will be name of the font (editable)
 *
 * At the bottom, there will be 4 buttons (up to 4 buttons)
 *
 */

Widget FedCreateFontChooserDialog(
    Widget parent, char *name, ArgList arglist, Cardinal argcount,
    XtCallbackProc okButtonCB, XtPointer okData, XtCallbackProc applyButtonCB,
    XtPointer applyData, XtCallbackProc cancelButtonCB, XtPointer cancelData,
    XtCallbackProc helpButtonCB, XtPointer helpData) {
  XmString s;
  Widget buttonSeparator;
  Widget w1;
  Arg args[20];
  XmFontList fontList;

  /* first create the shell */
  shell = XtVaCreateWidget(name, xmDialogShellWidgetClass, parent, XmNtitle,
                           "Font Selection", XmNallowShellResize, True,
                           XmNmarginWidth, 5, XmNmarginHeight, 5,
                           XmNdeleteResponse, XmUNMAP, NULL);
  XtSetValues(shell, arglist, argcount);

  /* obtain all the needed fonts from the X server, and create
   * the 3 lists (of font families, styles and sizes) */
  createLists();

  /* now create the form */
  form = XtVaCreateWidget("fontChooserForm", xmFormWidgetClass, shell,
                          XmNmarginWidth, 5, XmNmarginHeight, 5, XmNnoResize,
                          True, XmNresizePolicy, XmRESIZE_ANY, NULL);

  /* default font button - left bottom corner*/
  s = XmStringCreateLocalized("Default Font");
  defaultFontButton = XtVaCreateManagedWidget(
      "defaultFontButton", xmPushButtonWidgetClass, form, XmNleftAttachment,
      XmATTACH_FORM, XmNbottomAttachment, XmATTACH_FORM, XmNmarginWidth, 3,
      XmNmarginHeight, 1, XmNlabelString, s, NULL);
  XmStringFree(s);
  XtRemoveAllCallbacks(defaultFontButton, XmNactivateCallback);
  XtAddCallback(defaultFontButton, XmNactivateCallback, defaultFontCB, NULL);

  /* help, cancel, ok, apply buttons in the right bottom corner */
  /* HELP button: */
  s = XmStringCreateLocalized("Help");
  helpButton = XtVaCreateManagedWidget("helpButton", xmPushButtonWidgetClass,
                                       form, XmNrightAttachment, XmATTACH_FORM,
                                       XmNbottomAttachment, XmATTACH_FORM,
                                       XmNlabelString, s, NULL);
  XmStringFree(s);
  XtRemoveAllCallbacks(helpButton, XmNactivateCallback);
  /* CANCEL button: */
  s = XmStringCreateLocalized("Cancel");
  cancelButton = XtVaCreateManagedWidget(
      "cancelButton", xmPushButtonWidgetClass, form, XmNrightWidget, helpButton,
      XmNrightAttachment, XmATTACH_WIDGET, XmNrightOffset, 5,
      XmNbottomAttachment, XmATTACH_FORM, XmNlabelString, s, NULL);
  XmStringFree(s);
  XtRemoveAllCallbacks(cancelButton, XmNactivateCallback);
  /* APPLY button: */
  s = XmStringCreateLocalized("Apply");
  applyButton = XtVaCreateManagedWidget(
      "applyButton", xmPushButtonWidgetClass, form, XmNrightWidget,
      cancelButton, XmNrightAttachment, XmATTACH_WIDGET, XmNrightOffset, 5,
      XmNbottomAttachment, XmATTACH_FORM, XmNlabelString, s, NULL);
  XmStringFree(s);
  XtRemoveAllCallbacks(applyButton, XmNactivateCallback);
  applyButtonWorking = (applyButtonCB != NULL);
  /* OK button: */
  s = XmStringCreateLocalized("OK");
  okButton = XtVaCreateManagedWidget(
      "okButton", xmPushButtonWidgetClass, form, XmNrightWidget, applyButton,
      XmNrightAttachment, XmATTACH_WIDGET, XmNrightOffset, 5,
      XmNbottomAttachment, XmATTACH_FORM, XmNlabelString, s, NULL);
  XmStringFree(s);
  XtRemoveAllCallbacks(okButton, XmNactivateCallback);
  okButtonWorking = (okButtonCB != NULL);
  /* add a separator */
  buttonSeparator = XtVaCreateManagedWidget(
      "separator", xmSeparatorWidgetClass, form, XmNbottomWidget, cancelButton,
      XmNbottomOffset, 5, XmNbottomAttachment, XmATTACH_WIDGET,
      XmNleftAttachment, XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM,
      NULL);
  /* make the 'unused' buttons insensitive */
  if (helpButtonCB != NULL)
    XtAddCallback(helpButton, XmNactivateCallback, helpButtonCB, helpData);
  else
    XtSetSensitive(helpButton, False);
  if (cancelButtonCB != NULL)
    XtAddCallback(cancelButton, XmNactivateCallback, cancelButtonCB,
                  cancelData);
  else
    XtAddCallback(cancelButton, XmNactivateCallback, defaultCancelButtonCB,
                  NULL);
  if (applyButtonCB != NULL)
    XtAddCallback(applyButton, XmNactivateCallback, applyButtonCB, applyData);
  else
    XtSetSensitive(applyButton, False);
  if (okButtonCB != NULL)
    XtAddCallback(okButton, XmNactivateCallback, okButtonCB, okData);
  else
    XtSetSensitive(okButton, False);
  /*obtain the lists for family, style and size names */
  font_get_lists(&familyNames, &nFamilyNames, &familyValid, &styleNames,
                 &nStyleNames, &styleValid, &sizeNames, &nSizeNames,
                 &sizeValid);

  /* now the label for the font family */
  s = XmStringCreateLocalized("Family:");
  w1 = XtVaCreateManagedWidget("clabel", xmLabelWidgetClass, form,
                               XmNtopAttachment, XmATTACH_FORM, XmNtopOffset,
                               10, XmNleftAttachment, XmATTACH_FORM,
                               XmNlabelString, s, NULL);
  XmStringFree(s);
  /* now we create the list of family fonts */
  int n = 0;
  XtSetArg(args[n], XmNtopWidget, w1);
  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);
  n++;
  XtSetArg(args[n], XmNtopOffset, 5);
  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM);
  n++;
  XtSetArg(args[n], XmNvisibleItemCount, 10);
  n++;
  XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT);
  n++;
  XtSetArg(args[n], XmNlistSizePolicy, XmCONSTANT);
  n++;
  XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmSTATIC);
  n++;
  familyList = XmCreateScrolledList(form, "familyList", args, n);
  XtManageChild(familyList);
  XtAddCallback(familyList, XmNsingleSelectionCallback, familyListCB, NULL);
  XtVaGetValues(familyList, XmNfontList, &fontList, NULL);
  XtVaSetValues(familyList, XmNwidth,
                10 + maxStrWidth(familyNames, nFamilyNames, fontList) + 10,
                NULL);
  /* label for the Style list */
  s = XmStringCreateLocalized("Style:");
  w1 = XtVaCreateManagedWidget(
      "clabel", xmLabelWidgetClass, form, XmNleftWidget, familyList,
      XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset, 5, XmNtopAttachment,
      XmATTACH_FORM, XmNtopOffset, 10, XmNlabelString, s, NULL);
  XmStringFree(s);
  /* list for the styles */
  n = 0;
  XtSetArg(args[n], XmNtopWidget, w1);
  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);
  n++;
  XtSetArg(args[n], XmNtopOffset, 5);
  n++;
  XtSetArg(args[n], XmNleftWidget, familyList);
  n++;
  XtSetArg(args[n], XmNleftOffset, 5);
  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET);
  n++;
  XtSetArg(args[n], XmNvisibleItemCount, 10);
  n++;
  XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT);
  n++;
  XtSetArg(args[n], XmNlistSizePolicy, XmCONSTANT);
  n++;
  XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmSTATIC);
  n++;
  styleList = XmCreateScrolledList(form, "styleList", args, n);
  XtManageChild(styleList);
  XtAddCallback(styleList, XmNsingleSelectionCallback, styleListCB, NULL);
  XtVaGetValues(styleList, XmNfontList, &fontList, NULL);
  XtVaSetValues(styleList, XmNwidth,
                10 + maxStrWidth(styleNames, nStyleNames, fontList) + 10, NULL);
  //  unmanage the horizontal scroll bar
  Widget styl_h_bar;
  XtVaGetValues(styleList, XmNhorizontalScrollBar, &styl_h_bar, NULL);
  XtUnmanageChild(styl_h_bar);
  /* label for the Size list */
  s = XmStringCreateLocalized("Size:");
  w1 = XtVaCreateManagedWidget(
      "clabel", xmLabelWidgetClass, form, XmNleftWidget, styleList,
      XmNleftAttachment, XmATTACH_WIDGET, XmNleftOffset, 5, XmNtopAttachment,
      XmATTACH_FORM, XmNtopOffset, 10, XmNlabelString, s, NULL);
  XmStringFree(s);
  /* list for the styles */
  n = 0;
  XtSetArg(args[n], XmNtopWidget, w1);
  n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET);
  n++;
  XtSetArg(args[n], XmNtopOffset, 5);
  n++;
  XtSetArg(args[n], XmNleftWidget, styleList);
  n++;
  XtSetArg(args[n], XmNleftOffset, 5);
  n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET);
  n++;
  XtSetArg(args[n], XmNvisibleItemCount, 10);
  n++;
  XtSetArg(args[n], XmNselectionPolicy, XmSINGLE_SELECT);
  n++;
  XtSetArg(args[n], XmNlistSizePolicy, XmCONSTANT);
  n++;
  XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmSTATIC);
  n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM);
  n++;
  XtSetArg(args[n], XmNrightOffset, 5);
  n++;
  sizeList = XmCreateScrolledList(form, "sizeList", args, n);
  XtManageChild(sizeList);
  XtAddCallback(sizeList, XmNsingleSelectionCallback, sizeListCB, NULL);
  XtVaGetValues(sizeList, XmNfontList, &fontList, NULL);
  XtVaSetValues(sizeList, XmNwidth,
                10 + maxStrWidth(sizeNames, nSizeNames, fontList) + 10, NULL);
  /* separator */
  w1 = XtVaCreateManagedWidget(
      "Separator4", xmSeparatorWidgetClass, form, XmNleftAttachment,
      XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM, XmNtopWidget,
      familyList, XmNtopAttachment, XmATTACH_WIDGET, XmNtopOffset, 5, NULL);
  /* the label for the text */
  s = XmStringCreateLocalized(" ");
  testTextLabel = XtVaCreateManagedWidget(
      "testFontLabel", xmLabelWidgetClass, form, XmNleftAttachment,
      XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM, XmNtopWidget, w1,
      XmNtopAttachment, XmATTACH_WIDGET, XmNtopOffset, 5, XmNheight, 70,
      XmNrecomputeSize, False, XmNlabelString, s, NULL, NULL);
  XmStringFree(s);
  /* separator */
  w1 = XtVaCreateManagedWidget(
      "Separator5", xmSeparatorWidgetClass, form, XmNleftAttachment,
      XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM, XmNtopWidget,
      testTextLabel, XmNtopAttachment, XmATTACH_WIDGET, XmNtopOffset, 5, NULL);
  /* label for "Font Name" */
  s = XmStringCreateLocalized("Font Name:");
  w1 = XtVaCreateManagedWidget("clabel", xmLabelWidgetClass, form,
                               XmNleftAttachment, XmATTACH_FORM, XmNtopWidget,
                               w1, XmNtopAttachment, XmATTACH_WIDGET,
                               XmNlabelString, s, NULL);
  XmStringFree(s);
  /* the text field for the font name */
  fontNameTextWidget = XtVaCreateManagedWidget(
      "fontNameText", xmTextFieldWidgetClass, form, XmNleftAttachment,
      XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM, XmNtopWidget, w1,
      XmNtopAttachment, XmATTACH_WIDGET, XmNbottomWidget, buttonSeparator,
      XmNbottomOffset, 5, XmNbottomAttachment, XmATTACH_WIDGET, NULL);
  XtAddCallback(fontNameTextWidget, XmNactivateCallback, fontNameTextCB, NULL);

  /* set the choices for the font */
  FedFontChooserSetFontName(shell, DEFAULT_FONT);

  /* and now update the interface */
  strcpy(fontName, DEFAULT_FONT);
  set_choices_from_font_name(fontName);
  update_font_lists();
  update_test_font();
  update_font_name();
  update_buttons();

  return form;
}

/******************************************************************************
 *
 * extracts the name of the font from the fontChooser
 *
 * If no font has been chosen, return NULL
 *
 * The returned string should be freed up by XtFree
 *
 */

char *FedFontChooserGetFontName(Widget) {
  if (*fontName == '\0')
    return NULL;
  else
    return (char *)XtNewString(fontName);
}

/******************************************************************************
 *
 * will set the font in the fontChooser, and accordingly update the
 * lists and the test font
 *
 */

void FedFontChooserSetFontName(Widget, char *str) {
  if (fontname_valid(str)) {
    strcpy(fontName, str);
    set_choices_from_font_name(fontName);
  } else {
    familyChoice = -1;
    styleChoice = -1;
    sizeChoice = -1;
    strcpy(fontName, "");
  }
  update_font_lists();
  update_test_font();
  update_font_name();
  update_buttons();
}

/******************************************************************************
*******************************************************************************
***
***                         LOCAL FUNCTIONS
***
******************************************************************************
******************************************************************************/

/******************************************************************************
 *
 * default callback for the 'Cancel' button
 *
 */

static void defaultCancelButtonCB(Widget, XtPointer, XtPointer) {
  /* XtUnmanageChild( shell);*/
  XtUnmanageChild(form);
}

/******************************************************************************
 *
 * callback for the 'default' button
 *
 */

static void defaultFontCB(Widget, XtPointer, XtPointer) {

  strcpy(fontName, DEFAULT_FONT);

  set_choices_from_font_name(fontName);
  update_font_lists();
  update_test_font();
  update_font_name();
  update_buttons();
}

/******************************************************************************
 *
 * return the lists of family, style & size names, their numbers, and
 * Boolean fields pointers to the validity fields
 *
 */

static void font_get_lists(char ***familyNamesPtr, int *nFamilyNamesPtr,
                           Bool **familyValidPtr, char ***styleNamesPtr,
                           int *nStyleNamesPtr, Bool **styleValidPtr,
                           char ***sizeNamesPtr, int *nSizeNamesPtr,
                           Bool **sizeValidPtr) {

  *familyNamesPtr = familyNames;
  *nFamilyNamesPtr = nFamilyNames;
  *familyValidPtr = familyValid;
  *styleNamesPtr = styleNames;
  *nStyleNamesPtr = nStyleNames;
  *styleValidPtr = styleValid;
  *sizeNamesPtr = sizeNames;
  *nSizeNamesPtr = nSizeNames;
  *sizeValidPtr = sizeValid;
}

/******************************************************************************
 *
 * returns the pointer into the 'str', right after the 'n'-th character 'c'.
 *
 * if there is not 'n' characters 'c' in 'str', it will return NULL
 *
 */

static char *skip(int n, char c, char *str) {
  char *s;

  s = str;

  while (n > 0) {
    if (*s == '\0')
      return NULL;
    if (*s == c)
      n--;
    s++;
  }

  return s;
}

/******************************************************************************
 *
 * will extract the field n into buffer from str, where str represents
 * a font
 *
 */

static char *extractField(int n, char *str, char *buffer) {
  char *s;
  int j;

  /* set s to the beginning of that field */
  s = skip(n, '-', str);
  if (s == NULL)
    return NULL;

  /* extract the name into buffer */
  for (j = 0; (s[j] != '-') && (s[j] != '\0'); j++)
    buffer[j] = s[j];
  buffer[j] = '\0';

  return s;
}

/******************************************************************************
 *
 * this routine is to be only used by font_choice_update()
 *
 * it will return True if the choice is valid using the current validity
 * arrays and fonts, otherwise it will return False
 *
 */

static Bool validChoice(int familyChoice, int styleChoice, int sizeChoice) {
  int j;

  for (j = 0; j < nFonts; j++) {
    if (fontItem[j].valid &&
        ((familyChoice == -1) || (familyChoice == fontItem[j].familyIndex)) &&
        ((styleChoice == -1) || (styleChoice == fontItem[j].styleIndex)) &&
        ((sizeChoice == -1) || (sizeChoice == fontItem[j].sizeIndex)))

      return True;
  }
  return False;
}

/******************************************************************************
 *
 * input: familyChoice, styleChoice, sizeChoice
 *
 * output: The boolean arrays familyChoice, styleChoice and sizeChoice
 *         will contain True or False, depending whether for the given
 *         choice the corresponding item can still be selected
 *
 */

static void font_choice_update(int familyChoice, int styleChoice,
                               int sizeChoice) {
  int i;

  /* set all the validity fields */
  for (i = 0; i < nFamilyNames; i++)
    familyValid[i] = validChoice(i, styleChoice, sizeChoice);
  for (i = 0; i < nStyleNames; i++)
    styleValid[i] = validChoice(familyChoice, i, sizeChoice);
  for (i = 0; i < nSizeNames; i++)
    sizeValid[i] = validChoice(familyChoice, styleChoice, i);
}

/******************************************************************************
 *
 * Based on the choice it will return the full name of the font that
 * matches this choice, otherwise it returns NULL
 *
 */

static char *font_choice_get_name(int familyChoice, int styleChoice,
                                  int sizeChoice) {
  int i;

  for (i = 0; i < nFonts; i++)
    if (fontItem[i].valid)
      if ((fontItem[i].familyIndex == familyChoice) &&
          (fontItem[i].styleIndex == styleChoice) &&
          (fontItem[i].sizeIndex == sizeChoice))
        return fontItem[i].fullName;

  return NULL;
}

/******************************************************************************
 *
 * will set the choices according to a font
 *
 */

static void set_choices_from_font_name(char *name) {
  char **fontNames2;
  int nFonts2;
  int i;

  fontNames2 = XListFonts(XtDisplay(shell), name, 1, &nFonts2);

  if (nFonts2 == 0) {
    familyChoice = -1;
    styleChoice = -1;
    sizeChoice = -1;
  } else {
    for (i = 0; i < nFonts; i++)
      if (xstrcmp(fontNames2[0], fontNames[i]) == 0)
        break;
    if (i == nFonts) {
      /* font not found in our list - that doesn't mean that the font is
       * not valid :-) */
      familyChoice = -1;
      styleChoice = -1;
      sizeChoice = -1;
    } else {
      familyChoice = fontItem[i].familyIndex;
      styleChoice = fontItem[i].styleIndex;
      sizeChoice = fontItem[i].sizeIndex;
    }
  }

  XFreeFontNames(fontNames2);
}

/******************************************************************************
 *
 * finds out whether this fontname is valid, by trying to get the fontinfo
 * about this font from the Xserver
 *
 */

static Bool fontname_valid(char *fontName) {
  Bool valid;
  XFontStruct *fontInfo;

  fontInfo = XLoadQueryFont(XtDisplay(shell), fontName);

  valid = (fontInfo != NULL);

  if (valid)
    XFreeFont(XtDisplay(shell), fontInfo);

  return valid;
}

/******************************************************************************
 *
 * Obtain all the font names from the Xserver, and create the 3 lists of
 * family, style and size names
 *
 */

static void createLists(void) {
  int i, j;
  char tmpStr[4096];
  char foundryStr[4096];
  char familyStr[4096];
  char styleStr[4096];
  char sizeStr[4096];

  /* obtain the names of all fonts available on the system,
   * and parse them (obtaining the information about */
  fontNames = XListFonts(XtDisplay(shell), "*", 10000, &nFonts);

  if (fontNames == NULL) {
    fprintf(stderr, "Error! No font names obtained!\n\n");
    exit(-1);
  }

  /* now extract the necessary information from each font name
   * (i.e. the family, style & font size) into the familyNames,
   * styleNames and sizeNames */
  fontItem = (FontItem *)XtMalloc(sizeof(FontItem) * nFonts);
  for (i = 0; i < nFonts; i++) {

    /* assign the item the full name */
    fontItem[i].fullName = fontNames[i];
    /* set the font to be a valid font */
    fontItem[i].valid = True;

    /* extract the foundry name */
    if (NULL == extractField(1, fontNames[i], foundryStr)) {
      fontItem[i].valid = False; /* invalid item */
      continue;
    }
    /* extract the family name into tmpStr*/
    if (NULL == extractField(2, fontNames[i], tmpStr)) {
      fontItem[i].valid = False; /* invalid item */
      continue;
    }
    if (*tmpStr == '\0')
      sprintf(tmpStr, "%s", "<noname>");
    /* copy the family name together with foundry into the 'familyStr' */
    sprintf(familyStr, "%s (%s)", tmpStr, foundryStr);

    /* extract the style name (weight) */
    if (NULL == extractField(3, fontNames[i], styleStr)) {
      fontItem[i].valid = False; /* invalid item */
      continue;
    }
    if (*styleStr == '\0') {
      fontItem[i].valid = False; /* invalid item */
      continue;
    }

    /* extract the slant string */
    if (NULL != extractField(4, fontNames[i], tmpStr))
      if (*tmpStr == 'i')
        strcat(styleStr, " italic");
      else if (*tmpStr == 'o')
        strcat(styleStr, " oblique");

    /* extract the width string */
    if (NULL != extractField(5, fontNames[i], tmpStr))
      if (*tmpStr != '\0')
        if (xstrcmp(tmpStr, "normal") != 0) {
          strcat(styleStr, " ");
          strcat(styleStr, tmpStr);
        }

    /* extract the size */
    if (NULL == extractField(7, fontNames[i], sizeStr)) {
      fontItem[i].valid = False; /* invalid item */
      continue;
    }
    if (*sizeStr == '\0') {
      fontItem[i].valid = False; /* invalid item */
      continue;
    }

    /* now we want to insert the new family name into the list of
     * family names, but first we check if this family name is not already
     * in the array */
    for (j = 0; j < nFamilyNames; j++)
      if (xstrcmp(familyNames[j], familyStr) == 0) {
        fontItem[i].familyStr = familyNames[j];
        break;
      }
    if (j == nFamilyNames) {
      nFamilyNames += 1;
      familyNames = (char **)XtRealloc((char *)familyNames,
                                       sizeof(char *) * nFamilyNames);
      familyNames[nFamilyNames - 1] = strdup(familyStr);
      fontItem[i].familyStr = familyNames[nFamilyNames - 1];
    }

    /* do the same thing for the style name */
    for (j = 0; j < nStyleNames; j++)
      if (xstrcmp(styleNames[j], styleStr) == 0) {
        fontItem[i].styleStr = styleNames[j];
        break;
      }
    if (j == nStyleNames) {
      nStyleNames += 1;
      styleNames =
          (char **)XtRealloc((char *)styleNames, sizeof(char *) * nStyleNames);
      styleNames[nStyleNames - 1] = strdup(styleStr);
      fontItem[i].styleStr = styleNames[nStyleNames - 1];
    }

    /* do the same thing for the size name */
    for (j = 0; j < nSizeNames; j++)
      if (xstrcmp(sizeNames[j], sizeStr) == 0) {
        fontItem[i].sizeStr = sizeNames[j];
        break;
      }
    if (j == nSizeNames) {
      nSizeNames += 1;
      sizeNames =
          (char **)XtRealloc((char *)sizeNames, sizeof(char *) * nSizeNames);
      sizeNames[nSizeNames - 1] = strdup(sizeStr);
      fontItem[i].sizeStr = sizeNames[nSizeNames - 1];
    }
  }

  /* now sort the names in all three lists */
  qsort(familyNames, nFamilyNames, sizeof(char *), strptrcmp);
  qsort(styleNames, nStyleNames, sizeof(char *), strptrcmp);
  qsort(sizeNames, nSizeNames, sizeof(char *), strptrnumcmp);

  /* assign to each fontItem the indices into the family-, style- &
   * size- names arrays */
  for (i = 0; i < nFonts; i++) {
    if (!fontItem[i].valid)
      continue;
    for (j = 0; j < nFamilyNames; j++)
      if (fontItem[i].familyStr == familyNames[j]) {
        fontItem[i].familyIndex = j;
        break;
      }
    for (j = 0; j < nStyleNames; j++)
      if (fontItem[i].styleStr == styleNames[j]) {
        fontItem[i].styleIndex = j;
        break;
      }
    for (j = 0; j < nSizeNames; j++)
      if (fontItem[i].sizeStr == sizeNames[j]) {
        fontItem[i].sizeIndex = j;
        break;
      }
  }

  /* create the boolean arrays for each item in the 3 lists */
  familyValid = (Bool *)XtMalloc(sizeof(Bool) * nFamilyNames);
  styleValid = (Bool *)XtMalloc(sizeof(Bool) * nStyleNames);
  sizeValid = (Bool *)XtMalloc(sizeof(Bool) * nSizeNames);

  /*
      printf("There is %d family names in the list\n", nFamilyNames);
      for( i = 0 ; i < nFamilyNames ; i ++)
          printf("%3d:%s\n", i, familyNames[ i]);

      printf("There is %d style names in the list\n", nStyleNames);
      for( i = 0 ; i < nStyleNames ; i ++)
          printf("%3d:%s\n", i, styleNames[ i]);

      printf("There is %d size names in the list\n", nSizeNames);
      for( i = 0 ; i < nSizeNames ; i ++)
          printf("%3d:%s\n", i, sizeNames[ i]);

      font = ( XFontStruct **) XtMalloc( sizeof( XFontStruct *) * nFonts);
      for( i = 0 ; i < nFonts ; i ++) {
          printf( "XLoadQueryFont(%d)\n", i);
          font[ i] = XLoadQueryFont( xdisplay, fontNames[ i]);
      }

      fontList = XmFontListCreate( font[ 0], "f0");
      for( i = 1 ; i < nFonts ; i ++) {
          sprintf( tmpStr, "f%d", i);
          fontList = XmFontListAdd( fontList, font[ i], tmpStr);
      }
      */
}

/******************************************************************************
 *
 * updates the font lists based on the selection
 *
 */

static void update_font_lists(void) {
  int i, pos;

  /* get new 'validity' :-) arrays for the choices */
  font_choice_update(familyChoice, styleChoice, sizeChoice);

  /* delete everything from all lists */
  XmListDeleteAllItems(familyList);
  XmListDeleteAllItems(styleList);
  XmListDeleteAllItems(sizeList);

  /* add the family names */
  for (i = 0; i < nFamilyNames; i++)
    if (familyValid[i])
      addToList(familyList, familyNames[i], i + 1, familyChoice == i);
  if (familyChoice != -1) {
    pos = 1;
    for (i = 0; i < familyChoice; i++)
      if (familyValid[i])
        pos += 1;
    XmListSetBottomPos(familyList, pos);
  }
  /* add the style names */
  for (i = 0; i < nStyleNames; i++)
    if (styleValid[i])
      addToList(styleList, styleNames[i], i + 1, styleChoice == i);
  if (styleChoice != -1) {
    pos = 1;
    for (i = 0; i < styleChoice; i++)
      if (styleValid[i])
        pos += 1;
    XmListSetBottomPos(styleList, pos);
  }
  /* add the size names */
  for (i = 0; i < nSizeNames; i++)
    if (sizeValid[i])
      addToList(sizeList, sizeNames[i], i + 1, sizeChoice == i);
  if (sizeChoice != -1) {
    pos = 1;
    for (i = 0; i < sizeChoice; i++)
      if (sizeValid[i])
        pos += 1;
    XmListSetBottomPos(sizeList, pos);
  }
}

/******************************************************************************
 *
 * returns the max. width of all the strings in the table of strings when
 * converted to the xmStrings
 *
 */

static int maxStrWidth(char **table, int nItems, XmFontList fontList) {

  int max, i;

  XmString str;
  int width;
  str = XmStringCreateLocalized(table[0]);
  max = XmStringWidth(fontList, str);
  XmStringFree(str);
  for (i = 1; i < nItems; i++) {
    str = XmStringCreateLocalized(table[i]);
    width = XmStringWidth(fontList, str);
    XmStringFree(str);
    if (max < width)
      max = width;
  }

  return max;
}

/******************************************************************************
 *
 * update the font-name text-field widget (according to the choice)
 *
 */

static void update_font_name(void) {

  XtVaSetValues(fontNameTextWidget, XmNvalue, fontName, NULL);
}

/******************************************************************************
 *
 * update the label that displays the example of the font (test-font)
 * The test font will be the one in fontName
 *
 */

static void update_test_font(void) {
  XmString s;
  static XFontStruct *old_font_info = NULL;
  XmFontList fontList;

  if (*fontName != '\0') {
    XFontStruct *fontInfo = XLoadQueryFont(XtDisplay(shell), fontName);
    if (fontInfo != NULL) {
      fontList = XmFontListCreate(fontInfo, "c1");
      if (fontList != NULL) {
        s = XmStringCreateSimple("ABCDEFGHIJ klmnopqrstuvwxyz 1234567890");
        XtVaSetValues(testTextLabel, XmNlabelString, s, XmNfontList, fontList,
                      NULL);
        XmStringFree(s);
        XmFontListFree(fontList);
        if (old_font_info != NULL)
          XFreeFont(XtDisplay(shell), old_font_info);
        old_font_info = fontInfo;
      } else {
        s = XmStringCreateSimple("Could not load this font...");
        XtVaSetValues(testTextLabel, XmNlabelString, s, NULL);
        XmStringFree(s);
      }
    }
  } else {
    // invalid font - don't display any test font
    s = XmStringCreateLocalized(" ");
    XtVaSetValues(testTextLabel, XmNlabelString, s, NULL);
    XmStringFree(s);
  }
}

/******************************************************************************
 *
 * this is basically just a wrapper for the XmListAddItemUnselected,
 * which will create an XmString from the regular String,
 * and then destroys it
 */

static void addToList(Widget w, char *itemName, int position, Bool selected) {
  XmString s;

  s = XmStringCreateLocalized(itemName);
  XmListAddItemUnselected(w, s, position);
  if (selected)
    XmListSelectItem(w, s, False);
  XmStringFree(s);
}

/******************************************************************************
 *
 * compare two strings to which 2 double-pointers are passed (good for
 * qsort(), or compare two numbers (stored as strings)  )
 *
 */

static int strptrcmp(const void *s1, const void *s2) {
  return xstrcmp(*((char **)s1), *((char **)s2));
}

static int strptrnumcmp(const void *s1, const void *s2) {
  int n1, n2;

  sscanf(*((char **)s1), "%d", &n1);
  sscanf(*((char **)s2), "%d", &n2);
  if (n1 == n2)
    return 0;
  if (n1 < n2)
    return -1;
  else
    return 1;
}

/******************************************************************************
 *
 * when the user inputs a font in the fontName text field
 *
 */

static void fontNameTextCB(Widget w, XtPointer, XtPointer) {
  char *str;

  str = XmTextFieldGetString(w);
  if (fontname_valid(str)) {
    strcpy(fontName, str);
    set_choices_from_font_name(fontName);
  } else {
    familyChoice = -1;
    styleChoice = -1;
    sizeChoice = -1;
    strcpy(fontName, "");
  }

  update_font_lists();
  update_test_font();
  update_buttons();

  XtFree(str);
}

/******************************************************************************
 *
 * callback for the familyNamesList events
 *
 */

static void familyListCB(Widget w, XtPointer, XtPointer cd) {
  char *str;

  XmListCallbackStruct *listCD = (XmListCallbackStruct *)cd;
  int n;

  XtVaGetValues(w, XmNselectedItemCount, &n, NULL);
  if (n == 0) {
    familyChoice = -1;
    strcpy(fontName, "");
  } else {
    familyChoice = xmStringLookup(listCD->item, familyNames, nFamilyNames);
    str = font_choice_get_name(familyChoice, styleChoice, sizeChoice);
    if (str != NULL)
      strcpy(fontName, str);
    else
      strcpy(fontName, "");
  }

  update_font_lists();
  update_font_name();
  update_test_font();
  update_buttons();
}

/******************************************************************************
 *
 * callback for the styleNamesList events
 *
 */

static void styleListCB(Widget w, XtPointer, XtPointer cd) {
  char *str;

  XmListCallbackStruct *listCD = (XmListCallbackStruct *)cd;
  int n;

  XtVaGetValues(w, XmNselectedItemCount, &n, NULL);
  if (n == 0) {
    styleChoice = -1;
    strcpy(fontName, "");
  } else {
    styleChoice = xmStringLookup(listCD->item, styleNames, nStyleNames);
    str = font_choice_get_name(familyChoice, styleChoice, sizeChoice);
    if (str != NULL)
      strcpy(fontName, str);
    else
      strcpy(fontName, "");
  }

  update_font_lists();
  update_font_name();
  update_test_font();
  update_buttons();
}

/******************************************************************************
 *
 * callback for the sizeNamesList events
 *
 */

static void sizeListCB(Widget w, XtPointer, XtPointer cd) {
  char *str;

  XmListCallbackStruct *listCD = (XmListCallbackStruct *)cd;
  int n;

  XtVaGetValues(w, XmNselectedItemCount, &n, NULL);
  if (n == 0) {
    sizeChoice = -1;
    strcpy(fontName, "");
  } else {
    sizeChoice = xmStringLookup(listCD->item, sizeNames, nSizeNames);
    str = font_choice_get_name(familyChoice, styleChoice, sizeChoice);
    if (str != NULL)
      strcpy(fontName, str);
    else
      strcpy(fontName, "");
  }

  update_font_lists();
  update_font_name();
  update_test_font();
  update_buttons();
}

/******************************************************************************
 *
 * looks up the xmString in the array of strings 'names', which contains
 * 'nNames' entries, and returns the index to the element if found otherwise
 * it returns '-1'
 *
 */

static int xmStringLookup(XmString xmString, char **names, int nNames) {
  char *name;
  int i;

  if (!XmStringGetLtoR(xmString, XmSTRING_DEFAULT_CHARSET, &name)) {
    assert(1 == 2);
  }

  for (i = 0; i < nNames; i++)
    if (xstrcmp(name, names[i]) == 0)
      return i;

  return -1;
}

/******************************************************************************
 *
 * will update the sensitivity of the buttons 'ok' and 'apply' according
 * to whether there is a valid font selected
 *
 * If no valid font is selected, the buttons 'OK' and "Apply" are greyed
 * out, otherwise they are set.
 *
 */

void update_buttons(void) {
  Bool valid;

  valid = (*fontName != '\0');

  if (applyButtonWorking)
    XtSetSensitive(applyButton, valid);
  if (okButtonWorking)
    XtSetSensitive(okButton, valid);
}
