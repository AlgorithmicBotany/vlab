#include "resids.h"

IDR_BROWSER MENU 
BEGIN
    POPUP "&Browser"
    BEGIN
        //MENUITEM "Reconcile Guids",             ID_BROWSER_RECONCILE_GUIDS
        //MENUITEM "Fix oofs",                    ID_BROWSER_FIXOOFS
        //MENUITEM SEPARATOR
        MENUITEM "&Close",                      ID_BROWSER_CLOSE
    END
    POPUP "&View"
    BEGIN
        MENUITEM "Show &extensions\tGrey (+)",  ID_VIEW_SHOWEXTENSIONS
        MENUITEM "Show &all extensions\tShift+Grey (+)", 
                                                ID_VIEW_SHOWALLEXTENSIONS
        MENUITEM SEPARATOR
        MENUITEM "Show &icon\tGrey (*)",        ID_VIEW_SHOWICON
        MENUITEM "Show all i&cons\tShift+Grey (*)", ID_VIEW_SHOWALLICONS
        MENUITEM "&Hide all icons",             ID_VIEW_HIDEALLICONS
    END
    POPUP "&Object"
    BEGIN
        MENUITEM "&Open\tEnter",                ID_OBJECT_OPEN
        MENUITEM "&Delete...\tDel",             ID_OBJECT_DELETE
        MENUITEM "&Rename...",                  ID_OBJECT_RENAME
        MENUITEM SEPARATOR
        MENUITEM "&Find...",                    ID_OBJECT_FIND
        MENUITEM "Show hyperlink &target",      ID_OBJECT_SHOWHYPERLINKTARGET
        MENUITEM SEPARATOR
        MENUITEM "Cop&y object",                ID_OBJECT_COPYOBJECT
        MENUITEM "&Hypercopy object",           ID_OBJECT_HYPERCOPYOBJECT
        MENUITEM "&Copy subtree",               ID_OBJECT_COPYSUBTREE
        MENUITEM "&Paste object(s)",            ID_OBJECT_PASTEOBJECTS
        MENUITEM SEPARATOR
        MENUITEM "Create object group"          ID_OBJECT_EXTENDOBJECT
    END
END

IDB_BITMAP1             BITMAP                  "browser\vlbicon48.bmp"

IDD_READINGICONS DIALOG  0, 0, 186, 50
STYLE DS_SETFONT | DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | 
    WS_SYSMENU
CAPTION "Reading icons"
FONT 8, "MS Sans Serif"
BEGIN
    PUSHBUTTON      "Cancel",IDCANCEL,68,29,50,14
    CONTROL         "",IDC_PROGRESS,"msctls_progress32",WS_BORDER,7,
                    7,172,13
END

IDD_READINGOBJECTS DIALOG  0, 0, 269, 53
STYLE DS_SETFONT | DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | 
    WS_SYSMENU
CAPTION "Reading objects"
FONT 8, "MS Sans Serif"
BEGIN
    PUSHBUTTON      "Cancel",IDCANCEL,110,32,50,14
    LTEXT           "Static",IDC_PATH,7,7,255,20,SS_SUNKEN
END

IDD_RENAME DIALOGEX 0, 0, 152, 55
STYLE DS_SETFONT | DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | 
    WS_SYSMENU
CAPTION "Rename object"
FONT 8, "MS Sans Serif", 0, 0, 0x1
BEGIN
    EDITTEXT        IDC_NEWNAME,9,7,133,12,ES_AUTOHSCROLL
    DEFPUSHBUTTON   "OK",IDOK,17,30,50,14
    PUSHBUTTON      "Cancel",IDCANCEL,83,30,50,14
END

IDD_EXTENDOBJECT DIALOGEX 0, 0, 152, 55
STYLE DS_SETFONT | DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | 
    WS_SYSMENU
CAPTION "Object group"
FONT 8, "MS Sans Serif", 0, 0, 0x1
BEGIN
    EDITTEXT        IDC_NEWNAME,9,7,133,12,ES_AUTOHSCROLL
    DEFPUSHBUTTON   "OK",IDOK,17,30,50,14
    PUSHBUTTON      "Cancel",IDCANCEL,83,30,50,14
END


STRINGTABLE 
BEGIN
IDERR_EMPTYNAME "Name cannot be empty"
END

IDI_BRICON ICON "browser\browser.ico"
