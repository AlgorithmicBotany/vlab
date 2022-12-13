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




#ifndef __QTBROWSER_H
#define __QTBROWSER_H

#include <QWidget>
#include <QLabel>
#include <QEvent>
#include <QScrollBar>
#include <QScrollArea>
#include <QMenu>
#include <QMainWindow>
#include <QSocketNotifier>

#include <QPixmap>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QCloseEvent>
#include "BrowserSettings.h"
#include "tree.h"

#include <QMainWindow>
#include <QByteArray>
#include <QTime>

#ifdef USE_PLUGINS
#include <QFileSystemWatcher>
#include <QTimer>
#endif

class QTimer;
class QTGLbrowser;
class CustomizeDialog;
class QPixmap;
class FinderWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QShowEvent;
class FindDialog;

static const int DEFAULT_W = 600;              //Default window width
static const int DEFAULT_H = 400;              //Default window height
static const int SCROLL_MAX_INT  = 1000000;      //Scroll max int scale
static const int SCROLL_MAX_FLT = 1000000;    //Scroll max float scale

class QTbrowser : public QMainWindow
{
    Q_OBJECT

public:
    QTbrowser(QWidget* parent=0);
    ~QTbrowser();

    void update_menus();
    void show_status( char * cur_status);
    void updateDisplay();
    int topX();
    int topY();
    // returns a default icon for objects (if none can be read in...)
    static const QPixmap & defaultIcon( int size = -1 );
    static const QPixmap & hourglassIcon( int size = -1 );

    const BrowserSettings & browserSettings();

    void scheduleIdleIconLoader( bool yes, int delay = 0 );

    bool quitCB();

public slots:
    // loads some icon(s) when idle. Schedules itself if necessary
    void idleIconLoader();
    bool loadArbitraryIcon( const QRect & view, bool & update );
    void newBrowserCB();
    void openShellCB();
    void fileopenCB();
    void customizeCB();
    void checkDatabaseCB();
    void showExtCB();
    void showAllExtCB();
    void showIconCB();
    void hideAllIconsCB();
    void showAllIconsCB();
    void centreCB();
    void centreHyperlinkTargetCB();
    void beginTreeCB();
    void beginFromRootCB();
    void getObjectCB();
    void cut_menu_cb();
    void copy_node_menu_cb();
    void copy_subtree_menu_cb();
    void paste_menu_cb();
    void delete_menu_cb();
    void add_object_menu_cb();
    void add_Hobject_menu_cb();
    void hypercopy_node_cb();
    void hypercopy_subtree_cb();
    void links_button_cb();
    void about_vlab_cb();
    void about_Rayshade_cb();
    void helpCB();
    void newEnviroHelp();
    void rayshadeHelp();
    void newCpfgHelp();
    void newLpfgHelp();
    void newVlabHelp();
    void quickHelp();
    void gettingStartedHelp();
    void vlabToolsHelp();

    void find_menu_cb();
    void importObject();
    void exportObject();
    void inputReady(int socket);

    void fileopenconsoleCB();
    // callback for the customization dialog (called every time the settings change)
    void applySettings( const BrowserSettings & bset );
    void applySettingsColors( const BrowserSettings & bset );
    void rename_cb();
    void pluginsChanged();

protected:
    void closeEvent(QCloseEvent* ev);
    void resizeEvent(QResizeEvent* ev);
    void keyPressEvent(QKeyEvent* ev);
    void dragEnterEvent(QDragEnterEvent *ev);
    void dropEvent(QDropEvent *ev);
    void showEvent(QShowEvent *ev);

private:
    void cut_cb();
    void copy_node_cb();
    void copy_subtree_cb();
    void paste_cb();
    void delete_cb();
    void new_object_cb();
    void new_Hobject_cb();
    void hyperpaste_cb();
    char* genName(NODE* node);
#ifdef USE_PLUGINS
    QFileSystemWatcher fs_watcher;
    QTimer fs_timer;
#endif

    QTGLbrowser* globj;
    QSocketNotifier* notifier;  //Network socket handler

    QTime lastIconLoaded;

    QString exportPath;
    int exportArchiveType;
    QScrollArea *scroll;

    int nb_of_objectDiagonals;


public:
    int winWidth;
    int winHeight;

    // remember the last icon size so that we don't have to resize the icons
    // if it does not change
    int lastIconSize;
    // customization dialog
    CustomizeDialog * _customize_dialog;
    bool _idleIconLoaderInstalled;
    // finder widget
    FinderWidget * _finderWidget;
    // find dialog
    FindDialog * _findDialog;
};

#endif
