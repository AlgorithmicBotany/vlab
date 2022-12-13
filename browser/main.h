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




#ifndef __main_h__
#define __main_h__

/* This is the utlity definitions for the X11/Motif  template */

#include <string>
#include <QLabel>
#include "QTbrowser.h"
#include "tree.h"
#include "buttons.h"
#include "libvlabd.h"
#include "xmemory.h"

#include <QByteArray>
#include <QMenu>
#include <QString>

// data structure for paste info
class PasteInfo {
public:
    QString user_name;
    QString user_password;
    char * host_name;
    char * prefix;
    char * node_path;
    PasteInfo() {
        host_name = NULL;
        prefix = NULL;
        node_path = NULL;
    }
    ~PasteInfo() {
        if (host_name)
            xfree( host_name);
        if (prefix)
            xfree( prefix);
        if (node_path)
            xfree( node_path);
    }
};

class PasteLinkInfo {
public:
    std::string dirNamex;
    QUuid  uuid;
};

/* these are the global variables of the program */

struct SystemInfo {
    int          argc;           /* Command Line after X Stuff removed*/
    char **      argv;

    RA_Connection * connection; // RA connection to the database
    char *       database;	// full name of the database
    char *       host_name;	// parsed database name
    QString      login_name; //  ||
    char *       oofs_dir;	//  ||
    char         oofs_dir_rp[4096]; // realpath of oofs_dir
    QString      password;	// password
    VlabD *      vlabd;	// interface to vlabd
    char *       origCWD;	// the CWD when the program starts
    QTbrowser*   mainForm; // the main form of the main window
    QTGLbrowser* qgl; // the opengl widget
    float        scrnAspect; // Aspect Ratio of the screen
    int          winWidth;   // width of the display window
    int          winHeight;	// height of the display window
    int 	 treeHeight; // the height of the whole tree in pixels
    int          treeWidth;	// the width of the whole tree in pixels
    unsigned long background; // Background colour
    NODE *       wholeTree;	// the tree structure of the database
    NODE *       beginTree;	// the beginning of the tree on the screen
    int          fontHeight; // max height of the font
    int          fontWidth;	// max width of the font
    NODE *       selNode;	// pointer to the selected node
    BUTTONS      buttons;	// the buttons on the display
    char         fontName[4096]; // the name of the current font
    int	     pid;	// the process id
    char *       tmpDir;	// name of the temporary directory
#ifdef USE_PLUGINS
    char *       pluginDir;     // Name of the plugin directory
    char *       systemDir;     // Name of the system directory
    char *       storePath;
#endif

    char         paste_dir[4096]; // directory for 'paste'-temporary data
    PasteInfo *  paste_info;
    PasteLinkInfo paste_link_info;

    QMenu*     fileOpenButtonP;
    QAction*          fileOpenButtonID;
    QMenu*     newShellButtonP;
    QAction*          newShellButtonID;
#ifdef __APPLE__
    QMenu*     fileOpenConsoleP;
    QAction*        fileOpenConsoleID;
#endif
    QMenu*     iconButtonP;
    QAction*          iconButtonID;
    QMenu*     hideAllIcsButtonP;
    QAction*          hideAllIcsButtonID;
    QMenu*     showAllIcsButtonP;
    QAction*          showAllIcsButtonID;
    QMenu*     extButtonP;
    QAction*          extButtonID;
    QMenu*     allExtButtonP;
    QAction*          allExtButtonID;
    QMenu*     centreButtonP;
    QAction*          centreButtonID;
    QMenu *    centreHyperlinkTargetButtonP;
    QAction*          centreHyperlinkTargetButtonID;
    QMenu*     begTreeButtonP;
    QAction*          begTreeButtonID;
    QMenu*     renameButtonP;
    QAction*          renameButtonID;
    QMenu*     cutButtonP;
    QAction*          cutButtonID;
    QMenu*     pasteButtonP;
    QAction*          pasteButtonID;
    QMenu*     copyNodeButtonP;
    QAction*          copyNodeButtonID;
    QMenu*     copySubtreeButtonP;
    QAction*          copySubtreeButtonID;
    QMenu*      importButtonP;
    QAction*         importButtonID;
    QMenu*      exportButtonP;
    QAction*         exportButtonID;
    QMenu*     deleteButtonP;
    QAction*          deleteButtonID;
    QMenu*     addButtonP;
    QAction*          addButtonID;
    QMenu*     addHButtonP;
    QAction*          addHButtonID;
    QMenu*     hypercopyNodeButtonP;
    QAction*          hypercopyNodeButtonID;
    QMenu*     hypercopySubtreeButtonP;
    QAction*          hypercopySubtreeButtonID;
    QMenu*     getButtonP;
    QAction*          getButtonID;
    QMenu*     customizeButtonP;
    QAction*          customizeButtonID;
    QMenu*     links_buttonP;
    QAction*          links_buttonID;

    bool         move_links;

    /*** cut/copy ***/
    bool	 pasteReady;	/* is there some data for paste */
    bool   pasteLinkReady;
    bool   copyHyperobject;

    // is forking off enabled?
    bool noFork_;

  int obj_posx ;
  int obj_posy ;



    // error log...
    std::string errorLog;
    std::string getErrorLog ()
    {
        std::string res = errorLog;
        errorLog = "";
        return res;
    }

};

extern SystemInfo sysInfo;	// Global Variable used for Glx/Graphics info

void vlab_close( void);
int vlab_open( void);

#ifdef USE_PLUGINS
void updatePath();
#endif

void newBrowser( QByteArray oofs_spec, bool do_fork = true );
void interpretFile(QString filename);

void display_splash( void);

#endif /* ifndef __main_h__ */

