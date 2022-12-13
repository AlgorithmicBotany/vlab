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



#include "config.h"
#ifdef USE_OVERLAYS

/* International message interface with app-defaults.
   LoadIntlResources( toplevel_widget, subname, subclassname );

   Your resources should be of the form
        <applicationname>.<subname>.<message-tag>
    eg:
        mwm.intl.m7: Cannot allocate space for menu
*/

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <malloc.h>
#include <stdio.h>

#ifdef SG_IS_STATIC
/*
 * This hideous hack is to allow XmRoxy to compile and
 * use this code without actually adding any symbols to
 * the Xm library.
 *       #ifdef SG_IS_STATIC
 *       static
 *       #endif
 * is placed at the beginning of each function definition;
 * XmRoxy will define SG_IS_STATIC.
 * Now that more than one subsystem (window manager, XmRoxy)
 * is using this code, perhaps it should be put in a more
 * central place (libSgt?).  At that time this hack can
 * be removed.
 */
#endif

static char *_intl[][2] = {
    {"_SGI_MMX_mwm_nogrmem", "Insufficient memory for graphics data"},
    {"_SGI_MMX_mwm_divideby0", "Fatal Error: Divide by 0 in SetMwmIconInfo\n"},
    {"_SGI_MMX_mwm_iconmisfit", "Could not make icon to go in icon box"},
    {"_SGI_MMX_mwm_noiconmem", "Insufficient memory for icon creation"},
    {"_SGI_MMX_mwm_noclientwindowmem",
     "Insufficient memory for client window framing"},
    {"_SGI_MMX_mwm_readerr_title", "Error reading Title Elements\n"},
    {"_SGI_MMX_mwm_noiconputmem", "Insufficient memory for icon placement"},
    {"_SGI_MMX_mwm_nomem_icondata",
     "Insufficient memory to create icon box data"},
    {"_SGI_MMX_mwm_nomem_windata",
     "Insufficient memory for window management data"},
    {"_SGI_MMX_mwm_nomem_winflags",
     "Insufficient memory for window manager flags"},
    {"_SGI_MMX_mwm_nomem_atom",
     "Insufficient memory to XInternAtom _MOTIF_WM_QUERY_nn"},
    {"_SGI_MMX_mwm_ownfailed", "Failed to own _MOTIF_WM_QUERY_nn selection"},
    {"_SGI_MMX_mwm_nomem_winmgrdata",
     "Insufficient memory for window manager data"},
    {"_SGI_MMX_mwm_listsel", "Lost _MOTIF_WM_QUERY_nn selection"},
    {"_SGI_MMX_mwm_nomem_cvtsel",
     "Insufficient memory to convert _MOTIF_WM_QUERY_nn selection"},
    {"_SGI_MMX_mwm_cant_restartwm", "Cannot restart the window manager"},
    {"_SGI_MMX_mwm_cantwrap", "Unable to warp to unmanaged screen %d\n"},
    {"_SGI_MMX_mwm_bad_iconbmp", "Invalid icon bitmap"},
    {"_SGI_MMX_mwm_bad_rooticonbmp", "Invalid root for icon bitmap"},
    {"_SGI_MMX_mwm_cip_notsup", "Warning color icon pixmap not supported"},
    {"_SGI_MMX_mwm_nomem_bevel", "Insufficient memory to bevel icon image"},
    {"_SGI_MMX_mwm_nomem_bmp", "Insufficient memory for bitmap %s\n"},
    {"_SGI_MMX_mwm_bad_bmp", "Unable to read bitmap file %s\n"},
    {"_SGI_MMX_mwm_invalid_bmp", "Invalid bitmap file %s\n"},
    {"_SGI_MMX_mwm_nomem_scrdata", "Insufficient memory for Screen data"},
    {"_SGI_MMX_mwm_twowm", "Another window manager is running on screen %d"},
    {"_SGI_MMX_mwm_toomany_scr", "Unable to manage any screens on display."},
    {"_SGI_MMX_mwm_bad_X", "Cannot configure X connection"},
    {"_SGI_MMX_mwm_nomem_wkspdata", "Insufficient memory for Workspace data"},
    {"_SGI_MMX_mwm_nomem_dpystr", "Insufficient memory for displayString"},
    {"_SGI_MMX_mwm_few_scrnames", "Insufficient memory for screen names"},
    {"_SGI_MMX_mwm_nomenu", "Menu specification %s not found\n"},
    {"_SGI_MMX_mwm_nomem_menu", "Insufficient memory for menu %s\n"},
    {"_SGI_MMX_mwm_menu_recursion", "Menu recursion detected for %s\n"},
    {"_SGI_MMX_mwm_nomem_cdata", "Insufficient memory for client data"},
    {"_SGI_MMX_mwm_locale_msg", "[XmbTextPropertyToTextList]:\n     Locale "
                                "(%.100s) not supported. (Check $LANG)."},
    {"_SGI_MMX_mwm_no_iconbox", "Couldn`t make icon box"},
    {"_SGI_MMX_mwm_badcvt",
     "mwm cannot convert property %.100s as clientTitle/iconTitle: "
     "XmbTextPropertyToTextList."},
    {"_SGI_MMX_mwm_nomem_cvt",
     "insufficient memory to convert property %.100s as clientTitle/iconTitle: "
     "XmbTextPropertyToTextList."},
    {"_SGI_MMX_mwm_unknown_prop",
     "mwm receives unknown property as clientTitle/iconName %.100s property "
     "ignored."},
    {"_SGI_MMX_mwm_retry", "Retrying - using builtin window menu\n"},
    {"_SGI_MMX_mwm_cantopen", "%s: can't open %s.\n"},
    {"_SGI_MMX_mwm_rmsg", "Received %s message:  %s\n"},
    {"_SGI_MMX_mwm_visual_mismatch", "matching visual NOT found\n"},
    {"_SGI_MMX_mwm_SGwarn", "%s (%s): %s\n"},
    {"_SGI_MMX_mwm_xerr_msg1", "XError"},
    {"_SGI_MMX_mwm_xerr_defstr1", "X Error"},
    {"_SGI_MMX_mwm_xerr_name", "XlibMessage"},
    {"_SGI_MMX_mwm_xerr_fmt1", "%s:  %s\n  "},
    {"_SGI_MMX_mwm_xerr_msg2", "MajorCode"},
    {"_SGI_MMX_mwm_xerr_defstr2", "Request Major code %d"},
    {"_SGI_MMX_mwm_xerr_name3", "XRequest"},
    {"_SGI_MMX_mwm_xerr_fmt3", " (%s)\n  "},
    {"_SGI_MMX_mwm_xerr_msg4", "MinorCode"},
    {"_SGI_MMX_mwm_xerr_defstr4", "Request Minor code %d"},
    {"_SGI_MMX_mwm_xerr_fmt5_1", "%s.%d"},
    {"_SGI_MMX_mwm_xerr_name5", "XRequest"},
    {"_SGI_MMX_mwm_xerr_fmt5", " (%s)"},
    {"_SGI_MMX_mwm_xerr_fmte", "\n  "},
    {"_SGI_MMX_mwm_xerr_fmt6_1", "%s.%d"},
    {"_SGI_MMX_mwm_xerr_fmt6_2", "Value"},
    {"_SGI_MMX_mwm_xerr_msg7", "Value"},
    {"_SGI_MMX_mwm_xerr_defstr7", "Value 0x%x"},
    {"_SGI_MMX_mwm_xerr_msg8", "AtomID"},
    {"_SGI_MMX_mwm_xerr_defstr8", "AtomID 0x%x"},
    {"_SGI_MMX_mwm_xerr_msg9", "ResourceID"},
    {"_SGI_MMX_mwm_xerr_defstr9", "ResourceID 0x%x"},
    {"_SGI_MMX_mwm_xerr_msg10", "ErrorSerial"},
    {"_SGI_MMX_mwm_xerr_defstr10", "Error Serial #%d"},
    {"_SGI_MMX_mwm_xerr_msg11", "CurrentSerial"},
    {"_SGI_MMX_mwm_xerr_defstr11", "Current Serial #%d"},
    {"_SGI_MMX_mwm_xerr_io", "mwm got an X I/O error\n"},
    {"_SGI_MMX_mwm_xerr_xt", "mwm got an Xt error\n\t%s\n"},
    {"_SGI_MMX_mwm_xerr_xtw", "mwm got an Xt warning\n\t%s\n"},
    {"_SGI_MMX_mwm_tdb", "Toggle to Default Behavior?"},
    {"_SGI_MMX_mwm_tcb", "Toggle to Custom Behavior?"},
    {"_SGI_MMX_mwm_domwm", "Restart Mwm?"},
    {"_SGI_MMX_mwm_quitwm", "QUIT Mwm?"},
    {"_SGI_MMX_mwm_ldfont_failed", "failed to load font: %.100s\0"},
    {"_SGI_MMX_mwm_noapp_font", "cannot find an appropriate font: %.100s\0"},
    {"_SGI_MMX_mwm_inconsis_variable",
     "ERROR: couldn't get a consistent set of visuals\n"},
    {"_SGI_MMX_mwm_no_keybind",
     "Key bindings %s not found, using builtin key bindings\n"},
    {"_SGI_MMX_mwm_no_btnbind",
     "Button bindings %s not found, using builtin button bindings\n"},
    {"_SGI_MMX_mwm_no_configfile", "Cannot open configuration file"},
    {"_SGI_MMX_mwm_nomem_menuacc", "Insufficient memory for menu accelerators"},
    {"_SGI_MMX_mwm_nomem_LANG",
     "Insufficient memory to get LANG environment variable."},
    {"_SGI_MMX_mwm_nomem_menu1", "Insufficient memory for menu"},
    {"_SGI_MMX_mwm_parser1", "Expected '{' after menu name"},
    {"_SGI_MMX_mwm_nomem_menuitem", "Insufficient memory for menu item"},
    {"_SGI_MMX_mwm_bad_mnemonic", "Invalid mnemonic specification"},
    {"_SGI_MMX_mwm_nomem_accspec",
     "Insufficient memory for accelerator specification"},
    {"_SGI_MMX_mwm_bad_accspec", "Invalid accelerator specification"},
    {"_SGI_MMX_mwm_nomem", "Insufficient memory"},
    {"_SGI_MMX_mwm_no_grspec", "Missing group specification"},
    {"_SGI_MMX_mwm_bad_grspec", "Invalid group specification"},
    {"_SGI_MMX_mwm_bad_numspec", "Invalid number specification"},
    {"_SGI_MMX_mwm_parser2", "Expected '{' after button set name"},
    {"_SGI_MMX_mwm_nomem_btnspec",
     "Insufficient memory for button specification"},
    {"_SGI_MMX_mwm_bad_btnspec", "Invalid button specification"},
    {"_SGI_MMX_mwm_bad_btnctxt", "Invalid button context"},
    {"_SGI_MMX_mwm_parser3", "Expected '{' after key set name"},
    {"_SGI_MMX_mwm_nomwm_keyspec", "Insufficient memory for key specification"},
    {"_SGI_MMX_mwm_bad_keyspec", "Invalid key specification"},
    {"_SGI_MMX_mwm_bad_keyctxt", "Invalid key context"},
    {"_SGI_MMX_mwm_parser4",
     "%s: %s on line %d of configuration file \"%s\"\n"},
    {"_SGI_MMX_mwm_parser5", "%s: %s on line %d of specification string\n"},
    {"_SGI_MMX_mwm_visual1",
     "SG_visualType \"%s\" contains a non-alphabetic character \"%c\"."},
    {"_SGI_MMX_mwm_visual2", "Using \"%s\" instead."},
    {"_SGI_MMX_mwm_nouplane", "underlay planes not supported from X11"},
    {"_SGI_MMX_mwm_noreq_visual",
     "Unable to get requested visualType (%s) with visualDepth (%d)."},
    {"_SGI_MMX_mwm_visual3",
     "Setting visual resources to server default values."},
    {"_SGI_MMX_mwm_visual4",
     "Using visualType \"%s\" & visualDepth \"%d\" instead."},
    {"_SGI_MMX_mwm_nogood_visual", "Couldn't get suitable visual"},
    {"_SGI_MMX_mwm_nogood_cmap", "Couldn't get a matching colormap"},
    {"_SGI_MMX_mwm_nogood_dble", "Couldn't get a matching drawable"},
    {"_SGI_MMX_mwm_visual5", "Setting visual resources to default values."},
};

#ifdef SG_IS_STATIC
static
#endif /* SG_IS_STATIC */
    char *_imsgs[XtNumber(_intl)];

void LoadIntlResources(Widget w, char *substr, char *Substr) {
  char **AppDataPtr = _imsgs;
  XtResource *resources, *pr;
  int n = XtNumber(_intl), i;

  resources = (XtResource *)malloc(sizeof(XtResource) * n);

  for (pr = resources, i = 0; i < n; i++, pr++) {
    pr->resource_class = NULL;
    pr->resource_type = XtRString;
    pr->resource_size = sizeof(String);
    pr->default_type = XtRString;

    pr->resource_offset = i * sizeof(char *);
    pr->resource_name = _intl[i][0];
    pr->default_addr = _intl[i][1];
  }

  XtGetSubresources(w, AppDataPtr, substr, Substr, resources, n, NULL, 0);

  free(resources);
}

/* End of file: sgi_intl.c */

#endif
