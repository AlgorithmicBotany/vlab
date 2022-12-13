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



/* File: _sgigui.h
   This file contains the message numbers and their defines.
*/
#ifdef SG_IS_STATIC
#else
extern char *_imsgs[];
#endif /* SG_IS_STATIC */

#define _SGI_INTL_NAME "intl"
#define _SGI_INTL_CLASS "Intl"
#define _SGI_INTL_MSG(n) _imsgs[n]

#define _SGI_MMX_mwm_nogrmem 0
#define _SGI_MMX_mwm_divideby0 1
#define _SGI_MMX_mwm_iconmisfit 2
#define _SGI_MMX_mwm_noiconmem 3
#define _SGI_MMX_mwm_noclientwindowmem 4
#define _SGI_MMX_mwm_readerr_title 5
#define _SGI_MMX_mwm_noiconputmem 6
#define _SGI_MMX_mwm_nomem_icondata 7
#define _SGI_MMX_mwm_nomem_windata 8
#define _SGI_MMX_mwm_nomem_winflags 9
#define _SGI_MMX_mwm_nomem_atom 10
#define _SGI_MMX_mwm_ownfailed 11
#define _SGI_MMX_mwm_nomem_winmgrdata 12
#define _SGI_MMX_mwm_listsel 13
#define _SGI_MMX_mwm_nomem_cvtsel 14
#define _SGI_MMX_mwm_cant_restartwm 15
#define _SGI_MMX_mwm_cantwrap 16
#define _SGI_MMX_mwm_bad_iconbmp 17
#define _SGI_MMX_mwm_bad_rooticonbmp 18
#define _SGI_MMX_mwm_cip_notsup 19
#define _SGI_MMX_mwm_nomem_bevel 20
#define _SGI_MMX_mwm_nomem_bmp 21
#define _SGI_MMX_mwm_bad_bmp 22
#define _SGI_MMX_mwm_invalid_bmp 23
#define _SGI_MMX_mwm_nomem_scrdata 24
#define _SGI_MMX_mwm_twowm 25
#define _SGI_MMX_mwm_toomany_scr 26
#define _SGI_MMX_mwm_bad_X 27
#define _SGI_MMX_mwm_nomem_wkspdata 28
#define _SGI_MMX_mwm_nomem_dpystr 29
#define _SGI_MMX_mwm_few_scrnames 30
#define _SGI_MMX_mwm_nomenu 31
#define _SGI_MMX_mwm_nomem_menu 32
#define _SGI_MMX_mwm_menu_recursion 33
#define _SGI_MMX_mwm_nomem_cdata 34
#define _SGI_MMX_mwm_locale_msg 35
#define _SGI_MMX_mwm_no_iconbox 36
#define _SGI_MMX_mwm_badcvt 37
#define _SGI_MMX_mwm_nomem_cvt 38
#define _SGI_MMX_mwm_unknown_prop 39
#define _SGI_MMX_mwm_retry 40
#define _SGI_MMX_mwm_cantopen 41
#define _SGI_MMX_mwm_rmsg 42
#define _SGI_MMX_mwm_visual_mismatch 43
#define _SGI_MMX_mwm_SGwarn 44
#define _SGI_MMX_mwm_xerr_msg1 45
#define _SGI_MMX_mwm_xerr_defstr1 46
#define _SGI_MMX_mwm_xerr_name 47
#define _SGI_MMX_mwm_xerr_fmt1 48
#define _SGI_MMX_mwm_xerr_msg2 49
#define _SGI_MMX_mwm_xerr_defstr2 50
#define _SGI_MMX_mwm_xerr_name3 51
#define _SGI_MMX_mwm_xerr_fmt3 52
#define _SGI_MMX_mwm_xerr_msg4 53
#define _SGI_MMX_mwm_xerr_defstr4 54
#define _SGI_MMX_mwm_xerr_fmt5_1 55
#define _SGI_MMX_mwm_xerr_name5 56
#define _SGI_MMX_mwm_xerr_fmt5 57
#define _SGI_MMX_mwm_xerr_fmte 58
#define _SGI_MMX_mwm_xerr_fmt6_1 59
#define _SGI_MMX_mwm_xerr_fmt6_2 60
#define _SGI_MMX_mwm_xerr_msg7 61
#define _SGI_MMX_mwm_xerr_defstr7 62
#define _SGI_MMX_mwm_xerr_msg8 63
#define _SGI_MMX_mwm_xerr_defstr8 64
#define _SGI_MMX_mwm_xerr_msg9 65
#define _SGI_MMX_mwm_xerr_defstr9 66
#define _SGI_MMX_mwm_xerr_msg10 67
#define _SGI_MMX_mwm_xerr_defstr10 68
#define _SGI_MMX_mwm_xerr_msg11 69
#define _SGI_MMX_mwm_xerr_defstr11 70
#define _SGI_MMX_mwm_xerr_io 71
#define _SGI_MMX_mwm_xerr_xt 72
#define _SGI_MMX_mwm_xerr_xtw 73
#define _SGI_MMX_mwm_tdb 74
#define _SGI_MMX_mwm_tcb 75
#define _SGI_MMX_mwm_domwm 76
#define _SGI_MMX_mwm_quitwm 77
#define _SGI_MMX_mwm_ldfont_failed 78
#define _SGI_MMX_mwm_noapp_font 79
#define _SGI_MMX_mwm_inconsis_variable 80
#define _SGI_MMX_mwm_no_keybind 81
#define _SGI_MMX_mwm_no_btnbind 82
#define _SGI_MMX_mwm_no_configfile 83
#define _SGI_MMX_mwm_nomem_menuacc 84
#define _SGI_MMX_mwm_nomem_LANG 85
#define _SGI_MMX_mwm_nomem_menu1 86
#define _SGI_MMX_mwm_parser1 87
#define _SGI_MMX_mwm_nomem_menuitem 88
#define _SGI_MMX_mwm_bad_mnemonic 89
#define _SGI_MMX_mwm_nomem_accspec 90
#define _SGI_MMX_mwm_bad_accspec 91
#define _SGI_MMX_mwm_nomem 92
#define _SGI_MMX_mwm_no_grspec 93
#define _SGI_MMX_mwm_bad_grspec 94
#define _SGI_MMX_mwm_bad_numspec 95
#define _SGI_MMX_mwm_parser2 96
#define _SGI_MMX_mwm_nomem_btnspec 97
#define _SGI_MMX_mwm_bad_btnspec 98
#define _SGI_MMX_mwm_bad_btnctxt 99
#define _SGI_MMX_mwm_parser3 100
#define _SGI_MMX_mwm_nomwm_keyspec 101
#define _SGI_MMX_mwm_bad_keyspec 102
#define _SGI_MMX_mwm_bad_keyctxt 103
#define _SGI_MMX_mwm_parser4 104
#define _SGI_MMX_mwm_parser5 105
#define _SGI_MMX_mwm_visual1 106
#define _SGI_MMX_mwm_visual2 107
#define _SGI_MMX_mwm_nouplane 108
#define _SGI_MMX_mwm_noreq_visual 109
#define _SGI_MMX_mwm_visual3 110
#define _SGI_MMX_mwm_visual4 111
#define _SGI_MMX_mwm_nogood_visual 112
#define _SGI_MMX_mwm_nogood_cmap 113
#define _SGI_MMX_mwm_nogood_dble 114
#define _SGI_MMX_mwm_visual5 115
/* End of file: sgi_intl.h */
